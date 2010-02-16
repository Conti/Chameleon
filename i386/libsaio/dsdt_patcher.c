/*
 * Copyright 2008 mackerintel
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "acpi.h"
#include "efi_tables.h"
#include "fake_efi.h"
#include "dsdt_patcher.h"
#include "platform.h"

#ifndef DEBUG_DSDT
#define DEBUG_DSDT 0
#endif

#if DEBUG_DSDT==2
#define DBG(x...)  {printf(x); sleep(1);}
#elif DEBUG_DSDT==1
#define DBG(x...)  printf(x)
#else
#define DBG(x...)
#endif

/* Gets the ACPI 1.0 RSDP address */
static struct acpi_2_rsdp* getAddressOfAcpiTable()
{
    /* TODO: Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
	
    void *acpi_addr = (void*)ACPI_RANGE_START;
    for(; acpi_addr <= (void*)ACPI_RANGE_END; acpi_addr += 16)
    {
        if(*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE)
        {
            uint8_t csum = checksum8(acpi_addr, 20);
            if(csum == 0)
            {
                // Only return the table if it is a true version 1.0 table (Revision 0)
                if(((struct acpi_2_rsdp*)acpi_addr)->Revision == 0)
                    return acpi_addr;
            }
        }
    }
    return NULL;
}

/* Gets the ACPI 2.0 RSDP address */
static struct acpi_2_rsdp* getAddressOfAcpi20Table()
{
    /* TODO: Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
	
    void *acpi_addr = (void*)ACPI_RANGE_START;
    for(; acpi_addr <= (void*)ACPI_RANGE_END; acpi_addr += 16)
    {
        if(*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE)
        {
            uint8_t csum = checksum8(acpi_addr, 20);

            /* Only assume this is a 2.0 or better table if the revision is greater than 0
             * NOTE: ACPI 3.0 spec only seems to say that 1.0 tables have revision 1
             * and that the current revision is 2.. I am going to assume that rev > 0 is 2.0.
             */

            if(csum == 0 && (((struct acpi_2_rsdp*)acpi_addr)->Revision > 0))
            {
                uint8_t csum2 = checksum8(acpi_addr, sizeof(struct acpi_2_rsdp));
                if(csum2 == 0)
                    return acpi_addr;
            }
        }
    }
    return NULL;
}
/** The folowing ACPI Table search algo. should be reused anywhere needed:*/
int search_and_get_acpi_fd(const char * filename, const char ** outDirspec)
{
  int fd=0;
  const char * overriden_pathname=NULL;
  static char dirspec[512]="";
  static bool first_time =true; 
  int len=0;

  /// Take in accound user overriding if it's DSDT only
  if (strstr(filename, "DSDT") && 
      getValueForKey(kDSDT, &overriden_pathname, &len,  
			   &bootInfo->bootConfig))
    {
      sprintf(dirspec, "%s", overriden_pathname);
      fd=open (dirspec,0);
      if (fd>=0) goto success_fd;
    }
  // Check that dirspec is not already assigned with a path
  if (!first_time && *dirspec) 
  { // it is so start searching this cached patch first
      //extract path
      for (len=strlen(dirspec)-1; len; len--)
          if (dirspec[len]=='/' || len==0)
          {
                  dirspec[len]='\0';
                  break;
          }
      // now concat with the filename
      strncat(dirspec, "/", sizeof(dirspec));
      strncat(dirspec, filename, sizeof(dirspec));
      // and test to see if we don't have our big boy here:
      fd=open (dirspec,0);
      if (fd>=0) 
      {
          // printf("ACPI file search cache hit: file found at %s\n", dirspec);
          goto success_fd;
      }
  }
  // Start searching any potential location for ACPI Table
  // search the Extra folders first
  sprintf(dirspec,"/Extra/%s",filename); 
  fd=open (dirspec,0);
  if (fd>=0) goto success_fd;

  sprintf(dirspec,"bt(0,0)/Extra/%s",filename);
  fd=open (dirspec,0);
  if (fd>=0) goto success_fd;

  sprintf(dirspec, "%s", filename); // search current dir
  fd=open (dirspec,0);
  if (fd>=0) goto success_fd;

  sprintf(dirspec, "/%s", filename); // search root
  fd=open (dirspec,0);
  if (fd>=0) goto success_fd;

  // NOT FOUND:
  verbose("ACPI Table not found: %s\n", filename);
  if (outDirspec) *outDirspec = "";
  first_time = false;
  return -1;
  // FOUND
success_fd:
  first_time = false;
  if (outDirspec) *outDirspec = dirspec; 
  return fd;
}

void *loadACPITable (const char * filename)
{
	void *tableAddr;
	const char * dirspec=NULL;
	
	int fd = search_and_get_acpi_fd(filename, &dirspec);

	if (fd>=0)
	{
	  tableAddr=(void*)AllocateKernelMemory(file_size (fd));
	  if (tableAddr)
	  {
	      if (read (fd, tableAddr, file_size (fd))!=file_size (fd))
	      {
		  printf("Couldn't read table %s\n",dirspec);
		  free (tableAddr);
		  close (fd);
		  return NULL;
	      }
	      
	      DBG("Table %s read and stored at: %x\n", dirspec, tableAddr);
	      close (fd);
	      return tableAddr;
	  }
	  close (fd);
          printf("Couldn't allocate memory for table \n", dirspec);
	}  
	printf("Couldn't find table %s\n", filename);
	return NULL;
}


struct acpi_2_fadt *
patch_fadt(struct acpi_2_fadt *fadt, void *new_dsdt)
{

        extern void setupSystemType(); 
	
	struct acpi_2_fadt *fadt_mod;
	bool fadt_rev2_needed = false;
	bool fix_restart;
	const char * value;
	
	// Restart Fix
	if (Platform.CPU.Vendor == 0x756E6547) {	/* Intel */
		fix_restart = true;
		getBoolForKey(kRestartFix, &fix_restart, &bootInfo->bootConfig);
	} else {
		verbose ("Not an Intel platform: Restart Fix not applied !!!\n");
		fix_restart = false;
	}

	if (fix_restart) fadt_rev2_needed = true;

	// Allocate new fadt table
	if (fadt->Length < 0x84 && fadt_rev2_needed)
	{
		fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(0x84);
		memcpy(fadt_mod, fadt, fadt->Length);
		fadt_mod->Length   = 0x84;
		fadt_mod->Revision = 0x02; // FADT rev 2 (ACPI 1.0B MS extensions)
	}
	else
	{
		fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(fadt->Length);
		memcpy(fadt_mod, fadt, fadt->Length);
	}
	// Determine system type / PM_Model
	if ( (value=getStringForKey(kSystemType, &bootInfo->bootConfig))!=NULL)
	{
	  if (Platform.Type > 6)  
	  {
	    if(fadt_mod->PM_Profile<=6)
	      Platform.Type = fadt_mod->PM_Profile; // get the fadt if correct
	    else 
	      Platform.Type = 1;		/* Set a fixed value (Desktop) */
	    verbose("Error: system-type must be 0..6. Defaulting to %d !\n", Platform.Type);
	  }
	  else
	    Platform.Type = (unsigned char) strtoul(value, NULL, 10);
	}
	// Set PM_Profile from System-type if only if user wanted this value to be forced
	if (fadt_mod->PM_Profile != Platform.Type) 
	{
	    if (value) 
	      { // user has overriden the SystemType so take care of it in FACP
		verbose("FADT: changing PM_Profile from 0x%02x to 0x%02x\n", fadt_mod->PM_Profile, Platform.Type);
		fadt_mod->PM_Profile = Platform.Type;
	    }
	    else
	    { // PM_Profile has a different value and no override has been set, so reflect the user value to ioregs
	      Platform.Type = fadt_mod->PM_Profile <= 6 ? fadt_mod->PM_Profile : 1;
	    }  
	}
	// We now have to write the systemm-type in ioregs: we cannot do it before in setupDeviceTree()
	// because we need to take care of facp original content, if it is correct.
	setupSystemType();

	// Patch FADT to fix restart
	if (fix_restart)
	{
		fadt_mod->Flags|= 0x400;
		fadt_mod->Reset_SpaceID		= 0x01;   // System I/O
		fadt_mod->Reset_BitWidth	= 0x08;   // 1 byte
		fadt_mod->Reset_BitOffset	= 0x00;   // Offset 0
		fadt_mod->Reset_AccessWidth	= 0x01;   // Byte access
		fadt_mod->Reset_Address		= 0x0cf9; // Address of the register
		fadt_mod->Reset_Value		= 0x06;   // Value to write to reset the system
		verbose("FADT: Restart Fix applied !\n");
	}

	// Patch DSDT Address
	DBG("DSDT: Old @%x,%x, ",fadt_mod->DSDT,fadt_mod->X_DSDT);

	fadt_mod->DSDT=(uint32_t)new_dsdt;
	if ((uint32_t)(&(fadt_mod->X_DSDT))-(uint32_t)fadt_mod+8<=fadt_mod->Length)
		fadt_mod->X_DSDT=(uint32_t)new_dsdt;

	DBG("New @%x,%x\n",fadt_mod->DSDT,fadt_mod->X_DSDT);

	// Correct the checksum
	fadt_mod->Checksum=0;
	fadt_mod->Checksum=256-checksum8(fadt_mod,fadt_mod->Length);

	return fadt_mod;
}

/* Setup ACPI without replacing DSDT. */
int setupAcpiNoMod()
{
//	addConfigurationTable(&gEfiAcpiTableGuid, getAddressOfAcpiTable(), "ACPI");
//	addConfigurationTable(&gEfiAcpi20TableGuid, getAddressOfAcpi20Table(), "ACPI_20");
	/* XXX aserebln why uint32 cast if pointer is uint64 ? */
	acpi10_p = (uint32_t)getAddressOfAcpiTable();
	acpi20_p = (uint32_t)getAddressOfAcpi20Table();
	addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
	if(acpi20_p) addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
	return 1;
}

/* Setup ACPI. Replace DSDT if DSDT.aml is found */
int setupAcpi(void)
{
	int version;
	void *new_dsdt;

	bool drop_ssdt;
	
	// Load replacement DSDT
	new_dsdt=loadACPITable("DSDT.aml");
	if (!new_dsdt)
	{
		return setupAcpiNoMod();
	}

	DBG("New DSDT Loaded in memory\n");
	
	{
		bool tmp;
		drop_ssdt=getBoolForKey(kDropSSDT, &tmp, &bootInfo->bootConfig)&&tmp;
	}

	// Do the same procedure for both versions of ACPI
	for (version=0; version<2; version++) {
		struct acpi_2_rsdp *rsdp, *rsdp_mod;
		struct acpi_2_rsdt *rsdt, *rsdt_mod;
		int rsdplength;
		
		// Find original rsdp
		rsdp=(struct acpi_2_rsdp *)(version?getAddressOfAcpi20Table():getAddressOfAcpiTable());
		if (!rsdp)
		{
			DBG("No ACPI version %d found. Ignoring\n", version+1);
			if (version)
				addConfigurationTable(&gEfiAcpi20TableGuid, NULL, "ACPI_20");
			else
				addConfigurationTable(&gEfiAcpiTableGuid, NULL, "ACPI");
			continue;
		}
		rsdplength=version?rsdp->Length:20;

		DBG("RSDP version %d found @%x. Length=%d\n",version+1,rsdp,rsdplength);

		/* FIXME: no check that memory allocation succeeded 
		 * Copy and patch RSDP,RSDT, XSDT and FADT
		 * For more info see ACPI Specification pages 110 and following
		 */

		rsdp_mod=(struct acpi_2_rsdp *) AllocateKernelMemory(rsdplength);
		memcpy(rsdp_mod, rsdp, rsdplength);    
		rsdt=(struct acpi_2_rsdt *)(rsdp->RsdtAddress);

		DBG("RSDT @%x, Length %d\n",rsdt, rsdt->Length);
		
		if (rsdt && (uint32_t)rsdt !=0xffffffff && rsdt->Length<0x10000)
		{
			uint32_t *rsdt_entries;
			int rsdt_entries_num;
			int dropoffset=0, i;
			
			rsdt_mod=(struct acpi_2_rsdt *)AllocateKernelMemory(rsdt->Length); 
			memcpy (rsdt_mod, rsdt, rsdt->Length);
			rsdp_mod->RsdtAddress=(uint32_t)rsdt_mod;
			rsdt_entries_num=(rsdt_mod->Length-sizeof(struct acpi_2_rsdt))/4;
			rsdt_entries=(uint32_t *)(rsdt_mod+1);
			for (i=0;i<rsdt_entries_num;i++)
			{
				char *table=(char *)(rsdt_entries[i]);
				if (!table)
					continue;

				DBG("TABLE %c%c%c%c,",table[0],table[1],table[2],table[3]);

				rsdt_entries[i-dropoffset]=rsdt_entries[i];
				if (drop_ssdt && table[0]=='S' && table[1]=='S' && table[2]=='D' && table[3]=='T')
				{
					dropoffset++;
					continue;
				}
				if (table[0]=='D' && table[1]=='S' && table[2]=='D' && table[3]=='T')
				{
					DBG("DSDT found\n");
					rsdt_entries[i-dropoffset]=(uint32_t)new_dsdt;
					continue;
				}
				if (table[0]=='F' && table[1]=='A' && table[2]=='C' && table[3]=='P')
				{
					struct acpi_2_fadt *fadt, *fadt_mod;
					fadt=(struct acpi_2_fadt *)rsdt_entries[i];

					DBG("FADT found @%x, Length %d\n",fadt, fadt->Length);

					if (!fadt || (uint32_t)fadt == 0xffffffff || fadt->Length>0x10000)
					{
						printf("FADT incorrect. Not modified\n");
						continue;
					}
					
					fadt_mod = patch_fadt(fadt, new_dsdt);
					rsdt_entries[i-dropoffset]=(uint32_t)fadt_mod;
					continue;
				}
			}
			DBG("\n");

			// Correct the checksum of RSDT
			rsdt_mod->Length-=4*dropoffset;

			DBG("RSDT: Original checksum %d, ", rsdt_mod->Checksum);

			rsdt_mod->Checksum=0;
			rsdt_mod->Checksum=256-checksum8(rsdt_mod,rsdt_mod->Length);

			DBG("New checksum %d at %x\n", rsdt_mod->Checksum,rsdt_mod);
		}
		else
		{
			rsdp_mod->RsdtAddress=0;
			printf("RSDT not found or RSDT incorrect\n");
		}

		if (version)
		{
			struct acpi_2_xsdt *xsdt, *xsdt_mod;

			// FIXME: handle 64-bit address correctly

			xsdt=(struct acpi_2_xsdt*) ((uint32_t)rsdp->XsdtAddress);
			DBG("XSDT @%x;%x, Length=%d\n", (uint32_t)(rsdp->XsdtAddress>>32),(uint32_t)rsdp->XsdtAddress,
					xsdt->Length);
			if (xsdt && (uint64_t)rsdp->XsdtAddress<0xffffffff && xsdt->Length<0x10000)
			{
				uint64_t *xsdt_entries;
				int xsdt_entries_num, i;
				int dropoffset=0;
				
				xsdt_mod=(struct acpi_2_xsdt*)AllocateKernelMemory(xsdt->Length); 
				memcpy(xsdt_mod, xsdt, xsdt->Length);
				rsdp_mod->XsdtAddress=(uint32_t)xsdt_mod;
				xsdt_entries_num=(xsdt_mod->Length-sizeof(struct acpi_2_xsdt))/8;
				xsdt_entries=(uint64_t *)(xsdt_mod+1);
				for (i=0;i<xsdt_entries_num;i++)
				{
					char *table=(char *)((uint32_t)(xsdt_entries[i]));
					if (!table)
						continue;
					xsdt_entries[i-dropoffset]=xsdt_entries[i];
					if (drop_ssdt && table[0]=='S' && table[1]=='S' && table[2]=='D' && table[3]=='T')
					{
						dropoffset++;
						continue;
					}					
					if (table[0]=='D' && table[1]=='S' && table[2]=='D' && table[3]=='T')
					{
						DBG("DSDT found\n");

						xsdt_entries[i-dropoffset]=(uint32_t)new_dsdt;

						DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);
						
						continue;
					}
					if (table[0]=='F' && table[1]=='A' && table[2]=='C' && table[3]=='P')
					{
						struct acpi_2_fadt *fadt, *fadt_mod;
						fadt=(struct acpi_2_fadt *)(uint32_t)xsdt_entries[i];

						DBG("FADT found @%x,%x, Length %d\n",(uint32_t)(xsdt_entries[i]>>32),fadt, 
								 fadt->Length);

						if (!fadt || (uint64_t)xsdt_entries[i] >= 0xffffffff || fadt->Length>0x10000)
						{
							verbose("FADT incorrect or after 4GB. Dropping XSDT\n");
							goto drop_xsdt;
						}

						fadt_mod = patch_fadt(fadt, new_dsdt);
						xsdt_entries[i-dropoffset]=(uint32_t)fadt_mod;

						DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);

						continue;
					}

					DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);

				}

				// Correct the checksum of XSDT
				xsdt_mod->Length-=8*dropoffset;
				xsdt_mod->Checksum=0;
				xsdt_mod->Checksum=256-checksum8(xsdt_mod,xsdt_mod->Length);
			}
			else
			{
			drop_xsdt:

				DBG("About to drop XSDT\n");

				/*FIXME: Now we just hope that if MacOS doesn't find XSDT it reverts to RSDT. 
				 * A Better strategy would be to generate
				 */

				rsdp_mod->XsdtAddress=0xffffffffffffffffLL;
				verbose("XSDT not found or XSDT incorrect\n");
			}
		}

		// Correct the checksum of RSDP      

		DBG("RSDP: Original checksum %d, ", rsdp_mod->Checksum);

		rsdp_mod->Checksum=0;
		rsdp_mod->Checksum=256-checksum8(rsdp_mod,20);

		DBG("New checksum %d\n", rsdp_mod->Checksum);

		if (version)
		{
			DBG("RSDP: Original extended checksum %d", rsdp_mod->ExtendedChecksum);

			rsdp_mod->ExtendedChecksum=0;
			rsdp_mod->ExtendedChecksum=256-checksum8(rsdp_mod,rsdp_mod->Length);

			DBG("New extended checksum %d\n", rsdp_mod->ExtendedChecksum);

		}
		
		verbose("Patched ACPI version %d DSDT\n", version+1);
		if (version)
		{
	/* XXX aserebln why uint32 cast if pointer is uint64 ? */
			acpi20_p = (uint32_t)rsdp_mod;
			addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
		}
		else
		{
	/* XXX aserebln why uint32 cast if pointer is uint64 ? */
			acpi10_p = (uint32_t)rsdp_mod;
			addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
		}
	}
#if DEBUG_DSDT
	printf("Press a key to continue... (DEBUG_DSDT)\n");
	getc();
#endif
	return 1;
}
