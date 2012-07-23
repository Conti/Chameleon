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

#ifndef  ppmsetup_h
#define  ppmsetup_h

#include "datatype.h"

#define CURRENT_PPM_RCM_INTERFACE_SPECIFICATION 11

#ifndef nameseg_defined
#define nameseg_defined
#define NAMESEG(s)   (((U32)(s[0]) << 0)  \
                     |((U32)(s[1]) << 8)  \
                     |((U32)(s[2]) << 16) \
                     |((U32)(s[3]) << 24))
#endif

typedef enum ssdt_loc_flag {
    // Flag indicating the SSDT ACPI structure should be built in a stack-based
    // buffer. If the SSDT is successfully created, then the following occurs:
    // (1) the RSDT ACPI structure is moved lower in memory and updated to
    //     include a pointer to the new SSDT.
    // (2) the SSDT APCI structure is copied into memory just above the moved
    //     RSDT structure
    // (3) the RSD ACPI structure is updated to included the new location of
    //     the just moved RSDT ACPI structure
    // Note: The XSDT is not updated!
    //
    // NOTE: THIS OPTION WILL DEPRECATED AND REMOVED
    // IN A FUTURE VERSION OF THIS SPECIFICATION.
    SSDT_LOC_FLAG_ACPI_RECLAIM = 0,

    // Flag indicating the SSDT should be built directly in the memory region
    // provided by the ssdt_mem_addr option parameter.
    // If the SSDT is successfully created, then the following occurs:
    // (1) the RSDT ACPI structure is moved lower in memory and updated to
    //     include a pointer to the new SSDT.
    // (2) the RSD ACPI structure is updated to include the new location of the
    //     RSDT ACPI structure.
    // Note: The XSDT is not updated!
    //
    // NOTE: THIS OPTION WILL DEPRECATED AND REMOVED
    // IN A FUTURE VERSION OF THIS SPECIFICATION.
    SSDT_LOC_FLAG_ADDR_PROVIDED = 1,

    // Flag indicating the SSDT should be built directly in the memory region
    // provided by the ssdt_mem_addr option parameter.
    // After SSDT is successfully created, no further processing occurs.
    SSDT_LOC_FLAG_ADDR_PROVIDED_NO_INSERT = 2,

    // Flag indicating the SSDT should be built directly in the memory region
    // provided by the ssdt_mem_addr option parameter.
    // After SSDT is successfully created, then the following occurs:
    // (1) the RSDT ACPI structure is not moved but is updated to
    //     include a 32-bit pointer to the new SSDT.
    // (2) If the XSDT exists, it is not moved but is updated to
    //     include a 64-bit pointer to the new SSDT.
    SSDT_LOC_FLAG_ADDR_PROVIDED_INSERT = 3,
} SSDT_LOC_FLAG;

typedef enum exit_code {
    // PPM RCM completed all initialization successfully
    EXIT_CODE_PPM_COMPLETED = 1,

    // Failed building P-state table
    EXIT_CODE_FAILED_BUILD_PSTATES = 2,

    EXIT_CODE_PPM_EIST_DISABLED = 3,

    // Failed to find ACPI tables
    EXIT_CODE_FAILED_FIND_ACPI_TABLES = 4,

    // Failed to process the ACPI MADT structure
    EXIT_CODE_FAILED_PROCESS_MADT = 5,

    // Failed to resolve ACPI MADT structure against available logical
    // processors
    EXIT_CODE_FAILED_PROCESS_MADT_INFO = 6,

    // Failed to build ACPI SSDT structure
    EXIT_CODE_FAILED_PROCESS_SSDT = 7,

    // Failed to build and intialize HOST structure
    EXIT_CODE_FAILED_HOST_INIT = 8,

    // Failed during wake of all NBSP to gather processor information
    EXIT_CODE_FAILED_COLLECT_SOCKET_INFO = 9,

    // Failed to resolve target configuration between desired options and
    // processor features
    EXIT_CODE_FAILED_DETERMINE_CONFIGURATION = 0x0a,

    // No SSDT ACPI struture was created
    EXIT_CODE_NO_SSDT_CREATED = 0x0b,

    // Failed to build Cstates correctly
    EXIT_CODE_FAILED_BUILD_CSTATES = 0x0c,

    // Failed to build Tstates correctly
    EXIT_CODE_FAILED_BUILD_TSTATES = 0x0d,

    // Failed to find package index of logical processor listed in MADT ACPI table
    EXIT_CODE_FAILED_FIND_PKG_INDEX_FROM_LAPIC = 0x0e,

    // Failed with invalid input provided for SSDT location flag
    EXIT_CODE_FAILED_INVALID_SSDT_LOCATION_FLAG = 0x0f,

    // Failed with no logical processors found in MADT
    EXIT_CODE_FAILED_NO_LAPIC_FOUND_IN_MADT = 0x10,

    // Failed with SSDT size exceeded during SSDT creation
    EXIT_CODE_FAILED_SSDT_SIZE_EXCEEDED = 0x11,

    // Failed to build ACPI SSDT structure
    EXIT_CODE_FAILED_BUILD_SSDT = 0x12,

    // Failed with core index of logical processor listed in MADT ACPI table exceeding max
    EXIT_CODE_MAX_CORES_EXCEEDED = 0x13,

    // Failed to find CPU Scope from array of ACPI processor number to ACPI CPU NameSeg structures
    EXIT_CODE_FAILED_FIND_CPU_SCOPE_NAME_SEG = 0x14,

    // Failed to update FADT
    EXIT_CODE_FAILED_UPDATE_FADT = 0x15,

    // GPF detected
    EXIT_CODE_GPF_DETECTED = 0x16,

    // Failed with invalid SSDT buffer address
    EXIT_CODE_INVALID_SSDT_ADDR = 0x17,

    // Failed with invalid SSDT buffer length
    EXIT_CODE_INVALID_SSDT_LEN = 0x18,
} EXIT_CODE;

typedef struct exit_state {
    // 1 = success, 0 = failure
    U32 return_status;

    // Number of Failure or Informative codes included in the buffer
    U32 error_code_count;

    // Buffer of Failure or Informative codes
    U32 error_codes[10];

    // This 32-bit physical memory address specifies the final location for the
    // SSDT ACPI structure built by the PPM RC.
    U32 ssdt_mem_addr;

    // This value specifies the final size of the SSDT ACPI structure for the
    // SSDT ACPI structure built by the PPM RC.
    U32 ssdt_mem_size;

    // The final state for the P-state initialization
    // 1=enabled; 0=disabled
    U32 pstates_enabled;

    // The final state for the Turbo feature initialization
    // 1=enabled; 0=disabled
    U32 turbo_enabled;

    // The final state for the C-state initialization
    // 1=enabled; 0=disabled
    U32 cstates_enabled;

    // The final state for the T-state initialization
    // 1=enabled; 0=disabled
    U32 tstates_enabled;
} EXIT_STATE;

typedef enum cpu_namespace_flag {
    // Flag indicating the SSDT ACPI structure should be built
    // using ACPI 1.0 compliant processor namespace "_PR"
    CPU_NAMESPACE_PR = 0,

    // Flag indicating the SSDT ACPI structure should be built
    // using ACPI 2.0+ compliant processor namespace "_SB"
    CPU_NAMESPACE_SB = 1,
} CPU_NAMESPACE_FLAG;

// Define the total number of required NameSegs to reach the DSDT processor
// device or object declarations
#define MAX_SUPPORTED_CPU_NAMESEGS  3

typedef struct processor_number_to_nameseg {
    // Contains one of the ACPI processor ID values used in a
    // ACPI Declare Processor statement in the DSDT or XSDT
    U32 acpi_processor_number;

    // Number of NameSpace segments in NamePath to processor devices/objects
    U32 seg_count;

    // Contains the corresponding ACPI Name Scope in the form
    // of a series of NameSegs constituting the NamePath to a
    // processor device or object declaration
    U32 nameseg[MAX_SUPPORTED_CPU_NAMESEGS];
} PROCESSOR_NUMBER_TO_NAMESEG;

typedef struct ppm_setup_options {
    // This 32-bit physical memory address specifies a read-write memory region
    // below 1MB. Minimum size is 8KB.  This memory is used by the callback as
    // the SIPI target and stack for each AP. This region is not required to be
    // cacheable.
    U32 mem_region_below_1M;

    // Number of CPU sockets which exist on the platform
    U32 num_sockets;

    // Desired state for the P-state initialization
    // 1=enabled; 0=disabled
    U32 pstates_enabled;

    // Desired state for the P-state hardware coordination
    // ACPI_PSD_COORD_TYPE_SW_ALL = 0xFC
    // ACPI_PSD_COORD_TYPE_SW_ANY = 0xFD
    // ACPI_PSD_COORD_TYPE_HW_ALL = 0xFE
    U32 pstate_coordination;

    // Desired state for the Turbo state initialization
    // 1=enabled; 0=disabled
    U32 turbo_enabled;

    // Desired state for the C-state initialization
    // 1=enabled; 0=disabled
    U32 cstates_enabled;

    // Desired state for the C1E initialization
    // 1=enabled; 0=disabled
    U32 c1e_enabled;

    // Desired state for the processor core C3 state included in the _CST
    // 0= processor core C3 cannot be used as an ACPI C state
    // 2= processor core C3 can be used as an ACPI C2 state
    // 3= processor core C3 can be used as an ACPI C3 state
    // 4= processor core C3 can be used as an ACPI C2 state
    //    if Invariant APIC Timer detected, else not used as ACPI C state
    // 5= processor core C3 can be used as an ACPI C2 state
    //    if Invariant APIC Timer detected, else APIC C3 state
    U32 c3_enabled;

    // Desired state for the processor core C6 state included in the _CST as an
    // ACPI C3 state.
    // 1= processor core C6 can be used as an ACPI C3 state
    // 0= processor core C6 cannot be used as an ACPI C3 state
    U32 c6_enabled;

    // Desired state for the processor core C7 state included in the _CST as an
    // ACPI C3 state.
    // 1= processor core C7 can be used as an ACPI C7 state
    // 0= processor core C7 cannot be used as an ACPI C7 state
    U32 c7_enabled;

    // Desired state for providing alternate ACPI _CST structure using MWAIT
    // extensions
    // 1= Alternate _CST using MWAIT extension is enabled for OSPM use
    // 0= Alternate _CST using MWAIT extension is disabled for OSPM use
    U32 mwait_enabled;

    // Power management base address used for processors
    U32 pmbase;

    // Desired state for the C-state package limit.
    // Note: The C-state package limit may be further limited by the
    // capabilities of the processor
    // 000b = C0 (No package C-state support)
    // 001b = C1 (Behavior is the same as 000b)
    // 010b = C3
    // 011b = C6
    // 100b = C7
    // 111b = No package C-state limit
    U32 package_cstate_limit;

    // Desired state for the T-state initialization
    // 1=enabled; 0=disabled
    U32 tstates_enabled;

    // This 32-bit physical memory address specifies a read-write memory region
    // for the SSDT ACPI structure built by the PPM RC. Minimum size is 16KB.
    U32 ssdt_mem_addr;

    // This value specifies the size of the SSDT memory region. Minimum size is
    // 16KB.
    U32 ssdt_mem_size;

    // This value specifies the PPM RCM behavior related to creation and
    // incorporation of the new SSDT ACPI structure. See definition of the
    // SSDT_LOC_FLAG for acceptable values.
    U32 ssdt_loc_flag;

    // This value specifies the PPM RCM behavior related to creation and
    // incorporation of the new SSDT ACPI structure. If all power management
    // features are disabled by input options, the SSDT can still be created
    // for debug review.
    // 1 = Create SSDT even if all power management features are disabled
    // 0 = Do not create SSDT if all power management features are disabled
    U32 ssdt_force_creation;

    // Exit structure intended to convey state to the caller and/or subsequent
    // init code
    EXIT_STATE exit_state;

    // Flag indicating the processor namespace that should be used in the
    // SSDT ACPI structure for each CPU.
    // See definition of the CPU_NAMESPACE_FLAG for acceptable values.
    U32 cpu_namespace_flag;

    // This version number identifies the PPM RCM specification.
    // Specifically denotes the version of this structure definition is compliant
    // with with file nehalem-ppm-rcm-vX.txt where X is the version number.
    // PPMSETUP.C should always use the version definition from the top of
    // this file called CURRENT_PPM_RCM_INTERFACE_SPECIFICATION.
    U32 ppm_rcm_interface_specification;

    // This flag indicates whether or not after all AP Wakes are completed,
    // that the AP should be forced to jump to the real mode address specified
    // in the realmode_callback_address field below.
    // realmode_callback = 0 means leave AP in INIT or Wait For SIPI (WFS) state
    // realmode_callback = 1 means AP should jump to real mode address specified below
    U32 realmode_callback_flag;

    // This file contains the real mode callback address which AP must jump to after
    // INIT_SIPI_SIPI sequences used to force AP initalization for PPM.
    // Upper 16-bits specify target real mode segment for a far 16-bit jump instruction
    // Lower 16-bits specify target real mode offset for a far 16-bit jump instruction
    U32 realmode_callback_address;

    // Number of ACPI processor number to ACPI CPU NameSeg structures
    U32 cpu_map_count;

    // Array of ACPI processor number to ACPI CPU NameSeg structures
    PROCESSOR_NUMBER_TO_NAMESEG *cpu_map;

    // This flag indicates whether or not PPM RC should update an existing ACPI FADT.
    // modify_fadt_flag = 0 means do not modify existing ACPI FADT structure
    // modify_fadt_flag = 1 means do check and if needed, modify existing ACPI FADT structure
    U32 modify_fadt_flag;

    // Desired state for the performance_per_watt optimizations
    // performance_per_watt = 2 means "Low Power"
    // performance_per_watt = 1 means "Power Optimized or Power Balanced"
    // performance_per_watt = 0 means "Traditional or Max Performance"
    U32 performance_per_watt;
} PPM_SETUP_OPTIONS;

#endif // ppmsetup_h
