/* Per ACPI 3.0a spec */

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
#ifndef __LIBSAIO_INTEL_ACPI_H
#define __LIBSAIO_INTEL_ACPI_H

#include "datatype.h"
#include "ppmsetup.h"
//
// All tables and structures must be byte-packed to match the ACPI specification
//#pragma pack(1)

#define ACPI_SIG_DSDT      "DSDT" // Differentiated System Description Table
#define ACPI_SIG_FADT      "FACP" // Fixed ACPI Description Table
#define ACPI_SIG_FACS      "FACS" // Firmware ACPI Control Structure
#define ACPI_SIG_PSDT      "PSDT" // Persistent System Description Table
#define ACPI_SIG_RSDP      "RSD PTR " // Root System Description Pointer
#define ACPI_SIG_RSDT      "RSDT" // Root System Description Table
#define ACPI_SIG_XSDT      "XSDT" // Extended  System Description Table
#define ACPI_SIG_SSDT      "SSDT" // Secondary System Description Table
#define ACPI_RSDP_NAME     "RSDP" // Short name for RSDP, not signature
#define ACPI_SIG_MADT      "APIC" // Multiple APIC Description Table
#define ACPI_SIG_SBST      "SBST" // Smart Battery Specification Table
#define ACPI_SIG_ECDT      "ECDT" // Embedded Controller Boot Resources Table 
#define ACPI_SIG_ASF       "ASF!" // Alert Standard Format table 
#define ACPI_SIG_DMAR      "DMAR" // DMA Remapping table 
#define ACPI_SIG_HPET      "HPET" // High Precision Event Timer table 
#define ACPI_SIG_MCFG      "MCFG" // PCI Memory Mapped Configuration table 
#define ACPI_SIG_UEFI      "UEFI" // Uefi Boot Optimization Table 


#ifndef nameseg_defined
#define nameseg_defined
#define NAMESEG(s)   (((U32)(s[0]) << 0)  \
|((U32)(s[1]) << 8)  \
|((U32)(s[2]) << 16) \
|((U32)(s[3]) << 24))
#endif

#define NAMESEG64(s) (((U64)(s[0]) << 0)   \
|((U64)(s[1]) << 8)   \
|((U64)(s[2]) << 16)  \
|((U64)(s[3]) << 24)  \
|((U64)(s[4]) << 32)  \
|((U64)(s[5]) << 40)  \
|((U64)(s[6]) << 48)  \
|((U64)(s[7]) << 56)) \


// Data Objects Encoding values.
#define  AML_EXT_OP_PREFIX    0x5B

// Name Space Modifier Objects Encoding values.
#define  AML_NAME_OP          0x08 // Name operator.
#define  AML_SCOPE_OP         0x10 // Scope operator.

// Named Objects Encoding values.
#define  AML_MUTEX_OP         0x01
#define  AML_METHOD_OP        0x14 // Method operator.
#define  AML_OPREGION_OP      0x80 // Operation Region operator.
#define  AML_FIELD_OP         0x81
#define  AML_DEVICE_OP        0x82
#define  AML_PROCESSOR_OP     0x83 // Processor operator.

// Type2 Opcodes Encoding values.
#define  AML_NULL_NAME         0x00
#define  AML_ZERO_OP           0x00
#define  AML_ALIAS_OP          0x06
#define  AML_ONE_OP            0x01
#define  AML_BYTE_OP           0x0a
#define  AML_WORD_OP           0x0b
#define  AML_DWORD_OP          0x0c
#define  AML_STRING_OP         0x0d
#define  AML_QWORD_OP          0x0e
#define  AML_BUFFER_OP         0x11
#define  AML_PACKAGE_OP        0x12
#define  AML_COND_REF_OF_OP    0x12 // Requires AML_EXT_OP_PREFIX
#define  AML_CREATE_FIELD_OP   0x13 // Requires AML_EXT_OP_PREFIX
#define  AML_DUAL_NAME_PREFIX  0x2e
#define  AML_MULTI_NAME_PREFIX 0x2f
#define  AML_REVISION_OP       0x30 // Requires AML_EXT_OP_PREFIX
#define  AML_DEBUG_OP          0x31
#define  AML_ROOT_PREFIX       0x5c
#define  AML_PARENT_PREFIX     0x5e
#define  AML_LOCAL0_OP         0x60
#define  AML_LOCAL1_OP         0x61
#define  AML_LOCAL2_OP         0x62
#define  AML_LOCAL3_OP         0x63
#define  AML_LOCAL4_OP         0x64
#define  AML_LOCAL5_OP         0x65
#define  AML_LOCAL6_OP         0x66
#define  AML_LOCAL7_OP         0x67
#define  AML_ARG0_OP           0x68
#define  AML_ARG1_OP           0x69
#define  AML_ARG2_OP           0x6A
#define  AML_ARG3_OP           0x6B
#define  AML_ARG4_OP           0x6C
#define  AML_ARG5_OP           0x6D
#define  AML_ARG6_OP           0x6E
#define  AML_STORE_OP          0x70
#define  AML_CONCAT_OP         0x73
#define  AML_SUBTRACT_OP       0x74
#define  AML_MULTIPLY_OP       0x77
#define  AML_AND_OP            0x7B
#define  AML_END_TAG_OP        0x79
#define  AML_GEN_REG_FIELD     0x82
#define  AML_PROCESSOR_OP      0x83
#define  AML_INDEXFIELD_OP     0x86  // Requires AML_EXT_OP_PREFIX
#define  AML_SIZEOF_OP         0x87
#define  AML_INDEX_OP          0x88
#define  AML_CREATE_DWORD_FIELD_OP  0x8A
#define  AML_LAND_OP           0x90
#define  AML_LOR_OP            0x91
#define  AML_LNOT_OP           0x92
#define  AML_LEQUAL_OP         0x93
#define  AML_LGREATER_OP       0x94
#define  AML_LLESS_OP          0x95
#define  AML_IF_OP             0xA0
#define  AML_ELSE_OP           0xA1
#define  AML_RETURN_OP         0xA4
#define  AML_ONES_OP           0xFF

#define  GAS_TYPE_FFH          0x7f
#define  GAS_TYPE_SYSTEM_IO    0x01
#define  GAS_VENDOR_INTEL      0x01
#define  GAS_CLASS_CODE_NATIVE 0x02

// Define the Generic System Description Table Structure.
// This common header is used by all tables except the RSDP and FACS.
// The define is used for direct inclusion of header into other ACPI tables
typedef struct acpi_table_header {
    U8 Signature[4]; // ASCII table signature
    U32 Length; // Length of table in bytes, including this header
    U8 Revision; // ACPI Specification minor version #
    U8 Checksum; // To make checksum of entire table == 0
    U8 OemId[6]; // ASCII OEM identification
    U8 OemTableId[8]; // ASCII OEM table identification
    U32 OemRevision; // OEM revision number
    U8 AslCompilerId[4]; // ASCII ASL compiler vendor ID
    U32 AslCompilerRevision; // ASL compiler version
} __attribute__((packed))ACPI_TABLE_HEADER;

// GAS - Generic Address Structure (ACPI 2.0+)
typedef struct acpi_generic_address {
    U8 SpaceId; // Address space where struct or register exists
    U8 BitWidth; // Size in bits of given register
    U8 BitOffset; // Bit offset within the register
    U8 AccessWidth; // Minimum Access size (ACPI 3.0)
    U64 Address; // 64-bit address of struct or register
} __attribute__((packed))ACPI_GENERIC_ADDRESS;

// RSDP - Root System Description Pointer (Signature is "RSD PTR ")
typedef struct acpi_table_rsdp {
    U8 Signature[8]; // ACPI signature, contains "RSD PTR "
    U8 Checksum; // ACPI 1.0 checksum
    U8 OemId[6]; // OEM identification
    U8 Revision; // Must be (0) for ACPI 1.0 or (2) for ACPI 2.0+
    U32 RsdtPhysicalAddress; // 32-bit physical address of the RSDT
    U32 Length; // Table length in bytes, including header (ACPI 2.0+)
    U64 XsdtPhysicalAddress; // 64-bit physical address of the XSDT (ACPI 2.0+)
    U8 ExtendedChecksum; // Checksum of entire table (ACPI 2.0+)
    U8 Reserved[3]; // Reserved, must be zero
} __attribute__((packed))ACPI_TABLE_RSDP;

#define ACPI_RSDP_REV0_SIZE     20 // Size of original ACPI 1.0 RSDP

// RSDT - Root System Description Table
typedef struct acpi_table_rsdt {
    ACPI_TABLE_HEADER Header; // Common ACPI table header
    U32 TableOffsetEntry[1]; // Array of pointers to ACPI tables
} __attribute__((packed))ACPI_TABLE_RSDT;


// XSDT - Root System Description Table
typedef struct acpi_table_xsdt {
    ACPI_TABLE_HEADER Header; // Common ACPI table header
    U64 TableOffsetEntry[1]; // Array of pointers to ACPI tables
} __attribute__((packed))ACPI_TABLE_XSDT;

// DSDT - Differentiated System Description Table
typedef struct acpi_table_dsdt {
    ACPI_TABLE_HEADER Header; // Common ACPI table header
    U32 EntryStart;
} __attribute__((packed))ACPI_TABLE_DSDT;

// FACS - Firmware ACPI Control Structure (FACS)
typedef struct acpi_table_facs {
    U8 Signature[4]; // ASCII table signature
    U32 Length; // Length of structure, in bytes
    U32 HardwareSignature; // Hardware configuration signature
    U32 FirmwareWakingVector; // 32-bit physical address of the Firmware Waking Vector
    U32 GlobalLock; // Global Lock for shared hardware resources
    U32 Flags;
    U64 XFirmwareWakingVector; // 64-bit version of the Firmware Waking Vector (ACPI 2.0+)
    U8 Version; // Version of this table (ACPI 2.0+)
    U8 Reserved[31]; // Reserved, must be zero
} __attribute__((packed))ACPI_TABLE_FACS;

// SBST - Smart Battery Specification Table - Version 1
typedef struct acpi_table_sbst
{
    ACPI_TABLE_HEADER    Header;             /* Common ACPI table header */
    U32                  WarningLevel;
    U32                  LowLevel;
    U32                  CriticalLevel;
	
} __attribute__((packed))ACPI_TABLE_SBST;

// ASF - Alert Standard Format table (Signature "ASF!")
typedef struct acpi_table_asf
{
    ACPI_TABLE_HEADER       Header;             /* Common ACPI table header */
	
} ACPI_TABLE_ASF;

// DMAR - DMA Remapping table -  Version 1
typedef struct acpi_table_dmar
{
    ACPI_TABLE_HEADER       Header;             /* Common ACPI table header */
    U8						Width;              /* Host Address Width */
    U8						Flags;
    U8						Reserved[10];
	
} __attribute__((packed))ACPI_TABLE_DMAR;

// HPET - High Precision Event Timer table - Version 1
typedef struct acpi_table_hpet
{
    ACPI_TABLE_HEADER       Header;             /* Common ACPI table header */
    U32						Id;                 /* Hardware ID of event timer block */
    ACPI_GENERIC_ADDRESS    Address;            /* Address of event timer block */
    U8						Sequence;           /* HPET sequence number */
    U16						MinimumTick;        /* Main counter min tick, periodic mode */
    U8						Flags;
	
} __attribute__((packed))ACPI_TABLE_HPET;

//MCFG - PCI Memory Mapped Configuration table and sub-table -  Version 1
typedef struct acpi_table_mcfg
{
    ACPI_TABLE_HEADER       Header;             /* Common ACPI table header */
    U8						Reserved[8];
	
} ACPI_TABLE_MCFG;

//UEFI - UEFI Boot optimization Table - Version 1
typedef struct acpi_table_uefi
{
    ACPI_TABLE_HEADER       Header;             /* Common ACPI table header */
    U8						Identifier[16];     /* UUID identifier */
    U16						DataOffset;         /* Offset of remaining data in table */
	
} __attribute__((packed))ACPI_TABLE_UEFI;

// ECDT - Embedded Controller Boot Resources Table - Version 1 
typedef struct acpi_table_ecdt
{
    ACPI_TABLE_HEADER       Header;             /* Common ACPI table header */
    ACPI_GENERIC_ADDRESS    Control;            /* Address of EC command/status register */
    ACPI_GENERIC_ADDRESS    Data;               /* Address of EC data register */
    U32						Uid;                /* Unique ID - must be same as the EC _UID method */
    U8						Gpe;                /* The GPE for the EC */
    U8						Id[1];              /* Full namepath of the EC in the ACPI namespace */
	
} __attribute__((packed))ACPI_TABLE_ECDT;

// FADT - Fixed ACPI Description Table (Signature "FACP")
typedef struct acpi_table_fadt {
    ACPI_TABLE_HEADER Header; // Common ACPI table header
    U32 Facs; // 32-bit physical address of FACS
    U32 Dsdt; // 32-bit physical address of DSDT
    U8 Model; // System Interrupt Model (ACPI 1.0) - not used in ACPI 2.0+
    U8 PreferredProfile; // Conveys preferred power management profile to OSPM.
    U16 SciInterrupt; // System vector of SCI interrupt
    U32 SmiCommand; // 32-bit Port address of SMI command port
    U8 AcpiEnable; // Value to write to smi_cmd to enable ACPI
    U8 AcpiDisable; // Value to write to smi_cmd to disable ACPI
    U8 S4BiosRequest; // Value to write to SMI CMD to enter S4BIOS state
    U8 PstateControl; // Processor performance state control
    U32 Pm1aEventBlock; // 32-bit Port address of Power Mgt 1a Event Reg Blk
    U32 Pm1bEventBlock; // 32-bit Port address of Power Mgt 1b Event Reg Blk
    U32 Pm1aControlBlock; // 32-bit Port address of Power Mgt 1a Control Reg Blk
    U32 Pm1bControlBlock; // 32-bit Port address of Power Mgt 1b Control Reg Blk
    U32 Pm2ControlBlock; // 32-bit Port address of Power Mgt 2 Control Reg Blk
    U32 PmTimerBlock; // 32-bit Port address of Power Mgt Timer Ctrl Reg Blk
    U32 Gpe0Block; // 32-bit Port address of General Purpose Event 0 Reg Blk
    U32 Gpe1Block; // 32-bit Port address of General Purpose Event 1 Reg Blk
    U8 Pm1EventLength; // Byte Length of ports at Pm1xEventBlock
    U8 Pm1ControlLength; // Byte Length of ports at Pm1xControlBlock
    U8 Pm2ControlLength; // Byte Length of ports at Pm2ControlBlock
    U8 PmTimerLength; // Byte Length of ports at PmTimerBlock
    U8 Gpe0BlockLength; // Byte Length of ports at Gpe0Block
    U8 Gpe1BlockLength; // Byte Length of ports at Gpe1Block
    U8 Gpe1Base; // Offset in GPE number space where GPE1 events start
    U8 CstControl; // Support for the _CST object and C States change notification
    U16 C2Latency; // Worst case HW latency to enter/exit C2 state
    U16 C3Latency; // Worst case HW latency to enter/exit C3 state
    U16 FlushSize; // Processor's memory cache line width, in bytes
    U16 FlushStride; // Number of flush strides that need to be read
    U8 DutyOffset; // Processor duty cycle index in processor's P_CNT reg
    U8 DutyWidth; // Processor duty cycle value bit width in P_CNT register.
    U8 DayAlarm; // Index to day-of-month alarm in RTC CMOS RAM
    U8 MonthAlarm; // Index to month-of-year alarm in RTC CMOS RAM
    U8 Century; // Index to century in RTC CMOS RAM
    U16 BootFlags; // IA-PC Boot Architecture Flags. See Table 5-10 for description
    U8 Reserved; // Reserved, must be zero
    U32 Flags; // Miscellaneous flag bits (see below for individual flags)
    ACPI_GENERIC_ADDRESS ResetRegister; // 64-bit address of the Reset register
    U8 ResetValue; // Value to write to the ResetRegister port to reset the system
    U8 Reserved4[3]; // Reserved, must be zero
    U64 XFacs; // 64-bit physical address of FACS
    U64 XDsdt; // 64-bit physical address of DSDT
    ACPI_GENERIC_ADDRESS XPm1aEventBlock; // 64-bit Extended Power Mgt 1a Event Reg Blk address
    ACPI_GENERIC_ADDRESS XPm1bEventBlock; // 64-bit Extended Power Mgt 1b Event Reg Blk address
    ACPI_GENERIC_ADDRESS XPm1aControlBlock; // 64-bit Extended Power Mgt 1a Control Reg Blk address
    ACPI_GENERIC_ADDRESS XPm1bControlBlock; // 64-bit Extended Power Mgt 1b Control Reg Blk address
    ACPI_GENERIC_ADDRESS XPm2ControlBlock; // 64-bit Extended Power Mgt 2 Control Reg Blk address
    ACPI_GENERIC_ADDRESS XPmTimerBlock; // 64-bit Extended Power Mgt Timer Ctrl Reg Blk address
    ACPI_GENERIC_ADDRESS XGpe0Block; // 64-bit Extended General Purpose Event 0 Reg Blk address
    ACPI_GENERIC_ADDRESS XGpe1Block; // 64-bit Extended General Purpose Event 1 Reg Blk address
} __attribute__((packed))ACPI_TABLE_FADT;

// SSDT - Secondary System Description Table
typedef struct acpi_table_ssdt {
    ACPI_TABLE_HEADER Header; // Common ACPI table header
} __attribute__((packed))ACPI_TABLE_SSDT;

//MADT - Multiple APIC Description Table
typedef struct acpi_table_madt {
    ACPI_TABLE_HEADER Header; // Common ACPI table header
    U32 Address; // Physical address of local APIC
    U32 Flags;
} __attribute__((packed))ACPI_TABLE_MADT;


// Values for subtable type in ACPI_SUBTABLE_HEADER
enum AcpiMadtType {
    ACPI_MADT_TYPE_LOCAL_APIC = 0,
    ACPI_MADT_TYPE_IO_APIC = 1,
    ACPI_MADT_TYPE_INTERRUPT_OVERRIDE = 2,
    ACPI_MADT_TYPE_NMI_SOURCE = 3,
    ACPI_MADT_TYPE_LOCAL_APIC_NMI = 4,
    ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE = 5,
    ACPI_MADT_TYPE_IO_SAPIC = 6,
    ACPI_MADT_TYPE_LOCAL_SAPIC = 7,
    ACPI_MADT_TYPE_INTERRUPT_SOURCE = 8,
    ACPI_MADT_TYPE_X2APIC = 9,
    ACPI_MADT_TYPE_RESERVED = 10 // 10 and greater are reserved
};

// Common Sub-table header (used in MADT, SRAT, etc.)
typedef struct acpi_subtable_header {
    U8 Type;
    U8 Length;
} __attribute__((packed))ACPI_SUBTABLE_HEADER;

// MADT Sub-tables, correspond to Type in ACPI_SUBTABLE_HEADER

// 0: Processor Local APIC
typedef struct acpi_madt_local_apic {
    ACPI_SUBTABLE_HEADER Header;
    U8 ProcessorId; // ACPI processor id
    U8 Id; // Processor's local APIC id
    U32 LapicFlags;
} __attribute__((packed))ACPI_MADT_LOCAL_APIC;

// 1: IO APIC
typedef struct acpi_madt_io_apic {
    ACPI_SUBTABLE_HEADER Header;
    U8 Id; // I/O APIC ID
    U8 Reserved; // Reserved - must be zero
    U32 Address; // APIC physical address
    U32 GlobalIrqBase; // Global system interrupt where INTI lines start
} __attribute__((packed))ACPI_MADT_IO_APIC;

// 2: Interrupt Override
typedef struct acpi_madt_interrupt_override {
    ACPI_SUBTABLE_HEADER Header;
    U8 Bus; // 0 - ISA
    U8 SourceIrq; // Interrupt source (IRQ)
    U32 GlobalIrq; // Global system interrupt
    U16 IntiFlags;
} __attribute__((packed))ACPI_MADT_INTERRUPT_OVERRIDE;

// 3: NMI Source
typedef struct acpi_madt_nmi_source {
    ACPI_SUBTABLE_HEADER Header;
    U16 IntiFlags;
    U32 GlobalIrq; // Global system interrupt
} __attribute__((packed))ACPI_MADT_NMI_SOURCE;

// 4: Local APIC NMI
typedef struct acpi_madt_local_apic_nmi {
    ACPI_SUBTABLE_HEADER Header;
    U8 ProcessorId; // ACPI processor id
    U16 IntiFlags;
    U8 Lint; // LINTn to which NMI is connected
} __attribute__((packed))ACPI_MADT_LOCAL_APIC_NMI;

// 5: Address Override
typedef struct acpi_madt_local_apic_override {
    ACPI_SUBTABLE_HEADER Header;
    U16 Reserved; // Reserved, must be zero
    U64 Address; // APIC physical address
} __attribute__((packed))ACPI_MADT_LOCAL_APIC_OVERRIDE;

// 6: I/O Sapic
typedef struct acpi_madt_io_sapic {
    ACPI_SUBTABLE_HEADER Header;
    U8 Id; // I/O SAPIC ID
    U8 Reserved; // Reserved, must be zero
    U32 GlobalIrqBase; // Global interrupt for SAPIC start
    U64 Address; // SAPIC physical address
} __attribute__((packed))ACPI_MADT_IO_SAPIC;

// 7: Local Sapic
typedef struct acpi_madt_local_sapic {
    ACPI_SUBTABLE_HEADER Header;
    U8 ProcessorId; // ACPI processor id
    U8 Id; // SAPIC ID
    U8 Eid; // SAPIC EID
    U8 Reserved[3]; // Reserved, must be zero
    U32 LapicFlags;
    U32 Uid; // Numeric UID - ACPI 3.0
    char UidString[1]; // String UID  - ACPI 3.0
} __attribute__((packed))ACPI_MADT_LOCAL_SAPIC;

// 8: Platform Interrupt Source
typedef struct acpi_madt_interrupt_source {
    ACPI_SUBTABLE_HEADER Header;
    U16 IntiFlags;
    U8 Type; // 1=PMI, 2=INIT, 3=corrected
    U8 Id; // Processor ID
    U8 Eid; // Processor EID
    U8 IoSapicVector; // Vector value for PMI interrupts
    U32 GlobalIrq; // Global system interrupt
    U32 Flags; // Interrupt Source Flags
} __attribute__((packed))ACPI_MADT_INTERRUPT_SOURCE;

// 9: Processor X2APIC
typedef struct acpi_madt_x2apic {
    ACPI_SUBTABLE_HEADER Header;
    U16 Reserved; // Must be zero
    U32 x2apicId; // Processor's X2APIC ID
    U32 x2apicFlags;
    U32 UID;
} __attribute__((packed))ACPI_MADT_X2APIC;

// Common flags fields for MADT subtables

// MADT Local APIC flags (LapicFlags)
#define ACPI_MADT_ENABLED           (1) // 00: Processor is usable if set

// MADT MPS INTI flags (IntiFlags)
#define ACPI_MADT_POLARITY_MASK     (3) // 00-01: Polarity of APIC I/O input signals
#define ACPI_MADT_TRIGGER_MASK      (3<<2) // 02-03: Trigger mode of APIC input signals

// Values for MPS INTI flags
#define ACPI_MADT_POLARITY_CONFORMS       0
#define ACPI_MADT_POLARITY_ACTIVE_HIGH    1
#define ACPI_MADT_POLARITY_RESERVED       2
#define ACPI_MADT_POLARITY_ACTIVE_LOW     3

#define ACPI_MADT_TRIGGER_CONFORMS        (0)
#define ACPI_MADT_TRIGGER_EDGE            (1<<2)
#define ACPI_MADT_TRIGGER_RESERVED        (2<<2)
#define ACPI_MADT_TRIGGER_LEVEL           (3<<2)

#define ACPI_COORD_TYPE_SW_ALL  0xFC
#define ACPI_COORD_TYPE_SW_ANY  0xFD
#define ACPI_COORD_TYPE_HW_ALL  0xFE

typedef struct packageLength {
    U8 packageLength0;
    U8 packageLength1;
} __attribute__((packed))ACPI_PACKAGE_LENGTH;

typedef struct acpi_scope {
    U8 scopeOpcode;
    ACPI_PACKAGE_LENGTH pkgLength;
    U8 rootChar;
} __attribute__((packed))ACPI_SCOPE;

typedef struct dual_name_path {
    U8 prefix;
    U32 nameseg[2];
} __attribute__((packed))DUAL_NAME_PATH;

typedef struct multi_name_path {
    U8 prefix;
    U8 segCount;
    U32 nameseg[MAX_SUPPORTED_CPU_NAMESEGS];
} __attribute__((packed))MULTI_NAME_PATH;

typedef struct generic_register {
    U8 genericRegisterField;
    ACPI_PACKAGE_LENGTH pkgLength;
    ACPI_GENERIC_ADDRESS gas;
} __attribute__((packed))ACPI_GENERIC_REGISTER;

typedef struct package {
    U8 packageOpcode;
    ACPI_PACKAGE_LENGTH pkgLength;
    U8 numElements;
} __attribute__((packed))ACPI_PACKAGE;

typedef struct small_package {
    U8 packageOpcode;
    U8 packageLength;
    U8 numElements;
} __attribute__((packed))ACPI_SMALL_PACKAGE;

typedef struct small_buffer {
    U8 bufferOpcode;
    U8 packageLength;
} __attribute__((packed))ACPI_SMALL_BUFFER;

typedef struct end_tag {
    U8 endTagField;
    U8 checksum;
} __attribute__((packed))ACPI_END_TAG;

typedef struct return_name_seg {
    U8 returnOpcode;
    U32 name;
} __attribute__((packed))ACPI_RETURN_NAME_SEG;

typedef struct return_package {
    U8 returnOpcode;
    ACPI_PACKAGE package;
} __attribute__((packed))ACPI_RETURN_PACKAGE;

typedef struct return_zero {
    U8 returnOpcode;
    U8 zeroOpcode;
} __attribute__((packed))ACPI_RETURN_ZERO;

typedef struct return_opcode {
    U8 returnOpcode;
    U8 opcodeToReturn;
} __attribute__((packed))ACPI_RETURN_OPCODE;

typedef struct byteConst {
    U8 byteOpcode;
    U8 byteData;
} __attribute__((packed))ACPI_BYTE_CONST;

typedef struct wordConst {
    U8 wordOpcode;
    U16 wordData;
} __attribute__((packed))ACPI_WORD_CONST;

typedef struct dwordConst {
    U8 dwordOpcode;
    U32 dwordData;
} __attribute__((packed))ACPI_DWORD_CONST;

typedef struct small_method {
    U8 methodOpcode;
    U8 packageLength;
    U32 name;
    U8 methodFlags;
} __attribute__((packed))ACPI_SMALL_METHOD;

typedef struct method {
    U8 methodOpcode;
    ACPI_PACKAGE_LENGTH pkgLength;
    U32 name;
    U8 methodFlags;
} __attribute__((packed))ACPI_METHOD;

typedef struct namePath {
    U8 nameOpcode;
    U32 name;
} __attribute__((packed))ACPI_NAME_PATH;

typedef struct named_dword {
    ACPI_NAME_PATH namePath;
    ACPI_DWORD_CONST dword;
} __attribute__((packed))ACPI_NAMED_DWORD;

typedef struct rootNamePath {
    U8 nameOpcode;
    U8 rootPrefix;
    U32 name;
} ACPI_ROOT_NAME_PATH;

typedef struct root_named_dword {
    ACPI_ROOT_NAME_PATH rootNamePath;
    ACPI_DWORD_CONST dword;
} __attribute__((packed))ACPI_ROOT_NAMED_DWORD;

typedef struct named_object {
    ACPI_NAME_PATH namePath;
    ACPI_PACKAGE package;
} __attribute__((packed))ACPI_NAMED_OBJECT;

typedef struct small_named_object {
    ACPI_NAME_PATH namePath;
    ACPI_SMALL_PACKAGE package;
} __attribute__((packed))ACPI_SMALL_NAMED_OBJECT;

typedef struct create_dword_field {
    ACPI_NAME_PATH namePath;
    ACPI_SMALL_PACKAGE package;
} __attribute__((packed))ACPI_CREATE_DWORD_FIELD;

typedef struct tstate_package {
    ACPI_SMALL_PACKAGE package;
    ACPI_DWORD_CONST FreqPercent;
    ACPI_DWORD_CONST Power;
    ACPI_DWORD_CONST TransLatency;
    ACPI_DWORD_CONST Control;
    ACPI_DWORD_CONST Status;
} __attribute__((packed))ACPI_TSTATE_PACKAGE;

typedef struct pstate_package {
    ACPI_SMALL_PACKAGE package;
    ACPI_DWORD_CONST CoreFreq;
    ACPI_DWORD_CONST Power;
    ACPI_DWORD_CONST TransLatency;
    ACPI_DWORD_CONST BMLatency;
    ACPI_DWORD_CONST Control;
    ACPI_DWORD_CONST Status;
} __attribute__((packed))ACPI_PSTATE_PACKAGE;

typedef struct psd_package {
    ACPI_SMALL_PACKAGE package;
    ACPI_BYTE_CONST NumberOfEntries;
    ACPI_BYTE_CONST Revision;
    ACPI_DWORD_CONST Domain;
    ACPI_DWORD_CONST CoordType;
    ACPI_DWORD_CONST NumProcessors;
} __attribute__((packed))ACPI_PSD_PACKAGE;

typedef struct csd_package {
    ACPI_SMALL_PACKAGE package;
    ACPI_BYTE_CONST NumberOfEntries;
    ACPI_BYTE_CONST Revision;
    ACPI_DWORD_CONST Domain;
    ACPI_DWORD_CONST CoordType;
    ACPI_DWORD_CONST NumProcessors;
    ACPI_DWORD_CONST Index;
} __attribute__((packed))ACPI_CSD_PACKAGE;

typedef struct tsd_package {
    ACPI_SMALL_PACKAGE package;
    ACPI_BYTE_CONST NumberOfEntries;
    ACPI_BYTE_CONST Revision;
    ACPI_DWORD_CONST Domain;
    ACPI_DWORD_CONST CoordType;
    ACPI_DWORD_CONST NumProcessors;
} __attribute__((packed))ACPI_TSD_PACKAGE;

typedef struct processor {
    U8 processorOpCode;
    U8 packageLength;
    U8 numElements;
    ACPI_BYTE_CONST ProcID;
    ACPI_DWORD_CONST PblkAddr;
    ACPI_BYTE_CONST PblkLen;
} __attribute__((packed))ACPI_PROCESSOR;

//#pragma pack()

#endif /* !__LIBSAIO_INTEL_ACPI_H */
