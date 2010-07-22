/*
 * Copyright 2008 mackerintel
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "acpi.h"
#include "efi_tables.h"
#include "fake_efi.h"
#include "acpi_patcher.h"
#include "platform.h"
#include "cpu.h"
#include "aml_generator.h"

#ifndef DEBUG_ACPI
#define DEBUG_ACPI 0
#endif

#if DEBUG_ACPI==2
#define DBG(x...)  {printf(x); sleep(1);}
#elif DEBUG_ACPI==1
#define DBG(x...)  printf(x)
#else
#define DBG(x...)
#endif

// Slice: New signature compare function
boolean_t tableSign(char *table, const char *sgn)
{
	int i;
	for (i=0; i<4; i++) {
		if ((table[i] &~0x20) != (sgn[i] &~0x20)) {
			return FALSE;
		}
	}
	return TRUE;
}

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
	//verbose("ACPI Table not found: %s\n", filename);
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
	//printf("Couldn't find table %s\n", filename);
	return NULL;
}

uint8_t	acpi_cpu_count = 0;
char* acpi_cpu_name[32];

void find_acpi_cpu_names(unsigned char* dsdt, int length)
{
	int i;
	
	for (i=0; i<length-7; i++) 
	{
		if (dsdt[i] == 0x5B && dsdt[i+1] == 0x83) // ProcessorOP
		{
			uint8_t offset = i+2+(dsdt[i+2] >> 6) + 1, j;
			bool add_name = TRUE;

			for (j=0; j<4; j++) 
			{
				char c = dsdt[offset+j];
				
				if (!aml_isvalidchar(c)) 
				{
					add_name = FALSE;
					verbose("Invalid characters found in ProcessorOP!\n");
					break;
				}
			}
			
			if (add_name && dsdt[offset+5] < 32 ) 
			{
				acpi_cpu_name[acpi_cpu_count] = malloc(5);
				memcpy(acpi_cpu_name[acpi_cpu_count], dsdt+offset, 4);
				
				verbose("Found %c%c%c%c (from DSDT)\n", acpi_cpu_name[acpi_cpu_count][0], acpi_cpu_name[acpi_cpu_count][1], acpi_cpu_name[acpi_cpu_count][2], acpi_cpu_name[acpi_cpu_count][3]);
				
				if (++acpi_cpu_count == 32) return;
			}
		}
	}
}

struct acpi_2_ssdt *generate_cst_ssdt(struct acpi_2_fadt* fadt)
{
	char ssdt_header[] =
	{
		0x53, 0x53, 0x44, 0x54, 0xE7, 0x00, 0x00, 0x00, /* SSDT.... */
		0x01, 0x17, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x41, /* ..PmRefA */
		0x43, 0x70, 0x75, 0x43, 0x73, 0x74, 0x00, 0x00, /* CpuCst.. */
		0x00, 0x10, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* ....INTL */
		0x31, 0x03, 0x10, 0x20 							/* 1.._		*/
	};
	
	char chunk_name_body[] =
	{
		0x5C, 0x5F, 0x50, 0x52, 0x5F, 0x08, 0x43, 0x53, /* \_PR_.CS */
		0x54, 0x5F										/* T_		*/
	};
	
	char chunk_c1[] =
	{
		0x12, 0x1C, 0x04, 0x11, 0x14, 0x0A, 0x11, 0x82,
		0x0C, 0x00, 0x7F, 0x01, 0x02, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79, 0x00,
		0x01, 0x01, 0x0B, 0xE8, 0x03 					
	};
	
	char chunk_c2[] =
	{
		0x12, 0x1E, 0x04, 0x11, 0x14, 0x0A, 0x11, 0x82,
		0x0C, 0x00, 0x7F, 0x01, 0x02, 0x01, 0x10, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79, 0x00,
		0x0A, 0x02, 0x0A, 0x40, 0x0B, 0xF4, 0x01 					
	};
	
	char chunk_c3[] =
	{
		0x12, 0x1F, 0x04, 0x11, 0x14, 0x0A, 0x11, 0x82,
		0x0C, 0x00, 0x7F, 0x01, 0x02, 0x01, 0x20, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79, 0x00,
		0x0A, 0x03, 0x0B, 0x60, 0x03, 0x0B, 0x5E, 0x01
	};
		
	char chunk_alias[] = 
	{
		0x10, 0x14, 0x5C, 0x2E, 0x5F, 0x50, 0x52, 0x5F, /* ..\._PR_ */
		0x43, 0x50, 0x55, 0x30, 0x06, 0x43, 0x53, 0x54, /* CPU0.CST */
		0x5F, 0x5F, 0x43, 0x53, 0x54					/* __CST	*/
	};
	
	if (fadt == NULL) {
		verbose ("FACP not exists: C-States not generated !!!\n");
		return NULL;
	}
	
	struct acpi_2_dsdt* dsdt = (void*)fadt->DSDT;
	
	if (dsdt == NULL) {
		verbose ("DSDT not found: C-States not generated !!!\n");
		return NULL;
	}
	
	if (acpi_cpu_count == 0) 
		find_acpi_cpu_names((void*)dsdt, dsdt->Length);
	
	if (acpi_cpu_count > 0) {
		bool c2_enabled = fadt->C2_Latency < 100, c3_enabled = fadt->C3_Latency < 1000;
		
		// Setup C2 Latency
		if (c2_enabled)
			chunk_c2[27] = fadt->C2_Latency & 0xff;
		
		// Setup C3 Latency
		if (c3_enabled) {
			chunk_c3[27] = fadt->C3_Latency & 0xff;
			chunk_c3[28] = (fadt->C3_Latency >> 8) & 0xff;
		}
		
		// Generating SSDT
		uint32_t package_length = 
			4 +
			sizeof(chunk_c1) + 
			c2_enabled * sizeof(chunk_c2) +
			c3_enabled * sizeof(chunk_c3);
		
		if (package_length > 0x3f) 
			package_length++;
		
		uint32_t name_length = 
			1 +
			sizeof(chunk_name_body) + 
			1 + package_length;
		
		if (name_length > 0x3f) 
			name_length++;
		
		uint32_t ssdt_size = 
			sizeof(ssdt_header) + 
			1 + name_length +
			acpi_cpu_count * sizeof(chunk_alias);
		
		struct acpi_2_ssdt *ssdt = (void*)AllocateKernelMemory(ssdt_size);
		int fd = openmem((char*)ssdt, ssdt_size);

		// Header
		write(fd, ssdt_header, sizeof(ssdt_header));
		
		// Scope (\_PR) { Name (CST
		writebyte(fd, 0x10); // id
		if (name_length > 0x3f) 
		{
			writebyte(fd, 0x40 | (name_length & 0xf)); // lo half-byte
			writebyte(fd, name_length >> 4); // hi byte
		}
		else 
		{
			writebyte(fd, name_length); // length
		}
		write(fd, chunk_name_body, sizeof(chunk_name_body));
		
		//Package (0x04) { 0x03,
		writebyte(fd, 0x12); // id
		if (package_length > 0x3f) 
		{
			writebyte(fd, 0x40 | (package_length & 0xf)); // lo half-byte
			writebyte(fd, package_length >> 4); // hi byte
		}
		else 
		{
			writebyte(fd, package_length); // length
		}
		uint8_t cstates_count = 1 + c2_enabled + c3_enabled;
		writebyte(fd, cstates_count + 1);
		writebyte(fd, 0x0A); // first entry - number of c-states
		writebyte(fd, cstates_count);
		
		// C1
		write(fd, chunk_c1, sizeof(chunk_c1));
		
		// C2
		if (c2_enabled) 
			write(fd, chunk_c2, sizeof(chunk_c2));
		
		// C3
		if (c3_enabled) 
			write(fd, chunk_c3, sizeof(chunk_c3));
		
		// Write aliases
		int i;
		for (i = 0; i < acpi_cpu_count; i++) {
			int j;
			for (j = 0; j < 4; j++)
				chunk_alias[8+j] = acpi_cpu_name[i][j];
			write(fd, chunk_alias, sizeof(chunk_alias));
		}
		
		close(fd);
		
		ssdt->Length = ssdt_size;
		ssdt->Checksum = 0;
		ssdt->Checksum = 256 - checksum8(ssdt, ssdt->Length);
		
		//dumpPhysAddr("C-States SSDT content: ", ssdt, ssdt_size);
		
		verbose ("SSDT with CPU C-States generated successfully\n");
		
		return ssdt;
	}
	else {
		verbose ("DSDT CPUs not found: C-States not generated !!!\n");
	}

	return NULL;
}

struct acpi_2_ssdt *generate_pss_ssdt(struct acpi_2_dsdt* dsdt)
{	
	char ssdt_header[] =
	{
		0x53, 0x53, 0x44, 0x54, 0x7E, 0x00, 0x00, 0x00, /* SSDT.... */
		0x01, 0x6A, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x00, /* ..PmRef. */
		0x43, 0x70, 0x75, 0x50, 0x6D, 0x00, 0x00, 0x00, /* CpuPm... */
		0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* .0..INTL */
		0x31, 0x03, 0x10, 0x20,							/* 1.._		*/
	};
	
	char chunk_name_body[] =
	{
		0x5C, 0x5F, 0x50, 0x52, 0x5F, 0x08, 0x50, 0x53, /* \_PR_.PS */
		0x53, 0x5F										/* S_		*/
	};
	
	char chunk_alias[] = 
	{
		0x10, 0x14, 0x5C, 0x2E, 0x5F, 0x50, 0x52, 0x5F, /* ..\._PR_ */
		0x43, 0x50, 0x55, 0x30, 0x06, 0x50, 0x53, 0x53, /* CPU0.PSS */
		0x5F, 0x5F, 0x50, 0x53, 0x53					/* __PSS	*/
	};
	
	if (Platform.CPU.Vendor != 0x756E6547) {
		verbose ("Not an Intel platform: P-States will not be generated !!!\n");
		return NULL;
	}
	
	if (!(Platform.CPU.Features & CPU_FEATURE_MSR)) {
		verbose ("Unsupported CPU: P-States will not be generated !!!\n");
		return NULL;
	}
	
	if (acpi_cpu_count == 0) 
		find_acpi_cpu_names((void*)dsdt, dsdt->Length);
	
	if (acpi_cpu_count > 0) 
	{	
		bool cpu_dynamic_fsb = false, cpu_noninteger_bus_ratio = (rdmsr64(MSR_IA32_PERF_STATUS) & (1ULL << 46));
		struct p_state initial, maximum, minimum, p_states[32];
		uint8_t p_states_count;		
		
		// Retrieving P-States, ported from code by superhai (c)
		

		
		switch (Platform.CPU.Family) {
			case 0x06: 
			{
				switch (Platform.CPU.Model) 
				{
					case 0x0F: // Intel Core (65nm)
					case 0x17: // Intel Core (45nm)
					case 0x1C: // Intel Atom (45nm)
					case 0x1A: // Intel Core i7 LGA1366 (45nm)
					case 0x1E: // Intel Core i5, i7 LGA1156 (45nm)
					case 0x25: // Intel Core i3, i5, i7 LGA1156 (32nm)
					case 0x2C: // Intel Core i7 LGA1366 (32nm) 6 Core
						if (rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 27)) 
						{
							wrmsr64(MSR_IA32_EXT_CONFIG, (rdmsr64(MSR_IA32_EXT_CONFIG) | (1 << 28))); 
							delay(1);
							cpu_dynamic_fsb = rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 28);
						}
						break;
				}
			}
		}
		
		initial.Control = rdmsr64(MSR_IA32_PERF_STATUS);
		
		maximum.Control = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 32) & 0x1F3F) | (0x4000 * cpu_noninteger_bus_ratio);
		maximum.CID = ((maximum.FID & 0x1F) << 1) | cpu_noninteger_bus_ratio;
		
		minimum.FID = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 24) & 0x1F) | (0x80 * cpu_dynamic_fsb);
		minimum.VID = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 48) & 0x3F);
		
		if (minimum.FID == 0) 
		{
			uint8_t i;
			// Probe for lowest fid
			for (i = maximum.FID; i >= 0x6; i--) 
			{
				wrmsr64(MSR_IA32_PERF_CONTROL, (rdmsr64(MSR_IA32_PERF_CONTROL) & 0xFFFFFFFFFFFF0000ULL) | (i << 8) | minimum.VID);
				intel_waitforsts();
				minimum.FID = (rdmsr64(MSR_IA32_PERF_STATUS) >> 8) & 0x1F; 
				delay(1);
			}
			
			wrmsr64(MSR_IA32_PERF_CONTROL, (rdmsr64(MSR_IA32_PERF_CONTROL) & 0xFFFFFFFFFFFF0000ULL) | (maximum.FID << 8) | maximum.VID);
			intel_waitforsts();
		}
		
		if (minimum.VID == maximum.VID) 
		{	
			uint8_t i;
			// Probe for lowest vid
			for (i = maximum.VID; i > 0xA; i--) 
			{
				wrmsr64(MSR_IA32_PERF_CONTROL, (rdmsr64(MSR_IA32_PERF_CONTROL) & 0xFFFFFFFFFFFF0000ULL) | (minimum.FID << 8) | i);
				intel_waitforsts();
				minimum.VID = rdmsr64(MSR_IA32_PERF_STATUS) & 0x3F; 
				delay(1);
			}
			
			wrmsr64(MSR_IA32_PERF_CONTROL, (rdmsr64(MSR_IA32_PERF_CONTROL) & 0xFFFFFFFFFFFF0000ULL) | (maximum.FID << 8) | maximum.VID);
			intel_waitforsts();
		}
		
		minimum.CID = ((minimum.FID & 0x1F) << 1) >> cpu_dynamic_fsb;
		
		// Sanity check
		if (maximum.CID < minimum.CID) 
		{
			DBG("Insane FID values!");
			p_states_count = 1;
		}
		else
		{
			// Finalize P-States
			// Find how many P-States machine supports
			p_states_count = maximum.CID - minimum.CID + 1;
			
			if (p_states_count > 32) 
				p_states_count = 32;
			
			uint8_t vidstep;
			uint8_t i = 0, u, invalid = 0;
			
			vidstep = ((maximum.VID << 2) - (minimum.VID << 2)) / (p_states_count - 1);
			
			for (u = 0; u < p_states_count; u++) 
			{
				i = u - invalid;
				
				p_states[i].CID = maximum.CID - u;
				p_states[i].FID = (p_states[i].CID >> 1);
				
				if (p_states[i].FID < 0x6) 
				{
					if (cpu_dynamic_fsb) 
						p_states[i].FID = (p_states[i].FID << 1) | 0x80;
				} 
				else if (cpu_noninteger_bus_ratio) 
				{
					p_states[i].FID = p_states[i].FID | (0x40 * (p_states[i].CID & 0x1));
				}
				
				if (i && p_states[i].FID == p_states[i-1].FID)
					invalid++;
				
				p_states[i].VID = ((maximum.VID << 2) - (vidstep * u)) >> 2;
				
				uint32_t multiplier = p_states[i].FID & 0x1f;		// = 0x08
				bool half = p_states[i].FID & 0x40;					// = 0x01
				bool dfsb = p_states[i].FID & 0x80;					// = 0x00
				uint32_t fsb = Platform.CPU.FSBFrequency / 1000000; // = 400
				uint32_t halffsb = (fsb + 1) >> 1;					// = 200
				uint32_t frequency = (multiplier * fsb);			// = 3200
				
				p_states[i].Frequency = (frequency + (half * halffsb)) >> dfsb;	// = 3200 + 200 = 3400
			}
			
			p_states_count -= invalid;
		}
		
		// Generating SSDT
		
		if (p_states_count > 0) 
		{	
			uint32_t i, pss_entries_size = 33 * p_states_count, pss_package_length = pss_entries_size + 2;
			
			if (pss_package_length > 0x3f) pss_package_length++; // for chunks > 0x3f bytes length have 2 bytes encoding
			
			uint32_t pss_name_length = (1 /* id=0x12 */ + pss_package_length) + (1 + 10);
			
			if (pss_name_length > 0x3f) pss_name_length++;
			
			uint32_t ssdt_size = 36 + (1 /* id=0x10 */ + pss_name_length) + acpi_cpu_count * sizeof(chunk_alias);
			
			struct acpi_2_ssdt *ssdt = (void*)AllocateKernelMemory(ssdt_size);
			int fd = openmem((char*)ssdt, ssdt_size);
			
			// write header
			write(fd, ssdt_header, sizeof(ssdt_header));
			
			// write Scope (\_PR) {	Name (PSS, ...
			writebyte(fd, 0x10); // id
			if (pss_name_length > 0x3f) 
			{
				writebyte(fd, 0x40 | (pss_name_length & 0xf)); // lo half-byte
				writebyte(fd, pss_name_length >> 4); // hi byte
			}
			else 
			{
				writebyte(fd, pss_name_length); // length
			}
			write(fd, chunk_name_body, sizeof(chunk_name_body));
			
			// write Package(p_states_count) { ...
			writebyte(fd, 0x12); // id
			if (pss_package_length > 0x3f) 
			{
				writebyte(fd, 0x40 | (pss_package_length & 0xf)); // lo half-byte
				writebyte(fd, pss_package_length >> 4); // hi byte
			}
			else 
			{
				writebyte(fd, pss_package_length); // length
			}
			writebyte(fd, p_states_count); // entries
			
			for (i = 0; i < p_states_count; i++) 
			{
				DBG("P-State: Frequency %d MHz, FID 0x%x, VID 0x%x\n", p_states[i].Frequency, p_states[i].FID, p_states[i].VID);
				
				writebyte(fd, 0x12); // chunk id
				writebyte(fd, 32); // chunk length without id 
				writebyte(fd, 6); // entries
				
				writebyte(fd, 0x0C); /* id */ writeint(fd, p_states[i].Frequency); // value
				writebyte(fd, 0x0C); /* id */ writeint(fd, 0x00000000); // value
				writebyte(fd, 0x0C); /* id */ writeint(fd, 0x0000000A); // value
				writebyte(fd, 0x0C); /* id */ writeint(fd, 0x0000000A); // value
				writebyte(fd, 0x0C); /* id */ writeint(fd, p_states[i].Control); // value
				writebyte(fd, 0x0C); /* id */ writeint(fd, i + 1); // value
			}
			
			// Write aliases
			for (i = 0; i < acpi_cpu_count; i++) {
				int j;
				for (j = 0; j < 4; j++)
					chunk_alias[8+j] = acpi_cpu_name[i][j];
				write(fd, chunk_alias, sizeof(chunk_alias));
			}
			
			ssdt->Length = ssdt_size;
			ssdt->Checksum = 0;
			ssdt->Checksum = 256 - checksum8(ssdt, ssdt->Length);
			
			//dumpPhysAddr("P-States SSDT content: ", ssdt, ssdt_size);
			verbose ("SSDT with CPU P-States generated successfully\n");
			
			return ssdt;
		}
	}
	else {
		verbose ("DSDT CPUs not found: P-States not generated !!!\n");
	}
	
	return NULL;
}

struct acpi_2_fadt *patch_fadt(struct acpi_2_fadt *fadt, struct acpi_2_dsdt *new_dsdt)
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
		verbose("FADT: Restart Fix applied!\n");
	}
	
	// Patch DSDT Address if we have loaded DSDT.aml
	if(new_dsdt)
	{
		DBG("DSDT: Old @%x,%x, ",fadt_mod->DSDT,fadt_mod->X_DSDT);
		
		fadt_mod->DSDT=(uint32_t)new_dsdt;
		if ((uint32_t)(&(fadt_mod->X_DSDT))-(uint32_t)fadt_mod+8<=fadt_mod->Length)
			fadt_mod->X_DSDT=(uint32_t)new_dsdt;
		
		DBG("New @%x,%x\n",fadt_mod->DSDT,fadt_mod->X_DSDT);
		
		verbose("FADT: Using custom DSDT!\n");
	}
	
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
	
	// Load replacement DSDT
	new_dsdt=loadACPITable("DSDT.aml");
	// Mozodojo: going to patch FACP and load SSDT's even if DSDT.aml is not present
	/*if (!new_dsdt)
	 {
	 return setupAcpiNoMod();
	 }*/
	
	// Mozodojo: Load additional SSDTs
	struct acpi_2_ssdt *new_ssdt[32]; // 30 + 2 additional tables for pss & cst
	int  ssdt_count=0;
	
	// SSDT Options
	bool drop_ssdt=false, generate_pstates=false, generate_cstates=false; 
	
	getBoolForKey(kDropSSDT, &drop_ssdt, &bootInfo->bootConfig);
	getBoolForKey(kGeneratePStates, &generate_pstates, &bootInfo->bootConfig);
	getBoolForKey(kGenerateCStates, &generate_cstates, &bootInfo->bootConfig);
	
	{
		int i;
		
		for (i=0; i<30; i++)
		{
			char filename[512];

			sprintf(filename, i>0?"SSDT-%d.aml":"SSDT.aml", i);
			
			if(new_ssdt[ssdt_count] = loadACPITable(filename)) 
			{				
				ssdt_count++;
			}
			else 
			{
				break;
			}
		}
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
			
			// mozo: using malloc cos I didn't found how to free already allocated kernel memory
			rsdt_mod=(struct acpi_2_rsdt *)malloc(rsdt->Length); 
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
				
				if (drop_ssdt && tableSign(table, "SSDT"))
				{
					dropoffset++;
					continue;
				}
				if (tableSign(table, "DSDT"))
				{
					DBG("DSDT found\n");
					
					if(new_dsdt)
						rsdt_entries[i-dropoffset]=(uint32_t)new_dsdt;
										
					continue;
				}
				if (tableSign(table, "FACP"))
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
					
					// Generate _CST SSDT
					if (generate_cstates && (new_ssdt[ssdt_count] = generate_cst_ssdt(fadt_mod))) 
						ssdt_count++;
					
					// Generating _PSS SSDT
					if (generate_pstates && (new_ssdt[ssdt_count] = generate_pss_ssdt((void*)fadt_mod->DSDT)))
						ssdt_count++;
					
					continue;
				}
			}
			DBG("\n");
			
			// Allocate rsdt in Kernel memory area
			rsdt_mod->Length += rsdt_mod->Length + 4*ssdt_count - 4*dropoffset;
			struct acpi_2_rsdt *rsdt_copy = (struct acpi_2_rsdt *)AllocateKernelMemory(rsdt_mod->Length);
			memcpy (rsdt_copy, rsdt_mod, rsdt_mod->Length);
			free(rsdt_mod); rsdt_mod = rsdt_copy;
			rsdp_mod->RsdtAddress=(uint32_t)rsdt_mod;
			rsdt_entries_num=(rsdt_mod->Length-sizeof(struct acpi_2_rsdt))/4;
			rsdt_entries=(uint32_t *)(rsdt_mod+1);
			
			// Mozodojo: Insert additional SSDTs into RSDT
			if(ssdt_count>0)
			{
				int j;
				
				for (j=0; j<ssdt_count; j++)
					rsdt_entries[i-dropoffset+j]=(uint32_t)new_ssdt[j];
					
				verbose("RSDT: Added %d SSDT table(s)\n", ssdt_count);
			}

			// Correct the checksum of RSDT
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
				
				// mozo: using malloc cos I didn't found how to free already allocated kernel memory
				xsdt_mod=(struct acpi_2_xsdt*)malloc(xsdt->Length); 
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
					
					if (drop_ssdt && tableSign(table, "SSDT"))
					{
						dropoffset++;
						continue;
					}					
					if (tableSign(table, "DSDT"))
					{
						DBG("DSDT found\n");
						
						if (new_dsdt) 
							xsdt_entries[i-dropoffset]=(uint32_t)new_dsdt;
						
						DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);
						
						continue;
					}
					if (tableSign(table, "FACP"))
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
						
						// Generate _CST SSDT
						if (generate_cstates && (new_ssdt[ssdt_count] = generate_cst_ssdt(fadt_mod))) 
							ssdt_count++;
						
						// Generating _PSS SSDT
						if (generate_pstates && (new_ssdt[ssdt_count] = generate_pss_ssdt((void*)fadt_mod->DSDT)))
							ssdt_count++;
						
						continue;
					}
					
					DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);
					
				}
				
				// Allocate xsdt in Kernel memory area
				xsdt_mod->Length += 8*ssdt_count - 8*dropoffset;
				struct acpi_2_xsdt *xsdt_copy = (struct acpi_2_xsdt *)AllocateKernelMemory(xsdt_mod->Length);
				memcpy(xsdt_copy, xsdt_mod, xsdt_mod->Length);
				free(xsdt_mod); xsdt_mod = xsdt_copy;
				rsdp_mod->XsdtAddress=(uint32_t)xsdt_mod;
				xsdt_entries_num=(xsdt_mod->Length-sizeof(struct acpi_2_xsdt))/8;
				xsdt_entries=(uint64_t *)(xsdt_mod+1);
				
				// Mozodojo: Insert additional SSDTs into XSDT
				if(ssdt_count>0)
				{
					int j;
					
					for (j=0; j<ssdt_count; j++)
						xsdt_entries[i-dropoffset+j]=(uint32_t)new_ssdt[j];
						
					verbose("Added %d SSDT table(s) into XSDT\n", ssdt_count);
				}

				// Correct the checksum of XSDT
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
		
		//verbose("Patched ACPI version %d DSDT\n", version+1);
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
#if DEBUG_ACPI
	printf("Press a key to continue... (DEBUG_ACPI)\n");
	getc();
#endif
	return 1;
}
