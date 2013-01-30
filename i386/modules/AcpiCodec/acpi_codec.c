/*
 * Copyright 2008 mackerintel
 */

/* 
 Copyright (c) 2010, Intel Corporation
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 * Neither the name of Intel Corporation nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 2011,2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "acpi.h"
#include "acpidecode.h"
#include "acpicode.h"
#include "efi_tables.h"
#include "fake_efi.h"
#include "acpi_codec.h"
#include "platform.h"
#include "cpu.h"
#include "xml.h"
#include "sl.h"
#include "convert.h"
#include "modules.h"
#include "pci.h"
#include "pci_root.h"

U64 rsd_p;
ACPI_TABLES acpi_tables;
U32 uuid32;
U32 Model32;
bool checkOem = false;

extern EFI_STATUS addConfigurationTable();

extern EFI_GUID gEfiAcpiTableGuid;
extern EFI_GUID gEfiAcpi20TableGuid;

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

#define OLD_SSDT 0
#define BETA 0
#define BUILD_ACPI_TSS 0
#define pstate_power_support 1

#if BETA
#ifdef pstate_power_support
#undef pstate_power_support
#endif
#define pstate_power_support 1
#endif

#if DEBUG_ACPI
static void print_nameseg(U32 i);
#endif

static ACPI_TABLE_HEADER * get_new_table_in_list(U32 *new_table_list, U32 Signature, U8 *retIndex );
static U8 get_number_of_tables_in_list(U32 *new_table_list, U32 Signature );
static U8 get_0ul_index_in_list(U32 *new_table_list, bool reserved );
static void sanitize_new_table_list(U32 *new_table_list );
static void move_table_list_to_kmem(U32 *new_table_list );
static ACPI_TABLE_RSDP * gen_alloc_rsdp_v2_from_v1(ACPI_TABLE_RSDP *rsdp );
static ACPI_TABLE_RSDT * gen_alloc_rsdt_from_xsdt(ACPI_TABLE_XSDT *xsdt);
static ACPI_TABLE_XSDT * gen_alloc_xsdt_from_rsdt(ACPI_TABLE_RSDT *rsdt);
#if 0
static void MakeAcpiSgn(void);
#endif
static void *loadACPITable(U32 *new_table_list, char *dirspec, const char *filename );
static int generate_cpu_map_from_acpi(ACPI_TABLE_DSDT * DsdtPointer);
static ACPI_GENERIC_ADDRESS FillGASStruct(U32 Address, U8 Length);
static U32 process_xsdt (ACPI_TABLE_RSDP *rsdp_mod , U32 *new_table_list);
static U32 process_rsdt(ACPI_TABLE_RSDP *rsdp_mod , bool gen_xsdt, U32 *new_table_list);
static ACPI_TABLE_FADT * patch_fadt(ACPI_TABLE_FADT *fadt, ACPI_TABLE_DSDT *new_dsdt, bool UpdateFADT);


#define IA32_MISC_ENABLES 0x01A0
#define MSR_TURBO_POWER_CURRENT_LIMIT 0x1AC
#define MSR_PKG_CST_CONFIG_CONTROL 0x00E2
#define MSR_RAPL_POWER_UNIT 0x606
#define MSR_PKG_RAPL_POWER_LIMIT 0x610
static U32 turbo_enabled = 0;
static U32 ProcessMadt(ACPI_TABLE_MADT * madt, MADT_INFO * madt_info, void * buffer, U32 bufferSize, U32 NB_CPU);
static U32 buildMADT(U32 * new_table_list, ACPI_TABLE_DSDT *dsdt, MADT_INFO * madt_info);
static U32 BuildSsdt(MADT_INFO * madt_info, ACPI_TABLE_DSDT *dsdt, void * buffer, U32 bufferSize, bool enable_cstates, bool enable_pstates,  bool enable_tstates);
static bool is_sandybridge(void);
static bool is_jaketown(void);
static U32 encode_pstate(U32 ratio);
static void collect_cpu_info(CPU_DETAILS * cpu);
#ifndef BETA
//static U32 BuildCoreIPstateInfo(CPU_DETAILS * cpu);
#endif
static U32 BuildCstateInfo(CPU_DETAILS * cpu, U32 pmbase);
static U32 BuildPstateInfo(CPU_DETAILS * cpu);
static U32 ProcessSsdt(U32 * new_table_list, ACPI_TABLE_DSDT *dsdt, MADT_INFO * madt_info, bool enable_cstates, bool enable_pstates, bool enable_tstates );
static void * buildCpuScope (void * current, U32 cpu_namespace, PROCESSOR_NUMBER_TO_NAMESEG * aslCpuNamePath);
static void * buildPDC(void * current);
static void * buildOSC(void * current);
static void * buildPSS(void * current, PKG_PSTATES * pkg_pstates);
static void * buildPSD(void * current, U32 domain, U32 cpusInDomain, U32 pstate_coordination);
static void * buildPPC(void * current);
static void * buildPCT(void * current);
static void * buildCstate(void * current, ACPI_GENERIC_ADDRESS * gas, CSTATE * cstate);
static void * buildReturnPackageCST(void * current, PKG_CSTATES * pkg_cstates);
static void * buildCST(void * current, PKG_CSTATES * mwait_pkg_cstates, PKG_CSTATES * io_pkg_cstates);
#if BUILD_ACPI_CSD
static void * buildCSD(void * current, U32 domain, U32 cpusInDomain, PKG_CSTATES * pkg_cstates);
#endif
#if BUILD_ACPI_TSS
static U32 BuildTstateInfo(CPU_DETAILS * cpu);
static void * buildTPC(void * current);
static void * buildPTC(void * current);
static void * buildTSS(void * current, PKG_TSTATES * pkg_tstates);
static void * buildTSD(void * current, U32 domain, U32 cpusInDomain);
#endif
#if pstate_power_support
static U64 mulU64byU64(U64 a, U64 b, U64 * high);
static U32 compute_pstate_power(CPU_DETAILS * cpu, U32 ratio, U32 TDP);
#endif
#if BUILD_ACPI_TSS || pstate_power_support
static U64 divU64byU64(U64 n, U64 d, U64 * rem);
static U32 compute_tdp(CPU_DETAILS * cpu);
#endif
static bool is_sandybridge(void);
static bool is_jaketown(void);
static U32 get_bclk(void);
static void GetMaxRatio(U32 * max_non_turbo_ratio);
//static U32 computePstateRatio(const U32 max, const U32 min, const U32 turboEnabled, const U32 numStates, const U32 pstate);
//static U32 computeNumPstates(const U32 max, const U32 min, const U32 turboEnabled, const U32 pssLimit);

#if UNUSED
static ACPI_TABLE_FACS* generate_facs(bool updatefacs );
#endif

#define MAX_NON_SSDT_TABLE 15
#define MAX_SSDT_TABLE 15 // 15 additional SSDT tables  
#define MAX_ACPI_TABLE MAX_NON_SSDT_TABLE + MAX_SSDT_TABLE

// Security space for SSDT , FACP & MADT table generation,
// the size can be increased 
// note: the table will not placed in the reserved space if the 'normal' space is not full
#define RESERVED_AERA 3

#define ACPI_TABLE_LIST_FULL MAX_ACPI_TABLE + RESERVED_AERA + 1

#define ACPI_TABLE_LIST_FULL_NON_RESERVED MAX_ACPI_TABLE + 1

#ifndef ULONG_MAX_32
#define ULONG_MAX_32 4294967295UL
#endif

#define __RES(s, u)												\
static inline unsigned u										\
resolve_##s(unsigned u defaultentry, char *str, int base)       \
{																\
unsigned u entry  = defaultentry;							\
if (str && (strcmp(str,"Default") != 0)) {					\
entry  = strtoul((const char *)str, NULL,base);				\
}															\
return entry;												\
}

__RES(pss, long)    
__RES(cst, int)  

static ACPI_TABLE_HEADER * get_new_table_in_list(U32 *new_table_list, U32 Signature, U8 *retIndex )
{
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) new_table_list;
	U8 index ;
	*retIndex = 0;
	
	for (index = 0; index < (MAX_ACPI_TABLE + RESERVED_AERA); index++)
	{
		if (*(U32 *) (table_array[index]->Signature) == Signature)
		{
			*retIndex = index;
			return table_array[index] ;
		}
	}
	return (void*)0ul;
}

static U8 get_number_of_tables_in_list(U32 *new_table_list, U32 Signature )
{
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) new_table_list;
	U8 index ;
	U8 InstalledTables = 0;
	
	for (index = 0; index < (MAX_ACPI_TABLE + RESERVED_AERA); index++)
	{
		if (*(U32 *) (table_array[index]->Signature) == Signature)
		{
			InstalledTables++ ;
		}
	}
	return InstalledTables;
}

static U8 get_0ul_index_in_list(U32 *new_table_list, bool reserved )
{
	U8 index ;
	
	U8 maximum = (reserved == true) ? MAX_ACPI_TABLE + RESERVED_AERA : MAX_ACPI_TABLE;
	
	for (index = 0; index < maximum; index++)
	{
		if (new_table_list[index] == 0ul)
		{
			return index ;
		}
	}
	return (reserved == true)? ACPI_TABLE_LIST_FULL : ACPI_TABLE_LIST_FULL_NON_RESERVED;
}

/* cparm : This time we check it by the acpi signature */
static void sanitize_new_table_list(U32 *new_table_list )
{
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) new_table_list;
	U8 index ;
	
	for (index = 0; index < MAX_ACPI_TABLE; index++)
	{
		U32 current_sig = *(U32 *) (table_array[index]->Signature);
		
		if ((current_sig == NAMESEG(ACPI_SIG_FACS) /* not supported */ ) 
			|| (current_sig == NAMESEG(ACPI_SIG_XSDT)) 
			|| (current_sig == NAMESEG(ACPI_SIG_RSDT)) || (*(volatile U64 *)table_array[index] == NAMESEG64(ACPI_SIG_RSDP)) )
		{
			
			void *buf = (void*)new_table_list[index];
			free(buf);
			new_table_list[index] = 0ul ;
		}
	}
}

/* cparm : move all tables to kernel memory */
static void move_table_list_to_kmem(U32 *new_table_list )
{
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) new_table_list;
	U8 index ;
	
	for (index = 0; index < MAX_ACPI_TABLE; index++)
	{
		if (new_table_list[index] != 0ul)
		{
            
			U32 current_sig = *(U32 *) (table_array[index]->Signature);
			if ((current_sig != NAMESEG(ACPI_SIG_FACS) /* not supported */ ) 
				&& (current_sig != NAMESEG(ACPI_SIG_XSDT)) 
				&& (current_sig != NAMESEG(ACPI_SIG_RSDT)) && (*(volatile U64 *)table_array[index] != NAMESEG64(ACPI_SIG_RSDP))
				&& (GetChecksum(table_array[index], table_array[index]->Length) == 0))
			{
				
				void *tableAddr=(void*)AllocateKernelMemory(table_array[index]->Length);
				if (!tableAddr) {
					printf("Unable to allocate kernel memory for aml file ");
					
					void *buf = (void*)new_table_list[index];
					free(buf);
					new_table_list[index] = 0ul ;
					continue;
				}
				bcopy(table_array[index], tableAddr, table_array[index]->Length);
				new_table_list[index] = 0ul ;
				new_table_list[index] = (U32)tableAddr ;
				
			} 
			else
			{
				
				void *buf = (void*)new_table_list[index];
				free(buf);
				new_table_list[index] = 0ul ;
			}			
		}
	}
}

static ACPI_TABLE_RSDP * gen_alloc_rsdp_v2_from_v1(ACPI_TABLE_RSDP *rsdp )
{
	
	ACPI_TABLE_RSDP * rsdp_conv = (ACPI_TABLE_RSDP *)AllocateKernelMemory(sizeof(ACPI_TABLE_RSDP));
	
	if (rsdp_conv) {
		bzero(rsdp_conv, sizeof(ACPI_TABLE_RSDP));
		memcpy(rsdp_conv, rsdp, ACPI_RSDP_REV0_SIZE);
		
		/* Add/change fields */
		rsdp_conv->Revision = 2; /* ACPI version 3 */
		rsdp_conv->Length = sizeof(ACPI_TABLE_RSDP);
		
		/* Correct checksums */    
		setRsdpchecksum(rsdp_conv);
		setRsdpXchecksum(rsdp_conv);
	}	    
    
    return (rsdp_conv) ? rsdp_conv : (void*)0ul ;
}

static ACPI_TABLE_RSDT * gen_alloc_rsdt_from_xsdt(ACPI_TABLE_XSDT *xsdt)
{
    U32 index;
    U32 num_tables;
    
	DBG("Attempting to generate RSDT from XSDT \n");
	
    num_tables= get_num_tables64(xsdt);
    
    ACPI_TABLE_RSDT * rsdt_conv=(ACPI_TABLE_RSDT *)AllocateKernelMemory(sizeof(ACPI_TABLE_HEADER)+(num_tables * 4));
	if (!rsdt_conv)
	{
		printf("Unable to allocate kernel memory for rsdt conv\n");
		return (void*)0ul;
	}
	
	
	bzero(rsdt_conv, sizeof(ACPI_TABLE_HEADER)+(num_tables * 4));
    memcpy(&rsdt_conv->Header, &xsdt->Header, sizeof(ACPI_TABLE_HEADER));
    
    rsdt_conv->Header.Signature[0] = 'R';
    rsdt_conv->Header.Signature[1] = 'S';
    rsdt_conv->Header.Signature[2] = 'D';
    rsdt_conv->Header.Signature[3] = 'T';
    rsdt_conv->Header.Length = sizeof(ACPI_TABLE_HEADER)+(num_tables * 4);
    
	for (index=0;index<num_tables;index++)
    {
		U64 ptr = xsdt->TableOffsetEntry[index];
		
		{				
			if (ptr > ULONG_MAX)
			{
#if DEBUG_ACPI						
				printf("Warning xsdt->TableOffsetEntry[%d]: Beyond addressable memory in this CPU mode, ignored !!!\n",index);
#endif
				continue;	
			}
#if DEBUG_ACPI	
			printf("* Processing : ");
			print_nameseg(*(U32 *) ((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Signature);
			printf("\n");			
#endif					
			int method = 0;
			getIntForKey(kAcpiMethod, &method, &bootInfo->chameleonConfig);
			
			
			if (method != 0x2)
			{
				if (GetChecksum(((ACPI_TABLE_HEADER *) (unsigned long)ptr), 
								((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Length) != 0)
				{
#if DEBUG_ACPI						
					printf("Warning : Invalide checksum, ignored !!!\n",index);
#endif
					continue;
				}
			}			
			
		}			
        
		{
			if (*(U32 *) ((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Signature == NAMESEG(ACPI_SIG_FADT))
			{
				ACPI_TABLE_FADT *fadt=(ACPI_TABLE_FADT *)((U32)ptr);
				
				ACPI_TABLE_FADT *fadt_conv = (void*)0ul;
				
				if (fadt->Header.Revision > 1)
				{
					U8 buffer[0x74];
					DBG("Downgrading ACPI V%d FADT to ACPI V1 FADT \n", fadt->Header.Revision);
					fadt_conv=(ACPI_TABLE_FADT *)buffer;
					memcpy(fadt_conv, fadt, 0x74);
					fadt_conv->Header.Length   = 0x74;
					fadt_conv->Header.Revision = 0x01;					
					SetChecksum(&fadt_conv->Header);
				} 
				else
				{
					fadt_conv = fadt;
				}
				
				ACPI_TABLE_FADT *fadt_mod = patch_fadt(fadt_conv, ((ACPI_TABLE_DSDT*)((U32)fadt->XDsdt)), false); 
				if (fadt_mod == (void*)0ul)
				{
					printf("Error: Failed to patch FADT Table, trying wiht the original fadt pointer\n");
					fadt_mod = fadt;
				}
				
				rsdt_conv->TableOffsetEntry[index] = ((U32)fadt_mod);
#if DEBUG_ACPI			
				print_nameseg(*(U32 *) ((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Signature);
				printf(" table converted and added succesfully\n");			
#endif
				continue;
			}
		}
        
		{
			rsdt_conv->TableOffsetEntry[index] = (U32)ptr;
#if DEBUG_ACPI			
			print_nameseg(*(U32 *) ((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Signature);
			printf(" table converted and added succesfully\n");	
#endif					
		}
        
    }
    DBG("RSDT_CONV : Original checksum %d\n", rsdt_conv->Header.Checksum);
    SetChecksum(&rsdt_conv->Header);
    DBG("New checksum %d\n", rsdt_conv->Header.Checksum);
	
    return rsdt_conv;
}

static ACPI_TABLE_XSDT * gen_alloc_xsdt_from_rsdt(ACPI_TABLE_RSDT *rsdt)
{	
	U32 index;
    U32 num_tables;	
    
	DBG("Attempting to generate XSDT from RSDT \n");
	
    num_tables= get_num_tables(rsdt);
    
    ACPI_TABLE_XSDT * xsdt_conv=(ACPI_TABLE_XSDT *)AllocateKernelMemory(sizeof(ACPI_TABLE_HEADER)+(num_tables * 8));
	
	if (!xsdt_conv) {
		printf("Unable to allocate kernel memory for xsdt conv\n");
		return (void*)0ul;
	}
	
	bzero(xsdt_conv, sizeof(ACPI_TABLE_HEADER)+(num_tables * 8));
    memcpy(&xsdt_conv->Header, &rsdt->Header, sizeof(ACPI_TABLE_HEADER));
    
    xsdt_conv->Header.Signature[0] = 'X';
    xsdt_conv->Header.Signature[1] = 'S';
    xsdt_conv->Header.Signature[2] = 'D';
    xsdt_conv->Header.Signature[3] = 'T';
    xsdt_conv->Header.Length = sizeof(ACPI_TABLE_HEADER)+(num_tables * 8);
    
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) rsdt->TableOffsetEntry;
	
    for (index=0;index<num_tables;index++)
    {
        {				
#if DEBUG_ACPI	
            printf("* Processing : ");
            print_nameseg(*(U32*) (table_array[index]->Signature));
            printf("\n");			
#endif			
            int method = 0;
			getIntForKey(kAcpiMethod, &method, &bootInfo->chameleonConfig);
			
			
			if (method != 0x2)
            {
                if (GetChecksum(table_array[index], table_array[index]->Length) != 0)
                {
#if DEBUG_ACPI						
                    printf("Warning : Invalide checksum, ignored !!!\n",index);
#endif
                    continue;
                }
            }
            
        }
        
        {
            if (*(U32 *) (table_array[index]->Signature) == NAMESEG(ACPI_SIG_FADT))
            {
                ACPI_TABLE_FADT *FacpPointer = ((ACPI_TABLE_FADT*)table_array[index]);
                ACPI_TABLE_FADT *fadt_mod = (ACPI_TABLE_FADT *)patch_fadt(FacpPointer,((ACPI_TABLE_DSDT*)FacpPointer->Dsdt),true);
                if (fadt_mod == (void*)0ul) 
                {
                    printf("Error: Failed to patch (& update) FADT Table, fallback to original fadt pointer\n");
                    fadt_mod = FacpPointer;
                }
                xsdt_conv->TableOffsetEntry[index] = ((U64)((U32)fadt_mod));
                
                continue;
            }
        }
        
        xsdt_conv->TableOffsetEntry[index] = ((U64)((U32)table_array[index])); 
    }
    DBG("XSDT_CONV : Original checksum %d\n", xsdt_conv->Header.Checksum);
    SetChecksum(&xsdt_conv->Header);
    DBG("New checksum %d\n", xsdt_conv->Header.Checksum);
	
    return xsdt_conv;
}

#if ACPISGN
static void MakeAcpiSgn(void)
{
    
	char * DefaultplatformName = NULL;
	Model32 = 0;
	
	if ((DefaultplatformName = readDefaultPlatformName()))
	{
		Model32 = OSSwapHostToBigInt32(adler32( (unsigned char *) DefaultplatformName, strlen(DefaultplatformName)));
	}
	
	uuid32 = 0;    
	
	const char *uuidStr = getStringFromUUID((int8_t*)(uint32_t)get_env(envSysId));
	
	if (strlen(uuidStr)) 
	{
		uuid32 = OSSwapHostToBigInt32(adler32( (unsigned char *) uuidStr, UUID_STR_LEN ));
	}
    
}
#endif

static void *loadACPITable(U32 *new_table_list, char *dirspec, const char *filename )
{	
	int fd = -1;
	char acpi_file[512];
    
	DBG("Searching for %s file ...\n", filename);
	// Check booting partition	
    
	sprintf(acpi_file, "%s%s",dirspec, filename); 
	
	fd=open(acpi_file,0);
	
	if (fd<0)
	{							
		DBG("Couldn't open ACPI Table: %s\n", acpi_file);
		return (void *)0ul ;				
	}		
	
	void *tableAddr=(void*)malloc(file_size (fd));
    
	if (tableAddr)
	{
		if (read (fd, tableAddr, file_size (fd))!=file_size (fd))
		{
			printf("Couldn't read table %s\n",acpi_file);
			free (tableAddr);
			close (fd);
			return (void *)0ul ;
		}
		
		close (fd);
		
		ACPI_TABLE_HEADER * header = (ACPI_TABLE_HEADER *)tableAddr;		
		
		if (*(U32*)(header->Signature) != NAMESEG("SSDT"))
		{
			U8 dummy = 0;
			if (get_new_table_in_list(new_table_list, *(U32*)(header->Signature), &dummy))
			{
#if DEBUG_ACPI
				printf("Warning: A ");
				print_nameseg(*(U32*) (header->Signature));
				printf(" Aml file is already loaded and registred, file skipped !!\n");
#endif
				free(tableAddr);
				return (void*)0ul;
			} 
		} 
		else
		{
			if (get_number_of_tables_in_list(new_table_list, NAMESEG("SSDT")) >= MAX_SSDT_TABLE)
			{
				DBG("Warning: Max number of SSDT aml files reached, file skipped !!\n");
				free(tableAddr);
				return (void*)0ul;
			}
		}
        
		
		if (checkOem == true)
		{			
			if (header->OemRevision == Model32)
			{
				goto continue_loading;
			}
			
			if (header->OemRevision == uuid32)
			{
				goto continue_loading;
			}
			
			DBG("Bad signature aka Oem Revision (0x%08lx) for Aml file (%s), file skipped !!\n", header->OemRevision, acpi_file);
			DBG("uuid32 (0x%08lx) , model32 (0x%08lx)\n", uuid32, Model32);
			
			free(tableAddr);
			return (void*)0ul;			
		} 
		
    continue_loading:
		
		if (GetChecksum(header, header->Length) == 0)
		{
			DBG("Found valid AML file : %s ", filename);
			verbose("[ %s ] read and stored at: %x", acpi_file, tableAddr);
			printf("\n");
			return tableAddr;
		} 
		else
		{
			printf("Warning : Incorrect cheksum for the file : %s,", acpi_file);
			printf("		  this file will be dropped.\n");
			free(tableAddr);
			return (void*)0ul;
		}		
	}
	else 
	{
		printf("Couldn't allocate memory for table %s\n", acpi_file);
		close (fd);
	}	
	
	return (void *)0ul ;
}

static U32 pmbase;
static short cpuNamespace;
PROCESSOR_NUMBER_TO_NAMESEG cpu_map[CPU_MAP_LIMIT];
unsigned int cpu_map_count;
int cpu_map_error;

#if DEBUG_ACPI
static void print_nameseg(U32 i)
{
    printf("%c%c%c%c",
           (int)(i & 0x000000ff),
           (int)((i & 0x0000ff00) >> 8),
           (int)((i & 0x00ff0000) >> 16),
           (int)(i >> 24));
}
#endif

static int generate_cpu_map_from_acpi(ACPI_TABLE_DSDT * DsdtPointer)
{
    PROCESSOR_NUMBER_TO_NAMESEG *map = cpu_map;
    U32 processor_namespace = 0;
    U32 cpu;
    U8 *current, *end;
    ACPI_TABLE_HEADER *header;
    struct acpi_namespace ns;
    
	if ((cpu_map_error == 1) || (DsdtPointer == (void*)0ul)) 
		return 1;
	else if (cpu_map_count > 0) 
		return 0;
	
    DBG("Attempting to autodetect CPU map from ACPI DSDT; wish me luck\n");	
    
    current = (U8 *) DsdtPointer;
    current = decodeTableHeader(current, &header);
    end = current - sizeof(*header) + header->Length;
    ns.depth = 0;
    acpi_processor_count = 0;
	//DBG("* DSDT debug start\n");
    parse_acpi_termlist(&ns, current, end);
	//DBG("* DSDT debug end\n");
	
    if (acpi_processor_count > CPU_MAP_LIMIT)
	{
		verbose("Too many processors: found %u processors\n", acpi_processor_count);
        return (cpu_map_error = 1);
	}
    if (acpi_processor_count == 0)
	{
		verbose( "Found no processors in ACPI\n");
        return (cpu_map_error = 1);
	}
    for (cpu = 0; cpu < acpi_processor_count; cpu++)
	{
        U32 nameseg;
        if (acpi_processors[cpu].pmbase)
		{
            U32 cpu_pmbase = acpi_processors[cpu].pmbase - 0x10;
            if (pmbase && cpu_pmbase != pmbase)
			{
				verbose("Found inconsistent pmbase addresses in ACPI: 0x%x and 0x%x\n", pmbase, cpu_pmbase);
				return (cpu_map_error = 1);
			}
            pmbase = cpu_pmbase;
        }
        if (acpi_processors[cpu].ns.depth > MAX_SUPPORTED_CPU_NAMESEGS + 1)
		{
			verbose("Processor path too deep: depth %u\n", acpi_processors[cpu].ns.depth);
			return (cpu_map_error = 1);
		}
        if (processor_namespace && acpi_processors[cpu].ns.nameseg[0] != processor_namespace)
		{
			verbose("Processor namespaces inconsistent\n");
			return (cpu_map_error = 1);
		}
        processor_namespace = acpi_processors[cpu].ns.nameseg[0];
        map->acpi_processor_number = acpi_processors[cpu].id;
        map->seg_count = acpi_processors[cpu].ns.depth - 1;
        for (nameseg = 0; nameseg < map->seg_count; nameseg++)
            map->nameseg[nameseg] = acpi_processors[cpu].ns.nameseg[nameseg + 1];
        map++;
    }
    if (!pmbase)
	{
		verbose("No pmbase found in ACPI\n");
		return (cpu_map_error = 1);
	}
    if (processor_namespace == NAMESEG("_PR_"))
        cpuNamespace = CPU_NAMESPACE_PR;
    else if (processor_namespace == NAMESEG("_SB_"))
        cpuNamespace = CPU_NAMESPACE_SB;
    else
	{
        verbose("Found processors in invalid namespace; not _PR_ or _SB_\n");
		return (cpu_map_error = 1);
	}
    cpu_map_count = map - cpu_map;
	
#if DEBUG_ACPI
	verbose("Found %d processors in ACPI, pmbase : 0x%x, cpu_map_count : %d, namespace : ",acpi_processor_count, pmbase, cpu_map_count );
	print_nameseg(processor_namespace); 
	verbose("\n");
    U32 i;
    verbose("Found processors name : \n" );
    for ( i = 0; i<cpu_map_count; i++)
	{			
		U32 nseg = *(U32*)cpu_map[i].nameseg;
        print_nameseg(nseg); 
		verbose(" ");
    }
    verbose("\n");
#endif
	
	// TODO: Save the cpu map into the device tree
    return (cpu_map_error = 0);
}

static bool is_sandybridge(void)
{
    return Platform.CPU.Model == CPU_MODEL_SANDYBRIDGE;
}

static bool is_jaketown(void)
{
    return Platform.CPU.Model == CPU_MODEL_JAKETOWN;
}

static U32 get_bclk(void)
{
	return (is_jaketown() || is_sandybridge()) ? 100 : 133;
}
/*
 //-----------------------------------------------------------------------------
 static U32 computePstateRatio(const U32 max, const U32 min, const U32 turboEnabled, const U32 numStates, const U32 pstate)
 {
 U32 ratiorange = max-min;
 U32 numGaps = numStates-1-turboEnabled;
 U32 adjPstate = pstate-turboEnabled;
 return (pstate == 0)     ? (max + turboEnabled) :
 (ratiorange == 0) ? max                  :
 max-(((adjPstate*ratiorange)+(numGaps/2))/numGaps);
 }
 //-----------------------------------------------------------------------------
 static U32 computeNumPstates(const U32 max, const U32 min, const U32 turboEnabled, const U32 pssLimit)
 {
 U32 ratiorange, maxStates, numStates;
 
 ratiorange = max - min + 1;
 maxStates = ratiorange + (turboEnabled ? 1 : 0);
 numStates = (pssLimit < maxStates) ? pssLimit : maxStates;
 return (numStates < 2) ? 0 : numStates;
 }
 */
#if BUILD_ACPI_TSS || pstate_power_support
static U64 divU64byU64(U64 n, U64 d, U64 * rem)
{
    U32 i;
    U64 q = n;
    U64 r = 0;
	
    for (i = 0; i < 64; i++) {
        r <<= 1;
        r |= (q & (1ULL << 63)) >> 63;
        q <<= 1;
        if (r >= d) {
            r -= d;
            q |= 1;
        }
    }
    if (rem)
        *rem = r;
    return q;
}

static U32 compute_tdp(CPU_DETAILS * cpu)
{ 
    {
        if (is_jaketown() || is_sandybridge())
        {
            U64 power_limit_1 = cpu->package_power_limit & ((1ULL << 15) - 1);
            U64 power_unit = cpu->package_power_sku_unit & ((1ULL << 4) - 1);
            U64 tdp = divU64byU64(power_limit_1, 1 << power_unit, NULL);
            return (U32)tdp;
        }
        else
        {
            // tdp = (TURBO_POWER_CURRENT_LIMIT MSR 1ACh bit [14:0] / 8) Watts
            return cpu->tdp_limit / 8;
        }
    }
	return (0);
}
#endif // BUILD_ACPI_TSS || pstate_power_support

#if pstate_power_support
static U64 mulU64byU64(U64 a, U64 b, U64 * high)
{
    U64 b_high = 0;
    U64 r_high = 0, r_low = 0;
    U64 bit;
	
    for (bit = 1; bit; bit <<= 1) {
        if (a & bit) {
            if (r_low + b < r_low)
                r_high++;
            r_low += b;
            r_high += b_high;
        }
        b_high <<= 1;
        b_high |= (b & (1ULL << 63)) >> 63;
        b <<= 1;
    }
	
    if (high)
        *high = r_high;
    return r_low;
}

static U32 compute_pstate_power(CPU_DETAILS * cpu, U32 ratio, U32 TDP)
{
	if (is_jaketown() || is_sandybridge())
	{
		U32 P1_Ratio = cpu->max_ratio_as_mfg;
		U64 M, pstate_power;
		
		// M = ((1.1 - ((P1_ratio - ratio) * 0.00625)) / 1.1) ^2
		// To prevent loss of precision compute M * 10^5 (preserves 5 decimal places)
		M = (P1_Ratio - ratio) * 625;
		M = (110000 - M);
		M = divU64byU64(M, 11, NULL);
		M = divU64byU64(mulU64byU64(M, M, NULL), 1000, NULL);
		
		// pstate_power = ((ratio/p1_ratio) * M * TDP)
		// Divide the final answer by 10^5 to remove the precision factor
		pstate_power = mulU64byU64(ratio, M, NULL);
		pstate_power = mulU64byU64(pstate_power, TDP, NULL);
		pstate_power = divU64byU64(pstate_power, P1_Ratio, NULL);
		pstate_power = divU64byU64(pstate_power, 100000, NULL);
		return (U32)pstate_power; // in Watts
	}
	else
	{
		// pstate_power[ratio] = (ratio/P1_ratio)^3 * Core_TDP + Uncore_TDP
		
		// Core_TDP = (TURBO_POWER_CURRENT_LIMIT MSR 1ACh bit [30:16] / 8) Watts
		U32 Core_TDP = cpu->tdc_limit / 8;
		
		// Uncore_TDP = TDP - Core_TDP
		U32 Uncore_TDP = TDP - Core_TDP;
		
		// max_ratio_as_mfg = P1_Ratio derived from Brand String returned by CPUID instruction
		U32 P1_Ratio = cpu->max_ratio_as_mfg;
		
#define PRECISION_FACTOR         (U32) 30
#define PRECISION_FACTOR_CUBED   (U32) (PRECISION_FACTOR * PRECISION_FACTOR * PRECISION_FACTOR)
		
		U32 ratio_factor = (ratio * PRECISION_FACTOR)/P1_Ratio;
		return ((ratio_factor * ratio_factor * ratio_factor * Core_TDP) / PRECISION_FACTOR_CUBED) + Uncore_TDP;
	}
    return (0);
}
#endif // pstate_power_support

static U32 encode_pstate(U32 ratio)
{
	if (is_jaketown() || is_sandybridge())
		return ratio << 8;
	return ratio;
}

//-----------------------------------------------------------------------------
static void GetMaxRatio(U32 * max_non_turbo_ratio)
{
	U32 index;
	U32 max_ratio=0;
	U32 frequency=0;
    U32 multiplier = 0;
    char			*BrandString;
	// Verify CPUID brand string function is supported
	if (Platform.CPU.CPUID[CPUID_80][0] < 80000004)
	{
		*max_non_turbo_ratio = max_ratio;
		return;
	}	
	BrandString = (char *)Platform.CPU.BrandString;
    // -2 to prevent buffer overrun because looking for y in yHz, so z is +2 from y
    for (index=0; index<48-2; index++) {
        // format is either “x.xxyHz” or “xxxxyHz”, where y=M,G,T and x is digits
        // Search brand string for “yHz” where y is M, G, or T
        // Set multiplier so frequency is in MHz
        if ( BrandString[index+1] == 'H' && BrandString[index+2] == 'z')
        {
            if (BrandString[index] == 'M')
                multiplier = 1;
            else if (BrandString[index] == 'G')
                multiplier = 1000;
            else if (BrandString[index] == 'T')
                multiplier = 1000000;
        }
        if (multiplier > 0 && index >= 4 /* who can i call that, buffer underflow :-) ??*/)
        {
            // Copy 7 characters (length of “x.xxyHz”)
            // index is at position of y in “x.xxyHz”
            
            // Compute frequency (in MHz) from brand string
            if (BrandString[index-3] == '.')
            { // If format is “x.xx”
                if (isdigit(BrandString[index-4]) && isdigit(BrandString[index-2]) &&
                    isdigit(BrandString[index-1]))
                {
                    frequency  = (U32)(BrandString[index-4] - '0') * multiplier;
                    frequency += (U32)(BrandString[index-2] - '0') * (multiplier / 10);
                    frequency += (U32)(BrandString[index-1] - '0') * (multiplier / 100);
                }                
            }
            else
            { // If format is xxxx
                if (isdigit(BrandString[index-4]) && isdigit(BrandString[index-3]) && 
                    isdigit(BrandString[index-2]) && isdigit(BrandString[index-1]))
                {
                    frequency  = (U32)(BrandString[index-4] - '0') * 1000;
                    frequency += (U32)(BrandString[index-3] - '0') * 100;
                    frequency += (U32)(BrandString[index-2] - '0') * 10;
                    frequency += (U32)(BrandString[index-1] - '0');
                    frequency *= multiplier;
                }
                
            }
            
            max_ratio = frequency / get_bclk();
            break; 
        }
    }
	
	// Return non-zero Max Non-Turbo Ratio obtained from CPUID brand string
	// or return 0 indicating Max Non-Turbo Ratio not available
	*max_non_turbo_ratio = max_ratio;
}

//-----------------------------------------------------------------------------
static void collect_cpu_info(CPU_DETAILS * cpu)
{  
	boolean_t	dynamic_acceleration = 0;
	U32	sub_Cstates = 0;
	U32 extensions = 0;    
	boolean_t	invariant_APIC_timer = 0;
	boolean_t	fine_grain_clock_mod = 0;

#if BUILD_ACPI_TSS || pstate_power_support
	if (Platform.CPU.CPUID[CPUID_0][0] >= 0x5) {        
		/*
		 * Extract the Monitor/Mwait Leaf info:
		 */
        sub_Cstates  = Platform.CPU.CPUID[CPUID_5][3];
        extensions   = Platform.CPU.CPUID[CPUID_5][2];	
	}
	
	if (Platform.CPU.CPUID[CPUID_0][0] >= 6) {
		dynamic_acceleration = bitfield(Platform.CPU.CPUID[CPUID_6][0], 1, 1); // "Dynamic Acceleration Technology (Turbo Mode)"
		invariant_APIC_timer = bitfield(Platform.CPU.CPUID[CPUID_6][0], 2, 2); //  "Invariant APIC Timer"
        fine_grain_clock_mod = bitfield(Platform.CPU.CPUID[CPUID_6][0], 4, 4);
	}
    cpu->turbo_available = (U32)dynamic_acceleration;
	
	{
		U32 temp32 = 0;
		U64 temp64=  0;
		int tdp;
		if (getIntForKey("TDP", &tdp, &bootInfo->chameleonConfig))
		{
			temp32 = (U32) (tdp*8) ; 
			
			int tdc;
			if (getIntForKey("TDC", &tdc, &bootInfo->chameleonConfig))
			{
				temp32 = (U32) (temp32) | tdc<<16 ; 
				
			}
			else if (tdp)
			{
				temp32 = (U32) (temp32) | ((tdp)*8)<<16 ;
			}
			
		}
		else if (!is_sandybridge() && !is_jaketown())
		{
			if (turbo_enabled && cpu->turbo_available)
			{
				temp64 = rdmsr64(MSR_TURBO_POWER_CURRENT_LIMIT);
				temp32 = (U32)temp64;
			} 
			else 
			{
				// Unfortunately, Intel don't provide a better method for non turbo processors
				// and it will give a TDP of 95w (for ex. mine is 65w) , to fix this issue,
				// you can set this value by simply adding the option TDP = XX (XX is an integer)
				// in your boot.plist 
				temp32 = (U32)0x02a802f8;
			}
			
		}
		if (temp32) {
			cpu->tdp_limit = ( temp32 & 0x7fff );
			cpu->tdc_limit = ( (temp32 >> 16) & 0x7fff );
		}
	}    
    
#endif
    
	switch (Platform.CPU.Family)
	{
		case 0x06: 
		{
			switch (Platform.CPU.Model) 
			{
				case CPU_MODEL_DOTHAN: 
				case CPU_MODEL_YONAH: // Yonah
				case CPU_MODEL_MEROM: // Merom
				case CPU_MODEL_PENRYN: // Penryn
				case CPU_MODEL_ATOM: // Intel Atom (45nm)
				{
					
					cpu->core_c1_supported = ((sub_Cstates >> 4) & 0xf) ? 1 : 0;
					cpu->core_c4_supported = ((sub_Cstates >> 16) & 0xf) ? 1 : 0;
					
					if (Platform.CPU.Model == CPU_MODEL_ATOM)
					{
						cpu->core_c2_supported = cpu->core_c3_supported = ((sub_Cstates >> 8) & 0xf) ? 1 : 0;
						cpu->core_c6_supported = ((sub_Cstates >> 12) & 0xf) ? 1 : 0;
                        
					} 
					else
					{
						cpu->core_c3_supported = ((sub_Cstates >> 12) & 0xf) ? 1 : 0;
						cpu->core_c2_supported = ((sub_Cstates >> 8) & 0xf) ? 1 : 0;
						cpu->core_c6_supported = 0;
                        
					}
                    
					cpu->core_c7_supported = 0;
                    
#if BETA
					GetMaxRatio(&cpu->max_ratio_as_mfg);
					U64 msr = rdmsr64(MSR_IA32_PERF_STATUS);
					U16 idlo = (msr >> 48) & 0xffff;
					U16 idhi = (msr >> 32) & 0xffff;
					cpu->min_ratio        = (U32) (idlo  >> 8) & 0xff;
					cpu->max_ratio_as_cfg = (U32) (idhi  >> 8) & 0xff;
					
#else
					if (Platform.CPU.MaxCoef) 
					{
						if (Platform.CPU.MaxDiv) 
						{
							cpu->max_ratio_as_cfg = cpu->max_ratio_as_mfg = (U32) (Platform.CPU.MaxCoef * 10) + 5;
						}
						else 
						{
							cpu->max_ratio_as_cfg = cpu->max_ratio_as_mfg = (U32) Platform.CPU.MaxCoef * 10;
						}
					}
#endif
					
					break;
				} 
				case CPU_MODEL_FIELDS:
				case CPU_MODEL_DALES:
				case CPU_MODEL_DALES_32NM:
				case CPU_MODEL_NEHALEM: 
				case CPU_MODEL_NEHALEM_EX:
				case CPU_MODEL_WESTMERE:
				case CPU_MODEL_WESTMERE_EX:
				case CPU_MODEL_SANDYBRIDGE:
				case CPU_MODEL_JAKETOWN:
				{		
					
					cpu->core_c1_supported = ((sub_Cstates >> 4) & 0xf) ? 1 : 0;
					cpu->core_c3_supported = ((sub_Cstates >> 8) & 0xf) ? 1 : 0;
					cpu->core_c6_supported = ((sub_Cstates >> 12) & 0xf) ? 1 : 0;
					cpu->core_c7_supported = ((sub_Cstates >> 16) & 0xf) ? 1 : 0;
					cpu->core_c2_supported = 0;
					cpu->core_c4_supported = 0;
					
					GetMaxRatio(&cpu->max_ratio_as_mfg);
                    U64 platform_info = rdmsr64(MSR_PLATFORM_INFO);                    
                    cpu->max_ratio_as_cfg = (U32) ((U32)platform_info >> 8) & 0xff; 
					cpu->min_ratio        = (U32) ((platform_info >> 40) & 0xff);
					
                    cpu->tdc_tdp_limits_for_turbo_flag = (platform_info & (1ULL << 29)) ? 1 : 0;
					cpu->ratio_limits_for_turbo_flag   = (platform_info & (1ULL << 28)) ? 1 : 0;
					cpu->xe_available = cpu->tdc_tdp_limits_for_turbo_flag | cpu->ratio_limits_for_turbo_flag;
					

                    
					if (is_sandybridge() || is_jaketown())
					{
						cpu->package_power_limit = rdmsr64(MSR_PKG_RAPL_POWER_LIMIT);
						cpu->package_power_sku_unit = rdmsr64(MSR_RAPL_POWER_UNIT);
					}
					break;
				}
				default:
					verbose ("Unsupported CPU\n");
					return /*(0)*/;
					break;
			}
		}
		default:			
			break;
	}
    
	cpu->mwait_supported = (extensions & (1UL << 0)) ? 1 : 0;	
    
    cpu->invariant_apic_timer_flag = (U32)invariant_APIC_timer;
    
#if DEBUG_ACPI
	printf("CPU INFO : \n");
#if BETA    
	printf("min_ratio : %d\n", cpu->min_ratio);
#endif
	printf("max_ratio_as_cfg : %d\n", cpu->max_ratio_as_cfg);
	printf("max_ratio_as_mfg : %d\n", cpu->max_ratio_as_mfg);
    
	printf("turbo_available : %d\n",cpu->turbo_available);
    
	printf("core_c1_supported : %d\n",cpu->core_c1_supported);
	printf("core_c2_supported : %d\n",cpu->core_c1_supported);
	printf("core_c3_supported : %d\n",cpu->core_c3_supported);
	printf("core_c6_supported : %d\n",cpu->core_c6_supported);
	printf("core_c7_supported : %d\n",cpu->core_c7_supported);
	printf("mwait_supported : %d\n",cpu->mwait_supported);
    
#if BUILD_ACPI_TSS || pstate_power_support
	if (is_sandybridge() || is_jaketown())
	{
        
		printf("package_power_limit : %d\n",cpu->package_power_limit);
		printf("package_power_sku_unit : %d\n",cpu->package_power_sku_unit); 
        
	}
#endif
	
	DBG("invariant_apic_timer_flag : %d\n",cpu->invariant_apic_timer_flag);
	
    
#endif
}

#if BETA
//-----------------------------------------------------------------------------
static U32 BuildPstateInfo(CPU_DETAILS * cpu)
{
	// Build P-state table info based on verified options
    
	// Compute the number of p-states based on the ratio range
	cpu->pkg_pstates.num_pstates = computeNumPstates(cpu->max_ratio_as_cfg, cpu->min_ratio, cpu->turbo_available, MAX_PSTATES);
	
	if (!cpu->pkg_pstates.num_pstates)
	{
		return (0);
	}
	
	// Compute pstate data
	{
		U32 TDP = compute_tdp(cpu);
		
		U32 index;
		for (index=0; index < cpu->pkg_pstates.num_pstates; index ++)
		{
			PSTATE * pstate = &cpu->pkg_pstates.pstate[index];
			
			// Set ratio
			pstate->ratio = computePstateRatio(cpu->max_ratio_as_cfg, cpu->min_ratio, cpu->turbo_available, cpu->pkg_pstates.num_pstates, index);
			
			// Compute frequency based on ratio
			if ((index != 0) || (cpu->turbo_available == 0))
				pstate->frequency = pstate->ratio * get_bclk();
			else
				pstate->frequency = ((pstate->ratio - 1) * get_bclk()) + 1;
			
			// Compute power based on ratio and other data
			if (pstate->ratio >= cpu->max_ratio_as_mfg)
				// Use max power in mW
				pstate->power = TDP * 1000;
			else
			{
				pstate->power = compute_pstate_power(cpu, pstate->ratio, TDP);
				
				// Convert to mW
				pstate->power*= 1000;
			}
		}
	}		
    
	return (1);
}
#else
/*
 //-----------------------------------------------------------------------------
 static U32 BuildCoreIPstateInfo(CPU_DETAILS * cpu)
 {
 // Build P-state table info based on verified options
 
 // Compute the number of p-states based on the ratio range
 cpu->pkg_pstates.num_pstates = computeNumPstates(cpu->max_ratio_as_cfg, cpu->min_ratio, cpu->turbo_available, MAX_PSTATES);
 
 if (!cpu->pkg_pstates.num_pstates)
 {
 return (0);
 }
 
 // Compute pstate data
 {
 #ifdef pstate_power_support
 U32 TDP = compute_tdp(cpu);								
 #endif
 
 U32 index;
 for (index=0; index < cpu->pkg_pstates.num_pstates; index ++)
 {
 PSTATE * pstate = &cpu->pkg_pstates.pstate[index];
 
 // Set ratio
 pstate->ratio = computePstateRatio(cpu->max_ratio_as_cfg, cpu->min_ratio, cpu->turbo_available, cpu->pkg_pstates.num_pstates, index);
 
 // Compute frequency based on ratio
 if ((index != 0) || (cpu->turbo_available == 0))
 pstate->frequency = pstate->ratio * get_bclk();
 else
 pstate->frequency = ((pstate->ratio - 1) * get_bclk()) + 1;
 
 #ifdef pstate_power_support
 // Compute power based on ratio and other data
 if (pstate->ratio >= cpu->max_ratio_as_mfg)
 // Use max power in mW
 pstate->power = TDP * 1000;
 else
 {
 pstate->power = compute_pstate_power(cpu, pstate->ratio, TDP);
 
 // Convert to mW
 pstate->power*= 1000;
 }
 #else
 pstate->power = 0;
 #endif	
 }
 }		
 
 return (1);
 }
 */
//-----------------------------------------------------------------------------
static U32 BuildPstateInfo(CPU_DETAILS * cpu)
{	
    
	struct p_state p_states[32];
	U8 p_states_count = 0;	
	
    if (!cpu)
    {
        return (0);
    }
    
	{
#if UNUSED
		struct p_state initial;
#endif	
		struct p_state maximum, minimum;
		// Retrieving P-States, ported from code by superhai (c)
		switch (Platform.CPU.Family)
		{
			case 0x06: 
			{
				switch (Platform.CPU.Model) 
				{
					case CPU_MODEL_DOTHAN: 
					case CPU_MODEL_YONAH: // Yonah
					case CPU_MODEL_MEROM: // Merom
					case CPU_MODEL_PENRYN: // Penryn
					case CPU_MODEL_ATOM: // Intel Atom (45nm)
					{
						bool cpu_dynamic_fsb = false;
						
						if (rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 27)) 
						{
							wrmsr64(MSR_IA32_EXT_CONFIG, (rdmsr64(MSR_IA32_EXT_CONFIG) | (1 << 28))); 
							delay(1);
							cpu_dynamic_fsb = rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 28);
						}
						
						bool cpu_noninteger_bus_ratio = (rdmsr64(MSR_IA32_PERF_STATUS) & (1ULL << 46));
#if UNUSED
						//initial.Control = rdmsr64(MSR_IA32_PERF_STATUS);
#endif			
						maximum.Control = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 32) & 0x1F3F) | (0x4000 * cpu_noninteger_bus_ratio);
						maximum.CID = ((maximum.FID & 0x1F) << 1) | cpu_noninteger_bus_ratio;
						
						minimum.FID = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 24) & 0x1F) | (0x80 * cpu_dynamic_fsb);
						minimum.VID = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 48) & 0x3F);
						
						if (minimum.FID == 0) 
						{
							U64 msr;
							U8 i;
							// Probe for lowest fid
							for (i = maximum.FID; i >= 0x6; i--) 
							{
								msr = rdmsr64(MSR_IA32_PERF_CONTROL);
								wrmsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (i << 8) | minimum.VID);
								intel_waitforsts();
								minimum.FID = (rdmsr64(MSR_IA32_PERF_STATUS) >> 8) & 0x1F; 
								delay(1);
							}
							
							msr = rdmsr64(MSR_IA32_PERF_CONTROL);
							wrmsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (maximum.FID << 8) | maximum.VID);
							intel_waitforsts();
						}
                        
						if (minimum.VID == maximum.VID) 
						{	
							U64 msr;
							U8 i;
							// Probe for lowest vid
							for (i = maximum.VID; i > 0xA; i--) 
							{
								msr = rdmsr64(MSR_IA32_PERF_CONTROL);
								wrmsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (minimum.FID << 8) | i);
								intel_waitforsts();
								minimum.VID = rdmsr64(MSR_IA32_PERF_STATUS) & 0x3F; 
								delay(1);
							}
							
							msr = rdmsr64(MSR_IA32_PERF_CONTROL);
							wrmsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (maximum.FID << 8) | maximum.VID);
							intel_waitforsts();
						}
                        
						minimum.CID = ((minimum.FID & 0x1F) << 1) >> cpu_dynamic_fsb;
						
						// Sanity check
						if (maximum.CID < minimum.CID) 
						{
							DBG("Insane FID values!");
							p_states_count = 0;
						}
						else
						{
							// Finalize P-States
							// Find how many P-States machine supports
							p_states_count = maximum.CID - minimum.CID + 1;
							
							if (p_states_count > MAX_PSTATES) // was 32
								p_states_count = MAX_PSTATES; // was 32
							
							U8 vidstep;
							U8 i = 0, u, invalid = 0;
							
							vidstep = ((maximum.VID << 2) - (minimum.VID << 2)) / (p_states_count - 1);
							
                            U32 fsb = (U32)divU64byU64(Platform.CPU.FSBFrequency , 1000000 , NULL); 
                            
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
								
								U32 multiplier = p_states[i].FID & 0x1f;		// = 0x08
								bool half = p_states[i].FID & 0x40;					// = 0x01
								bool dfsb = p_states[i].FID & 0x80;					// = 0x00
								//U32 fsb = (U32)get_env(envFSBFreq) / 1000000; // = 400
								U32 halffsb = (fsb + 1) >> 1;					// = 200
								U32 frequency = (multiplier * fsb);			// = 3200
								
								p_states[i].Frequency = (frequency + (half * halffsb)) >> dfsb;	// = 3200 + 200 = 3400
							}
							
							p_states_count -= invalid;
						}
						break;
					} 
					case CPU_MODEL_FIELDS:
					case CPU_MODEL_DALES:
					case CPU_MODEL_DALES_32NM:
					case CPU_MODEL_NEHALEM: 
					case CPU_MODEL_NEHALEM_EX:
					case CPU_MODEL_WESTMERE:
					case CPU_MODEL_WESTMERE_EX:
					case CPU_MODEL_SANDYBRIDGE:
					case CPU_MODEL_JAKETOWN:
					{		
						
						maximum.Control = rdmsr64(MSR_IA32_PERF_STATUS) & 0xff; // Seems it always contains maximum multiplier value (with turbo, that's we need)...
						minimum.Control = (rdmsr64(MSR_PLATFORM_INFO) >> 40) & 0xff;
						
						DBG("P-States: min 0x%x, max 0x%x\n", minimum.Control, maximum.Control);
						
						// Sanity check
						if (maximum.Control < minimum.Control) 
						{
							DBG("Insane control values!");
							p_states_count = 0;
						}
						else
						{
							U8 i;
							p_states_count = 0;
							U32 fsb = (U32)divU64byU64(Platform.CPU.FSBFrequency , 1000000, NULL) ;
							for (i = maximum.Control; i >= minimum.Control; i--) 
							{
								p_states[p_states_count].Control = i;
								p_states[p_states_count].CID = p_states[p_states_count].Control << 1;
								p_states[p_states_count].Frequency = (U32)fsb * i;
								p_states_count++;
								if (p_states_count >= MAX_PSTATES) { // was 32
									
									if (p_states_count > MAX_PSTATES) // was 32
										p_states_count = MAX_PSTATES; // was 32
									
									break;
								}
							}
						}
                        
						/*
                         U32 sta = BuildCoreIPstateInfo(cpu);
                         if (sta) 
                         {
                         DBG("_PSS PGK generated successfully\n");
                         return (1);
                         
                         }
                         else
                         {
                         verbose("CoreI _PSS Generation failed !!\n");
                         return (0);
                         }
                         */
						break;
					}
					default:
						verbose ("Unsupported CPU: P-States will not be generated !!!\n");
						return (0);
						break;
				}
			}
			default:				
				break;
		}
	}
	
	// Generating Pstate PKG
	if (p_states_count > 0) 
	{									
        U32 fsb = (U32)Platform.CPU.FSBFrequency;
		U8 minPSratio = divU64byU64(p_states[p_states_count-1].Frequency , divU64byU64(fsb , 10000000 , NULL ) , NULL);
		U8 maxPSratio = divU64byU64(p_states[0].Frequency , divU64byU64(fsb , 10000000 , NULL ) , NULL);
		U8 cpu_ratio = 0;
		
		{
			U8 cpu_div = (U8)Platform.CPU.CurrDiv;
			U8 cpu_coef = (U8)Platform.CPU.CurrCoef;
            
			if (cpu_div) 								
				cpu_ratio = (cpu_coef * 10) + 5;								
			else 								
				cpu_ratio = cpu_coef * 10;
		}
		
		
		{
			int user_max_ratio = 0;
			getIntForKey(kMaxRatio, &user_max_ratio, &bootInfo->chameleonConfig);
			if (user_max_ratio >= minPSratio && maxPSratio >= user_max_ratio)
			{									
				
				U8 maxcurrdiv = 0, maxcurrcoef = (int)divU64byU64(user_max_ratio , 10, NULL);									
				
				U8 maxdiv = user_max_ratio - (maxcurrcoef * 10);
				if (maxdiv > 0)
					maxcurrdiv = 1;
				
				if (maxcurrdiv) 									
					cpu_ratio = (maxcurrcoef * 10) + 5;									
				else 									
					cpu_ratio = maxcurrcoef * 10;																
			}
		}
		
		{
			int user_min_ratio = 0;
			getIntForKey(kMinRatio, &user_min_ratio, &bootInfo->chameleonConfig);
			if (user_min_ratio >= minPSratio && cpu_ratio >= user_min_ratio)
			{
				
				U8 mincurrdiv = 0, mincurrcoef = (int)divU64byU64(user_min_ratio , 10 , NULL);									
				
				U8 mindiv = user_min_ratio - (mincurrcoef * 10);
				
				if (mindiv > 0)
					mincurrdiv = 1;									
				
				if (mincurrdiv) 									
					minPSratio = (mincurrcoef * 10) + 5;									
				else 									
					minPSratio = mincurrcoef * 10;																		
				
			}
		}
		
		
		if (maxPSratio >= cpu_ratio && cpu_ratio >= minPSratio)	maxPSratio = cpu_ratio;													
		
		{
			int  base = 16;								
			U8 expert = 0; /* Default: 0 , mean mixed mode 
						    * expert mode : 1 , mean add only p-states found in boot.plist
						    */
            
            TagPtr PstateTag;                           
            U32 pstate_tag_count = 0;
            
			{
                
                
                if (bootInfo->chameleonConfig.dictionary) 
                {
                    PstateTag = XMLCastDict(XMLGetProperty(bootInfo->chameleonConfig.dictionary, (const char*)"P-States"));
                    if (PstateTag) pstate_tag_count = XMLTagCount(PstateTag) ;
                }
                
                if (!pstate_tag_count) 
                    if ((PstateTag = XMLCastDict(XMLGetProperty(bootInfo->chameleonConfig.dictionary, (const char*)"P-States")))) pstate_tag_count = XMLTagCount(PstateTag);                    
                
                
				if ((pstate_tag_count > 0) && PstateTag)
				{
					char *tmpstr = XMLCastString(XMLGetProperty(PstateTag, (const char*)"Mode"));
					
					if (strcmp(tmpstr,"Expert") == 0)
					{
						p_states_count = pstate_tag_count - 1 ; // - 1 = - ("Mode" tag) 										
						expert = 1;
					}
					
					
					if ((tmpstr = XMLCastString(XMLGetProperty(PstateTag, (const char*)"Base"))))
					{
						
						if (expert) p_states_count--; // -=  ("Base" tag) 
						
						int mybase = strtol(tmpstr, NULL, 10);	
						
						if (mybase == 8 || mybase == 10 || mybase == 16 )
							base = mybase;									
					}
				}
				
			}
			
			{
				U32 dropPSS = 0, Pstatus = 0;
				char MatchStat[5];
#ifdef pstate_power_support
				U32 TDP = compute_tdp(cpu);								
#endif
				U32 i;	
                U32 fsb = (U32)Platform.CPU.FSBFrequency;
				for (i = 0; i < p_states_count; i++) 
				{			
					char *Lat1 = NULL, *clk = NULL, *Pw = NULL, *Lat2 = NULL, *Ctrl = NULL ;
					
					if ((pstate_tag_count > 0) && PstateTag)
					{
						sprintf(MatchStat, "%d",i);
						TagPtr match_Status = XMLGetProperty(PstateTag, (const char*)MatchStat); 								   
						
						if (match_Status  && (XMLTagCount(match_Status) > 0))
						{												
							
							clk  = XMLCastString(XMLGetProperty(match_Status, (const char*)"CoreFreq"));
							Pw   = XMLCastString(XMLGetProperty(match_Status, (const char*)"Power"));
							Lat1 = XMLCastString(XMLGetProperty(match_Status, (const char*)"Transition Latency"));
							Lat2 = XMLCastString(XMLGetProperty(match_Status, (const char*)"Bus Master Latency"));
							Ctrl = XMLCastString(XMLGetProperty(match_Status, (const char*)"Control"));
							
							
						} else if (expert) 
							continue;
					}
                    
					unsigned long Frequency  = 0x00000000;
					
					if (!expert || !pstate_tag_count) Frequency  = p_states[i].Frequency;
					
					if (clk) 
						Frequency  = strtoul((const char *)clk, NULL,base);
					
					if (!Frequency || Frequency > p_states[0].Frequency ) continue;
					
					U8 curr_ratio = (U8)divU64byU64(Frequency , divU64byU64(fsb , 10000000, NULL ), NULL);
                    
                    
                    {
						U8 fixed_ratio = (U8)divU64byU64(Frequency , divU64byU64(fsb , 1000000 , NULL ) , NULL) * 10;
						U8 diff = curr_ratio - fixed_ratio ;
						
						if (diff)
						{
							if (diff < 5)
							{
								curr_ratio = fixed_ratio;
							} 
							else
							{
								curr_ratio = fixed_ratio + 5;
							}
						}						
                        
					}
					
					if (curr_ratio > maxPSratio || minPSratio > curr_ratio)
						goto dropPstate;
                    
					{
						PSTATE * pstate = &cpu->pkg_pstates.pstate[Pstatus];
						
						pstate->ratio = curr_ratio; 
						
						pstate->frequency  = Frequency; // CoreFreq (in MHz).	
						
						U32 power = 0x00000000;
#ifdef pstate_power_support
						// Compute power based on ratio and other data
						if (pstate->ratio >= cpu->max_ratio_as_mfg)
							// Use max power in mW
							power = TDP * 1000;
						else
						{
							power = compute_pstate_power(cpu, pstate->ratio, TDP);
							
							// Convert to mW
							power*= 1000;
						}
#endif						
						pstate->power = resolve_pss(power, Pw, base); // Power (in milliWatts)									
						pstate->translatency = resolve_pss(0x0000000A, Lat1, base); // Transition Latency (in microseconds).									
						pstate->bmlatency = resolve_pss(0x0000000A, Lat2, base); // Bus Master Latency (in microseconds).									
						
						{
							U32 Control  = 0 /*encode_pstate(curr_ratio)*/ ;
							if (!expert || !pstate_tag_count) Control = p_states[i].Control;									
							pstate->control = resolve_pss(Control, Ctrl, base); // Control
						}														
						
						pstate->status = Pstatus+1; // Status
						
						DBG("state :: frequency :%d power: %d translatency: %d bmlatency: %d control: %d status: %d ratio :%d :: registred !! \n",pstate->frequency,pstate->power,
                            pstate->translatency,pstate->bmlatency,pstate->control,pstate->status,pstate->ratio );
					}
					
					
					Pstatus++;
					continue;															
                    
				dropPstate:					
					DBG("state with cpu frequency :%d and ratio :%d will be dropped\n",p_states[i].Frequency,curr_ratio);					
					dropPSS++;
					
					
				}
				
				if (Pstatus == 0)
				{
					verbose("No suitable P-states found, P-States will not be generated !!!\n");
					return (0);
				}
				cpu->pkg_pstates.num_pstates = Pstatus;
			}
		}			
	}	
	else 
	{
		verbose("ACPI CPUs not found: P-States will not be generated !!!\n");
		return (0);
	}
	
	DBG("_PSS PGK generated successfully\n");
	return (1);
}
#endif // BETA

//-----------------------------------------------------------------------------
static U32 BuildCstateInfo(CPU_DETAILS * cpu, U32 pmbase)
{
	{ 
        
		TagPtr CstateTag = NULL;
        U32 entry_count = 0;
        
        if (bootInfo->chameleonConfig.dictionary) 
        {
            CstateTag = XMLCastDict(XMLGetProperty(bootInfo->chameleonConfig.dictionary, (const char*)"C-States"));
        }       
        
		if (CstateTag)
		{
			int  base = 16;	
			
			entry_count = XMLTagCount(CstateTag);			
			
			if (entry_count > 0)
			{	
                {
                    char *tmpstr;
                    
                    if ((tmpstr = XMLCastString(XMLGetProperty(CstateTag, (const char*)"Base"))))
                    {
                        
                        entry_count--; // -=  ("Base" tag) 
                        
                        int mybase = strtol(tmpstr, NULL, 10);	
                        
                        if (mybase == 8 || mybase == 10 || mybase == 16 )
                            base = mybase;									
                    }
                }				
				
				cpu->pkg_io_cstates.num_cstates = 0;
				cpu->pkg_mwait_cstates.num_cstates = 0;
				U32 num_cstates = 0;
				
				{
					U32 i;
					char MatchStat[5];					
					
					for (i = 0; i < 32 ; i++)
					{
						char *Lat = NULL, *Pw = NULL, *BWidth= NULL, *BOffset= NULL, *Address= NULL, *AccessSize= NULL, *index= NULL;
						
						sprintf(MatchStat, "C%d",i);
						TagPtr match_Status = XMLGetProperty(CstateTag, (const char*)MatchStat);
						if (match_Status)
						{	
							Pw   = XMLCastString(XMLGetProperty(match_Status, (const char*)"Power"));
							Lat  = XMLCastString(XMLGetProperty(match_Status, (const char*)"Latency"));
							BWidth= XMLCastString(XMLGetProperty(match_Status, (const char*)"BitWidth"));
							
							BOffset = XMLCastString(XMLGetProperty(match_Status, (const char*)"BitOffset"));
							Address = XMLCastString(XMLGetProperty(match_Status, (const char*)"Latency"));
							AccessSize = XMLCastString(XMLGetProperty(match_Status, (const char*)"AccessSize"));
							index = XMLCastString(XMLGetProperty(match_Status, (const char*)"index"));
							
							if (Pw && Lat && BWidth && BOffset && Address && AccessSize && index)
							{
								U32 bw		= strtoul((const char *)BWidth, NULL,base);
								U32 boff	= strtoul((const char *)BOffset, NULL,base);
								U32 acs		= strtoul((const char *)AccessSize, NULL,base);
								U32 addr	= strtoul((const char *)Address, NULL,base);
								U32 idx		= strtoul((const char *)index, NULL,base);
								U32 lat		= strtoul((const char *)Lat, NULL,base);
								U32 pw		= strtoul((const char *)Pw, NULL,base);
								
								ACPI_GENERIC_ADDRESS mwait_gas  = {GAS_TYPE_FFH,bw,boff,acs,addr};  
								ACPI_GENERIC_ADDRESS io_gas = {(i == 1) ? GAS_TYPE_FFH : GAS_TYPE_SYSTEM_IO,bw,boff,acs,addr};  
								
								CSTATE mwait_cstate = {idx,lat,pw};
								CSTATE io_cstate = {idx,lat,pw};
								
								{
									cpu->pkg_mwait_cstates.cstate[cpu->pkg_mwait_cstates.num_cstates] = mwait_cstate;
									cpu->pkg_mwait_cstates.gas[cpu->pkg_mwait_cstates.num_cstates] = mwait_gas;
									cpu->pkg_mwait_cstates.num_cstates++;
								}
								
								{
									cpu->pkg_io_cstates.cstate[cpu->pkg_io_cstates.num_cstates] = io_cstate;
									cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates] = io_gas;
									cpu->pkg_io_cstates.num_cstates++;
								}
								num_cstates++;								
								
								if (num_cstates >= MAX_CSTATES)
								{
									break;
								}
							}
						} 
					}
				}
				
				if (num_cstates)
				{
					return (1);
				}				
			}
		}	
	}
    
	{
		static const ACPI_GENERIC_ADDRESS mwait_gas[] = {
			{GAS_TYPE_FFH,1,2,1,0x00},   // processor C1
			{GAS_TYPE_FFH,1,2,1,0x10},   // processor C3 as ACPI C2
			{GAS_TYPE_FFH,1,2,1,0x10},   // processor C3 as ACPI C3
			{GAS_TYPE_FFH,1,2,1,0x10},   // processor C3 as ACPI C4
			{GAS_TYPE_FFH,1,2,1,0x20},   // processor C6
			{GAS_TYPE_FFH,1,2,1,0x30},   // processor C7
		};
		
		static const ACPI_GENERIC_ADDRESS io_gas[] = {
			{GAS_TYPE_FFH,      0,0,0,0x00},   // processor C1
			{GAS_TYPE_SYSTEM_IO,8,0,0,0x14},   // processor C3 as ACPI C2 or processor C2
			{GAS_TYPE_SYSTEM_IO,8,0,0,0x14},   // processor C3 as ACPI C3
			{GAS_TYPE_SYSTEM_IO,8,0,0,0x15},   // processor C4 as ACPI C4
			{GAS_TYPE_SYSTEM_IO,8,0,0,0x15},   // processor C6
			{GAS_TYPE_SYSTEM_IO,8,0,0,0x16},   // processor C7
		};
		
		static const CSTATE mwait_cstate [] = {
			{1,0x01,0x3e8},      // processor C1
			{2,0x40,0x1f4},      // processor C3 as ACPI C2 or processor C2
			{3,0x40,0x1f4},      // processor C3 as ACPI C3
			{4,0x40,0x1f4},      // processor C4 
			{6/*was 3*/,0x60,0x15e},      // processor C6
			{7/*was 3*/,0x60,0x0c8},      // processor C7
		};
		
		static const CSTATE io_cstate [] = {
			{1,0x01,0x3e8},      // processor C1
			{2,0x40,0x1f4},      // processor C3 as ACPI C2 or processor C2
			{3,0x40,0x1f4},      // processor C3 as ACPI C3
			{4,0x40,0x1f4},      // processor C4 
			{6/*was 3*/,0x60,0x15e},      // processor C6
			{7/*was 3*/,0x60,0x0c8},      // processor C7
		};
		
		static const U32 cstate_2_index [] = {0,0,0,1,2,3,4,5};
		
		// Build C-state table info based on verified options
		
		// Desired state for the processor core C3 state included in the _CST as an
		// ACPI C2 state.
		// 1= processor core C3 can be used as an ACPI C2 state
		// 0= processor core C3 cannot be used as an ACPI C2 state
		int c2_enabled = 0;
		
		// Desired state for the processor core C3 state included in the _CST
		// 0= processor core C3 cannot be used as an ACPI C state
		// 2= processor core C3 can be used as an ACPI C2 state
		// 3= processor core C3 can be used as an ACPI C3 state
		// 4= processor core C3 can be used as an ACPI C2 state
		//    if Invariant APIC Timer detected, else not used as ACPI C state
		// 5= processor core C3 can be used as an ACPI C2 state
		//    if Invariant APIC Timer detected, else APIC C3 state
		// 6= processor core C3 can be used as an ACPI C4 state
		int c3_enabled = 3;
		
		// Desired state for the processor core C3 state included in the _CST as an
		// ACPI C4 state.
		// 1= processor core C3 can be used as an ACPI C4 state
		// 0= processor core C3 cannot be used as an ACPI C4 state
		int c4_enabled = 0;
		
		// Desired state for the processor core C6 state included in the _CST as an
		// ACPI C3 state.
		// 1= processor core C6 can be used as an ACPI C3 state
		// 0= processor core C6 cannot be used as an ACPI C3 state
		int c6_enabled = 0;
		
		// Desired state for the processor core C7 state included in the _CST as an
		// ACPI C3 state.
		// 1= processor core C7 can be used as an ACPI C7 state
		// 0= processor core C7 cannot be used as an ACPI C7 state
		int c7_enabled = 0;
		
		{		
			bool tmpval;
			
			
			if (getBoolForKey(kEnableC2State, &tmpval, &bootInfo->chameleonConfig))
			{
				c2_enabled = tmpval;
			}
            
			if (!getIntForKey("C3StateOption", &c3_enabled, &bootInfo->chameleonConfig))
			{
				c3_enabled = (getBoolForKey(kEnableC3State, &tmpval, &bootInfo->chameleonConfig)&&tmpval) ? 3 : 0;
			}		
			if (c3_enabled == 6)
			{
				c4_enabled = 1;
			}
			else 
			{
				c4_enabled = (getBoolForKey(kEnableC4State, &tmpval, &bootInfo->chameleonConfig)&&tmpval) ? 1 : 0;
			}
			c6_enabled = (getBoolForKey(kEnableC6State, &tmpval, &bootInfo->chameleonConfig)&&tmpval) ? 1 : 0;
			c7_enabled = (getBoolForKey(kEnableC7State, &tmpval, &bootInfo->chameleonConfig)&&tmpval) ? 1 : 0;		
		}
		
		cpu->pkg_mwait_cstates.num_cstates = 0;
		{
			{
				cpu->pkg_mwait_cstates.cstate[cpu->pkg_mwait_cstates.num_cstates] = mwait_cstate[cstate_2_index[CPU_C1]];
				cpu->pkg_mwait_cstates.gas[cpu->pkg_mwait_cstates.num_cstates] = mwait_gas[cstate_2_index[CPU_C1]];
				cpu->pkg_mwait_cstates.num_cstates++;
			}
			if (((cpu->core_c3_supported || cpu->core_c2_supported) && (c2_enabled)) && ((c3_enabled == 2) ||
                                                                                         ((c3_enabled == 4) && cpu->invariant_apic_timer_flag)))
			{
				cpu->pkg_mwait_cstates.cstate[cpu->pkg_mwait_cstates.num_cstates] = mwait_cstate[cstate_2_index[CPU_C3_ACPI_C2]];
				cpu->pkg_mwait_cstates.gas[cpu->pkg_mwait_cstates.num_cstates] = mwait_gas[cstate_2_index[CPU_C3_ACPI_C2]];
				cpu->pkg_mwait_cstates.num_cstates++;
			}
			if (cpu->core_c4_supported && c4_enabled)
			{
				
				cpu->pkg_mwait_cstates.cstate[cpu->pkg_mwait_cstates.num_cstates] = mwait_cstate[cstate_2_index[CPU_C4]];
				cpu->pkg_mwait_cstates.gas[cpu->pkg_mwait_cstates.num_cstates] = mwait_gas[cstate_2_index[CPU_C4]];
				cpu->pkg_mwait_cstates.num_cstates++;
				
			}
			else
			{
				
				if (cpu->core_c3_supported && ((c3_enabled == 3) ||
											   ((c3_enabled == 4) && !cpu->invariant_apic_timer_flag)))
				{
					cpu->pkg_mwait_cstates.cstate[cpu->pkg_mwait_cstates.num_cstates] = mwait_cstate[cstate_2_index[CPU_C3_ACPI_C3]];
					cpu->pkg_mwait_cstates.gas[cpu->pkg_mwait_cstates.num_cstates] = mwait_gas[cstate_2_index[CPU_C3_ACPI_C3]];
					cpu->pkg_mwait_cstates.num_cstates++;
				}
			}
			
			
			if (cpu->core_c6_supported && c6_enabled)
			{
				cpu->pkg_mwait_cstates.cstate[cpu->pkg_mwait_cstates.num_cstates] = mwait_cstate[cstate_2_index[CPU_C6]];
				cpu->pkg_mwait_cstates.gas[cpu->pkg_mwait_cstates.num_cstates] = mwait_gas[cstate_2_index[CPU_C6]];
				cpu->pkg_mwait_cstates.num_cstates++;
			}
			if (cpu->core_c7_supported && c7_enabled)
			{
				cpu->pkg_mwait_cstates.cstate[cpu->pkg_mwait_cstates.num_cstates] = mwait_cstate[cstate_2_index[CPU_C7]];
				cpu->pkg_mwait_cstates.gas[cpu->pkg_mwait_cstates.num_cstates] = mwait_gas[cstate_2_index[CPU_C7]];
				cpu->pkg_mwait_cstates.num_cstates++;
			}
		}
		
		cpu->pkg_io_cstates.num_cstates = 0;
		{
			{
				cpu->pkg_io_cstates.cstate[cpu->pkg_io_cstates.num_cstates] = io_cstate[cstate_2_index[CPU_C1]];
				cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates] = io_gas[cstate_2_index[CPU_C1]];
				cpu->pkg_io_cstates.num_cstates++;
			}
			if ((cpu->core_c3_supported || cpu->core_c2_supported) && (c2_enabled || (c3_enabled == 2) ||
                                                                       ((c3_enabled == 4) && cpu->invariant_apic_timer_flag)))
			{
				cpu->pkg_io_cstates.cstate[cpu->pkg_io_cstates.num_cstates] = io_cstate[cstate_2_index[CPU_C3_ACPI_C2]];
				cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates] = io_gas[cstate_2_index[CPU_C3_ACPI_C2]];
				cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates].Address += pmbase;
				cpu->pkg_io_cstates.num_cstates++;
			}
			if (cpu->core_c4_supported && c4_enabled)
			{
				
				cpu->pkg_io_cstates.cstate[cpu->pkg_io_cstates.num_cstates] = io_cstate[cstate_2_index[CPU_C4]];
				cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates] = io_gas[cstate_2_index[CPU_C4]];
				cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates].Address += pmbase;
				cpu->pkg_io_cstates.num_cstates++;	
				
			}
			else 
			{
				
				if (cpu->core_c3_supported && ((c3_enabled == 3) ||
											   ((c3_enabled == 4) && !cpu->invariant_apic_timer_flag)))
				{
					cpu->pkg_io_cstates.cstate[cpu->pkg_io_cstates.num_cstates] = io_cstate[cstate_2_index[CPU_C3_ACPI_C3]];
					cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates] = io_gas[cstate_2_index[CPU_C3_ACPI_C3]];
					cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates].Address += pmbase;
					cpu->pkg_io_cstates.num_cstates++;
				}
			}
			
			if (cpu->core_c6_supported && c6_enabled)
			{
				cpu->pkg_io_cstates.cstate[cpu->pkg_io_cstates.num_cstates] = io_cstate[cstate_2_index[CPU_C6]];
				cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates] = io_gas[cstate_2_index[CPU_C6]];
				cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates].Address += pmbase;
				cpu->pkg_io_cstates.num_cstates++;
			}
			if (cpu->core_c7_supported && c7_enabled)
			{
				cpu->pkg_io_cstates.cstate[cpu->pkg_io_cstates.num_cstates] = io_cstate[cstate_2_index[CPU_C7]];
				cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates] = io_gas[cstate_2_index[CPU_C7]];
				cpu->pkg_io_cstates.gas[cpu->pkg_io_cstates.num_cstates].Address += pmbase;
				cpu->pkg_io_cstates.num_cstates++;
			}
		}
	}
	
	return (1);
}

#if BUILD_ACPI_TSS
//-----------------------------------------------------------------------------
static U32 BuildTstateInfo(CPU_DETAILS * cpu)
{
    // Coarse grained clock modulation is available if cpuid.6.eax[5] = 0
    // Max of 8 T-states using 12.5% increments
    static const TSTATE tstate_coarse_grain [] = {
        {100,0,0,0x00,0},
        { 88,0,0,0x1e,0},
        { 75,0,0,0x1c,0},
        { 63,0,0,0x1a,0},
        { 50,0,0,0x18,0},
        { 38,0,0,0x16,0},
        { 25,0,0,0x14,0},
        { 13,0,0,0x12,0},
    };
    
    // Fine grained clock modulation is available if cpuid.6.eax[5] = 1
    // Max of 15 T-states using 6.25% increments
    static const TSTATE tstate_fine_grain [] = {
        {100,0,0,0x00,0},
        { 94,0,0,0x1f,0},
        { 88,0,0,0x1e,0},
        { 81,0,0,0x1d,0},
        { 75,0,0,0x1c,0},
        { 69,0,0,0x1b,0},
        { 63,0,0,0x1a,0},
        { 56,0,0,0x19,0},
        { 50,0,0,0x18,0},
        { 44,0,0,0x17,0},
        { 38,0,0,0x16,0},
        { 31,0,0,0x15,0},
        { 25,0,0,0x14,0},
        { 19,0,0,0x13,0},
        { 13,0,0,0x12,0},
    };
    
    // Build T-state table info based on verified options
    U32 num_cpu;
    const TSTATE * tstate;
    U32 num_tstates;
    
    for (num_cpu = 0; num_cpu < cpu_map_count; num_cpu ++)
    {        
        // Check if fine or coarse grained clock modulation is available
        if (get_env(envFineGrainClockMod))
        {
            // Fine grain thermal throttling is available
            num_tstates = 15;
            tstate = tstate_fine_grain;
        }
        else
        {
            // Coarse grain thermal throttling is available
            num_tstates = 8;
            tstate = tstate_coarse_grain;
        }
        
        cpu->pkg_tstates.num_tstates = num_tstates;
        {
            U32 index;
            for (index = 0; index < num_tstates; index++)
            {
                cpu->pkg_tstates.tstate[index] = tstate[index];
                cpu->pkg_tstates.tstate[index].power = 1000 * (compute_tdp(cpu) * (num_tstates - index)) / num_tstates;
            }
        } 
        
        
    }
    return (1);
}
#endif // BUILD_ACPI_TSS

//-----------------------------------------------------------------------------
U32 ProcessMadt(ACPI_TABLE_MADT * madt, MADT_INFO * madt_info, void * buffer, U32 bufferSize, U32 nb_cpu)
{
    void *current;
    void *currentOut;
    void *end;
    void * endOut;
    
    U32 LOCAL_APIC_NMI_CNT = 0, LOCAL_SAPIC_CNT = 0, INT_SRC_CNT = 0,  Length = 0;
    
    // Quick sanity check for a valid MADT
    if (madt == 0ul || !nb_cpu)
        return (0);
    
    // Confirm a valid MADT buffer was provided
    if (!buffer)
    {
        printf("Error: Invalid Buffer Address for MADT\n");
        return(0);
    }
    
    // Confirm a valid MADT buffer length was provided
    if (!bufferSize)
    {
        printf("Error: Invalid Buffer Length for MADT\n");
        return(0);
    }    
    
    madt_info->lapic_count = 0;           
    
    memcpy(buffer, madt, sizeof(ACPI_TABLE_MADT));       
    
    // Search MADT for Sub-tables with needed data
    current = madt + 1;
    currentOut = buffer + sizeof(ACPI_TABLE_MADT) ;    
    
    end = (U8 *) madt + madt->Header.Length;
    endOut = (U8 *)buffer + bufferSize; 
    
    // Check to confirm no MADT buffer overflow
    if ( (U8 *)currentOut > (U8 *)endOut )
    {
        printf("Error: MADT Buffer Length exceeded available space \n");
        return(0);
    }
    
    Length += sizeof(ACPI_TABLE_MADT);    
    
    while (current < end)
	{
        ACPI_SUBTABLE_HEADER *subtable = current;
        ACPI_SUBTABLE_HEADER *subtableOut = currentOut;
        
        
        switch (subtable->Type)
		{
                
			case ACPI_MADT_TYPE_LOCAL_APIC:
            {
                
                // Process sub-tables with Type as 0: Processor Local APIC
                ACPI_MADT_LOCAL_APIC *lapic = current;
                current = lapic + 1;                                
                
                if (!(lapic->LapicFlags & ACPI_MADT_ENABLED))
                    continue;
                
                if (madt_info->lapic_count >= nb_cpu)
                    continue;
                
                // copy subtable
                {                           
                    
                    memcpy(currentOut, lapic, lapic->Header.Length);
                    
                    currentOut = currentOut + lapic->Header.Length;
                    
                    // Check to confirm no MADT buffer overflow
                    if ( (U8 *)currentOut > (U8 *)endOut )
                    {
                        printf("Error: MADT Buffer Length exceeded available space \n");
                        return(0);
                    }
                }
                
                {
                    LAPIC_INFO *lapic_info = &madt_info->lapic[madt_info->lapic_count];
                    
                    lapic_info->processorId = lapic->ProcessorId;
                    lapic_info->apicId = lapic->Id;
                    lapic_info->madt_type = ACPI_MADT_TYPE_LOCAL_APIC;
                }
                
                madt_info->lapic_count++;
                
                Length += lapic->Header.Length;
                
                // Sanity check to verify compile time limit for max logical CPU is not exceeded
                if (madt_info->lapic_count > MAX_LOGICAL_CPU)
                    return (0);
                
                break;
            }
                
			case ACPI_MADT_TYPE_X2APIC:
            {
                
                // Process sub-tables with Type as 9: Processor X2APIC
                ACPI_MADT_X2APIC *x2apic = current;
                current = x2apic + 1;                 
                
                if (!(x2apic->x2apicFlags & ACPI_MADT_ENABLED))
                    continue;
                
                if (madt_info->lapic_count >= nb_cpu)
                    continue;
                
                // copy subtable
                { 
                    memcpy(currentOut, x2apic, x2apic->Header.Length);
                    
                    currentOut = currentOut + x2apic->Header.Length;
                    
                    // Check to confirm no MADT buffer overflow
                    if ( (U8 *)currentOut > (U8 *)endOut )
                    {
                        printf("Error: MADT Buffer Length exceeded available space \n");
                        return(0);
                    }
                    
                }
                
                {
                    LAPIC_INFO *lapic_info = &madt_info->lapic[madt_info->lapic_count];
                    
                    lapic_info->uid = x2apic->UID;
                    lapic_info->apicId = x2apic->x2apicId;
                    lapic_info->madt_type = ACPI_MADT_TYPE_X2APIC;
                }
                
                madt_info->lapic_count++;
                
                Length += x2apic->Header.Length;
                
                // Sanity check to verify compile time limit for max logical CPU is not exceeded
                if (madt_info->lapic_count > MAX_LOGICAL_CPU)
                    return (0);
                
                break;
            }
                
            case ACPI_MADT_TYPE_LOCAL_APIC_NMI:
            {
                // Process sub-tables with Type as 4: Local APIC NMI
                ACPI_MADT_LOCAL_APIC_NMI *nmi = current;
                current = nmi + 1;  
                /*
				 if (!(nmi->IntiFlags & ACPI_MADT_ENABLED))
				 continue;
				 */
                if (LOCAL_APIC_NMI_CNT >= nb_cpu)
                    continue;                
                
                memcpy(currentOut, nmi, nmi->Header.Length);   
                
                currentOut = currentOut + nmi->Header.Length;
                
                // Check to confirm no MADT buffer overflow
                if ( (U8 *)currentOut > (U8 *)endOut )
                {
                    printf("Error: MADT Buffer Length exceeded available space \n");
                    return(0);
                }
                
                
                LOCAL_APIC_NMI_CNT++;
                
                Length += nmi->Header.Length;
                
                // Sanity check to verify compile time limit for max logical CPU is not exceeded
                if (LOCAL_APIC_NMI_CNT > MAX_LOGICAL_CPU)
                    return (0);
                
                break;
            }
                
            case ACPI_MADT_TYPE_LOCAL_SAPIC:
            {
                // Process sub-tables with Type as 7: Local Sapic
                ACPI_MADT_LOCAL_SAPIC *sapic = current;
                current = sapic + 1;               
                /*
				 if (!(sapic->LapicFlags & ACPI_MADT_ENABLED))
				 continue;
				 */
                if (LOCAL_SAPIC_CNT >= nb_cpu)
                    continue;                
                
                memcpy(currentOut, sapic, sapic->Header.Length);   
                
                currentOut = currentOut + sapic->Header.Length;
                
                // Check to confirm no MADT buffer overflow
                if ( (U8 *)currentOut > (U8 *)endOut )
                {
                    printf("Error: MADT Buffer Length exceeded available space \n");
                    return(0);
                }
                
                
                LOCAL_SAPIC_CNT++;
                
                Length += sapic->Header.Length;
                
                // Sanity check to verify compile time limit for max logical CPU is not exceeded
                if (LOCAL_SAPIC_CNT > MAX_LOGICAL_CPU)
                    return (0);
                
                break;
            }
                
            case ACPI_MADT_TYPE_INTERRUPT_SOURCE:
            {
                // Process sub-tables with Type as 8: Platform Interrupt Source
                ACPI_MADT_INTERRUPT_SOURCE *intsrc = current;
                current = intsrc + 1;               
                /*
				 if (!(intsrc->IntiFlags & ACPI_MADT_ENABLED))
				 continue;
				 */
                if (INT_SRC_CNT >= nb_cpu)
                    continue;                
                
                memcpy(currentOut, intsrc, intsrc->Header.Length);   
                
                currentOut = currentOut + intsrc->Header.Length;
                
                // Check to confirm no MADT buffer overflow
                if ( (U8 *)currentOut > (U8 *)endOut )
                {
                    printf("Error: MADT Buffer Length exceeded available space \n");
                    return(0);
                }
                
                
                INT_SRC_CNT++;
                
                Length += intsrc->Header.Length;
                
                // Sanity check to verify compile time limit for max logical CPU is not exceeded
                if (INT_SRC_CNT > MAX_LOGICAL_CPU)
                    return (0);
                
                break;
            }
                
			default:
            {
                
                // Process all other sub-tables
                current = (U8 *) subtable + subtable->Length;
                currentOut = (U8 *) subtableOut + subtable->Length;
                
                memcpy(subtableOut, subtable, subtable->Length);
                
                // Check to confirm no MADT buffer overflow
                if ( (U8 *)currentOut > (U8 *)endOut )
                {
                    printf("Error: MADT Buffer Length exceeded available space \n");
                    return(0);
                }                       
                
                Length += subtable->Length;
                
                break;
            }
        } // switch
        
    } // while  
    
    {
        ACPI_TABLE_MADT * new_madt = (ACPI_TABLE_MADT * )buffer;       
        
        // Update the Lenght of the new MADT table
        new_madt->Header.Length = Length;       
        
        // Update the checksum of the new MADT table
		SetChecksum(&new_madt->Header);       
    }
    
    return (1);
}

static U32 buildMADT(U32 * new_table_list, ACPI_TABLE_DSDT *dsdt, MADT_INFO * madt_info)
{
    DBG("Build MADT\n");
	
	ACPI_TABLE_MADT * madt_file = (void*)0ul;
	ACPI_TABLE_MADT * MadtPointer = (void*)0ul;
    bool oem_apic=false;
    U8 new_table_index = 0;
    
    // Check that we have a valid cpu_map (if it's not already done, it will try to generate it)
    if (generate_cpu_map_from_acpi(dsdt) != 0)
    {
        return(0);
    }
    
    {		
		bool tmpval;		
		oem_apic=getBoolForKey(kOEMAPIC, &tmpval, &bootInfo->chameleonConfig)&&tmpval;		
	}
    
    if (oem_apic == true) 
    {
        return(0);
    }
    
    if ((madt_file = (ACPI_TABLE_MADT *)get_new_table_in_list(new_table_list, NAMESEG("APIC"), &new_table_index)) != (void *)0ul)
	{				
        MadtPointer = (ACPI_TABLE_MADT *)madt_file;
        
        new_table_list[new_table_index] = 0ul; // This way, the non-patched table will not be added in our new rsdt/xsdt table list 			
	} 
    else
    {
        MadtPointer = (acpi_tables.MadtPointer64 != (void*)0ul) ? (ACPI_TABLE_MADT *)acpi_tables.MadtPointer64 : (ACPI_TABLE_MADT *)acpi_tables.MadtPointer;
        
        new_table_index = get_0ul_index_in_list(new_table_list, true);
        
        // Check to confirm space is available
        if (new_table_index == ACPI_TABLE_LIST_FULL)
        {		
            printf("Error: not enought reserved space in the new acpi list for the MADT table,\n ");
            printf("       please increase the RESERVED_AERA\n");
            return(0);
        }
    }    
    
    // Create buffer for MADT
    //U8 memory_for_madt[2 * 1024]; // seems to bug with xcode4, need to found out what is going on (not enough memory in the stack ?)
    U8 *memory_for_madt = (U8*)AllocateKernelMemory(2 * 1024);
    
    // Build the new MADT
    if ( (ProcessMadt(MadtPointer, madt_info, memory_for_madt, 2 * 1024, cpu_map_count))== 0)    
	{
		printf("Error: Failed to build MADT table\n");
		return (0);
	}
	
	// insert MADT in the new_table_list
	{
		// Create pointer to MADT just built in the stack buffer
		ACPI_TABLE_MADT * old_madt = (ACPI_TABLE_MADT *)memory_for_madt;		
		
		// Reserved kernel memory for the madt table
		ACPI_TABLE_MADT *new_madt = (ACPI_TABLE_MADT *)AllocateKernelMemory(old_madt->Header.Length);
		
		if (!new_madt) 
		{
			printf("Unable to allocate kernel memory for MADT ");
			return (0);
		}
		// Move the old stack buffer to kernel memory
		memcpy(new_madt, old_madt, old_madt->Header.Length);
		
		// Add the new madt into an empty space of the new_table_list
		new_table_list[new_table_index] = (U32)new_madt;				
	}
    
    verbose ("MADT table successfully patched\n");
	return(1);
}

static U32 ProcessSsdt(U32 * new_table_list, ACPI_TABLE_DSDT *dsdt, MADT_INFO * madt_info, bool enable_cstates, bool enable_pstates, bool enable_tstates )
{
	DBG("Processing SSDT\n");	
    
	// Check we are on an intel platform
	if (Platform.CPU.Vendor != CPUID_VENDOR_INTEL) {
		verbose ("Not an Intel platform: SSDT will not be generated !!!\n");
		return(0);
	}
	
	// Check for the msr feature flag
	if (!(Platform.CPU.Features & CPU_FEATURE_MSR)) {
		verbose ("Unsupported CPU: SSDT will not be generated !!!\n");
		return(0);
	}
	
	if (dsdt == (void *)0ul) {
		verbose ("DSDT not found: SSDT will not be generated !!!\n");
		return (0);
	}
	
	// Get an empty space in the new_talbe_list (true = allow reserved space)
	U8 empty = get_0ul_index_in_list(new_table_list, true);
	
	// Check to confirm space is available
	if (empty == ACPI_TABLE_LIST_FULL)
	{		
		printf("Error: not enought reserved space in the new acpi list for the SSDT table,\n ");
		printf("       please increase the RESERVED_AERA\n");
		return(0);
	}
    
	// Create buffer for SSDT
	//U8 memory_for_ssdt[20 * 1024];	// seems to bug with xcode4, need to found out what is going on (not enough memory in the stack ?)	
	U8 *memory_for_ssdt =(U8*)AllocateKernelMemory(20 * 1024);
	
	// Build the SSDT
	if ( (BuildSsdt(madt_info, dsdt, memory_for_ssdt, 20 * 1024 /*sizeof(memory_for_ssdt)*/, enable_cstates, enable_pstates, enable_tstates)) == 0)
	{
		printf("Error: Failed to build SSDT table\n");
		return (0);
	}
	
	// insert SSDT in the new_table_list
	{
		// Create pointer to SSDT just built in the stack buffer
		ACPI_TABLE_SSDT * old_ssdt = (ACPI_TABLE_SSDT *)memory_for_ssdt;		
		
		// Reserved kernel memory for the ssdt table
		ACPI_TABLE_SSDT *new_ssdt = (ACPI_TABLE_SSDT *)AllocateKernelMemory(old_ssdt->Header.Length);
		
		if (!new_ssdt) 
		{
			printf("Unable to allocate kernel memory for SSDT ");
			return (0);
		}
		// Move the old stack buffer to kernel memory
		memcpy(new_ssdt, old_ssdt, old_ssdt->Header.Length);
		
		// Add the new ssdt into an empty space of the new_table_list
		new_table_list[empty] = (U32)new_ssdt;				
	}
	
	verbose ("SSDT table generated successfully\n");
	return(1);
}

//-----------------------------------------------------------------------------
static void * buildCpuScope (void * current, U32 cpu_namespace, PROCESSOR_NUMBER_TO_NAMESEG * aslCpuNamePath)
{
	ACPI_SCOPE * scope = current;
	current = scope + 1;
	
	scope->scopeOpcode = AML_SCOPE_OP;
	scope->rootChar = AML_ROOT_PREFIX;
	
	if (aslCpuNamePath->seg_count == 1)
	{
		DUAL_NAME_PATH * dualNamePath = current;
		current = dualNamePath + 1;
		dualNamePath->prefix = AML_DUAL_NAME_PREFIX;
		dualNamePath->nameseg[0] = cpu_namespace;
		dualNamePath->nameseg[1] = aslCpuNamePath->nameseg[0];
	}
	else
	{
		MULTI_NAME_PATH * multiNamePath = current;
		current = multiNamePath + 1;
		multiNamePath->prefix = AML_MULTI_NAME_PREFIX;
		// the nameseg count includes the root prefix and all other namesegs
		multiNamePath->segCount = (U8) aslCpuNamePath->seg_count+1;
		multiNamePath->nameseg[0] = cpu_namespace;
		{
			U32 i;
			for (i=0; i<aslCpuNamePath->seg_count; i++)
				multiNamePath->nameseg[i+1] = aslCpuNamePath->nameseg[i];
		}
	}
	return (current);
}
//-----------------------------------------------------------------------------
static void * buildPDC(void * current)
{
	ACPI_METHOD * pdc = current;
	current = buildMethod(current, NAMESEG("_PDC"), 1);
	
	// CreateDWordField (Arg0, 0x08, CAPA)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x08);
	current = buildNameSeg(current, NAMESEG("CAPA"));
	
	// Store (CAPA, TYPE)
	current = buildOpCode(current, AML_STORE_OP);
	current = buildNameSeg(current, NAMESEG("CAPA"));
	current = buildNameSeg(current, NAMESEG("TYPE"));
	
	// CreateDWordField (Arg0, 0x00, REVS)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x00);
	current = buildNameSeg(current, NAMESEG("REVS"));
	
	// CreateDWordField (Arg0, 0x04, SIZE)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x04);
	current = buildNameSeg(current, NAMESEG("SIZE"));
	
	// Store(SizeOf(Arg0), Local0)
	current = buildOpCode(current, AML_STORE_OP);
	current = buildOpCode(current, AML_SIZEOF_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildOpCode(current, AML_LOCAL0_OP);
	
	// Store(Subtract(Local0, 0x08),Local1)
	current = buildOpCode(current, AML_STORE_OP);
	current = buildOpCode(current, AML_SUBTRACT_OP);
	current = buildOpCode(current, AML_LOCAL0_OP);
	current = buildByteConst(current, 0x08);
	current = buildOpCode(current, AML_ZERO_OP);
	current = buildOpCode(current, AML_LOCAL1_OP);
	
	// CreateField (Arg0, 0x40, Multiply (Local1, 0x08), TEMP)
	current = buildOpCode(current, AML_EXT_OP_PREFIX);
	current = buildOpCode(current, AML_CREATE_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x40);
	current = buildOpCode(current, AML_MULTIPLY_OP);
	current = buildOpCode(current, AML_LOCAL1_OP);
	current = buildByteConst(current, 0x08);
	current = buildOpCode(current, AML_ZERO_OP);
	current = buildNameSeg(current, NAMESEG("TEMP"));
	
	// Name (STS0, Buffer (0x04) {0x00, 0x00, 0x00, 0x00})
	// Create STS0 as named buffer
	current = buildNamePath(current, NAMESEG("STS0"));
	{
		ACPI_SMALL_BUFFER * buff = current;
		current = buildSmallBuffer(current);
		
		// count of buffer elements
		current = buildByteConst(current, 4);
		
		current = buildOpCode(current, AML_ZERO_OP);
		current = buildOpCode(current, AML_ZERO_OP);
		current = buildOpCode(current, AML_ZERO_OP);
		current = buildOpCode(current, AML_ZERO_OP);
		{
			U32 length = (U8 *)current - (U8 *)buff;
			buff->packageLength = (U8)length - 1;
		}
	}
	
	//Concatenate (STS0, TEMP, Local2)
	current = buildOpCode(current, AML_CONCAT_OP);
	current = buildNameSeg(current, NAMESEG("STS0"));
	current = buildNameSeg(current, NAMESEG("TEMP"));
	current = buildOpCode(current, AML_LOCAL2_OP);
	
	//_OSC (Buffer (0x10)
	//      {
	//         /* 0000 */    0x16, 0xA6, 0x77, 0x40, 0x0C, 0x29, 0xBE, 0x47,
	//         /* 0008 */    0x9E, 0xBD, 0xD8, 0x70, 0x58, 0x71, 0x39, 0x53
	//      }, REVS, SIZE, Local2)
	current = buildNameSeg(current, NAMESEG("_OSC"));
	{
		ACPI_SMALL_BUFFER * buff = current;
		current = buildSmallBuffer(current);
		
		// count of buffer elements
		current = buildByteConst(current, 0x10);
		
		current = buildOpCode(current, 0x16);
		current = buildOpCode(current, 0xa6);
		current = buildOpCode(current, 0x77);
		current = buildOpCode(current, 0x40);
		current = buildOpCode(current, 0x0c);
		current = buildOpCode(current, 0x29);
		current = buildOpCode(current, 0xbe);
		current = buildOpCode(current, 0x47);
		current = buildOpCode(current, 0x9e);
		current = buildOpCode(current, 0xbd);
		current = buildOpCode(current, 0xd8);
		current = buildOpCode(current, 0x70);
		current = buildOpCode(current, 0x58);
		current = buildOpCode(current, 0x71);
		current = buildOpCode(current, 0x39);
		current = buildOpCode(current, 0x53);
		{
			U32 length = (U8 *)current - (U8 *)buff;
			buff->packageLength = (U8)length - 1;
		}
	}
	current = buildNameSeg(current, NAMESEG("REVS"));
	current = buildNameSeg(current, NAMESEG("SIZE"));
	current = buildOpCode(current, AML_LOCAL2_OP);
	
	// Update package length in PDC object
	//pdc->packageLength = (U8)((U8 *)current - (U8 *)&pdc->packageLength);
	setPackageLength(&pdc->pkgLength, (U8 *)current - (U8 *)&pdc->pkgLength);
	
	return(current);
}

//-----------------------------------------------------------------------------
static void * buildOSC(void * current)
{
	//
	//
	ACPI_METHOD * osc = current;
	current = buildMethod(current, NAMESEG("_OSC"), 4);
	
	// CreateDWordField (Arg3, 0x04, CAPA)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG3_OP);
	current = buildByteConst(current, 0x04);
	current = buildNameSeg(current, NAMESEG("CAPA"));
	
	// Store (CAPA, TYPE)
	current = buildOpCode(current, AML_STORE_OP);
	current = buildNameSeg(current, NAMESEG("CAPA"));
	current = buildNameSeg(current, NAMESEG("TYPE"));
	
	// CreateDWordField (Arg3, 0x00, STS0)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG3_OP);
	current = buildByteConst(current, 0x00);
	current = buildNameSeg(current, NAMESEG("STS0"));
	
	// CreateDWordField (Arg3, 0x04, CAP0)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG3_OP);
	current = buildByteConst(current, 0x04);
	current = buildNameSeg(current, NAMESEG("CAP0"));
	
	// CreateDWordField (Arg0, 0x00, IID0)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x00);
	current = buildNameSeg(current, NAMESEG("IID0"));
	
	// CreateDWordField (Arg0, 0x04, IID1)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x04);
	current = buildNameSeg(current, NAMESEG("IID1"));
	
	// CreateDWordField (Arg0, 0x08, IID2)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x08);
	current = buildNameSeg(current, NAMESEG("IID2"));
	
	// CreateDWordField (Arg0, 0x0C, IID3)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x0C);
	current = buildNameSeg(current, NAMESEG("IID3"));
	
	// Name (UID0, Buffer (0x10)
	// {
	//    0x16, 0xA6, 0x77, 0x40, 0x0C, 0x29, 0xBE, 0x47,
	//    0x9E, 0xBD, 0xD8, 0x70, 0x58, 0x71, 0x39, 0x53
	// })
	current = buildNamePath(current, NAMESEG("UID0"));
	{
		ACPI_SMALL_BUFFER * buff = current;
		current = buildSmallBuffer(current);
		
		// count of buffer elements
		current = buildByteConst(current, 0x10);
		
		current = buildOpCode(current, 0x16);
		current = buildOpCode(current, 0xa6);
		current = buildOpCode(current, 0x77);
		current = buildOpCode(current, 0x40);
		current = buildOpCode(current, 0x0c);
		current = buildOpCode(current, 0x29);
		current = buildOpCode(current, 0xbe);
		current = buildOpCode(current, 0x47);
		current = buildOpCode(current, 0x9e);
		current = buildOpCode(current, 0xbd);
		current = buildOpCode(current, 0xd8);
		current = buildOpCode(current, 0x70);
		current = buildOpCode(current, 0x58);
		current = buildOpCode(current, 0x71);
		current = buildOpCode(current, 0x39);
		current = buildOpCode(current, 0x53);
		
		{
			U32 length = (U8 *)current - (U8 *)buff;
			buff->packageLength = (U8)length - 1;
		}
	}
	
	// CreateDWordField (UID0, 0x00, EID0)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x00);
	current = buildNameSeg(current, NAMESEG("EID0"));
	
	// CreateDWordField (UID0, 0x04, EID1)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x04);
	current = buildNameSeg(current, NAMESEG("EID1"));
	
	// CreateDWordField (UID0, 0x08, EID2)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x08);
	current = buildNameSeg(current, NAMESEG("EID2"));
	
	// CreateDWordField (UID0, 0x0C, EID3)
	current = buildOpCode(current, AML_CREATE_DWORD_FIELD_OP);
	current = buildOpCode(current, AML_ARG0_OP);
	current = buildByteConst(current, 0x0C);
	current = buildNameSeg(current, NAMESEG("EID3"));
	
	// If (LNot (LAnd (LAnd (LEqual (IID0, EID0), LEqual (IID1, EID1)),
	//      LAnd (LEqual (IID2, EID2), LEqual (IID3, EID3)))))
	// {
	//      Store (0x06, Index (STS0, 0x00))
	//      Return (Arg3)
	// }
	{
		current = buildOpCode(current, AML_IF_OP);
		{
			ACPI_PACKAGE_LENGTH * packageLength = current;
			current = buildPackageLength(current, 0);
			
			current = buildOpCode(current, AML_LNOT_OP);
			current = buildOpCode(current, AML_LAND_OP);
			current = buildOpCode(current, AML_LAND_OP);
			current = buildOpCode(current, AML_LEQUAL_OP);
			current = buildNameSeg(current, NAMESEG("IID0"));
			current = buildNameSeg(current, NAMESEG("EID0"));
			
			current = buildOpCode(current, AML_LEQUAL_OP);
			current = buildNameSeg(current, NAMESEG("IID1"));
			current = buildNameSeg(current, NAMESEG("EID1"));
			
			current = buildOpCode(current, AML_LAND_OP);
			current = buildOpCode(current, AML_LEQUAL_OP);
			current = buildNameSeg(current, NAMESEG("IID2"));
			current = buildNameSeg(current, NAMESEG("EID2"));
			
			current = buildOpCode(current, AML_LEQUAL_OP);
			current = buildNameSeg(current, NAMESEG("IID3"));
			current = buildNameSeg(current, NAMESEG("EID3"));
			
			// Store (0x06, Index (STS0, 0x00))
			current = buildOpCode(current, AML_STORE_OP);
			current = buildByteConst(current, 0x06);
			current = buildOpCode(current, AML_INDEX_OP);
			current = buildNameSeg(current, NAMESEG("STS0"));
			current = buildByteConst(current, 0x00);
			current = buildOpCode(current, AML_ZERO_OP);
			
			// Return (Arg3)
			current = buildReturnOpcode(current, AML_ARG3_OP);
			
			setPackageLength(packageLength,
							 (U8 *)current - (U8 *)packageLength);
		}
	}
	
	// If (LNotEqual (Arg1, 0x01))
	// {
	//      Store (0x0A, Index (STS0, 0x00))
	//      Return (Arg3)
	// }
	{
		current = buildOpCode(current, AML_IF_OP);
		{
			ACPI_PACKAGE_LENGTH * packageLength = current;
			current = buildPackageLength(current, 0);
			
			// If ("LNotEqual (Arg1, 0x01)")
			current = buildOpCode(current, AML_LNOT_OP);
			current = buildOpCode(current, AML_LEQUAL_OP);
			current = buildOpCode(current, AML_ARG1_OP);
			current = buildByteConst(current, 0x01);
			
			// Store (0x0A, Index (STS0, 0x00))
			current = buildOpCode(current, AML_STORE_OP);
			current = buildByteConst(current, 0x0A);
			current = buildOpCode(current, AML_INDEX_OP);
			current = buildNameSeg(current, NAMESEG("STS0"));
			current = buildByteConst(current, 0x00);
			current = buildOpCode(current, AML_ZERO_OP);
			
			// Return (Arg3)
			current = buildReturnOpcode(current, AML_ARG3_OP);
			
			setPackageLength(packageLength,
							 (U8 *)current - (U8 *)packageLength);
		}
	}
	
	// If (And (STS0, 0x01))
	// {
	//    And (CAP0, 0x0BFF, CAP0)
	//    Return (Arg3)
	// }
	{
		current = buildOpCode(current, AML_IF_OP);
		{
			ACPI_PACKAGE_LENGTH * packageLength = current;
			current = buildPackageLength(current, 0);
			
			// If ("And (STS0, 0x01)")
			current = buildOpCode(current, AML_AND_OP);
			current = buildNameSeg(current, NAMESEG("STS0"));
			current = buildByteConst(current, 0x01);
			current = buildOpCode(current, AML_ZERO_OP);
			
			// And (CAP0, 0x0BFF, CAP0)
			current = buildOpCode(current, AML_AND_OP);
			current = buildNameSeg(current, NAMESEG("CAP0"));
			current = buildWordConst(current, 0x0BFF);
			current = buildNameSeg(current, NAMESEG("CAP0"));
			
			// Return (Arg3)
			current = buildReturnOpcode(current, AML_ARG3_OP);
			
			setPackageLength(packageLength,
							 (U8 *)current - (U8 *)packageLength);
		}
	}
	
	// And (CAP0, 0x0BFF, CAP0)
	current = buildOpCode(current, AML_AND_OP);
	current = buildNameSeg(current, NAMESEG("CAP0"));
	current = buildWordConst(current, 0x0BFF);
	current = buildNameSeg(current, NAMESEG("CAP0"));
	
	// Store (CAP0, TYPE)
	current = buildOpCode(current, AML_STORE_OP);
	current = buildNameSeg(current, NAMESEG("CAP0"));
	current = buildNameSeg(current, NAMESEG("TYPE"));
	
	// Return (Arg3)
	current = buildReturnOpcode(current, AML_ARG3_OP);
	
	// Set package length for the OSC object
	setPackageLength(&osc->pkgLength, (U8 *)current - (U8 *)&osc->pkgLength);
	
	return(current);
}

//-----------------------------------------------------------------------------
static void * buildPSS(void * current, PKG_PSTATES * pkg_pstates)
{
	//
	// IF (PSEN)
	// {
	//    Return (Package of Pstate Packages)
	// }
	// Return(Zero)
	//
	ACPI_METHOD * pss = current;
	current = buildMethod(current, NAMESEG("_PSS"), 0);
	
	{
		// "IF" (PSEN) -- IF Opcode
		current = buildOpCode(current, AML_IF_OP);
		{
			ACPI_PACKAGE_LENGTH * packageLength = current;
			current = buildPackageLength(current, 0);
			
			// IF "(PSEN)" -- IF Predicate
			current = buildNameSeg(current, NAMESEG("PSEN"));
			
			{
				ACPI_RETURN_PACKAGE * returnPkg = current;
				current = buildReturnPackage(current, (U8)pkg_pstates->num_pstates);
				
				// (3.3.3) For each P-state
				{
					U32 pstateIndex = 0;
					for (pstateIndex=0; pstateIndex < pkg_pstates->num_pstates; pstateIndex++)
					{
						// (3.3.3.1) Create P-state package
						ACPI_PSTATE_PACKAGE * pstate = current;
						current = pstate + 1;
						
						setSmallPackage(&pstate->package, 6);
						pstate->package.packageLength = (U8)(sizeof(ACPI_PSTATE_PACKAGE) - 1);
						
						setDwordConst(&pstate->CoreFreq, pkg_pstates->pstate[pstateIndex].frequency);// CoreFreq (in MHz).
						setDwordConst(&pstate->Power, pkg_pstates->pstate[pstateIndex].power);// Power (in milliWatts).
						setDwordConst(&pstate->TransLatency, pkg_pstates->pstate[pstateIndex].translatency);// Transition Latency (in microseconds).
						setDwordConst(&pstate->BMLatency, pkg_pstates->pstate[pstateIndex].bmlatency);// Bus Master Latency (in microseconds).
						setDwordConst(&pstate->Control, pkg_pstates->pstate[pstateIndex].control); // Control.
						
						setDwordConst(&pstate->Status, encode_pstate(pkg_pstates->pstate[pstateIndex].ratio));// Status.	
					} // for
				} // for block
				
				// (3.3.4) Update package length in return package
				setPackageLength(&returnPkg->package.pkgLength, (U8 *)current - (U8 *)&returnPkg->package.pkgLength);
			}
			
			// "IF (PSEN) and its body" -- Set package length
			setPackageLength(packageLength,
							 (U8 *)current - (U8 *)packageLength);
		}
		// "Return (ZERO)"
		current = buildReturnZero(current);
	}
	// Set package length for the _PSS object
	setPackageLength(&pss->pkgLength, (U8 *)current - (U8 *)&pss->pkgLength);
	
	return(current);
}

//-----------------------------------------------------------------------------
static void * buildPSD(void * current, U32 domain, U32 cpusInDomain, U32 pstate_coordination)
{
	// If (And(TYPE, 0x0820))
	// {
	//    Return (PSD Package)
	// }
	// Return(Zero)
	
	ACPI_METHOD * psdMethod = current;
	current = buildMethod(current, NAMESEG("_PSD"), 0);
	{
		// "IF" (And(TYPE, 0x0820)) -- IF Opcode
		current = buildOpCode(current, AML_IF_OP);
		{
			ACPI_PACKAGE_LENGTH * packageLength = current;
			current = buildPackageLength(current, 0);
			
			// IF ("And"(TYPE, 0x820)) -- AND Opcode
			current = buildOpCode(current, AML_AND_OP);
			
			// IF (And("TYPE", 0x820)) -- TYPE Term
			current = buildNameSeg(current, NAMESEG("TYPE"));
			
			// IF (And(TYPE, "0x0820")) -- DWORD Value Term
			current = buildDwordConst(current, 0x820);
			
			// IF ("And(TYPE, 0x200)") -- Target for And term (unused)
			current = buildOpCode(current, AML_ZERO_OP);
			
			// Build return package containing PSD package
			{
				ACPI_RETURN_PACKAGE * returnPkg = current;
				current = buildReturnPackage(current, 1);
				
				{
					// Create PSD package
					ACPI_PSD_PACKAGE * psd = current;
					current = psd + 1;
					
					setSmallPackage(&psd->package, 5);
					psd->package.packageLength = (U8)(sizeof(ACPI_PSD_PACKAGE) - 1);
					
					setByteConst(&psd->NumberOfEntries, 5);
					setByteConst(&psd->Revision, 0);
					setDwordConst(&psd->Domain, domain);
					setDwordConst(&psd->CoordType, pstate_coordination);
					setDwordConst(&psd->NumProcessors, cpusInDomain);
					
				} // PSD package
				
				setPackageLength(&returnPkg->package.pkgLength,
								 (U8 *)current - (U8 *)&returnPkg->package.pkgLength);
			}
			setPackageLength(packageLength, (U8 *)current - (U8 *)packageLength);
		}
		// "Return (ZERO)"
		current = buildReturnZero(current);
	}
	// Update length in _PSD method
	setPackageLength(&psdMethod->pkgLength, (U8 *)current - (U8 *)&psdMethod->pkgLength);
	
	return(current);
}

//-----------------------------------------------------------------------------
static void * buildPPC(void * current/*, U8 valueToReturn*/)
{
	ACPI_SMALL_METHOD * ppc = current;
	current = buildSmallMethod(current, NAMESEG("_PPC"), 0);
	
	current = buildReturnZero(current);
	
	//current = buildReturnOpcode(current, valueToReturn);
    
	// Update package length in PPC object
	ppc->packageLength = (U8) ( (U8 *)current - (U8 *)&ppc->packageLength );
	
	return(current);
}

#if UNUSED
//-----------------------------------------------------------------------------
static void * buildPDL(void * current, U8 valueToReturn)
{
	ACPI_SMALL_METHOD * pdl = current;
	current = buildSmallMethod(current, NAMESEG("_PDL"), 0);
	
	current = buildReturnOpcode(current, valueToReturn);
	
	// Update package length in PDL object
	pdl->packageLength = (U8) ( (U8 *)current - (U8 *)&pdl->packageLength );
	
	return(current);
}
#endif

//-----------------------------------------------------------------------------
static void * buildPCT(void * current)
{
	static const ACPI_GENERIC_ADDRESS pct_gas[] = {
		{0x7f,0x40,0,0,0x199},
		{0x7f,0x10,0,0,0x198},
	};
	
	ACPI_SMALL_METHOD * pct = current;
	current = buildSmallMethod(current, NAMESEG("_PCT"), 0);
	
	{
		ACPI_RETURN_PACKAGE * returnPkg = current;
		current = buildReturnPackage(current, 2);
		
		{
			ACPI_SMALL_BUFFER * buff = current;
			current = buildSmallBuffer(current);
			
			current = buildByteConst(current, sizeof(ACPI_GENERIC_REGISTER) + sizeof(ACPI_END_TAG) );
			current = buildGenericRegister(current, &pct_gas[0]);
			current = buildEndTag(current);
			
			{
				U32 length = (U8 *)current - (U8 *)buff;
				buff->packageLength = (U8)length - 1;
			}
		}
		{
			ACPI_SMALL_BUFFER * buff = current;
			current = buildSmallBuffer(current);
			
			current = buildByteConst(current, sizeof(ACPI_GENERIC_REGISTER) + sizeof(ACPI_END_TAG) );
			current = buildGenericRegister(current, &pct_gas[1]);
			current = buildEndTag(current);
			
			{
				U32 length = (U8 *)current - (U8 *)buff;
				buff->packageLength = (U8)length - 1;
			}
			
		}
		
		setPackageLength(&returnPkg->package.pkgLength,
						 (U8 *)current - (U8 *)&returnPkg->package.pkgLength);
	}
	
	// Update package length in PCT object
	pct->packageLength = (U8)((U8 *)current - (U8 *)&pct->packageLength);
	
	return(current);
}

//-----------------------------------------------------------------------------
static void * buildCstate(void * current, ACPI_GENERIC_ADDRESS * gas, CSTATE * cstate)
{
	//
	// Build a C-state
	//
	ACPI_SMALL_PACKAGE * pkg1 = current;
	current = buildSmallPackage(current, 4);
	
	{
		{
			ACPI_SMALL_BUFFER * buffer = current;
			current = buildSmallBuffer(current);
			
			{
				// Buffer length
				current = buildByteConst(current, sizeof(ACPI_GENERIC_REGISTER) + sizeof(ACPI_END_TAG) );
				current = buildGenericRegister(current, gas);
				current = buildEndTag(current);
			}
			{
				U32 length = (U8 *)current - (U8 *)buffer;
				buffer->packageLength = (U8)length - 1;
			}
		}
		
		{
			current = buildByteConst(current, cstate->type);
			current = buildWordConst(current, cstate->latency);
			current = buildDwordConst(current, cstate->power);
		}
	}
	pkg1->packageLength = (U8)((U8 *)current - (U8 *)&pkg1->packageLength);
	
	return(current);
}

//-----------------------------------------------------------------------------
static void * buildReturnPackageCST(void * current, PKG_CSTATES * pkg_cstates)
{
	// Create package returning C-states
	ACPI_RETURN_PACKAGE * returnPkg = current;
	current = buildReturnPackage(current, (U8)pkg_cstates->num_cstates + 1);
	
	{
		// Include number of C-states
		current = buildByteConst(current, (U8)pkg_cstates->num_cstates);
		
		{
			U32 cstateIndex = 0;
			for (cstateIndex=0; cstateIndex < pkg_cstates->num_cstates; cstateIndex++)
				// Build C-state
				current = buildCstate(current, &pkg_cstates->gas[cstateIndex], &pkg_cstates->cstate[cstateIndex]);
		}
	}
	
	// Update package length in return package
	setPackageLength(&returnPkg->package.pkgLength,
                     (U8 *)current - (U8 *)&returnPkg->package.pkgLength);
	
	return(current);
}

//-----------------------------------------------------------------------------
static void * buildCST(void * current, PKG_CSTATES * mwait_pkg_cstates, PKG_CSTATES * io_pkg_cstates)
{
	//
	// IF (CSEN)
	// {
	//    IF (LAnd(MWOS, And(TYPE, 0x200)))
	//    {
	//       Return package containing MWAIT C-states
	//    }
	//    Return package containing IO C-states
	// }
	// Return(Zero)
	//
	ACPI_METHOD * cst = current;
	current = buildMethod(current, NAMESEG("_CST"), 0);
	{
		// "IF" CSEN -- IF Opcode
		current = buildOpCode(current, AML_IF_OP);
		{
			ACPI_PACKAGE_LENGTH * packageLength1 = current;
			current = buildPackageLength(current, 0);
			
			// IF "(CSEN)" -- IF Predicate
			current = buildNameSeg(current, NAMESEG("CSEN"));
			
			// "IF" (LAnd(MWOS, And(TYPE, 0x200))) -- IF Opcode
			current = buildOpCode(current, AML_IF_OP);
			{
				ACPI_PACKAGE_LENGTH * packageLength2 = current;
				current = buildPackageLength(current, 0);
				
				// IF ("LAnd"(MWOS, And(TYPE, 0x200))) -- LAND Opcode
				current = buildOpCode(current, AML_LAND_OP);
				
				// IF (LAnd("MWOS", And(TYPE, 0x200))) -- MWOS Term
				current = buildNameSeg(current, NAMESEG("MWOS"));
				
				// IF (LAnd(MWOS, "And"(TYPE, 0x200))) -- AND Opcode
				current = buildOpCode(current, AML_AND_OP);
				
				// IF (LAnd(MWOS, And("TYPE", 0x200))) -- TYPE Term
				current = buildNameSeg(current, NAMESEG("TYPE"));
				
				// IF (LAnd(MWOS, And(TYPE, "0x200"))) -- DWORD Value Term
				current = buildWordConst(current, 0x200);
				
				// IF (LAnd(MWOS, "And(TYPE, 0x200)")) -- Target for And term (unused)
				current = buildOpCode(current, AML_ZERO_OP);
				
				// Build return package for mwait c-states
				current = buildReturnPackageCST(current, mwait_pkg_cstates);
				
				setPackageLength(packageLength2,
								 (U8 *)current - (U8 *)packageLength2);
			}
			
			// Build return package for io c-states
			current = buildReturnPackageCST(current, io_pkg_cstates);
			
			setPackageLength(packageLength1,
							 (U8 *)current - (U8 *)packageLength1);
		}
		// "Return (ZERO)"
		current = buildReturnZero(current);
	}
	// Update length in _CST method
	setPackageLength(&cst->pkgLength, (U8 *)current - (U8 *)&cst->pkgLength);
	
	return(current);
}

#if BUILD_ACPI_CSD
//-----------------------------------------------------------------------------
static void * buildCSD(void * current, U32 domain, U32 cpusInDomain, PKG_CSTATES * pkg_cstates)
{
    // If (And(TYPE, 0x0040))
    // {
    //    Return (CSD Package)
    // }
    // Return(Zero)
    
    ACPI_METHOD * csdMethod = current;
    current = buildMethod(current, NAMESEG("_CSD"), 0);
    {
        // "IF" (And(TYPE, 0x0040)) -- IF Opcode
        current = buildOpCode(current, AML_IF_OP);
        {
            ACPI_PACKAGE_LENGTH * packageLength = current;
            current = buildPackageLength(current, 0);
            
            // IF ("And"(TYPE, 0x0040)) -- AND Opcode
            current = buildOpCode(current, AML_AND_OP);
            
            // IF (And("TYPE", 0x0040)) -- TYPE Term
            current = buildNameSeg(current, NAMESEG("TYPE"));
            
            // IF (And(TYPE, "0x0040")) -- DWORD Value Term
            current = buildDwordConst(current, 0x0040);
            
            // IF ("And(TYPE, 0x0040)") -- Target for And term (unused)
            current = buildOpCode(current, AML_ZERO_OP);
            
            // Build return package containing CSD package(s)
            {
                ACPI_RETURN_PACKAGE * returnPkg = current;
                current = buildReturnPackage(current, (U8)pkg_cstates->num_cstates - 1);
                
                {
                    U32 cstateIndex;
                    for (cstateIndex=1; cstateIndex < pkg_cstates->num_cstates; cstateIndex++)
                    {
                        // Build CSD for this C-state
                        
                        // Create CSD package
                        ACPI_CSD_PACKAGE * csd = current;
                        current = csd + 1;
                        
                        setSmallPackage(&csd->package, 6);
                        csd->package.packageLength = (U8)(sizeof(ACPI_CSD_PACKAGE) - 1);
                        
                        setByteConst(&csd->NumberOfEntries, 6);
                        setByteConst(&csd->Revision, 0);
                        setDwordConst(&csd->Domain, domain);
                        setDwordConst(&csd->CoordType, ACPI_COORD_TYPE_HW_ALL);
                        setDwordConst(&csd->NumProcessors, cpusInDomain);
                        setDwordConst(&csd->Index, cstateIndex);
                    }
                }
                
                setPackageLength(&returnPkg->package.pkgLength,
                                 (U8 *)current - (U8 *)&returnPkg->package.pkgLength);
            }
            
            setPackageLength(packageLength, (U8 *)current - (U8 *)packageLength);
        }
        // "Return (ZERO)"
        current = buildReturnZero(current);
    }
    // Update length in _CSD method
    setPackageLength(&csdMethod->pkgLength, (U8 *)current - (U8 *)&csdMethod->pkgLength);
    
    return(current);
}
#endif // BUILD_ACPI_CSD

#if BUILD_ACPI_TSS
//-----------------------------------------------------------------------------
static void * buildTPC(void * current)
{
    ACPI_SMALL_METHOD * tpc = current;
    current = buildSmallMethod(current, NAMESEG("_TPC"), 0);
    
    current = buildReturnZero(current);
    
    // Update package length in PPC object
    tpc->packageLength = (U8) ( (U8 *)current - (U8 *)&tpc->packageLength );
    
    return(current);
}

//-----------------------------------------------------------------------------
static void * buildPTC(void * current)
{
    static const ACPI_GENERIC_ADDRESS ptc_gas[] = {
        {0x7f,0x00,0,0,0},
        {0x7f,0x00,0,0,0},
    };
    
    ACPI_SMALL_METHOD * ptc = current;
    current = buildSmallMethod(current, NAMESEG("_PTC"), 0);
    
    {
        ACPI_RETURN_PACKAGE * returnPkg = current;
        current = buildReturnPackage(current, 2);
        
        {
            ACPI_SMALL_BUFFER * buff = current;
            current = buildSmallBuffer(current);
            
            current = buildByteConst(current, sizeof(ACPI_GENERIC_REGISTER) + sizeof(ACPI_END_TAG) );
            current = buildGenericRegister(current, &ptc_gas[0]);
            current = buildEndTag(current);
            
            {
                U32 length = (U8 *)current - (U8 *)buff;
                buff->packageLength = (U8)length - 1;
            }
        }
        {
            ACPI_SMALL_BUFFER * buff = current;
            current = buildSmallBuffer(current);
            
            current = buildByteConst(current, sizeof(ACPI_GENERIC_REGISTER) + sizeof(ACPI_END_TAG) );
            current = buildGenericRegister(current, &ptc_gas[1]);
            current = buildEndTag(current);
            
            {
                U32 length = (U8 *)current - (U8 *)buff;
                buff->packageLength = (U8)length - 1;
            }
        }
        
        setPackageLength(&returnPkg->package.pkgLength,
                         (U8 *)current - (U8 *)&returnPkg->package.pkgLength);
    }
    
    // Update package length in PTC object
    ptc->packageLength = (U8)((U8 *)current - (U8 *)&ptc->packageLength);
    
    return(current);
}

//-----------------------------------------------------------------------------
static void * buildTSS(void * current, PKG_TSTATES * pkg_tstates)
{
    //
    // IF (LAnd(TSEN, And(TYPE,4)))
    // {
    //    Return (Package of Tstate Packages)
    // }
    // Return(Zero)
    //
    ACPI_METHOD * tss = current;
    current = buildMethod(current, NAMESEG("_TSS"), 0);
    
    {
        // "IF" (LAnd(TSEN, And(TYPE,4))) -- IF Opcode
        current = buildOpCode(current, AML_IF_OP);
        {
            ACPI_PACKAGE_LENGTH * packageLength = current;
            current = buildPackageLength(current, 0);
            
            // IF ("LAnd"(TSEN, And(TYPE, 4))) -- LAND Opcode
            current = buildOpCode(current, AML_LAND_OP);
            
            // IF (LAnd("TSEN", And(TYPE, 4))) -- TSEN Term
            current = buildNameSeg(current, NAMESEG("TSEN"));
            
            // IF (LAnd(TSEN, "And"(TYPE, 4))) -- AND Opcode
            current = buildOpCode(current, AML_AND_OP);
            
            // IF (LAnd(TSEN, And("TYPE", 4))) -- TYPE Term
            current = buildNameSeg(current, NAMESEG("TYPE"));
            
            // IF (LAnd(TSEN, And(TYPE, "4"))) -- DWORD Value Term
            current = buildWordConst(current, 4);
            
            // IF (LAnd(MWOS, "And(TYPE, 4)")) -- Target for And term (unused)
            current = buildOpCode(current, AML_ZERO_OP);
            
            // Return (Package of Tstate Packages)
            {
                ACPI_RETURN_PACKAGE * returnPkg = current;
                current = buildReturnPackage(current, (U8)pkg_tstates->num_tstates);
                
                // (3.3.3) For each T-state
                {
                    U32 tstateIndex = 0;
                    for (tstateIndex=0; tstateIndex < pkg_tstates->num_tstates; tstateIndex++)
                    {
                        // (3.3.3.1) Create T-state package
                        ACPI_TSTATE_PACKAGE * tstate = current;
                        current = tstate + 1;
                        
                        setSmallPackage(&tstate->package, 5);
                        tstate->package.packageLength = (U8)(sizeof(ACPI_TSTATE_PACKAGE) - 1);
                        
                        setDwordConst(&tstate->FreqPercent, pkg_tstates->tstate[tstateIndex].freqpercent);
                        setDwordConst(&tstate->Power, pkg_tstates->tstate[tstateIndex].power);
                        setDwordConst(&tstate->TransLatency, pkg_tstates->tstate[tstateIndex].latency);
                        setDwordConst(&tstate->Control, pkg_tstates->tstate[tstateIndex].control);
                        setDwordConst(&tstate->Status, pkg_tstates->tstate[tstateIndex].status);
                    } // for
                } // for block
                
                // (3.3.4) Update package length in return package
                setPackageLength(&returnPkg->package.pkgLength, (U8 *)current - (U8 *)&returnPkg->package.pkgLength);
            }
            
            // "IF (LAnd(TSEN, And(TYPE,4))) and its body" -- Set package length
            setPackageLength(packageLength, (U8 *)current - (U8 *)packageLength);
        }
        // "Return (ZERO)"
        current = buildReturnZero(current);
    }
    // Set package length for the _TSS object
    setPackageLength(&tss->pkgLength, (U8 *)current - (U8 *)&tss->pkgLength);
    
    return(current);
}

//-----------------------------------------------------------------------------
static void * buildTSD(void * current, U32 domain, U32 cpusInDomain)
{
    // If (And(TYPE, 0x0080))
    // {
    //    Return (Package containing TSD package)
    // }
    // Return(Zero)
    
    ACPI_METHOD * tsdMethod = current;
    current = buildMethod(current, NAMESEG("_TSD"), 0);
    {
        // "IF" (And(TYPE, 0x0080)) -- IF Opcode
        current = buildOpCode(current, AML_IF_OP);
        {
            ACPI_PACKAGE_LENGTH * packageLength = current;
            current = buildPackageLength(current, 0);
            
            // IF ("And"(TYPE, 0x0080)) -- AND Opcode
            current = buildOpCode(current, AML_AND_OP);
            
            // IF (And("TYPE", 0x0080)) -- TYPE Term
            current = buildNameSeg(current, NAMESEG("TYPE"));
            
            // IF (And(TYPE, "0x0080")) -- DWORD Value Term
            current = buildDwordConst(current, 0x0080);
            
            // IF ("And(TYPE, 0x0080)") -- Target for And term (unused)
            current = buildOpCode(current, AML_ZERO_OP);
            
            // Build package containing TSD package
            {
                ACPI_RETURN_PACKAGE * returnPkg = current;
                current = buildReturnPackage(current, 1);
                
                {
                    // Create PSD package
                    ACPI_TSD_PACKAGE * tsd = current;
                    current = tsd + 1;
                    
                    setSmallPackage(&tsd->package, 5);
                    tsd->package.packageLength = (U8)(sizeof(ACPI_TSD_PACKAGE) - 1);
                    
                    setByteConst(&tsd->NumberOfEntries, 5);
                    setByteConst(&tsd->Revision, 0);
                    setDwordConst(&tsd->Domain, domain);
                    setDwordConst(&tsd->CoordType, ACPI_COORD_TYPE_SW_ANY);
                    setDwordConst(&tsd->NumProcessors, cpusInDomain);
                    
                } // TSD package
                
                setPackageLength(&returnPkg->package.pkgLength,
                                 (U8 *)current - (U8 *)&returnPkg->package.pkgLength);
            }
            
            setPackageLength(packageLength, (U8 *)current - (U8 *)packageLength);
        }
        // "Return (ZERO)"
        current = buildReturnZero(current);
    }
    // Update length in _TSD method
    setPackageLength(&tsdMethod->pkgLength, (U8 *)current - (U8 *)&tsdMethod->pkgLength);
    
    return(current);
}
#endif // BUILD_ACPI_TSS

//-----------------------------------------------------------------------------
static U32 BuildSsdt(MADT_INFO * madt_info, ACPI_TABLE_DSDT *dsdt, void * buffer, U32 bufferSize, bool enable_cstates, bool enable_pstates,  bool enable_tstates)
{
	// Build SSDT
	{
		// (1) Setup pointers to SSDT memory location
		// (2) Create SSDT Definition Block
		//    (2.1) Save pointer to SSDT package length and checksum fields
		//    (2.2) Create variables in SSDT scope
		// (3) For each logical processor CPUn 
		//    (3.1) Create scope for CPUn
		//    (3.2) Create variables in CPU scope
		//    (3.3) Create _OSC and/or _PDC Methods
		//    (3.4) Create P-state related structures
		//       (3.4.1) Create _PSS Method
		//       (3.4.2) Create _PCT Object
		//       (3.4.3) Create _PPC Method
		//       (3.4.4) Create _PSD Object
		//    (3.5) Create C-state related structures
		//       (3.5.1) Create _CST Method
		//       (3.5.2) Create _CSD Method
		//    (3.6) Create T-state related structures (Optional)
		//       (3.6.1) Create _TPC Method
		//       (3.6.2) Create _PTC Method
		//       (3.6.3) Create _TSS Method
		//       (3.6.4) Create _TSD Method
		//    (3.7) Update length in CPUn Scope
		// (4) Update length and checksum in SSDT Definition Block
		DBG("Attempting to build SSDT\n");
        
		U32 pstates_enabled = 0;
		U32 cstates_enabled = 0;
		CPU_DETAILS cpu;
		cpu.max_ratio_as_mfg = 0;
		cpu.package_power_limit = 0;
		
		U8 ACPI_COORD_TYPE = ACPI_COORD_TYPE_SW_ANY; // default
		ACPI_TABLE_SSDT *SsdtPointer = (void*)0ul;
		
		// Desired state for providing alternate ACPI _CST structure using MWAIT
		// extensions
		// 1= Alternate _CST using MWAIT extension is enabled for OSPM use
		// 0= Alternate _CST using MWAIT extension is disabled for OSPM use
		bool enable_mwait = 1;
		
		// (1) Setup pointers to SSDT memory location
		void * current = buffer;
		void * end = (U8 *)buffer + bufferSize;		
        
		// Confirm a valid SSDT buffer was provided
		if (!buffer)
		{
			printf("Error: Invalid Buffer Address for SSDT\n");
			return(0);
		}
		
		// Confirm a valid SSDT buffer length was provided
		if (!bufferSize)
		{
			printf("Error: Invalid Buffer Length for SSDT\n");
			return(0);
		}		
		
		if (madt_info == (void*) 0ul)
		{
			return(0);
		}
		
		if (dsdt == (void*) 0ul)
		{
			return(0);
		}
		
		// Check that we have a valid cpu_map (if it's not already done, it will try to generate it)
		if (generate_cpu_map_from_acpi(dsdt) != 0)
		{
			return(0);
		}
		
		collect_cpu_info(&cpu);
		
		if (enable_cstates && pmbase)
		{
			DBG("Building Cstate Info\n");
            
			cstates_enabled = BuildCstateInfo(&cpu, pmbase);
			if (cstates_enabled)
			{
				getBoolForKey(KEnableMwait, &enable_mwait, &bootInfo->chameleonConfig);
			}
		}
		
		if (enable_pstates)
		{
			DBG("Building Pstate Info\n");
            
			pstates_enabled = BuildPstateInfo(&cpu);
			if (pstates_enabled)
			{
				const char *str = getStringForKey(KAcpiCoordType, &bootInfo->chameleonConfig);
				U8 tmp  = (U8)strtoul(str, NULL,16);
				if ((tmp == ACPI_COORD_TYPE_SW_ALL) || (tmp == ACPI_COORD_TYPE_SW_ANY) || (tmp == ACPI_COORD_TYPE_HW_ALL) )
				{
					ACPI_COORD_TYPE = tmp;
				}
			}
		}
#if BUILD_ACPI_TSS
        U32 tstates_enabled = 0;
        if (enable_tstates)
		{
			DBG("Building Pstate Info\n");
            
			tstates_enabled = BuildTstateInfo(&cpu);			
		}        
#endif
		
		SsdtPointer = (ACPI_TABLE_SSDT *)buffer;
		
		// (2) Create SSDT Definition Block
		// (2.1) Save pointer to SSDT package length and checksum fields
		current = buildTableHeader(current, NAMESEG("SSDT"), NAMESEG64("PPM RCM "));
		
		// Check to confirm no SSDT buffer overflow
		if ( (U8 *)current > (U8 *)end )
		{
			printf("Error: SSDT Buffer Length exceeded available space \n");
			return(0);
		}
		
		// (3) For each logical processor CPUn 
		// We will use the dsdt datas in place of madt,for the cpu(s) detection.
		// 'Cause most users use the dsdt table to change the numbers of cpu(s) that the OS and the bootloader should use,		
		// Note also that due to chameleon limit we use the same package per each cpu(s) for all objects and methods
		// (package detection for each cpu(s) is still in progress)  
		{			
			U32 lapic_index;
			for (lapic_index=0; lapic_index < cpu_map_count; lapic_index++)
			{
				// (3.1) Create scope for CPUn
				ACPI_SCOPE * scope = current;
				
				{
					DBG("Building CPU Scope\n");
					U32 cpu_namespace = (cpuNamespace == CPU_NAMESPACE_SB) ? NAMESEG("_SB_") : NAMESEG("_PR_");
					PROCESSOR_NUMBER_TO_NAMESEG * namepath = &cpu_map[lapic_index];
					current = buildCpuScope (current, cpu_namespace, namepath );
				}
				
				// Check to confirm no SSDT buffer overflow
				if ( (U8 *)current > (U8 *)end )
				{
					printf("Error: SSDT Buffer Length exceeded available space \n");
					return(0);
				}
				
				// (3.2) Create variables in CPU scope
				DBG("Creating variables in CPU scope\n");				// Build Type variable used to store PDC capabilities
				current = buildNamedDword(current, NAMESEG("TYPE"), 0);
				
				// Build PSEN variable used to store state of P-State Enable setup option
				current = buildNamedDword(current, NAMESEG("PSEN"), pstates_enabled);
				
				// Build CSEN variable used to store state of C-State Enable setup option
				current = buildNamedDword(current, NAMESEG("CSEN"), cstates_enabled);
				
				// Build MWOS variable used to store state of MWAIT OS setup option
				current = buildNamedDword(current, NAMESEG("MWOS"), (U32)(enable_mwait&&cpu.mwait_supported));
                
				// (3.3) Create _OSC and/or _PDC Methods
				{
					// Build _PDC method
					DBG("Building PDC method\n");
					current = buildPDC(current);
					
					// Check to confirm no SSDT buffer overflow
					if ( (U8 *)current > (U8 *)end )
					{
						printf("Error: SSDT Buffer Length exceeded available space \n");
						return(0);
					}
					
					// Build _OSC method
					DBG("Building _OSC method\n");
					current = buildOSC(current);
					
					// Check to confirm no SSDT buffer overflow
					if ( (U8 *)current > (U8 *)end )
					{
						printf("Error: SSDT Buffer Length exceeded available space \n");
						return(0);
					}
				}
				
				// (3.4) Create P-state related structures
				if (pstates_enabled == 1)
				{
					// (3.4.1) Create _PSS Method
					{
						DBG("Building _PSS method\n");
						PKG_PSTATES * pkg_pstates = &cpu.pkg_pstates;
						current = buildPSS(current, pkg_pstates);
					}
					
					// Check to confirm no SSDT buffer overflow
					if ( (U8 *)(current) > (U8 *)end )
					{
						printf("Error: SSDT Buffer Length exceeded available space \n");
						return(0);
					}
					
					// (3.4.2) Create _PCT Object
					DBG("Building _PCT Object\n");
					current = buildPCT(current);
					
					// Check to confirm no SSDT buffer overflow
					if ( (U8 *)(current) > (U8 *)end )
					{
						printf("Error: SSDT Buffer Length exceeded available space \n");
						return(0);
					}
					
					// (3.4.3) Create _PPC Method
					DBG("Building _PPC Method\n");
					current = buildPPC(current);
					
					// Check to confirm no SSDT buffer overflow
					if ( (U8 *)(current) > (U8 *)end )
					{
						printf("Error: SSDT Buffer Length exceeded available space \n");
						return(0);
					}
					
					// (3.4.4) Create PSD with hardware coordination
					{
						DBG("Building _PSD Method\n");
						U32 domain = madt_info->lapic[lapic_index].pkg_index;
                        
                        // In this (bad?) implementation we use the nb of cpu found in the dsdt
						U32 cpusInDomain = cpu_map_count;							
						current = buildPSD(current, domain, cpusInDomain, ACPI_COORD_TYPE);
					}
					
					// Check to confirm no SSDT buffer overflow
					if ( (U8 *)(current) > (U8 *)end )
					{
						printf("Error: SSDT Buffer Length exceeded available space \n");
						return(0);
					}
				}
                
				// (3.5) Create C-state related structures
				if (cstates_enabled == 1)
				{
					{
						PKG_CSTATES * mwait_pkg_cstates = &cpu.pkg_mwait_cstates;
						PKG_CSTATES * io_pkg_cstates = &cpu.pkg_io_cstates;
						
						// Build CST
						DBG("Building _CST Method\n");
						current = buildCST(current, mwait_pkg_cstates, io_pkg_cstates);						
					}
                    
#if BUILD_ACPI_CSD
                    {
                        // Use core_apic_id as domain
                        U32 domain = lapic->core_apic_id;
                        
                        // In this (bad?) implementation we use the nb of cpu found in the dsdt                        
                        U32 cpusInDomain = cpu_map_count;
                        
                        // Create CSD
                        current = buildCSD(current, domain, cpusInDomain, io_pkg_cstates);
                    }
#endif
					
					// Check to confirm no SSDT buffer overflow
					if ( (U8 *)(current) > (U8 *)end )
					{
						printf("Error: SSDT Buffer Length exceeded available space \n");
						return(0);
					}
				}
#if BUILD_ACPI_TSS				
                // (3.6) Create T-state related structures
                if (tstates_enabled == 1)
                {
                    // (3.6.1) Create _TPC Method
                    current = buildTPC(current);
                    
                    // (3.6.2) Create _PTC Method
                    current = buildPTC(current);
                    
                    // (3.6.3) Create _TSS Method
                    {
                        PKG_TSTATES * pkg_tstates  = &cpu.pkg_tstates;                        
                        current = buildTSS(current, pkg_tstates);
                    }
                    
                    // (3.6.4) Create _TSD Method
                    {
                        LAPIC_INFO * lapic = &madt_info.lapic[lapic_index];
                        
                        // Use core_apic_id as domain
                        U32 domain = lapic->core_apic_id;
                        
                        // In this (bad?) implementation we use the nb of cpu found in the dsdt                        
                        U32 cpusInDomain = cpu_map_count;
                        
                        current = buildTSD(current, domain, cpusInDomain);
                    }
                }
#endif		
				// (3.7) Update length in CPUn Scope
				setPackageLength(&scope->pkgLength, (U8 *)current - (U8 *)&scope->pkgLength);
				
			} // End for
			
			// (4) Update length and checksum in SSDT Definition Block
			{
				DBG("Updating length and checksum in SSDT Definition Block\n");
                
				SsdtPointer->Header.Length = (U8 *)current - (U8 *)SsdtPointer;				
				SetChecksum(&SsdtPointer->Header);
			}
			
			// Check to confirm no SSDT buffer overflow
			if ( (U8 *)current > (U8 *)end )
			{
				printf("Error: SSDT Buffer Length exceeded available space \n");
				return(0);
			}
			
		} // End build SSDT
		
	} // SSDT
	
	return(1);
}

#if UNUSED
static ACPI_TABLE_FACS* generate_facs(bool updatefacs )
{
    ACPI_TABLE_FACS* facs_mod=(ACPI_TABLE_FACS *)AllocateKernelMemory(sizeof(ACPI_TABLE_FACS));
	if (!facs_mod) 
	{
		printf("Unable to allocate kernel memory for facs mod\n");
		return (void*)0ul;
	}
    bzero(facs_mod, sizeof(ACPI_TABLE_FACS));
	
	ACPI_TABLE_FACS * FacsPointer =(acpi_tables.FacsPointer64 != (void *)0ul) ? 
	(ACPI_TABLE_FACS *)acpi_tables.FacsPointer64 : (ACPI_TABLE_FACS *)acpi_tables.FacsPointer;
	
    memcpy(facs_mod, FacsPointer , FacsPointer->Length);
    facs_mod->Length = sizeof(ACPI_TABLE_FACS);
    
	if (FacsPointer->Length < sizeof(ACPI_TABLE_FACS))
	{
		facs_mod->FirmwareWakingVector = 0;
		facs_mod->GlobalLock = 0;
		facs_mod->Flags = 0;
	}		
    
    if (updatefacs && FacsPointer->Version < 2)
	{
		if (FacsPointer->Version > 0)
		{
			facs_mod->XFirmwareWakingVector = FacsPointer->XFirmwareWakingVector;
		} 
		else
		{
			facs_mod->XFirmwareWakingVector = (U64)facs_mod->FirmwareWakingVector;
		}
        
		facs_mod->Version = 2; /* ACPI 1.0: 0, ACPI 2.0/3.0: 1, ACPI 4.0: 2 */
		
	}
    
    return facs_mod;
}
#endif

static ACPI_GENERIC_ADDRESS FillGASStruct(U32 Address, U8 Length)
{
	ACPI_GENERIC_ADDRESS TmpGAS;
	
	TmpGAS.SpaceId = 1; /* I/O Address */
	
	if (Address == 0)
	{
		TmpGAS.BitWidth = 0;
	} 
	else
	{
		TmpGAS.BitWidth = Length * 8;
	}
	
	TmpGAS.BitOffset = 0;
	TmpGAS.AccessWidth = 0; /* Not set for Legacy reasons... */
	TmpGAS.Address = (U64)Address;
	
	return (TmpGAS);
}

static ACPI_TABLE_FADT *
patch_fadt(ACPI_TABLE_FADT *fadt, ACPI_TABLE_DSDT *new_dsdt, bool UpdateFADT)
{		
	ACPI_TABLE_FADT *fadt_mod = (void*)0;
	bool fadt_rev2_needed = false;
	bool fix_restart = false;	
	const char * value;	
    
	// Restart Fix
	if (Platform.CPU.Vendor == CPUID_VENDOR_INTEL) 
	{	
		fix_restart = true;
		getBoolForKey(kRestartFix, &fix_restart, &bootInfo->chameleonConfig);
		
	} else {
		verbose ("Not an Intel platform: Restart Fix disabled !!!\n");
	}
	
	if (fix_restart)
		fadt_rev2_needed = true;
    
	// Allocate new fadt table
	if (UpdateFADT)        
	{      
        if (fadt->Header.Length < 0xF4)
        {
            fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(0xF4);
            if (!fadt_mod) 
            {
                printf("Unable to allocate kernel memory for fadt mod\n");
                return (void*)0ul;
            }
            bzero(fadt_mod, 0xF4);
            memcpy(fadt_mod, fadt, fadt->Header.Length);
            fadt_mod->Header.Length = 0xF4;
            
		}
		else
		{			
			fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(fadt->Header.Length);
			if (!fadt_mod) 
			{
				printf("Unable to allocate kernel memory for fadt mod\n");
				return (void*)0ul;
			}
			memcpy(fadt_mod, fadt, fadt->Header.Length);
		}		   
        
        
		//fadt_mod->Header.Revision = 0x04; // FADT rev 4
		fadt_mod->ResetRegister = FillGASStruct(0, 0);
		fadt_mod->ResetValue = 0;
		fadt_mod->Reserved4[0] = 0;
		fadt_mod->Reserved4[1] = 0;
		fadt_mod->Reserved4[2] = 0;
        
        fadt_mod->XPm1aEventBlock = FillGASStruct(fadt_mod->Pm1aEventBlock, fadt_mod->Pm1EventLength);
		fadt_mod->XPm1bEventBlock = FillGASStruct(fadt_mod->Pm1bEventBlock, fadt_mod->Pm1EventLength);
		fadt_mod->XPm1aControlBlock = FillGASStruct(fadt_mod->Pm1aControlBlock, fadt_mod->Pm1ControlLength);
		fadt_mod->XPm1bControlBlock = FillGASStruct(fadt_mod->Pm1bControlBlock, fadt_mod->Pm1ControlLength);
		fadt_mod->XPm2ControlBlock = FillGASStruct(fadt_mod->Pm2ControlBlock, fadt_mod->Pm2ControlLength);
		fadt_mod->XPmTimerBlock = FillGASStruct(fadt_mod->PmTimerBlock, fadt_mod->PmTimerLength);
		fadt_mod->XGpe0Block = FillGASStruct(fadt_mod->Gpe0Block, fadt_mod->Gpe0BlockLength);
		fadt_mod->XGpe1Block = FillGASStruct(fadt_mod->Gpe1Block, fadt_mod->Gpe1BlockLength);	
        if (fadt->Header.Revision < 4)
		{					
			fadt_mod->Header.Revision = 0x04; // FADT rev 4
			verbose("Converted ACPI V%d FADT to ACPI V4 FADT\n", fadt->Header.Revision);
            
		}
	}
	else
	{
		
		if (fadt_rev2_needed)
		{
			if (fadt->Header.Length < 0x84 )
			{
				fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(0x84);
				if (!fadt_mod) 
				{
					printf("Unable to allocate kernel memory for fadt mod\n");
					return (void*)0ul;
				}
				bzero(fadt_mod, 0x84);
				memcpy(fadt_mod, fadt, fadt->Header.Length);
				fadt_mod->Header.Length   = 0x84;
			}
			else
			{
				fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(fadt->Header.Length);
				if (!fadt_mod) 
				{
					printf("Unable to allocate kernel memory for fadt mod\n");
					return (void*)0ul;
				}
				memcpy(fadt_mod, fadt, fadt->Header.Length);
			}			
			
			if (fadt->Header.Revision < 2) 
			{					
				fadt_mod->Header.Revision = 0x02; // FADT rev 2 (ACPI 1.0B MS extensions) 
				verbose("Converted ACPI V%d FADT to ACPI V2 FADT\n", fadt->Header.Revision );
			}
		} 
		else
		{
			if (fadt->Header.Length < 0x74 )
			{
				fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(0x74);
				if (!fadt_mod) 
				{
					printf("Unable to allocate kernel memory for fadt mod\n");
					return (void*)0ul;
				}
				bzero(fadt_mod, 0x74);
				memcpy(fadt_mod, fadt, fadt->Header.Length);
				fadt_mod->Header.Length   = 0x74;
				fadt_mod->Header.Revision = 0x01; 
				verbose("Warning: ACPI FADT length was < 0x74 which is the minimum for the ACPI FADT V1 specification, \n", fadt->Header.Revision );
				verbose("         trying to convert it to Version 1. \n");				
                
			} 
			else
			{
				fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(fadt->Header.Length);
				if (!fadt_mod) 
				{
					printf("Unable to allocate kernel memory for fadt mod\n");
					return (void*)0ul;
				}
				memcpy(fadt_mod, fadt, fadt->Header.Length);
			}
		}		 		
	}
	bool intelfadtspec = true;
	U8 Type = PMProfileError;
	// Determine system type / PM_Model
	
	// Fix System-type if needed (should never happen)
	if (Platform.Type > MaxSupportedPMProfile)  
	{
		if(fadt_mod->PreferredProfile <= MaxSupportedPMProfile)
            Platform.Type = fadt_mod->PreferredProfile;// get the fadt if correct
		else 
			Platform.Type = 1;		/* Set a fixed value (Desktop) */
	}
	
	// If needed, set System-type from PM_Profile (if valid) else set PM_Profile with a fixed the System-type  
	// Give prior to the FADT pm profile, allow to also control this value with a patched FADT table
	if (fadt_mod->PreferredProfile != Platform.Type) 
	{
		bool val = false;  
		getBoolForKey("PreferInternalProfileDetect", &val, &bootInfo->chameleonConfig); // if true Give prior to the profile resolved trought the CPU model
		
		//val = get_env(envIsServer) ;
		
		if (fadt_mod->PreferredProfile <= MaxSupportedPMProfile && !val)
		{
			Platform.Type = fadt_mod->PreferredProfile;
		} 
		else
		{
			fadt_mod->PreferredProfile = (U8)Platform.Type;
		}		
		
	}
	
	// Set PM_Profile and System-type if user wanted this value to be forced
	if ( (value=getStringForKey("SystemType", &bootInfo->chameleonConfig))!=NULL)
	{
		if ((Type = (unsigned char) strtoul(value, NULL, 10) ) <= MaxSupportedPMProfile)
		{
			if (fadt_mod->PreferredProfile != Type)
			{
				verbose("FADT: changing Preferred_PM_Profile from %d to %d\n", fadt->PreferredProfile, Type);
				
				Platform.Type = (fadt_mod->PreferredProfile = Type);
			} 
			else
			{
				DBG("FADT: Preferred_PM_Profile was already set to %d, no need to be changed\n",Type);
			}
			
		} else printf("Error: system-type must be 0..6. Defaulting to %d !\n", (U8)Platform.Type);
	}		
	
	getBoolForKey(KIntelFADT, &intelfadtspec, &bootInfo->chameleonConfig);
	if ((pmbase == 0) && (cpu_map_error == 0) && (intelfadtspec == true)) 
	{
		ACPI_TABLE_DSDT *DsdtPointer ;
		if (new_dsdt != (void*)0ul) 
			DsdtPointer = new_dsdt;
		else if ((fadt_mod->Header.Revision >= 3) && (fadt_mod->XDsdt != 0ul))
			DsdtPointer = (ACPI_TABLE_DSDT *)((U32)fadt_mod->XDsdt);
		else
			DsdtPointer = (ACPI_TABLE_DSDT *)fadt_mod->Dsdt;
		
		generate_cpu_map_from_acpi(DsdtPointer);
	}	
	
	// Patch DSDT Address if we have loaded a DSDT table
	if(new_dsdt != (void*)0ul)		
		fadt_mod->Dsdt=(U32)new_dsdt;	
	
	fadt_mod->Facs= fadt->Facs;
	//fadt_mod->Facs=(U32)generate_facs(false);
    
	// Patch FADT to fix restart
	if (fadt_mod->Header.Revision >= 2 && fix_restart)
	{		
        fadt_mod->Flags|= 0x400;		
		
		int type = PCI_RESET_TYPE;
		getIntForKey(KResetType, &type, &bootInfo->chameleonConfig);
		if (type == KEYBOARD_RESET_TYPE)
		{
			//Azi: keyboard reset; http://forum.voodooprojects.org/index.php/topic,1056.msg9802.html#msg9802
			fadt_mod->ResetRegister = FillGASStruct(0x64, 1);
			fadt_mod->ResetValue = 0xfe;
		} 
		else
		{
			fadt_mod->ResetRegister = FillGASStruct(0x0cf9, 1);
			fadt_mod->ResetValue = 0x06;
		}
		verbose("FADT: Restart Fix applied (Type : %s) !\n", (type == 0) ? "PCI": "KEYBOARD");
	}
    
    if (fadt_mod->Header.Revision >= 3)
	{                
        
        
        if (UpdateFADT)
		{  
            
			//fadt_mod->XFacs= (U64)((U32)generate_facs(true));
            fadt_mod->XFacs=(U64)fadt->Facs;             
            
        } 
		else
		{
			fadt_mod->XFacs=(U64)fadt->XFacs;
		}
        
        
        if(new_dsdt != (void*)0ul)
            fadt_mod->XDsdt=((U64)(U32)new_dsdt);
		else if (UpdateFADT)
            fadt_mod->XDsdt=(U64)fadt_mod->Dsdt;
        
        
		//safe_set_env(envHardwareSignature,((ACPI_TABLE_FACS *)((U32)fadt_mod->XFacs))->HardwareSignature);
        
        
    }
#if 0
	else
	{
        
		safe_set_env(envHardwareSignature,((ACPI_TABLE_FACS *)fadt_mod->Facs)->HardwareSignature);
        
    } 
	
    
	DBG("setting hardware_signature to %x \n",(U32)get_env(envHardwareSignature));    
#endif

    
    
	if (pmbase && (intelfadtspec == true))
		ProcessFadt(fadt_mod, pmbase); // The checksum correction will be done by ProcessFadt
	else
		SetChecksum(&fadt_mod->Header); // Correct the checksum
    
	return fadt_mod;
}

static U32 process_xsdt (ACPI_TABLE_RSDP *rsdp_mod , U32 *new_table_list)
{
	TagPtr DropTables_p = 0;        
    int DropTables_tag_count = 0;
    
    if (bootInfo->chameleonConfig.dictionary) 
    {
        DropTables_p = XMLCastDict(XMLGetProperty(bootInfo->chameleonConfig.dictionary, (const char*)"ACPIDropTables"));
        if (DropTables_p) DropTables_tag_count = XMLTagCount(DropTables_p) ;
    }   
    
	U32 new_table = 0ul;
	U8 new_table_index = 0, table_added = 0;
	ACPI_TABLE_XSDT *xsdt = (void*)0ul, *xsdt_mod = (void*)0ul;
	ACPI_TABLE_RSDT *rsdt_conv	= (void *)0ul;
	
	// FIXME: handle 64-bit address correctly
	
	xsdt=(ACPI_TABLE_XSDT *)acpi_tables.XsdtPointer;
	
	verbose("* Processing XSDT: \n");
	
	DBG("  XSDT @%x, Length=%d\n", (U32)xsdt,
		xsdt->Header.Length);
	
	if (xsdt != (void *)0ul)
	{				
		U32 dropoffset=0, index;
		table_added = 0;
		
		xsdt_mod=(ACPI_TABLE_XSDT *)AllocateKernelMemory(xsdt->Header.Length);
		if (!xsdt_mod) 
		{
			printf("Unable to allocate kernel memory for xsdt mod\n");
			return (0);
		}
		bzero(xsdt_mod, xsdt->Header.Length);
		memcpy(&xsdt_mod->Header, &xsdt->Header, sizeof(ACPI_TABLE_HEADER));
		
		U32 num_tables=get_num_tables64(xsdt);
		
		for (index = 0; index < num_tables; index++)
		{
			
			U64 ptr = xsdt->TableOffsetEntry[index];
			
			{				
				if (ptr > ULONG_MAX)
				{
#if DEBUG_ACPI						
					printf("Warning xsdt->TableOffsetEntry[%d]: Beyond addressable memory in this CPU mode, ignored !!!\n",index);
#endif
					continue;	
				}
				
				int method = 0;
				getIntForKey(kAcpiMethod, &method, &bootInfo->chameleonConfig);
				
				
				if (method != 0x2)
				{
					if (GetChecksum(((ACPI_TABLE_HEADER *) (unsigned long)ptr), 
									((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Length) != 0)
					{
#if DEBUG_ACPI						
						printf("Warning xsdt->TableOffsetEntry[%d]: Invalide checksum, ignored !!!\n",index);
#endif
						continue;
					}
				}
                
			}
			
			xsdt_mod->TableOffsetEntry[index-dropoffset]=ptr;
			
			char tableSig[5];
			strlcpy(tableSig, (char*)((U32)ptr), sizeof(tableSig));
            
			DBG("** Processing %s,", tableSig );
			
			DBG(" @%x, Length=%d\n", (U32)ptr,
				((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Length);
			
			{
				bool oem = false;
				char oemOption[OEMOPT_SIZE];
				sprintf(oemOption, "oem%s",tableSig );
				if (getBoolForKey(oemOption, &oem, &bootInfo->chameleonConfig) && oem) // This method don't work for DSDT and FACS
				{ 
					
					DBG("   %s required\n", oemOption);
					
					if (get_new_table_in_list(new_table_list,(*(U32 *) ((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Signature), &new_table_index) != (void*)0ul) 
						new_table_list[new_table_index] = 0ul; // This way new table will not be added to the new rsdt list !!
					
					continue;
				}
			}
			
            if ((DropTables_tag_count > 0) && DropTables_p) 
			{
				TagPtr match_drop = XMLGetProperty(DropTables_p, (const char*)tableSig);                    
				if ( match_drop ) 
				{
					char *tmp = XMLCastString(match_drop);
					if (tmp && (strcmp(tmp,"No") != 0))
					{
						dropoffset++;
						DBG("   %s table dropped\n",tableSig);
						continue;
					}
				}			
			}
			
			{
				if ((new_table = (U32)get_new_table_in_list(new_table_list,(*(U32 *) ((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Signature), &new_table_index)) != 0ul)
				{
					DBG("   Found replacement for table %s\n",tableSig);
					xsdt_mod->TableOffsetEntry[index-dropoffset]=(U64)new_table;
					new_table_list[new_table_index] = 0ul; // table replaced !!
					continue;
				}
			}							
            
		}                
		
		
		{
			U8 i;
			for (i = 0; i< (MAX_ACPI_TABLE + RESERVED_AERA); i++)
			{
				if (new_table_list[i] != 0ul)
				{
#if DEBUG_ACPI
					ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) new_table_list;
					printf("Adding table : ");
					print_nameseg(*(U32 *) (table_array[i]->Signature));
					printf("\n");					
#endif
					xsdt_mod->TableOffsetEntry[index-dropoffset]=(U64)new_table_list[i];
					table_added++;
					index++;
				}
			}
		}
		
		// Correct the checksum of XSDT
		xsdt_mod->Header.Length-=8*dropoffset;
		xsdt_mod->Header.Length+=8*table_added;
		
		SetChecksum(&xsdt_mod->Header);
		
		update_rsdp_with_xsdt(rsdp_mod, xsdt_mod);
		
		verbose("* Creating new RSDT from XSDT table\n");
		
		rsdt_conv = (ACPI_TABLE_RSDT *)gen_alloc_rsdt_from_xsdt(xsdt_mod);
		
		if (rsdt_conv != (void*)0ul)
		{
#if DEBUG_ACPI
			DBG("Attempting to update RSDP with RSDT \n");
			{
				U32 ret = update_rsdp_with_rsdt(rsdp_mod, rsdt_conv);
				if (ret)
					DBG("RSDP update with RSDT successfully !!! \n");
			}
#else
			update_rsdp_with_rsdt(rsdp_mod, rsdt_conv);
#endif
		}		
		
	}
	else
	{				
		DBG("About to drop XSDT\n");
		
		/*FIXME: Now we just hope that if MacOS doesn't find XSDT it reverts to RSDT. 
		 * A Better strategy would be to generate
		 */
		
		rsdp_mod->XsdtPhysicalAddress=0xffffffffffffffffLL;
		verbose("XSDT not found or XSDT incorrect\n");
	}
	return (1);
    
}

static U32 process_rsdt(ACPI_TABLE_RSDP *rsdp_mod , bool gen_xsdt, U32 *new_table_list)
{			
	TagPtr DropTables_p = 0;        
    int DropTables_tag_count = 0;
    
    if (bootInfo->chameleonConfig.dictionary) 
    {
        DropTables_p = XMLCastDict(XMLGetProperty(bootInfo->chameleonConfig.dictionary, (const char*)"ACPIDropTables"));
        if (DropTables_p) DropTables_tag_count = XMLTagCount(DropTables_p) ;
    }    
   
	U32 new_table = 0ul;
	U8 new_table_index = 0, table_added = 0;
	U32 dropoffset=0, index;
	ACPI_TABLE_RSDT *rsdt		 = (void *)0ul, *rsdt_mod	= (void *)0ul;
	ACPI_TABLE_XSDT *xsdt_conv	 = (void *)0ul;
	
	rsdt=(ACPI_TABLE_RSDT *)acpi_tables.RsdtPointer;
    
	rsdt_mod=(ACPI_TABLE_RSDT *)AllocateKernelMemory(rsdt->Header.Length);
	
	if (!rsdt_mod) 
	{
		printf("Unable to allocate kernel memory for rsdt mod\n");
		return (0);
	}
	
	bzero(rsdt_mod, rsdt->Header.Length);
	memcpy (&rsdt_mod->Header, &rsdt->Header, sizeof(ACPI_TABLE_HEADER));
	
    // Compute number of table pointers included in RSDT
	U32 num_tables = get_num_tables(rsdt);                        
	
	verbose("* Processing RSDT: \n");
	
	DBG("  RSDT @%x, Length %d\n",rsdt, rsdt->Header.Length);
	
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) rsdt->TableOffsetEntry;
	
	for (index = 0; index < num_tables; index++)
	{
        
		{			
			
			int method = 0;
			getIntForKey(kAcpiMethod, &method, &bootInfo->chameleonConfig);
			
			
			if (method != 0x2)
			{
				if (GetChecksum(table_array[index], table_array[index]->Length) != 0)
				{
#if DEBUG_ACPI						
					printf("Warning rsdt->TableOffsetEntry[%d]: Invalide checksum, ignored !!!\n",index);
#endif
					continue;
				}
			}			
			
		}
		
		rsdt_mod->TableOffsetEntry[index-dropoffset]=rsdt->TableOffsetEntry[index];
				
		char tableSig[5];
		strlcpy(tableSig, (char*)(rsdt->TableOffsetEntry[index]), sizeof(tableSig));
		
		DBG("** Processing %s,", tableSig );
		
		DBG(" @%x, Length=%d\n", (U32)table_array[index],
			table_array[index]->Length);
		
		{
			bool oem = false;
			char oemOption[OEMOPT_SIZE];
			sprintf(oemOption, "oem%s",tableSig );
			if (getBoolForKey(oemOption, &oem, &bootInfo->chameleonConfig) && oem) // This method don't work for DSDT and FACS
			{ 
				DBG("   %s required\n", oemOption);
				
				if (get_new_table_in_list(new_table_list,(*(U32 *) (table_array[index]->Signature)), &new_table_index) != (void*)0ul ) 
					new_table_list[new_table_index] = 0ul; // This way new table will not be added to the new rsdt list !!
				
				continue;
			}
		}
        
        if ((DropTables_tag_count > 0) && DropTables_p)
		{
			TagPtr match_drop = XMLGetProperty(DropTables_p, (const char*)tableSig);
			if ( match_drop )
			{
				char *tmp = XMLCastString(match_drop);
				if (strcmp(tmp,"No") != 0)
				{						
					dropoffset++;
					DBG("   %s table dropped\n",tableSig);
					continue;
				}
			}
		}
		
		{
			if ((new_table = (U32)get_new_table_in_list(new_table_list,(*(U32 *) (table_array[index]->Signature)), &new_table_index)) != 0ul)
			{			
				DBG("   Found replacement for table %s\n",tableSig);
				
				rsdt_mod->TableOffsetEntry[index-dropoffset]=new_table;
				new_table_list[new_table_index] = 0ul; // table replaced !!
				continue;
			}
		}			
		
	}			
	DBG("\n");
	
	{
		U8 i;
		for (i = 0; i< (MAX_ACPI_TABLE + RESERVED_AERA); i++)
		{
			if (new_table_list[i] != 0ul)
			{
#if DEBUG_ACPI
				ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) new_table_list;
				printf("Adding table : ");
				print_nameseg(*(U32 *) (table_array[i]->Signature));
				printf("\n");				
#endif
				rsdt_mod->TableOffsetEntry[index-dropoffset]=new_table_list[i];
				table_added++;
				index++;
			}
		}
	}		
	
	// Correct the checksum of RSDT
	rsdt_mod->Header.Length-=4*dropoffset;
	rsdt_mod->Header.Length+=4*table_added;		
	
	DBG("RSDT: Original checksum %d\n", rsdt_mod->Header.Checksum);			
	
	SetChecksum(&rsdt_mod->Header);
	
	DBG("New checksum %d at %x\n", rsdt_mod->Header.Checksum,rsdt_mod);
	
	update_rsdp_with_rsdt(rsdp_mod, rsdt_mod);
	
	if (gen_xsdt)
	{
		verbose("* Creating new XSDT from RSDT table\n");
		xsdt_conv = (ACPI_TABLE_XSDT *)gen_alloc_xsdt_from_rsdt(rsdt_mod);
		
		if (xsdt_conv != (void *)0ul )
		{
#if DEBUG_ACPI
			DBG("Attempting to update RSDP with XSDT \n");
			{
				U32 ret = update_rsdp_with_xsdt(rsdp_mod, xsdt_conv);
				if (ret)
					DBG("RSDP update with XSDT successfully !!! \n");
			}
#else
			update_rsdp_with_xsdt(rsdp_mod, xsdt_conv);
#endif
            
		}
		
	}	
	return (1);
}

EFI_STATUS setup_Acpi(void)
{	
	U8 Revision = 0;
	
    cpu_map_error = 0;
    cpu_map_count = 0;
    pmbase = 0;
    
    EFI_STATUS Status = EFI_ABORTED;
	
	U32 new_table_list[MAX_ACPI_TABLE + RESERVED_AERA]; //max table + reserved aera 
	U8 new_table_index = 0;
	
	ACPI_TABLE_DSDT* DsdtPtr	 = (void *)0ul; // a Pointer to the dsdt table present in fadt_mod
	
	ACPI_TABLE_DSDT *new_dsdt	 = (void *)0ul;	// a Pointer to the dsdt file	
	ACPI_TABLE_FADT *fadt_mod	 = (void *)0ul; // a Pointer to the patched FACP table
	ACPI_TABLE_FADT *fadt_file	 = (void *)0ul; // a Pointer to the (non-patched) fadt file 
	ACPI_TABLE_FADT *FacpPointer = (void *)0ul; // a Pointer to the non-patched FACP table, it can be a file or the FACP table found in the RSDT/XSDT
	ACPI_TABLE_RSDP *rsdp_mod	 = (void *)0ul, *rsdp_conv	= (void *)0ul;
    
	U32 rsdplength;
	
	bool update_acpi=false, gen_xsdt=false;
	
	bool gen_csta=false, gen_psta=false, speed_step=false;
	bool gen_ssdt=false; // will force to generate ssdt even if gen_csta and gen_psta = false
    bool gen_tsta=false;
	bool oem_dsdt=false, oem_fadt=false;
	
	// Find original rsdp        
	if (!FindAcpiTables(&acpi_tables))
	{
		printf("Error: AcpiCodec Failed to detect ACPI tables.\n");
		getchar();
		return EFI_NOT_FOUND;
	}    

	{
		U8 i;
		
		for (i=0; i<(MAX_ACPI_TABLE + RESERVED_AERA); i++)
		{
			new_table_list[i] = 0ul;
		}
		bool tmpval;
		
		oem_dsdt=getBoolForKey(kOEMDSDT, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
		oem_fadt=getBoolForKey(kOEMFADT, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
        
		gen_csta=getBoolForKey(kGenerateCStates, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
		gen_psta=getBoolForKey(kGeneratePStates, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
		gen_ssdt=getBoolForKey(KForceSSDT, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
		update_acpi=getBoolForKey(kUpdateACPI, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
		
		speed_step=getBoolForKey(kSpeedstep, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
		turbo_enabled=(U32)getBoolForKey(kCoreTurbo, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
#if BUILD_ACPI_TSS 
		gen_tsta=(U32)getBoolForKey(kGenerateTStates, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
#endif
		checkOem=getBoolForKey(kOnlySignedAml, &tmpval, &bootInfo->chameleonConfig)&&tmpval;
	} 
    
	{
        long         ret, length, flags, time;
        long long	 index = 0;
        const char * name;
        
		U8 i = 0;
		char dirspec[512];
		bool acpidir_found = false;
		
		ret = GetFileInfo("rd(0,0)/Extra/", "Acpi", &flags, &time);
        if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory)) 
		{
            sprintf(dirspec, "rd(0,0)/Extra/Acpi/");
            acpidir_found = true;
            
        }
		else
		{
			
            ret = GetFileInfo("/Extra/", "Acpi", &flags, &time);
            if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory))
			{
                sprintf(dirspec, "/Extra/Acpi/");
                acpidir_found = true;
				
            }
			else
			{
                ret = GetFileInfo("bt(0,0)/Extra/", "Acpi", &flags, &time);
                if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory))
				{
                    sprintf(dirspec, "bt(0,0)/Extra/Acpi/");
                    acpidir_found = true;
					
                } 
            }
        }
        
		if (acpidir_found == true)
		{
#if ACPISGN
            if (checkOem == true)
            {
                MakeAcpiSgn();
            }
#endif
            
            while (1) {
                ret = GetDirEntry(dirspec, &index, &name, &flags, &time);
                if (ret == -1) break;
#if DEBUG_ACPI
                printf("testing %s\n", name);
#endif
                // Make sure this is a directory.
                if ((flags & kFileTypeMask) == kFileTypeDirectory) continue;
                
                // Make sure this is a kext.
                length = strlen(name);
                if (strcmp(name + length - 4, ".aml"))
                {
#if DEBUG_ACPI
                    printf("Ignoring %s\n", name);
#endif
                    continue;
                }
                
                // Some simple verifications to save time in case of those tables simply named as follow:
                if ((strncmp(name, "RSDT", 4) == 0) || (strncmp(name, "rsdt", 4) == 0) ||
                    (strncmp(name, "XSDT", 4) == 0) || (strncmp(name, "xsdt", 4) == 0) ||
                    (strncmp(name, "RSDP", 4) == 0) || (strncmp(name, "rsdp", 4) == 0))
                { 
#if DEBUG_ACPI
                    printf("Ignoring %s\n", name);
#endif
                    continue;
                }                    
                
                if ((strncmp(name, "FACS", 4) == 0) || (strncmp(name, "facs", 4) == 0)) // FACS is not supported
                { 
#if DEBUG_ACPI
                    printf("Ignoring %s\n", name);
#endif
                    continue;
                }					
                
                DBG("* Attempting to load acpi table: %s\n", name);			
                if ( (new_table_list[i]=(U32)loadACPITable(new_table_list,dirspec,name)))
                {
                    if (i < MAX_ACPI_TABLE)
                    {
                        i++;
                    } 
                    else
                    {
                        DBG("Max nb of allowed aml files reached, exiting .");
                        break;
                    }						
                }
            }
            
            if (i)
            {
                //sanitize the new tables list 
                sanitize_new_table_list(new_table_list);
                
                //move to kernel memory 
                move_table_list_to_kmem(new_table_list);
                
                DBG("New ACPI tables Loaded in memory\n");
            }          
			
			
		}
		
	}			
#if HARDCODED_DSDT
    do {
#include "dsdt_PRLSACPI.h"
        
        U8 index = 0;
        
        if ((get_new_table_in_list(new_table_list, NAMESEG("DSDT"), &new_table_index)) != (void*)0ul )
        {
            index = new_table_index;
        } 
        else
        {
            U8 empty = get_0ul_index_in_list(new_table_list, false);
            if (empty != ACPI_TABLE_LIST_FULL_NON_RESERVED)
            {
                index = empty;             
            } 
            else
            {
                printf("Error: not enought reserved space in the new acpi list for the Harcoded DSDT table,\n ");
                printf("       please increase the RESERVED_AERA\n");
                
                break;
            }
        }
        
		if (index)
		{
			
			ACPI_TABLE_DSDT *tmp = (ACPI_TABLE_DSDT *)DsdtAmlCode;			
			ACPI_TABLE_DSDT *hardcoded_dsdt	 = (void *)0ul;	
			
			hardcoded_dsdt = (ACPI_TABLE_DSDT *)AllocateKernelMemory(tmp->Header.Length);	
			memcpy(hardcoded_dsdt, tmp, tmp->Header.Length);
			new_table_list[index] = (U32)hardcoded_dsdt; // add the patched table to the list
		} 
		else
		{
			printf("Error: not enought reserved space in the new acpi list for the Harcoded DSDT table,\n ");
			printf("       please increase the RESERVED_AERA\n");
			
			break;
		}    
    } while (0);
#endif
	if (speed_step)
	{
		gen_psta= true;
		gen_csta= true;
	} 		
    
	
	ACPI_TABLE_RSDP *rsdp=(ACPI_TABLE_RSDP *)acpi_tables.RsdPointer;
	
	if (rsdp == (void*)0ul || (GetChecksum(rsdp, (rsdp->Revision == 0) ? ACPI_RSDP_REV0_SIZE:sizeof(ACPI_TABLE_RSDP)) != 0) )
	{
		printf("Error : ACPI RSD PTR Revision %d checksum is incorrect or table not found \n",rsdp->Revision );
		return EFI_UNSUPPORTED;
	}
	
	if ((update_acpi) && (rsdp->Revision == 0))
	{
		
		rsdp_conv = (ACPI_TABLE_RSDP *)gen_alloc_rsdp_v2_from_v1(rsdp);
		if (rsdp_conv != (void *)0ul)
		{
			gen_xsdt = true;            
			rsdp = rsdp_conv;
			verbose("Converted ACPI RSD PTR Revision 0 to Revision 2\n");
		}
		
	}
	
	Revision = rsdp->Revision  ;
	rsdplength=(Revision == 2)?rsdp->Length:ACPI_RSDP_REV0_SIZE;
	
	DBG("RSDP Revision %d found @%x. Length=%d\n",Revision,rsdp,rsdplength);
    
	if (gen_xsdt)
	{
		rsdp_mod=rsdp_conv;
	}
	else
	{
		rsdp_mod=(ACPI_TABLE_RSDP *) AllocateKernelMemory(rsdplength);
		
		if (!rsdp_mod) return EFI_OUT_OF_RESOURCES; 
		
		memcpy(rsdp_mod, rsdp, rsdplength);
	}	
	
    
	if ((fadt_file = (ACPI_TABLE_FADT *)get_new_table_in_list(new_table_list, NAMESEG("FACP"), &new_table_index)) != (void *)0ul)
	{
		
		if (oem_fadt == false)
			FacpPointer = (ACPI_TABLE_FADT *)fadt_file;
		
		new_table_list[new_table_index] = 0ul; // This way, the non-patched table will not be added in our new rsdt/xsdt table list
		
	} else
		FacpPointer = (acpi_tables.FacpPointer64 != (void *)0ul) ? 
		(ACPI_TABLE_FADT *)acpi_tables.FacpPointer64 : (ACPI_TABLE_FADT *)acpi_tables.FacpPointer;			
	
#if DEBUG_ACPI
	if ((FacpPointer != (void *)0ul) || (oem_fadt == false))
	{
		printf("FADT found @%x, Length %d\n",FacpPointer, FacpPointer->Header.Length);
		printf("Attempting to patch FADT entry of %s\n",(acpi_tables.FacpPointer64 != (void *)0ul) ? ACPI_SIG_XSDT : ACPI_SIG_RSDT);
	} 
	else if (oem_fadt == true)
	{
		ACPI_TABLE_FADT * FacpPtr = (acpi_tables.FacpPointer64 != (void *)0ul) ? 
		(ACPI_TABLE_FADT *)acpi_tables.FacpPointer64 : (ACPI_TABLE_FADT *)acpi_tables.FacpPointer;
		
		printf("FADT found @%x ( Length %d ) in %s \n",FacpPtr, FacpPtr->Header.Length, (acpi_tables.FacpPointer64 != (void *)0ul) ? ACPI_SIG_XSDT : ACPI_SIG_RSDT);
	}
#endif	
	
	if ((new_dsdt = (ACPI_TABLE_DSDT *)get_new_table_in_list(new_table_list, NAMESEG("DSDT"), &new_table_index)) != (void*)0ul )
	{
		new_table_list[new_table_index] = 0ul; // This way, the DSDT file will not be added in our new rsdt/xsdt table list, and it shouldn't be anyway
	}
	
	if (oem_fadt == false)
	{
		
		fadt_mod = patch_fadt(FacpPointer, (oem_dsdt == false) ? new_dsdt : (void*)0ul , (acpi_tables.FacpPointer64 != (void *)0ul ));	
        
		if (fadt_mod != (void*)0ul)
		{
			DsdtPtr = ((fadt_mod->Header.Revision >= 3) && (fadt_mod->XDsdt != 0)) ? (ACPI_TABLE_DSDT*)((U32)fadt_mod->XDsdt):(ACPI_TABLE_DSDT*)fadt_mod->Dsdt;
			
			U8 empty = get_0ul_index_in_list(new_table_list,true);
			if (empty != ACPI_TABLE_LIST_FULL)
			{
				new_table_list[empty] = (U32)fadt_mod; // add the patched table to the list
			} 
			else
			{
				printf("Error: not enought reserved space in the new acpi list for the Patched FACP table,\n ");
				printf("       please increase the RESERVED_AERA\n");
			}			
			
		} 
		else
		{
			printf("Error: Failed to patch the FADT Table, trying fallback to the FADT original pointer\n");
			fadt_mod = (acpi_tables.FacpPointer64 != (void *)0ul) ? 
			(ACPI_TABLE_FADT *)acpi_tables.FacpPointer64 : (ACPI_TABLE_FADT *)acpi_tables.FacpPointer;
			
			DsdtPtr = ((fadt_mod->Header.Revision >= 3) && (fadt_mod->XDsdt != 0)) ? (ACPI_TABLE_DSDT*)((U32)fadt_mod->XDsdt):(ACPI_TABLE_DSDT*)fadt_mod->Dsdt;
			
			U8 empty = get_0ul_index_in_list(new_table_list,true);
			if (empty != ACPI_TABLE_LIST_FULL)
			{
				new_table_list[empty] = (U32)fadt_mod; 
			} 
			else
			{
				printf("Error: not enought reserved space in the new acpi list for the FACP table,\n ");
				printf("       please increase the RESERVED_AERA\n");
			}
		}	
		
		if (oem_dsdt == false)
		{
			if (generate_cpu_map_from_acpi(DsdtPtr) == 0)
			{
				U8 new_uid = (U8)getPciRootUID();
				
				/* WARNING: THIS METHOD WORK PERFECTLY BUT IT CAN RESULT TO AN INCORRECT CHECKSUM */
				
				if (ProcessDsdt(DsdtPtr, UIDPointer, new_uid))
				{
					printf("PCI0 _UID patched to %d in the DSDT table\n", new_uid);
				}				
				
			}
		}
		
        
	} 
	else 
	{
		
		// here we use the variable fadt_mod only for SSDT Generation
		
		fadt_mod = (acpi_tables.FacpPointer64 != (void *)0ul) ? 
		(ACPI_TABLE_FADT *)acpi_tables.FacpPointer64 : (ACPI_TABLE_FADT *)acpi_tables.FacpPointer;
		
		DsdtPtr = ((fadt_mod->Header.Revision >= 3) && (fadt_mod->XDsdt != 0)) ? (ACPI_TABLE_DSDT*)((U32)fadt_mod->XDsdt)
        :(ACPI_TABLE_DSDT*)fadt_mod->Dsdt;
	}
    
    {
        MADT_INFO madt_info;
        bool strip_madt = true;
        
        getBoolForKey(kSTRIPAPIC, &strip_madt, &bootInfo->chameleonConfig);
        
        if ((strip_madt == false) || (!buildMADT(new_table_list, DsdtPtr, &madt_info ))) 
        {
            
            ACPI_TABLE_MADT * madt_file = (void*)0ul;
            ACPI_TABLE_MADT * MadtPointer = (void*)0ul;
            bool oem_apic=false;
            
            {		
                bool tmpval;		
                oem_apic=getBoolForKey(kOEMAPIC, &tmpval, &bootInfo->chameleonConfig)&&tmpval;		
            } 
            
            if ((madt_file = (ACPI_TABLE_MADT *)get_new_table_in_list(new_table_list, NAMESEG("APIC"), &new_table_index)) != (void *)0ul)
            {		
                if (oem_apic == false) 
                {
                    MadtPointer = (ACPI_TABLE_MADT *)madt_file;	                    
                }
                
            } else
                MadtPointer = (acpi_tables.MadtPointer64 != (void*)0ul) ? (ACPI_TABLE_MADT *)acpi_tables.MadtPointer64 : (ACPI_TABLE_MADT *)acpi_tables.MadtPointer;
            
            ProcessMadtInfo(MadtPointer, &madt_info);        
            
        }
        
        if (gen_ssdt || gen_csta || gen_psta || gen_tsta) 
        {
            ProcessSsdt(new_table_list, DsdtPtr, &madt_info, gen_csta, gen_psta, gen_tsta );		
        }
    }    
	
	if (rsdp_mod == (void *)0ul)
	{		
		printf("Error: rsdp_mod == null \n");
		return EFI_ABORTED;
	}
	
	if (!(rsdp_mod->Length >= ACPI_RSDP_REV0_SIZE)) 
	{
		printf("Error: rsdp_mod size is incorrect \n");
		return EFI_ABORTED;
		
	}
	
	do {
		
		if ((rsdp_mod->Revision == 0) || (gen_xsdt == true))
		{
			if (process_rsdt(rsdp_mod, gen_xsdt, new_table_list))
				break;
			printf("Error : ACPI RSD PTR Revision 1 is incorrect, \n");
		}
		
		if ((GetChecksum(rsdp_mod, sizeof(ACPI_TABLE_RSDP)) == 0) &&
			(Revision == 2) &&
			(rsdplength == sizeof(ACPI_TABLE_RSDP)))
		{
			if (process_xsdt(rsdp_mod, new_table_list))
				break;
			printf("Error : ACPI RSD PTR Revision 2 is incorrect \n");
		}		
		
		Revision = 0; // fallback to Revision 0
		
		if (process_rsdt(rsdp_mod, false, new_table_list))
			break;			
		
		printf("Error: Incorect ACPI RSD PTR or not found \n");
		return EFI_ABORTED;
		
	} while (0); 
	
	
	// Correct the checksum of RSDP      
	
	DBG("RSDP: Original checksum %d\n", rsdp_mod->Checksum);		
	
	setRsdpchecksum(rsdp_mod);
	
	DBG("New checksum %d\n", rsdp_mod->Checksum);
	
	if (Revision == 2)
	{
		DBG("RSDP: Original extended checksum %d\n", rsdp_mod->ExtendedChecksum);			
		
		setRsdpXchecksum(rsdp_mod);
		
		DBG("New extended checksum %d\n", rsdp_mod->ExtendedChecksum);
		
	}
	
	verbose("ACPI Revision %d successfully patched\n", Revision);
	
    if (Revision == 2)
	{
		/* XXX aserebln why uint32 cast if pointer is uint64 ? */
		rsd_p = (U32)rsdp_mod;
		addConfigurationTable(&gEfiAcpi20TableGuid, &rsd_p, "ACPI_20");
	}
	else
	{
		/* XXX aserebln why uint32 cast if pointer is uint64 ? */
		rsd_p = (U32)rsdp_mod;
		addConfigurationTable(&gEfiAcpiTableGuid, &rsd_p, "ACPI");
	}
	
	
#if DEBUG_ACPI==2
	printf("Press a key to continue... (DEBUG_ACPI)\n");
	getc();
#endif
	return Status;
}

int AcpiSetup(void)
{
	EFI_STATUS status = setup_Acpi();
	
	return (status == EFI_SUCCESS);
}
