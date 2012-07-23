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

#include "datatype.h"
#include "acpi.h"
#include "ppm.h"
#include "acpicode.h"

static void setPackage(ACPI_PACKAGE * package, U8 numElements);
static void setNamePath(ACPI_NAME_PATH * namePath, U32 name);

void SetChecksum(struct acpi_table_header *header)
{
    header->Checksum = 0;
    header->Checksum = 0 - GetChecksum(header, header->Length);
}

void setRsdpchecksum(ACPI_TABLE_RSDP *rsdp)
{        
    rsdp->Checksum = 0;
    rsdp->Checksum = 0 - GetChecksum(rsdp, ACPI_RSDP_REV0_SIZE);
}

void setRsdpXchecksum(ACPI_TABLE_RSDP *rsdp)
{        
    rsdp->ExtendedChecksum = 0;
    rsdp->ExtendedChecksum = 0 - GetChecksum(rsdp, rsdp->Length);
}

U32 update_rsdp_with_xsdt(ACPI_TABLE_RSDP *rsdp, ACPI_TABLE_XSDT *xsdt)
{
	// 1. Update the XSDT pointer in the RSDP
	// 2. Update the Xchecksum of the RSDP
	
	{
		// 1. Update the XSDT pointer in the RSDP
		rsdp->XsdtPhysicalAddress = ((U64)((U32)xsdt));
    }
	
	{
		// 2. Update the Xchecksum of the RSDP
		setRsdpXchecksum(rsdp);
	}
	
	return (1);
}

U32 update_rsdp_with_rsdt(ACPI_TABLE_RSDP *rsdp, ACPI_TABLE_RSDT *rsdt)
{
	// 1. Update the RSDT pointer in the RSDP
	// 2. Update the checksum of the RSDP
	
	{
		// 1. Update the RSDT pointer in the RSDP
		rsdp->RsdtPhysicalAddress = (U32)rsdt;    
    }
	
	{
		// 2. Update the checksum of the RSDP
		setRsdpchecksum(rsdp);
    }
	
	return (1);
}


//-----------------------------------------------------------------------------
U32 ProcessMadtInfo(ACPI_TABLE_MADT * madt, MADT_INFO * madt_info)
{
    void *current;
    void *end;

    // Quick sanity check for a valid MADT
    if (madt == 0ul)
        return (0ul);

    madt_info->lapic_count = 0;

    // Search MADT for Sub-tables with needed data
    current = madt + 1;
    end = (U8 *) madt + madt->Header.Length;

    while (current < end)
	{
        ACPI_SUBTABLE_HEADER *subtable = current;

        switch (subtable->Type)
		{
        
			case ACPI_MADT_TYPE_LOCAL_APIC:
            {
                // Process sub-tables with Type as 0: Processor Local APIC
                ACPI_MADT_LOCAL_APIC *lapic = current;
                current = lapic + 1;

                if (!(lapic->LapicFlags & ACPI_MADT_ENABLED))
                    continue;

                {
                    LAPIC_INFO *lapic_info = &madt_info->lapic[madt_info->lapic_count];

                    lapic_info->processorId = lapic->ProcessorId;
                    lapic_info->apicId = lapic->Id;
                    lapic_info->madt_type = ACPI_MADT_TYPE_LOCAL_APIC;
                }

                madt_info->lapic_count++;

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

                {
                    LAPIC_INFO *lapic_info = &madt_info->lapic[madt_info->lapic_count];

                    lapic_info->uid = x2apic->UID;
                    lapic_info->apicId = x2apic->x2apicId;
                    lapic_info->madt_type = ACPI_MADT_TYPE_X2APIC;
                }

                madt_info->lapic_count++;

                // Sanity check to verify compile time limit for max logical CPU is not exceeded
                if (madt_info->lapic_count > MAX_LOGICAL_CPU)
                    return (0);

                break;
            }
       
			default:
            {
                // Process all other sub-tables
                current = (U8 *) subtable + subtable->Length;
                break;
            }
        } // switch

    } // while

    return (1);
}

//-------------------------------------------------------------------------------
U32 ProcessDsdt(ACPI_TABLE_DSDT * DsdtPointer, U8 * PCIUIDPointer, U8 uid)
{
    // 1. Sanity check
	// 2. Replace UID value with the new value
    // 3. Update the checksum of the DSDT

	{		
        // 1. Sanity check
        if ((memcmp(&uid, PCIUIDPointer, sizeof(U8)) == 0) || (PCIUIDPointer == (U8*)0) || (DsdtPointer == (void*)0ul)) 
										return (0);
    }
	
    {		
        // 2. Replace UID value with the new value
		buildOpCode((void*)PCIUIDPointer, uid);
    }
	    	
    {
        // 3. Update the checksum of the DSDT
		SetChecksum(&DsdtPointer->Header);       
    }
	
	return (1);
}

//-------------------------------------------------------------------------------
void MoveRsdtInsertSsdt(ACPI_TABLE_RSDP * RsdPointer, ACPI_TABLE_RSDT * OldRsdtPointer, ACPI_TABLE_RSDT * NewRsdtPointer, ACPI_TABLE_SSDT * SsdtPointer)
{
    // 1. Move the RSDT in memory to the new location
    // 2. Add new pointer for the SSDT into the RSDT
    // 3. Update the size of the RSDT
    // 4. Update the checksum of the RSDT
    // 5. Update the RSDT pointer in the RSDP
    // 6. Update the checksum of the RSDP

    {
        // 1. Move the RSDT in memory to the new location
        memcpy(NewRsdtPointer, OldRsdtPointer, OldRsdtPointer->Header.Length);
    }

    {
        // 2. Add new pointer for the SSDT into the RSDT
        // 3. Update the size of the RSDT
        // 4. Update the checksum of the RSDT
        InsertSsdt(NewRsdtPointer, SsdtPointer);
    }

    {
        // 5. Update the RSDT pointer in the RSDP
        RsdPointer->RsdtPhysicalAddress = (U32) NewRsdtPointer;
    }

    {
        // 6. Update the checksum of the RSDP
		setRsdpchecksum(RsdPointer);        
    }
}

//-------------------------------------------------------------------------------
void InsertSsdt(ACPI_TABLE_RSDT * RsdtPointer, ACPI_TABLE_SSDT * SsdtPointer)
{
    // 1. Add new pointer for the SSDT into the RSDT
    // 2. Update the size of the RSDT
    // 3. Update the checksum of the RSDT

    {
        // 1. Add new pointer for the SSDT into the RSDT
        U32 index = get_num_tables(RsdtPointer);
        RsdtPointer->TableOffsetEntry[index] = (U32) SsdtPointer;
    }

    {
        // 2. Update the size of the RSDT
        RsdtPointer->Header.Length = RsdtPointer->Header.Length + sizeof(ACPI_TABLE_SSDT *);
    }

    {
        // 3. Update the checksum of the RSDT
        SetChecksum(&RsdtPointer->Header);
    }
}

//-------------------------------------------------------------------------------
void InsertSsdt64(ACPI_TABLE_XSDT * XsdtPointer, ACPI_TABLE_SSDT * SsdtPointer)
{
    {
        void *null = 0ul;
        if (XsdtPointer == null)
            return;
    }

    // 1. Add new pointer for the SSDT into the XSDT
    // 2. Update the size of the XSDT
    // 3. Update the checksum of the XSDT

    {
        // 1. Add new pointer for the SSDT into the XSDT
        U32 index = get_num_tables64(XsdtPointer);
        XsdtPointer->TableOffsetEntry[index] = (U64) ((U32) SsdtPointer);
    }

    {
        // 2. Update the size of the XSDT
        XsdtPointer->Header.Length = XsdtPointer->Header.Length + sizeof(U64);
    }

    {
        // 3. Update the checksum of the XSDT
        SetChecksum(&XsdtPointer->Header);
    }
}

//-----------------------------------------------------------------------------
void *buildNameSeg(void *current, U32 name)
{
    U32 *nameSeg = current;
    current = nameSeg + 1;

    *nameSeg = name;

    return (current);
}

//-----------------------------------------------------------------------------
void *buildOpCode(void *current, U8 opCode)
{
    U8 *op = current;
    current = op + 1;

    *op = opCode;

    return (current);
}

//-----------------------------------------------------------------------------
void *buildReturnPackage(void *current, U8 numElements)
{
    ACPI_RETURN_PACKAGE *returnPackage = current;
    current = returnPackage + 1;

    returnPackage->returnOpcode = AML_RETURN_OP;
    setPackage(&returnPackage->package, numElements);

    return (current);
}

//-----------------------------------------------------------------------------
void *buildReturnZero(void *current)
{
    ACPI_RETURN_ZERO *returnZero = current;
    current = returnZero + 1;

    returnZero->returnOpcode = AML_RETURN_OP;
    returnZero->zeroOpcode = AML_ZERO_OP;

    return (current);
}

//-----------------------------------------------------------------------------
void *buildReturnOpcode(void *current, U8 opcodeToReturn)
{
    ACPI_RETURN_OPCODE *returnOpcode = current;
    current = returnOpcode + 1;

    returnOpcode->returnOpcode = AML_RETURN_OP;
    returnOpcode->opcodeToReturn = opcodeToReturn;

    return (current);
}

//-----------------------------------------------------------------------------
void *buildMethod(void *current, U32 name, U8 methodFlags)
{
    ACPI_METHOD *method = current;
    current = method + 1;

    method->methodOpcode = AML_METHOD_OP;
    method->name = name;
    method->methodFlags = methodFlags;

    return (current);
}

//-----------------------------------------------------------------------------
void *buildSmallMethod(void *current, U32 name, U8 methodFlags)
{
    ACPI_SMALL_METHOD *method = current;
    current = method + 1;

    method->methodOpcode = AML_METHOD_OP;
    method->name = name;
    method->methodFlags = methodFlags;

    return (current);
}

//-----------------------------------------------------------------------------
void *buildNamedDword(void *current, U32 name, U32 dword)
{
    ACPI_NAMED_DWORD *namedDword = current;
    current = namedDword + 1;

    setNamePath(&namedDword->namePath, name);
    setDwordConst(&namedDword->dword, dword);

    return (current);
}

//-----------------------------------------------------------------------------
void *buildGenericRegister(void *current, const ACPI_GENERIC_ADDRESS * gas)
{
    ACPI_GENERIC_REGISTER *genReg = current;
    current = genReg + 1;

    genReg->genericRegisterField = AML_GEN_REG_FIELD;
    genReg->pkgLength.packageLength0 = 0x0c;
    genReg->pkgLength.packageLength1 = 0;

    genReg->gas.SpaceId = gas->SpaceId;
    genReg->gas.BitWidth = gas->BitWidth;
    genReg->gas.BitOffset = gas->BitOffset;
    genReg->gas.AccessWidth = gas->AccessWidth;
    genReg->gas.Address = gas->Address;

    return (current);
}

//-----------------------------------------------------------------------------
void *buildSmallBuffer(void *current)
{
    ACPI_SMALL_BUFFER *buffer = current;
    current = buffer + 1;

    buffer->bufferOpcode = AML_BUFFER_OP;

    return (current);
}

//-----------------------------------------------------------------------------
void *buildEndTag(void *current)
{
    ACPI_END_TAG *endTag = current;
    current = endTag + 1;

    endTag->endTagField = AML_END_TAG_OP;
    endTag->checksum = 0;

    return (current);
}

//-----------------------------------------------------------------------------
void setSmallPackage(ACPI_SMALL_PACKAGE * package, U8 numElements)
{
    package->packageOpcode = AML_PACKAGE_OP;
    package->numElements = numElements;
}

//-----------------------------------------------------------------------------
void *buildSmallPackage(void *current, U8 numElements)
{
    ACPI_SMALL_PACKAGE *package = current;
    current = package + 1;
    setSmallPackage(package, numElements);
    return (current);
}

//-----------------------------------------------------------------------------
static void setPackage(ACPI_PACKAGE * package, U8 numElements)
{
    package->packageOpcode = AML_PACKAGE_OP;
    package->numElements = numElements;
}

//-----------------------------------------------------------------------------
void setPackageLength(ACPI_PACKAGE_LENGTH * packageLength, U32 length)
{
    packageLength->packageLength0 = 0x40 + (U8) (length & 0xf);
    packageLength->packageLength1 = (U8) (length >> 4);
}

//-----------------------------------------------------------------------------
void *buildPackageLength(void *current, U32 length)
{
    ACPI_PACKAGE_LENGTH *packageLength = current;
    current = packageLength + 1;
    setPackageLength(packageLength, length);
    return (current);
}

//-----------------------------------------------------------------------------
static void setNamePath(ACPI_NAME_PATH * namePath, U32 name)
{
    namePath->nameOpcode = AML_NAME_OP;
    namePath->name = name;
}

//-----------------------------------------------------------------------------
void *buildNamePath(void *current, U32 name)
{
    ACPI_NAME_PATH *namePath = current;
    current = namePath + 1;
    setNamePath(namePath, name);
    return (current);
}

//-----------------------------------------------------------------------------
static void setTableHeader(ACPI_TABLE_HEADER * tableHeader, U32 signature, U64 oemTableId)
{
    *(U32 *) &tableHeader->Signature = signature;
    tableHeader->Length = 0;
    tableHeader->Revision = 1;
    tableHeader->Checksum = 0;
    memcpy(&tableHeader->OemId[0], "INTEL ", 6);
    *(U64 *) (tableHeader->OemTableId) = oemTableId;
    tableHeader->OemRevision = 0x80000001;
    *(U32 *) tableHeader->AslCompilerId = NAMESEG("INTL"); // ASCII ASL compiler vendor ID
    tableHeader->AslCompilerRevision = 0x20061109; // ASL compiler version
}

//-----------------------------------------------------------------------------
void *buildTableHeader(void *current, U32 signature, U64 oemTableId)
{
    ACPI_TABLE_HEADER *tableHeader = current;
    current = tableHeader + 1;
    setTableHeader(tableHeader, signature, oemTableId);
    return (current);
}

//-----------------------------------------------------------------------------
void setByteConst(ACPI_BYTE_CONST * byteConst, U8 byteData)
{
    byteConst->byteOpcode = AML_BYTE_OP;
    byteConst->byteData = byteData;
}

//-----------------------------------------------------------------------------
void *buildByteConst(void *current, U8 byteData)
{
    ACPI_BYTE_CONST *byteConst = current;
    current = byteConst + 1;
    setByteConst(byteConst, byteData);
    return (current);
}

//-----------------------------------------------------------------------------
void setWordConst(ACPI_WORD_CONST * wordConst, U16 wordData)
{
    wordConst->wordOpcode = AML_WORD_OP;
    wordConst->wordData = wordData;
}

//-----------------------------------------------------------------------------
void *buildWordConst(void *current, U16 wordData)
{
    ACPI_WORD_CONST *wordConst = current;
    current = wordConst + 1;
    setWordConst(wordConst, wordData);
    return (current);
}

//-----------------------------------------------------------------------------
void setDwordConst(ACPI_DWORD_CONST * dwordConst, U32 dwordData)
{
    dwordConst->dwordOpcode = AML_DWORD_OP;
    dwordConst->dwordData = dwordData;
}

//-----------------------------------------------------------------------------
void *buildDwordConst(void *current, U32 dwordData)
{
    ACPI_DWORD_CONST *dwordConst = current;
    current = dwordConst + 1;
    setDwordConst(dwordConst, dwordData);
    return (current);
}

//-------------------------------------------------------------------------------
U32 ProcessFadt(ACPI_TABLE_FADT * FadtPointer, U32 pmbase)
{
    {
        // Update fields in FADT

        // Update ACPI 1.0 fields first

        FadtPointer->Pm1aEventBlock = pmbase;
        FadtPointer->Pm1aControlBlock = pmbase + 4;
        FadtPointer->Pm2ControlBlock = pmbase + 0x50;
        FadtPointer->PmTimerBlock = pmbase + 8;
        FadtPointer->Pm1EventLength = 4;
        FadtPointer->Pm1ControlLength = 2;
        FadtPointer->Pm2ControlLength = 1;
        FadtPointer->PmTimerLength = 4;

        // No legacy C2
        FadtPointer->C2Latency = 101;

        // No legacy C3
        FadtPointer->C3Latency = 1001;

        // C1 power state is supported on all processors
        FadtPointer->BootFlags |= 1UL << 2;

        // No legacy C2 on MP systems
        FadtPointer->BootFlags &= ~(1UL << 3);

        // Update ACPI 2.0+ fields if supported
        if (FadtPointer->Header.Revision >= 3) {
            // Address space where struct or register exists - System IO
            FadtPointer->XPm1aEventBlock.SpaceId = 1;
            // Size in bits of given register
            FadtPointer->XPm1aEventBlock.BitWidth = 0x20;
            // Bit offset within the register
            FadtPointer->XPm1aEventBlock.BitOffset = 0;
            // Minimum Access size (ACPI 3.0)
            FadtPointer->XPm1aEventBlock.AccessWidth = 0;
            // 64-bit address of struct or register
            FadtPointer->XPm1aEventBlock.Address = pmbase;

            // Address space where struct or register exists - System IO
            FadtPointer->XPm1aControlBlock.SpaceId = 1;
            // Size in bits of given register
            FadtPointer->XPm1aControlBlock.BitWidth = 0x10;
            // Bit offset within the register
            FadtPointer->XPm1aControlBlock.BitOffset = 0;
            // Minimum Access size (ACPI 3.0)
            FadtPointer->XPm1aControlBlock.AccessWidth = 0;
            // 64-bit address of struct or register
            FadtPointer->XPm1aControlBlock.Address = pmbase + 4;

            // Address space where struct or register exists - System IO
            FadtPointer->XPm2ControlBlock.SpaceId = 1;
            // Size in bits of given register
            FadtPointer->XPm2ControlBlock.BitWidth = 0x08;
            // Bit offset within the register
            FadtPointer->XPm2ControlBlock.BitOffset = 0;
            // Minimum Access size (ACPI 3.0)
            FadtPointer->XPm2ControlBlock.AccessWidth = 0;
            // 64-bit address of struct or register
            FadtPointer->XPm2ControlBlock.Address = pmbase + 0x50;

            // Address space where struct or register exists - System IO
            FadtPointer->XPmTimerBlock.SpaceId = 1;
            // Size in bits of given register
            FadtPointer->XPmTimerBlock.BitWidth = 0x20;
            // Bit offset within the register
            FadtPointer->XPmTimerBlock.BitOffset = 0;
            // Minimum Access size (ACPI 3.0)
            FadtPointer->XPmTimerBlock.AccessWidth = 0;
            // 64-bit address of struct or register
            FadtPointer->XPmTimerBlock.Address = pmbase + 8;
        }
    }

    // Update checksum in FADT
    SetChecksum(&FadtPointer->Header);

    return (1);
}
