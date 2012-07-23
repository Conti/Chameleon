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

#ifndef  acpi_code_h
#define  acpi_code_h

#include "datatype.h"
#include "acpi.h"
#include "ppm.h"

void setRsdpchecksum(ACPI_TABLE_RSDP *rsdp);
void setRsdpXchecksum(ACPI_TABLE_RSDP *rsdp);
U32 update_rsdp_with_xsdt(ACPI_TABLE_RSDP *rsdp, ACPI_TABLE_XSDT *xsdt);
U32 update_rsdp_with_rsdt(ACPI_TABLE_RSDP *rsdp, ACPI_TABLE_RSDT *rsdt);

void SetChecksum(struct acpi_table_header *header);
U32 ProcessMadtInfo(ACPI_TABLE_MADT * madt, MADT_INFO * madt_info);
void MoveRsdtInsertSsdt(ACPI_TABLE_RSDP * RsdPointer, ACPI_TABLE_RSDT * OldRsdtPointer, ACPI_TABLE_RSDT * NewRsdtPointer, ACPI_TABLE_SSDT * SsdtPointer);
void InsertSsdt(ACPI_TABLE_RSDT * RsdtPointer, ACPI_TABLE_SSDT * SsdtPointer);
void InsertSsdt64(ACPI_TABLE_XSDT * XsdtPointer, ACPI_TABLE_SSDT * SsdtPointer);
U32 ProcessFadt(ACPI_TABLE_FADT * FadtPointer, U32 pmbase);
U32 ProcessDsdt(ACPI_TABLE_DSDT * DsdtPointer, U8 * PCIUIDPointer, U8 uid);

void setByteConst(ACPI_BYTE_CONST * byteConst, U8 byteData);
void *buildByteConst(void *current, U8 byteData);
void setWordConst(ACPI_WORD_CONST * wordConst, U16 wordData);
void *buildWordConst(void *current, U16 wordData);
void setDwordConst(ACPI_DWORD_CONST * dwordConst, U32 dwordData);
void *buildDwordConst(void *current, U32 dwordData);
void *buildSmallBuffer(void *current);
void *buildEndTag(void *current);
void *buildGenericRegister(void *current, const ACPI_GENERIC_ADDRESS * gas);

void *buildSmallMethod(void *current, U32 name, U8 methodFlags);
void *buildMethod(void *current, U32 name, U8 methodFlags);
void *buildReturnZero(void *current);
void *buildReturnOpcode(void *current, U8 opcodeToReturn);
void *buildReturnPackage(void *current, U8 numElements);
void *buildNamedDword(void *current, U32 name, U32 dword);
void *buildOpCode(void *current, U8 opCode);

void *buildNameSeg(void *current, U32 name);
void setSmallPackage(ACPI_SMALL_PACKAGE * package, U8 numElements);
void *buildSmallPackage(void *current, U8 numElements);
void setPackageLength(ACPI_PACKAGE_LENGTH * packageLength, U32 length);
void *buildPackageLength(void *current, U32 Length);
void *buildNamePath(void *current, U32 name);
void *buildTableHeader(void *current, U32 signature, U64 oemTableId);

#endif // acpi_code_h
