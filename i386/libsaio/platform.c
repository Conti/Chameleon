/*
 *  platform.c
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "pci.h"
#include "freq_detect.h"
#include "nvidia.h"
#include "spd.h"
#include "platform.h"

#ifndef DEBUG_PLATFORM
#define DEBUG_PLATFORM 0
#endif

#if DEBUG_PLATFORM
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

PlatformInfo_t    Platform;

void scan_cpu_amd()
{
	// AMD
	
	// TODO: Retrieve cpu brand string
	// TODO: Retrieve cpu core count
	// TODO: Retrieve cpu mobile info
	
}

void scan_cpu_intel()
{
	uint32_t	cpuid_reg[4];

	// Get Number of cores per package
	/*
	 Initially set the EAX register to 4 and the ECX register to 0 prior to executing the CPUID instruction.
	 After executing the CPUID instruction, (EAX[31:26] + 1) contains the number of cores.
	 */
	cpuid_reg[2]=1;
	do_cpuid(4, cpuid_reg);
	do_cpuid(4, cpuid_reg); // FIXME: why does this only work the 2nd time ?
	Platform.CPU.NoCores = bitfield(cpuid_reg[0], 31, 26) + 1;
	
	// Find Number of Concurrent Threads Processed (HyperThreading) 
	do_cpuid(1,cpuid_reg);
	if(bitfield(cpuid_reg[1], 23, 16) > 1)
		Platform.CPU.NoThreads=Platform.CPU.NoCores;
	else
		Platform.CPU.NoThreads=Platform.CPU.NoCores * 2;
	
	// Mobile CPU ?
	if (rdmsr64(0x17) & (1<<28))
		Platform.CPU.Mobile = 1;
	else
		Platform.CPU.Mobile = 0;
}

void scan_platform()
{
	uint32_t	cpuid_reg[4];

	build_pci_dt();

	calculate_freq();
	
	// Copy the values from calculate_freq()
	Platform.CPU.TSCFrequency = tscFrequency;
	Platform.CPU.FSBFrequency = fsbFrequency;
	Platform.CPU.CPUFrequency = cpuFrequency;
	
	do_cpuid(0, cpuid_reg);
	Platform.CPU.Vendor = cpuid_reg[1];
	
	do_cpuid(1, cpuid_reg);
	Platform.CPU.Model = bitfield(cpuid_reg[0], 7, 4);
	Platform.CPU.Family = bitfield(cpuid_reg[0], 11, 8);
	Platform.CPU.ExtModel = bitfield(cpuid_reg[0], 19, 16);
	Platform.CPU.ExtFamily = bitfield(cpuid_reg[0], 27, 20);

	// Get vendor specific cpu data 
	if((Platform.CPU.Vendor == 0x756E6547 /* Intel */) && ((Platform.CPU.Family == 0x06) || (Platform.CPU.Family == 0x0f)))
		scan_cpu_intel();
	else if((Platform.CPU.Vendor == 0x68747541 /* AMD */) && (Platform.CPU.Family == 0x0f))
		scan_cpu_amd();
}


