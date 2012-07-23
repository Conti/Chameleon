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
#include "libsaio.h"
#include "bootstruct.h"
#include "datatype.h"
#include "intel_acpi.h"
#include "ppm.h"
#include "acpi.h"

static U32 GetRsdtPointer(void *mem_addr, U32 mem_size, ACPI_TABLES * acpi_tables);
static U32 GetXsdtPointer(ACPI_TABLES * acpi_tables);
static ACPI_TABLE_HEADER *GetTablePtr(ACPI_TABLE_RSDT * rsdt, U32 signature);
static ACPI_TABLE_HEADER *GetTablePtr64(ACPI_TABLE_XSDT * xsdt, U32 signature);

//-------------------------------------------------------------------------------
//
// Procedure:    FindAcpiTables - Collects addresses for RSDP, RSDT, FADT, & DSDT.
//
// Description: Finds the differentiated system description table pointer
//   by scanning and checking ACPI tables.  This function will
//   get and store the following ACPI Table Pointers:
//   1) RSD Pointer in RsdPointer Variable
//   2) RSDT Pointer in RsdtPointer Variable			(RSDP->RSDT)
//   3) XSDT Pointer in XsdtPointer Variable			(RSDP->XSDT)
//   4) FACP Pointer in FacpPointer Variable			(RSDP->RSDT->FACP)
//   5) FACP(64) Pointer in FacpPointer64 Variable		(RSDP->XSDT->FACP)
//   6) DSDT Pointer in DsdtPointer Variable			(RSDP->RSDT->FACP->DSDT)
//   7) DSDT(64) Pointer in DsdtPointer64 Variable		(RSDP->XSDT->FACP->XDSDT)
//   8) FACS Pointer in FacsPointer Variable			(RSDP->RSDT->FACP->FACS)
//   9) FACS(64) Pointer in FacsPointer64 Variable		(RSDP->XSDT->FACP->XFACS)
//   A) MADT Pointer in FacsPointer Variable			(RSDP->RSDT->APIC)
//   B) MADT(64) Pointer in MadtPointer64 Variable		(RSDP->XSDT->APIC)
//
//-------------------------------------------------------------------------------
U32 FindAcpiTables(ACPI_TABLES * acpi_tables)
{
    U32 success = 0ul;
    
    // Perform init of ACPI table pointers
    {
        void *null = 0ul;
        acpi_tables->DsdtPointer = null;
        acpi_tables->DsdtPointer64 = null;
        acpi_tables->FacpPointer = null;
        acpi_tables->FacsPointer = null;
        acpi_tables->FacsPointer64 = null;
        acpi_tables->RsdPointer = null;
        acpi_tables->RsdtPointer = null;
        acpi_tables->MadtPointer = null;
		acpi_tables->MadtPointer64 = null;
        acpi_tables->XsdtPointer = null;
        acpi_tables->FacpPointer64 = null;
    }
    
    // Find the RSDT pointer by scanning EBDA/E000/F000 segments.
    
    // Init memory address as EBDA and scan 1KB region
    success = GetRsdtPointer((void *)(((U32) * (U16 *) 0x40E) << 4), 0x400, acpi_tables);
    
    // Init memory address as E000 segment and scan 64KB region
    if (!success)
        success = GetRsdtPointer((void *)0x0E0000, 0x10000, acpi_tables);
    
    // Init memory address as F000 segment and scan 64KB region
    if (!success)
        success = GetRsdtPointer((void *)0x0F0000, 0x10000, acpi_tables);
	
    if (!success || (acpi_tables->RsdtPointer == 0ul))
        return (0ul);
    
    success = GetXsdtPointer(acpi_tables);
    
    // Find FACP table pointer which is one of table pointers in the RDST
    acpi_tables->FacpPointer = (ACPI_TABLE_FADT *)
    GetTablePtr(acpi_tables->RsdtPointer, NAMESEG("FACP"));
    if (acpi_tables->FacpPointer == 0ul)
        return (0ul);    
    
    // Find the DSDT which is included in the FACP table
    acpi_tables->DsdtPointer = (ACPI_TABLE_DSDT *) acpi_tables->FacpPointer->Dsdt;
    if ((acpi_tables->DsdtPointer == 0ul) || (*(U32 *) (acpi_tables->DsdtPointer->Header.Signature) != NAMESEG("DSDT")) ||
        (GetChecksum(acpi_tables->DsdtPointer, acpi_tables->DsdtPointer->Header.Length) != 0))
        return (0ul);   
    
    // Find the FACS which is included in the FACP table
    acpi_tables->FacsPointer = (ACPI_TABLE_FACS *) acpi_tables->FacpPointer->Facs;
    if ((acpi_tables->FacsPointer == 0ul) || (*(U32 *) (acpi_tables->FacsPointer->Signature) != NAMESEG("FACS")))
        return (0ul);
    
    // Find the MADT table which is one of the table pointers in the RSDT
    acpi_tables->MadtPointer = (ACPI_TABLE_MADT *) GetTablePtr(acpi_tables->RsdtPointer, NAMESEG("APIC"));
    if (acpi_tables->MadtPointer == 0ul)
        return (0ul);
    
    do {
        
        if (!success || (acpi_tables->XsdtPointer == 0ul))
            break;
        
        // Find FACP(64) table pointer which is one of table pointers in the XDST
        acpi_tables->FacpPointer64 = (ACPI_TABLE_FADT *)
        GetTablePtr64(acpi_tables->XsdtPointer, NAMESEG("FACP"));
        
        if (acpi_tables->FacpPointer64 == 0ul)
            break;
        
		// Find the XDSDT which is included in the FACP(64) table
		ACPI_TABLE_DSDT *DsdtPointer64 = (ACPI_TABLE_DSDT *)((U32)acpi_tables->FacpPointer64->XDsdt);
        
        if (DsdtPointer64 == 0ul)
            break;
        
		if ((*(U32*) (DsdtPointer64->Header.Signature) == NAMESEG("DSDT")) &&
			(GetChecksum(DsdtPointer64, DsdtPointer64->Header.Length) == 0))
			acpi_tables->DsdtPointer64 = (ACPI_TABLE_DSDT *) DsdtPointer64;
        
        // Find the XFACS which is included in the FACP(64) table
		ACPI_TABLE_FACS *FacsPointer64 = (ACPI_TABLE_FACS *)((U32)acpi_tables->FacpPointer64->XFacs);
        
        if (FacsPointer64 == 0ul)
            break;
        
		if (*(U32*) (FacsPointer64->Signature) == NAMESEG("FACS"))
			acpi_tables->FacsPointer64 = (ACPI_TABLE_FACS *) FacsPointer64;
        
        
        // Find the MADT(64) table which is one of the table pointers in the XSDT
        acpi_tables->MadtPointer64 = (ACPI_TABLE_MADT *) GetTablePtr64(acpi_tables->XsdtPointer, NAMESEG("APIC"));
        
    } while (0);	
	
	
    return (1ul);
}

//-----------------------------------------------------------------------------
U32 get_num_tables(ACPI_TABLE_RSDT * rsdt)
{
    // Compute number of table pointers included in RSDT
    return ((rsdt->Header.Length - sizeof(ACPI_TABLE_HEADER))
            / sizeof(ACPI_TABLE_HEADER *));
}

//-----------------------------------------------------------------------------
U32 get_num_tables64(ACPI_TABLE_XSDT * xsdt)
{
    {
        void *null = 0ul;
        if (xsdt == null)
            return 0ul;
    }
    
    // Compute number of table pointers included in XSDT
    return ((xsdt->Header.Length - sizeof(ACPI_TABLE_HEADER))
            / sizeof(U64));
}

//-------------------------------------------------------------------------------
//
// Procedure:    GetTablePtr - Find ACPI table in RSDT with input signature.
//
//-------------------------------------------------------------------------------
static ACPI_TABLE_HEADER *GetTablePtr(ACPI_TABLE_RSDT * rsdt, U32 signature)
{
    U32 index;
    U32 num_tables;
    ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) rsdt->TableOffsetEntry;
    
    // Compute number of table pointers included in RSDT
    num_tables = get_num_tables(rsdt);
    
    for (index = 0; index < num_tables; index++) {
        if ((*(U32 *) (table_array[index]->Signature) == signature) &&
            (GetChecksum(table_array[index], table_array[index]->Length) == 0)) {
            return (table_array[index]);
        }
    }
    return (0);
}

//-------------------------------------------------------------------------------
//
// Procedure:    GetTablePtr64 - Find ACPI table in XSDT with input signature.
//
//-------------------------------------------------------------------------------
static ACPI_TABLE_HEADER *GetTablePtr64(ACPI_TABLE_XSDT * xsdt, U32 signature)
{
    U32 index;
    U32 num_tables;
	
	int method;
	
	// Compute number of table pointers included in XSDT
    num_tables = get_num_tables64(xsdt);
	
	getIntForKey(kAcpiMethod, &method, &bootInfo->chameleonConfig);
	switch (method) {
		case 0x2:
		{
			for (index = 0; index < num_tables; index++) {
				U64 ptr = xsdt->TableOffsetEntry[index];
				
				if ((*(U32 *) ((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Signature == signature) &&
					(GetChecksum(((ACPI_TABLE_HEADER *) (unsigned long)ptr), ((ACPI_TABLE_HEADER *) (unsigned long)ptr)->Length) == 0)) {
					return (((ACPI_TABLE_HEADER *) (unsigned long)ptr));
				}        
			}
			break;
		}
		case 0x1:			
		default:
		{
			ACPI_TABLE_HEADER *table = (ACPI_TABLE_HEADER *) xsdt->TableOffsetEntry;		
			
			for (index = 0; index < num_tables; index++) {
				if (((U32) (table->Signature) == signature) &&
					(GetChecksum(table, table->Length) == 0)) {
					return (table);
				}
				// Move array pointer to next 64-bit pointer
				table = (ACPI_TABLE_HEADER *) ((U32) table + sizeof(U64));
			}
			break;
		}
	}		
    
    return (0);
}

//-------------------------------------------------------------------------------
//
// Procedure:    GetChecksum - Performs byte checksum
//
//-------------------------------------------------------------------------------
U8 GetChecksum(void *mem_addr, U32 mem_size)
{
    U8 *current = mem_addr;
    U8 *end = current + mem_size;
    U8 checksum = 0;
    
    for (; current < end; current++)
        checksum = checksum + *current;
    
    return (checksum);
}

/*==========================================================================
 * Function to map 32 bit physical address to 64 bit virtual address
 */


//-------------------------------------------------------------------------------
//
// Procedure:    GetRsdtPointer - Scans given segment for RSDT pointer
//
// Description:  Scans for root system description table pointer signature
//       ('RSD PTR ') , verifies checksum, and returns pointer to
//       RSDT table if found.
//
//-------------------------------------------------------------------------------
static U32 GetRsdtPointer(void *mem_addr, U32 mem_size, ACPI_TABLES * acpi_tables)
{
    U8 *current = mem_addr;
    U8 *end = current + mem_size;
    
    // Quick sanity check for a valid start address
    if (current == 0ul)
        return (0ul);
    
    for (; current < end; current += 16) {
        if (*(volatile U64 *)current == NAMESEG64("RSD PTR ")) {
            if (GetChecksum(current, ACPI_RSDP_REV0_SIZE) == 0) {
                // RSD pointer structure checksum okay, lookup the RSDT pointer.                
                acpi_tables->RsdPointer = (ACPI_TABLE_RSDP *)current;
                acpi_tables->RsdtPointer = (ACPI_TABLE_RSDT *) acpi_tables->RsdPointer->RsdtPhysicalAddress;
                if ((acpi_tables->RsdPointer != (void*)0ul) && (acpi_tables->RsdtPointer != (void*)0ul))
                    return (1ul);
                else
                    return (0ul);
            }
        }
    }
    
    return (0);
}

//-------------------------------------------------------------------------------
//
// Procedure:    GetXsdtPointer
//
//-------------------------------------------------------------------------------
static U32 GetXsdtPointer(ACPI_TABLES * acpi_tables)
{
    if ((GetChecksum(acpi_tables->RsdPointer, sizeof(ACPI_TABLE_RSDP)) == 0) &&
        (acpi_tables->RsdPointer->Revision == 2) &&
        (acpi_tables->RsdPointer->Length == sizeof(ACPI_TABLE_RSDP))) {
        // RSD pointer structure checksum okay, lookup the XSDT pointer.
        acpi_tables->XsdtPointer = (ACPI_TABLE_XSDT *) (U32) acpi_tables->RsdPointer->XsdtPhysicalAddress;
        return (1ul);
    }
    
    return (0ul);
}
