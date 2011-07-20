/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#include "libsaio.h"
#include "platform.h"
#include "cpu.h"
#include "bootstruct.h"
#include "boot.h"

#ifndef DEBUG_CPU
#define DEBUG_CPU 0
#endif

#if DEBUG_CPU
#define DBG(x...)		printf(x)
#else
#define DBG(x...)		msglog(x)
#endif

/*
 * DFE: Measures the TSC frequency in Hz (64-bit) using the ACPI PM timer
 */
static uint64_t measure_tsc_frequency(void)
{
    uint64_t tscStart;
    uint64_t tscEnd;
    uint64_t tscDelta = 0xffffffffffffffffULL;
    unsigned long pollCount;
    uint64_t retval = 0;
    int i;
    
    /* Time how many TSC ticks elapse in 30 msec using the 8254 PIT
     * counter 2.  We run this loop 3 times to make sure the cache
     * is hot and we take the minimum delta from all of the runs.
     * That is to say that we're biased towards measuring the minimum
     * number of TSC ticks that occur while waiting for the timer to
     * expire.  That theoretically helps avoid inconsistencies when
     * running under a VM if the TSC is not virtualized and the host
     * steals time.  The TSC is normally virtualized for VMware.
     */
    for(i = 0; i < 10; ++i)
    {
        enable_PIT2();
        set_PIT2_mode0(CALIBRATE_LATCH);
        tscStart = rdtsc64();
        pollCount = poll_PIT2_gate();
        tscEnd = rdtsc64();
        /* The poll loop must have run at least a few times for accuracy */
        if(pollCount <= 1)
            continue;
        /* The TSC must increment at LEAST once every millisecond.  We
         * should have waited exactly 30 msec so the TSC delta should
         * be >= 30.  Anything less and the processor is way too slow.
         */
        if((tscEnd - tscStart) <= CALIBRATE_TIME_MSEC)
            continue;
        // tscDelta = MIN(tscDelta, (tscEnd - tscStart))
        if( (tscEnd - tscStart) < tscDelta )
            tscDelta = tscEnd - tscStart;
    }
    /* tscDelta is now the least number of TSC ticks the processor made in
     * a timespan of 0.03 s (e.g. 30 milliseconds)
     * Linux thus divides by 30 which gives the answer in kiloHertz because
     * 1 / ms = kHz.  But we're xnu and most of the rest of the code uses
     * Hz so we need to convert our milliseconds to seconds.  Since we're
     * dividing by the milliseconds, we simply multiply by 1000.
     */
    
    /* Unlike linux, we're not limited to 32-bit, but we do need to take care
     * that we're going to multiply by 1000 first so we do need at least some
     * arithmetic headroom.  For now, 32-bit should be enough.
     * Also unlike Linux, our compiler can do 64-bit integer arithmetic.
     */
    if(tscDelta > (1ULL<<32))
        retval = 0;
    else
    {
        retval = tscDelta * 1000 / 30;
    }
    disable_PIT2();
    return retval;
}

#if 0
/*
 * DFE: Measures the Max Performance Frequency in Hz (64-bit)
 */
static uint64_t measure_mperf_frequency(void)
{
    uint64_t mperfStart;
    uint64_t mperfEnd;
    uint64_t mperfDelta = 0xffffffffffffffffULL;
    unsigned long pollCount;
    uint64_t retval = 0;
    int i;
    
    /* Time how many MPERF ticks elapse in 30 msec using the 8254 PIT
     * counter 2.  We run this loop 3 times to make sure the cache
     * is hot and we take the minimum delta from all of the runs.
     * That is to say that we're biased towards measuring the minimum
     * number of MPERF ticks that occur while waiting for the timer to
     * expire.
     */
    for(i = 0; i < 10; ++i)
    {
        enable_PIT2();
        set_PIT2_mode0(CALIBRATE_LATCH);
        mperfStart = rdmsr64(MSR_AMD_MPERF);
        pollCount = poll_PIT2_gate();
        mperfEnd = rdmsr64(MSR_AMD_MPERF);
        /* The poll loop must have run at least a few times for accuracy */
        if(pollCount <= 1)
            continue;
        /* The MPERF must increment at LEAST once every millisecond.  We
         * should have waited exactly 30 msec so the MPERF delta should
         * be >= 30.  Anything less and the processor is way too slow.
         */
        if((mperfEnd - mperfStart) <= CALIBRATE_TIME_MSEC)
            continue;
        // tscDelta = MIN(tscDelta, (tscEnd - tscStart))
        if( (mperfEnd - mperfStart) < mperfDelta )
            mperfDelta = mperfEnd - mperfStart;
    }
    /* mperfDelta is now the least number of MPERF ticks the processor made in
     * a timespan of 0.03 s (e.g. 30 milliseconds)
     */
    
    if(mperfDelta > (1ULL<<32))
        retval = 0;
    else
    {
        retval = mperfDelta * 1000 / 30;
    }
    disable_PIT2();
    return retval;
}
#endif
/*
 * Measures the Actual Performance Frequency in Hz (64-bit)
 */
static uint64_t measure_aperf_frequency(void)
{
    uint64_t aperfStart;
    uint64_t aperfEnd;
    uint64_t aperfDelta = 0xffffffffffffffffULL;
    unsigned long pollCount;
    uint64_t retval = 0;
    int i;
    
    /* Time how many APERF ticks elapse in 30 msec using the 8254 PIT
     * counter 2.  We run this loop 3 times to make sure the cache
     * is hot and we take the minimum delta from all of the runs.
     * That is to say that we're biased towards measuring the minimum
     * number of APERF ticks that occur while waiting for the timer to
     * expire.  
     */
    for(i = 0; i < 10; ++i)
    {
        enable_PIT2();
        set_PIT2_mode0(CALIBRATE_LATCH);
        aperfStart = rdmsr64(MSR_AMD_APERF);
        pollCount = poll_PIT2_gate();
        aperfEnd = rdmsr64(MSR_AMD_APERF);
        /* The poll loop must have run at least a few times for accuracy */
        if(pollCount <= 1)
            continue;
        /* The TSC must increment at LEAST once every millisecond.  We
         * should have waited exactly 30 msec so the APERF delta should
         * be >= 30.  Anything less and the processor is way too slow.
         */
        if((aperfEnd - aperfStart) <= CALIBRATE_TIME_MSEC)
            continue;
        // tscDelta = MIN(tscDelta, (tscEnd - tscStart))
        if( (aperfEnd - aperfStart) < aperfDelta )
            aperfDelta = aperfEnd - aperfStart;
    }
    /* mperfDelta is now the least number of MPERF ticks the processor made in
     * a timespan of 0.03 s (e.g. 30 milliseconds)
     */

    if(aperfDelta > (1ULL<<32))
        retval = 0;
    else
    {
        retval = aperfDelta * 1000 / 30;
    }
    disable_PIT2();
    return retval;
}


/*
 * Calculates the FSB and CPU frequencies using specific MSRs for each CPU
 * - multi. is read from a specific MSR. In the case of Intel, there is:
 *     a max multi. (used to calculate the FSB freq.),
 *     and a current multi. (used to calculate the CPU freq.)
 * - fsbFrequency = tscFrequency / multi
 * - cpuFrequency = fsbFrequency * multi
 */

void scan_cpu(PlatformInfo_t *p)
{
	uint64_t	tscFrequency, fsbFrequency, cpuFrequency;
	uint64_t	msr, flex_ratio;
	uint8_t		maxcoef, maxdiv, currcoef, bus_ratio_max, currdiv;
	const char *newratio;
	int len, myfsb;
	uint8_t bus_ratio_min;
	uint32_t max_ratio, min_ratio;
    
	max_ratio = min_ratio = myfsb = bus_ratio_min = 0;
	maxcoef = maxdiv = bus_ratio_max = currcoef = currdiv = 0;
    
	/* get cpuid values */
	do_cpuid(0x00000000, p->CPU.CPUID[CPUID_0]);
	do_cpuid(0x00000001, p->CPU.CPUID[CPUID_1]);
	do_cpuid(0x00000002, p->CPU.CPUID[CPUID_2]);
	do_cpuid(0x00000003, p->CPU.CPUID[CPUID_3]);
    do_cpuid2(0x00000004, 0, p->CPU.CPUID[CPUID_4]);
	do_cpuid(0x80000000, p->CPU.CPUID[CPUID_80]);
    if ((p->CPU.CPUID[CPUID_80][0] & 0x0000000f) >= 8) {
        do_cpuid(0x80000008, p->CPU.CPUID[CPUID_88]);
        do_cpuid(0x80000001, p->CPU.CPUID[CPUID_81]);
	}
    else if ((p->CPU.CPUID[CPUID_80][0] & 0x0000000f) >= 1) {
		do_cpuid(0x80000001, p->CPU.CPUID[CPUID_81]);
	}
    
    
#if DEBUG_CPU
	{
		int		i;
		printf("CPUID Raw Values:\n");
		for (i=0; i<CPUID_MAX; i++) {
			printf("%02d: %08x-%08x-%08x-%08x\n", i,
                   p->CPU.CPUID[i][0], p->CPU.CPUID[i][1],
                   p->CPU.CPUID[i][2], p->CPU.CPUID[i][3]);
		}
	}
#endif
	p->CPU.Vendor		= p->CPU.CPUID[CPUID_0][1];
	p->CPU.Signature	= p->CPU.CPUID[CPUID_1][0];
	p->CPU.Stepping		= bitfield(p->CPU.CPUID[CPUID_1][0], 3, 0);
	p->CPU.Model		= bitfield(p->CPU.CPUID[CPUID_1][0], 7, 4);
	p->CPU.Family		= bitfield(p->CPU.CPUID[CPUID_1][0], 11, 8);
	p->CPU.ExtModel		= bitfield(p->CPU.CPUID[CPUID_1][0], 19, 16);
	p->CPU.ExtFamily	= bitfield(p->CPU.CPUID[CPUID_1][0], 27, 20);
	
    p->CPU.Model += (p->CPU.ExtModel << 4);
    
    if (p->CPU.Vendor == CPUID_VENDOR_INTEL && 
        p->CPU.Family == 0x06 && 
        p->CPU.Model >= CPUID_MODEL_NEHALEM && 
        p->CPU.Model != CPUID_MODEL_ATOM        // MSR is *NOT* available on the Intel Atom CPU
        )
    {
        msr = rdmsr64(MSR_CORE_THREAD_COUNT);									// Undocumented MSR in Nehalem and newer CPUs
        p->CPU.NoCores		= bitfield((uint32_t)msr, 31, 16);					// Using undocumented MSR to get actual values
        p->CPU.NoThreads	= bitfield((uint32_t)msr, 15,  0);					// Using undocumented MSR to get actual values
	}
    else if (p->CPU.Vendor == CPUID_VENDOR_AMD)
    {
        p->CPU.NoThreads	= bitfield(p->CPU.CPUID[CPUID_1][1], 23, 16);
        p->CPU.NoCores		= bitfield(p->CPU.CPUID[CPUID_88][2], 7, 0) + 1;
    }
    else
    {
        p->CPU.NoThreads	= bitfield(p->CPU.CPUID[CPUID_1][1], 23, 16);		// Use previous method for Cores and Threads
        p->CPU.NoCores		= bitfield(p->CPU.CPUID[CPUID_4][0], 31, 26) + 1;
	}
	
	/* get brand string (if supported) */
	/* Copyright: from Apple's XNU cpuid.c */
	if (p->CPU.CPUID[CPUID_80][0] > 0x80000004) {
		uint32_t	reg[4];
        char        str[128], *s;
		/*
		 * The brand string 48 bytes (max), guaranteed to
		 * be NULL terminated.
		 */
		do_cpuid(0x80000002, reg);
		bcopy((char *)reg, &str[0], 16);
		do_cpuid(0x80000003, reg);
		bcopy((char *)reg, &str[16], 16);
		do_cpuid(0x80000004, reg);
		bcopy((char *)reg, &str[32], 16);
		for (s = str; *s != '\0'; s++) {
			if (*s != ' ') break;
		}
		
		strlcpy(p->CPU.BrandString,	s, sizeof(p->CPU.BrandString));
		
		if (!strncmp(p->CPU.BrandString, CPU_STRING_UNKNOWN, MIN(sizeof(p->CPU.BrandString), strlen(CPU_STRING_UNKNOWN) + 1))) {
            /*
             * This string means we have a firmware-programmable brand string,
             * and the firmware couldn't figure out what sort of CPU we have.
             */
            p->CPU.BrandString[0] = '\0';
        }
	}
	
	/* setup features */
	if ((bit(23) & p->CPU.CPUID[CPUID_1][3]) != 0) {
		p->CPU.Features |= CPU_FEATURE_MMX;
	}
	if ((bit(25) & p->CPU.CPUID[CPUID_1][3]) != 0) {
		p->CPU.Features |= CPU_FEATURE_SSE;
	}
	if ((bit(26) & p->CPU.CPUID[CPUID_1][3]) != 0) {
		p->CPU.Features |= CPU_FEATURE_SSE2;
	}
	if ((bit(0) & p->CPU.CPUID[CPUID_1][2]) != 0) {
		p->CPU.Features |= CPU_FEATURE_SSE3;
	}
	if ((bit(19) & p->CPU.CPUID[CPUID_1][2]) != 0) {
		p->CPU.Features |= CPU_FEATURE_SSE41;
	}
	if ((bit(20) & p->CPU.CPUID[CPUID_1][2]) != 0) {
		p->CPU.Features |= CPU_FEATURE_SSE42;
	}
	if ((bit(29) & p->CPU.CPUID[CPUID_81][3]) != 0) {
		p->CPU.Features |= CPU_FEATURE_EM64T;
	}
	if ((bit(5) & p->CPU.CPUID[CPUID_1][3]) != 0) {
		p->CPU.Features |= CPU_FEATURE_MSR;
	}
	//if ((bit(28) & p->CPU.CPUID[CPUID_1][3]) != 0) {
	if (p->CPU.NoThreads > p->CPU.NoCores) {
		p->CPU.Features |= CPU_FEATURE_HTT;
	}
    
	tscFrequency = measure_tsc_frequency();
	fsbFrequency = 0;
	cpuFrequency = 0;
    
	if ((p->CPU.Vendor == CPUID_VENDOR_INTEL) && ((p->CPU.Family == 0x06) || (p->CPU.Family == 0x0f))) {
		int intelCPU = p->CPU.Model;
		if ((p->CPU.Family == 0x06 && p->CPU.Model >= 0x0c) || (p->CPU.Family == 0x0f && p->CPU.Model >= 0x03)) {
			/* Nehalem CPU model */
			if (p->CPU.Family == 0x06 && (p->CPU.Model == CPU_MODEL_NEHALEM || 
                                          p->CPU.Model == CPU_MODEL_FIELDS || 
                                          p->CPU.Model == CPU_MODEL_DALES || 
                                          p->CPU.Model == CPU_MODEL_DALES_32NM || 
                                          p->CPU.Model == CPU_MODEL_WESTMERE ||
                                          p->CPU.Model == CPU_MODEL_NEHALEM_EX ||
                                          p->CPU.Model == CPU_MODEL_WESTMERE_EX ||
                                          p->CPU.Model == CPU_MODEL_SANDY ||
                                          p->CPU.Model == CPU_MODEL_SANDY_XEON)) {
				msr = rdmsr64(MSR_PLATFORM_INFO);
                DBG("msr(%d): platform_info %08x\n", __LINE__, bitfield(msr, 31, 0));
                bus_ratio_max = bitfield(msr, 14, 8);
                bus_ratio_min = bitfield(msr, 46, 40); //valv: not sure about this one (Remarq.1)
				msr = rdmsr64(MSR_FLEX_RATIO);
                DBG("msr(%d): flex_ratio %08x\n", __LINE__, bitfield(msr, 31, 0));
                if (bitfield(msr, 16, 16)) {
                    flex_ratio = bitfield(msr, 14, 8);
					/* bcc9: at least on the gigabyte h67ma-ud2h,
                     where the cpu multipler can't be changed to
                     allow overclocking, the flex_ratio msr has unexpected (to OSX)
                     contents.  These contents cause mach_kernel to
                     fail to compute the bus ratio correctly, instead
                     causing the system to crash since tscGranularity
                     is inadvertently set to 0.
                     */
					if (flex_ratio == 0) {
						/* Clear bit 16 (evidently the
                         presence bit) */
						wrmsr64(MSR_FLEX_RATIO, (msr & 0xFFFFFFFFFFFEFFFFULL));
						msr = rdmsr64(MSR_FLEX_RATIO);
                        verbose("Unusable flex ratio detected.  Patched MSR now %08x\n", bitfield(msr, 31, 0));
					} else {
						if (bus_ratio_max > flex_ratio) {
							bus_ratio_max = flex_ratio;
						}
					}
				}
                
				if (bus_ratio_max) {
					fsbFrequency = (tscFrequency / bus_ratio_max);
				}
				//valv: Turbo Ratio Limit
				if ((intelCPU != 0x2e) && (intelCPU != 0x2f)) {
					msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);
					cpuFrequency = bus_ratio_max * fsbFrequency;
					max_ratio = bus_ratio_max * 10;
				} else {
					cpuFrequency = tscFrequency;
				}
				if ((getValueForKey(kbusratio, &newratio, &len, &bootInfo->chameleonConfig)) && (len <= 4)) {
					max_ratio = atoi(newratio);
					max_ratio = (max_ratio * 10);
					if (len >= 3) max_ratio = (max_ratio + 5);
                    
					verbose("Bus-Ratio: min=%d, max=%s\n", bus_ratio_min, newratio);
                    
					// extreme overclockers may love 320 ;)
					if ((max_ratio >= min_ratio) && (max_ratio <= 320)) {
						cpuFrequency = (fsbFrequency * max_ratio) / 10;
						if (len >= 3) maxdiv = 1;
						else maxdiv = 0;
					} else {
						max_ratio = (bus_ratio_max * 10);
					}
				}
				//valv: to be uncommented if Remarq.1 didn't stick
				/*if(bus_ratio_max > 0) bus_ratio = flex_ratio;*/
				p->CPU.MaxRatio = max_ratio;
				p->CPU.MinRatio = min_ratio;
                
				myfsb = fsbFrequency / 1000000;
				verbose("Sticking with [BCLK: %dMhz, Bus-Ratio: %d]\n", myfsb, max_ratio);
				currcoef = bus_ratio_max;
			} else {
				msr = rdmsr64(MSR_IA32_PERF_STATUS);
                DBG("msr(%d): ia32_perf_stat 0x%08x\n", __LINE__, bitfield(msr, 31, 0));
                currcoef = bitfield(msr, 12, 8);
				/* Non-integer bus ratio for the max-multi*/
                maxdiv = bitfield(msr, 46, 46);
				/* Non-integer bus ratio for the current-multi (undocumented)*/
                currdiv = bitfield(msr, 14, 14);
                
				if ((p->CPU.Family == 0x06 && p->CPU.Model >= 0x0e) || (p->CPU.Family == 0x0f)) // This will always be model >= 3
				{
					/* On these models, maxcoef defines TSC freq */
                    maxcoef = bitfield(msr, 44, 40);
				} else {
					/* On lower models, currcoef defines TSC freq */
					/* XXX */
					maxcoef = currcoef;
				}
                
				if (maxcoef) {
					if (maxdiv) {
						fsbFrequency = ((tscFrequency * 2) / ((maxcoef * 2) + 1));
					} else {
						fsbFrequency = (tscFrequency / maxcoef);
					}
					if (currdiv) {
						cpuFrequency = (fsbFrequency * ((currcoef * 2) + 1) / 2);
					} else {
						cpuFrequency = (fsbFrequency * currcoef);
					}
					DBG("max: %d%s current: %d%s\n", maxcoef, maxdiv ? ".5" : "",currcoef, currdiv ? ".5" : "");
				}
			}
		}
		/* Mobile CPU */
		if (rdmsr64(MSR_IA32_PLATFORM_ID) & (1<<28)) {
			p->CPU.Features |= CPU_FEATURE_MOBILE;
		}
	}
	else if((p->CPU.Vendor == CPUID_VENDOR_AMD) && (p->CPU.Family == 0x0f))
    {
        switch(p->CPU.ExtFamily)
        {
            case 0x00: /* K8 */
                msr = rdmsr64(K8_FIDVID_STATUS);
                maxcoef = bitfield(msr, 21, 16) / 2 + 4;
                currcoef = bitfield(msr, 5, 0) / 2 + 4;
                break;
                
            case 0x01: /* K10 */
                msr = rdmsr64(K10_COFVID_STATUS);
                do_cpuid2(0x00000006, 0, p->CPU.CPUID[CPUID_6]);
                if(bitfield(p->CPU.CPUID[CPUID_6][2], 0, 0) == 1)   // EffFreq: effective frequency interface
                {
                    //uint64_t mperf = measure_mperf_frequency();
                    uint64_t aperf = measure_aperf_frequency();
                    cpuFrequency = aperf;
                }
                // NOTE: tsc runs at the maccoeff (non turbo)
                //          *not* at the turbo frequency.
                maxcoef  = bitfield(msr, 54, 49) / 2 + 4;
                currcoef = bitfield(msr, 5, 0) + 0x10;
                currdiv = 2 << bitfield(msr, 8, 6);
                
                break;
                
            case 0x05: /* K14 */
                msr = rdmsr64(K10_COFVID_STATUS);
                currcoef  = (bitfield(msr, 54, 49) + 0x10) << 2;
                currdiv = (bitfield(msr, 8, 4) + 1) << 2;
                currdiv += bitfield(msr, 3, 0);

                break;
                
            case 0x02: /* K11 */
                // not implimented
                break;
        }
        
        if (maxcoef)
        {
            if (currdiv)
            {
                if(!currcoef) currcoef = maxcoef;
                if(!cpuFrequency)
                    fsbFrequency = ((tscFrequency * currdiv) / currcoef);
                else 
                    fsbFrequency = ((cpuFrequency * currdiv) / currcoef);

                DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
            } else {
                if(!cpuFrequency)
                    fsbFrequency = (tscFrequency / maxcoef);
                else 
                    fsbFrequency = (cpuFrequency / maxcoef);
                DBG("%d\n", currcoef);
            }
        }
        else if (currcoef)
        {
            if (currdiv)
            {
                fsbFrequency = ((tscFrequency * currdiv) / currcoef);
                DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
            } else {
                fsbFrequency = (tscFrequency / currcoef);
                DBG("%d\n", currcoef);
            }
        }
        if(!cpuFrequency) cpuFrequency = tscFrequency;
    }
#if 0
    if (!fsbFrequency) {
        fsbFrequency = (DEFAULT_FSB * 1000);
        cpuFrequency = tscFrequency;
        DBG("0 ! using the default value for FSB !\n");
    }
#endif
    
    p->CPU.MaxCoef = maxcoef;
    p->CPU.MaxDiv = maxdiv;
    p->CPU.CurrCoef = currcoef;
    p->CPU.CurrDiv = currdiv;
    p->CPU.TSCFrequency = tscFrequency;
    p->CPU.FSBFrequency = fsbFrequency;
    p->CPU.CPUFrequency = cpuFrequency;
    
    DBG("CPU: Brand String:             %s\n",				p->CPU.BrandString);
    DBG("CPU: Vendor/Family/ExtFamily:  0x%x/0x%x/0x%x\n",	p->CPU.Vendor, p->CPU.Family, p->CPU.ExtFamily);
    DBG("CPU: Model/ExtModel/Stepping:  0x%x/0x%x/0x%x\n",	p->CPU.Model, p->CPU.ExtModel, p->CPU.Stepping);
    DBG("CPU: MaxCoef/CurrCoef:         0x%x/0x%x\n",		p->CPU.MaxCoef, p->CPU.CurrCoef);
    DBG("CPU: MaxDiv/CurrDiv:           0x%x/0x%x\n",		p->CPU.MaxDiv, p->CPU.CurrDiv);
    DBG("CPU: TSCFreq:                  %dMHz\n",			p->CPU.TSCFrequency / 1000000);
    DBG("CPU: FSBFreq:                  %dMHz\n",			p->CPU.FSBFrequency / 1000000);
    DBG("CPU: CPUFreq:                  %dMHz\n",			p->CPU.CPUFrequency / 1000000);
    DBG("CPU: NoCores/NoThreads:        %d/%d\n",			p->CPU.NoCores, p->CPU.NoThreads);
    DBG("CPU: Features:                 0x%08x\n",			p->CPU.Features);
#if DEBUG_CPU
    pause();
#endif
}
