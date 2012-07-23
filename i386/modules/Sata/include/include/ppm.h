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

#ifndef  ppm_h
#define  ppm_h

#include "datatype.h"

#define PROFILE_ALL
//#define PROFILE_NEHALEM_EP_DP
//#define PROFILE_WESTMERE_EP_UP_DP
//#define PROFILE_NEHALEM_EX_MP
//#define PROFILE_WESTMERE_EX_MP
//#define PROFILE_SANDYBRIDGE_UP

#if defined(PROFILE_ALL) || defined(PROFILE_WESTMERE_EX_MP)

#define MAX_CPU_SOCKETS 8 // max count of cpu packages (any range of APIC IDs is ok)
#define MAX_LOGICAL_CPU 256 // max count of cpu found in MADT
#define MAX_CORES       32 // Based on full range of Core APID ID values (max of 5 bits for core APIC ID mask)

#elif defined(PROFILE_NEHALEM_EX_MP)

#define MAX_CPU_SOCKETS 8 // max count of cpu packages (any range of APIC IDs is ok)
#define MAX_LOGICAL_CPU 128 // max count of cpu found in MADT
#define MAX_CORES       16 // Based on full range of Core APID ID values (max of 4 bits for core APIC ID mask)

#elif defined(PROFILE_WESTMERE_EP_UP_DP)

#define MAX_CPU_SOCKETS 2 // max count of cpu packages (any range of APIC IDs is ok)
#define MAX_LOGICAL_CPU 64 // max count of cpu found in MADT
#define MAX_CORES       16 // Based on full range of Core APID ID values (max of 4 bits for core APIC ID mask)

#elif defined(PROFILE_NEHALEM_EP_UP_DP)

#define MAX_CPU_SOCKETS 2 // max count of cpu packages (any range of APIC IDs is ok)
#define MAX_LOGICAL_CPU 32 // max count of cpu found in MADT
#define MAX_CORES       8 // Based on full range of Core APID ID values (max of 3 bits for core APIC ID mask)

#elif defined(PROFILE_SANDY_BRIDGE_UP)

#define MAX_CPU_SOCKETS 1 // max count of cpu packages (any range of APIC IDs is ok)
#define MAX_LOGICAL_CPU 8 // max count of cpu found in MADT
#define MAX_CORES       4

#endif

#define MAX_PSTATES     16
#define MAX_CSTATES     4
#define MAX_TSTATES     15

//Define ACPI_CSD to force building ACPI _CSD
//#define BUILD_ACPI_CSD

#ifndef DWORD_REGS_TYPEDEF
#define DWORD_REGS_TYPEDEF
typedef struct dword_regs {
    U32 _eax;
    U32 _ebx;
    U32 _ecx;
    U32 _edx;
} DWORD_REGS;
#endif

typedef struct acpi_tables {
    // Define the Storage Locations for all the ACPI Table Pointers.
    ACPI_TABLE_DSDT *DsdtPointer; // Differentiated System Description Table  (RSDP->RSDT->FACP->DSDT)
    ACPI_TABLE_DSDT *DsdtPointer64; // Differentiated System Description Table  (RSDP->XSDT->FACP->XDSDT)
    ACPI_TABLE_FADT *FacpPointer; // Fixed ACPI Description Table             (RSDP->RSDT->FACP)
    ACPI_TABLE_FACS *FacsPointer; // Firmware ACPI Control Structure          (RSDP->RSDT->FACP->FACS)    
    ACPI_TABLE_FACS *FacsPointer64; // Firmware ACPI Control Structure          (RSDP->XSDT->FACP->XFACS)
    ACPI_TABLE_RSDP *RsdPointer; // Root System Description Pointer Structure (RSDP)
    ACPI_TABLE_RSDT *RsdtPointer; // Root System Description Table            (RSDP->RSDT)
    ACPI_TABLE_MADT *MadtPointer; // Multiple APIC Description Table          (RSDP->RSDT->APIC)
	ACPI_TABLE_MADT *MadtPointer64; // Multiple APIC Description Table        (RSDP->XSDT->APIC)
    ACPI_TABLE_XSDT *XsdtPointer; // Extended Root System Description Table   (RSDP->XSDT)
    ACPI_TABLE_FADT *FacpPointer64; // Fixed ACPI Description Table           (RSDP->XSDT->FACP)    
} ACPI_TABLES;

typedef struct pstate {
    U32 frequency;
    U32 power;
    U32 ratio;
	U32 translatency;
    U32 bmlatency;
	U32 control;
    U32 status;
} PSTATE;

typedef struct pkg_pstates {
    U32 num_pstates;
    PSTATE pstate[MAX_PSTATES];
} PKG_PSTATES;

typedef struct tstate {
    U32 freqpercent;
    U32 power;
    U32 latency;
    U32 control;
    U32 status;
} TSTATE;

typedef struct pkg_tstates {
    U32 num_tstates;
    TSTATE tstate[MAX_TSTATES];
} PKG_TSTATES;

typedef enum cstate_encoding {
    IO_REDIRECTION = 0,
    NATIVE_MWAIT = 1,
} CSTATE_ENCODING;

typedef enum cpu_cstate {
    CPU_C1 = 1,
	//CPU_C2 = 2,
    CPU_C3_ACPI_C2 = 3,
    CPU_C3_ACPI_C3 = 4,
	CPU_C4 = 5,
    CPU_C6 = 6,
    CPU_C7 = 7,
} CPU_CSTATE;

typedef struct cstate {
    U8 type;
    U16 latency;
    U32 power;
} CSTATE;

typedef struct pkg_cstates {
    U32 num_cstates;
    CSTATE cstate[MAX_CSTATES];
    ACPI_GENERIC_ADDRESS gas[MAX_CSTATES];
} PKG_CSTATES;

typedef struct cpu_details {
    U32 present;
    U32 x2apic_id;
    U32 socket_id;
    U32 intra_package_mask_width;
    U32 intra_package_mask;
    U32 smt_mask_width;
    U32 smt_select_mask;
    U32 core_select_mask;
    DWORD_REGS cpuid1;
    DWORD_REGS cpuid5;
    DWORD_REGS cpuid6;
    DWORD_REGS cpuidB_0;
    DWORD_REGS cpuidB_1;
    U32 eist_cpuid_feature_flag;
    U32 turbo_cpuid_feature_flag;
    U32 turbo_misc_enables_feature_flag;
    U32 invariant_apic_timer_flag;
    U32 tdc_limit;
    U32 tdp_limit;
    U32 turbo_available;
    U32 max_ratio_as_mfg;
    U32 max_ratio_as_cfg;
    U32 min_ratio;
    U32 tdc_tdp_limits_for_turbo_flag;
    U32 ratio_limits_for_turbo_flag;
    U32 xe_available;
    U32 logical_processor_count_from_madt;
    U32 core_logical_processor_count_from_madt[MAX_CORES];

    PKG_PSTATES pkg_pstates;

    PKG_CSTATES pkg_mwait_cstates;
    PKG_CSTATES pkg_io_cstates;

    PKG_TSTATES pkg_tstates;

    U32 package_cstate_limit;
    U32 core_c1_supported;
	U32 core_c2_supported;
    U32 core_c3_supported;
	U32 core_c4_supported;
    U32 core_c6_supported;
    U32 core_c7_supported;
    U32 mwait_supported;
    U32 acpi_support_cpuid_feature_flag;
    U32 energy_perf_bias_supported;

    U64 package_power_limit;
    U64 package_power_sku_unit;
} CPU_DETAILS;

typedef struct socket_info {
    U32 signature;
    U32 socket_count;
    CPU_DETAILS cpu[MAX_CPU_SOCKETS];
} SOCKET_INFO;

typedef struct lapic_info {
    U32 processorId;
    U32 apicId;
    U32 pkg_index;
    U32 core_apic_id;
    U32 core_index;
    PROCESSOR_NUMBER_TO_NAMESEG *namepath;
    U32 madt_type;
    U32 uid;
} LAPIC_INFO;

typedef struct proc_info {
    U32 processorId;
    U32 apicId;
} PROC_INFO;

typedef struct madt_info {
    U32 lapic_count;
    LAPIC_INFO lapic[MAX_LOGICAL_CPU];
} MADT_INFO;

typedef struct rsdt_info {
    U32 proc_count;
    PROC_INFO processor[MAX_LOGICAL_CPU];
} RSDT_INFO;

typedef struct smp_exit_state {
    U32 signature;

    // Number of Failure or Informative codes included in the buffer
    U32 error_code_count;

    // Buffer of Failure or Informative codes
    U32 error_codes[10];
} SMP_EXIT_STATE;

typedef enum smp_exit_code {
    // Generic successful
    SMP_EXIT_CODE_OK = 1,

    // Generic failure
    EXIT_CODE_FAILED = 2,

    // First logical processor for this socket unable to find available structure
    EXIT_CODE_FAILED_SOCKET_PROXY_SAVE = 3,
} SMP_EXIT_CODE;

typedef struct ppm_host {
    U32 signature;

    U32 pstates_enabled;
    U32 pstate_coordination;
    U32 turbo_enabled;
    U32 cstates_enabled;
    U32 tstates_enabled;
    U32 performance_per_watt;

    ACPI_TABLES acpi_tables;
    
        
    RSDT_INFO rsdt_info;
    MADT_INFO madt_info;
    SOCKET_INFO skt_info;

    PPM_SETUP_OPTIONS *options;

    SMP_EXIT_STATE smp_exit_state;

    U32 detected_cpu_family;
} PPM_HOST;

#endif // ppm_h
