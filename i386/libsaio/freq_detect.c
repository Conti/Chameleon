/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 */

#include "libsaio.h"
#include "freq_detect.h"

// DFE: enable_PIT2 and disable_PIT2 come from older xnu

/*
 * Enable or disable timer 2.
 * Port 0x61 controls timer 2:
 *   bit 0 gates the clock,
 *   bit 1 gates output to speaker.
 */
inline static void
enable_PIT2(void)
{
    /* Enable gate, disable speaker */
    __asm__ volatile(
        " inb   $0x61,%%al      \n\t"
        " and   $0xFC,%%al       \n\t"  /* & ~0x03 */
        " or    $1,%%al         \n\t"
        " outb  %%al,$0x61      \n\t"
        : : : "%al" );
}

inline static void
disable_PIT2(void)
{
    /* Disable gate and output to speaker */
    __asm__ volatile(
        " inb   $0x61,%%al      \n\t"
        " and   $0xFC,%%al      \n\t"	/* & ~0x03 */
        " outb  %%al,$0x61      \n\t"
        : : : "%al" );
}

// DFE: set_PIT2_mode0, poll_PIT2_gate, and measure_tsc_frequency are
// roughly based on Linux code

/* Set the 8254 channel 2 to mode 0 with the specified value.
   In mode 0, the counter will initially set its gate low when the
   timer expires.  For this to be useful, you ought to set it high
   before calling this function.  The enable_PIT2 function does this.
 */
static inline void set_PIT2_mode0(uint16_t value)
{
    __asm__ volatile(
        " movb  $0xB0,%%al      \n\t"
        " outb	%%al,$0x43	\n\t"
        " movb	%%dl,%%al	\n\t"
        " outb	%%al,$0x42	\n\t"
        " movb	%%dh,%%al	\n\t"
        " outb	%%al,$0x42"
        : : "d"(value) /*: no clobber */ );
}

/* Returns the number of times the loop ran before the PIT2 signaled */
static inline unsigned long poll_PIT2_gate(void)
{
    unsigned long count = 0;
    unsigned char nmi_sc_val;
    do {
        ++count;
        __asm__ volatile(
            "inb	$0x61,%0"
        : "=q"(nmi_sc_val) /*:*/ /* no input */ /*:*/ /* no clobber */);
    } while( (nmi_sc_val & 0x20) == 0);
    return count;
}

/*
 * DFE: Measures the TSC frequency in Hz (64-bit) using the ACPI PM timer
 */
uint64_t measure_tsc_frequency(void)
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
    for(i = 0; i < 3; ++i)
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
        // tscDelta = min(tscDelta, (tscEnd - tscStart))
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

uint64_t tscFrequency = 0;
uint64_t fsbFrequency = 0;
uint64_t cpuFrequency = 0;

/*
 * Calculates the FSB and CPU frequencies using specific MSRs for each CPU
 * - multi. is read from a specific MSR. In the case of Intel, there is:
 *     a max multi. (used to calculate the FSB freq.),
 *     and a current multi. (used to calculate the CPU freq.)
 * - fsbFrequency = tscFrequency / multi
 * - cpuFrequency = fsbFrequency * multi
 */

void calculate_freq(void)
{
	uint32_t	cpuid_reg[4], cpu_vendor;
	uint8_t		cpu_family, cpu_model, cpu_extfamily, cpu_extmodel;
	uint64_t	msr, flex_ratio;
	uint8_t		maxcoef, maxdiv, currcoef, currdiv;
	
	do_cpuid(0, cpuid_reg);
	cpu_vendor = cpuid_reg[1];
	
	do_cpuid(1, cpuid_reg);
	cpu_model = bitfield(cpuid_reg[0], 7, 4);
	cpu_family = bitfield(cpuid_reg[0], 11, 8);
	cpu_extmodel = bitfield(cpuid_reg[0], 19, 16);
	cpu_extfamily = bitfield(cpuid_reg[0], 27, 20);
	
	cpu_model += (cpu_extmodel << 4);

	DBG("\nCPU Model: %d - CPU Family: %d - CPU Ext. Family: %d\n", cpu_model, cpu_family, cpu_extfamily);
	DBG("The booter will now attempt to read the CPU Multiplier (using RDMSR).\n");
	DBG("Press any key to continue..\n\n");
#if DEBUG_FREQ
    getc();
#endif

	tscFrequency = measure_tsc_frequency();

	DBG("CPU Multiplier: ");

	if((cpu_vendor == 0x756E6547 /* Intel */) && ((cpu_family == 0x06) || (cpu_family == 0x0f)))
	{
		if ((cpu_family == 0x06 && cpu_model >= 0x0c) ||
			(cpu_family == 0x0f && cpu_model >= 0x03))
		{
			/* Nehalem CPU model */
			if (cpu_family == 0x06 && (cpu_model == 0x1a || cpu_model == 0x1e))
			{
				msr = rdmsr64(MSR_PLATFORM_INFO);
				currcoef = (msr >> 8) & 0xff;
				msr = rdmsr64(MSR_FLEX_RATIO);
				if ((msr >> 16) & 0x01)
				{
					flex_ratio = (msr >> 8) & 0xff;
					if (currcoef > flex_ratio)
						currcoef = flex_ratio;
				}

				if (currcoef)
				{
					DBG("%d\n", currcoef);
					fsbFrequency = (tscFrequency / currcoef);
				}
				cpuFrequency = tscFrequency;
			}
			else
			{
				msr = rdmsr64(IA32_PERF_STATUS);
				currcoef = (msr >> 8) & 0x1f;
				/* Non-integer bus ratio for the max-multi*/
				maxdiv = (msr >> 46) & 0x01;
				/* Non-integer bus ratio for the current-multi (undocumented)*/
				currdiv = (msr >> 14) & 0x01;

				if ((cpu_family == 0x06 && cpu_model >= 0x0e) ||
					(cpu_family == 0x0f)) // This will always be model >= 3
				{
					/* On these models, maxcoef defines TSC freq */
					maxcoef = (msr >> 40) & 0x1f;
				}
				else
				{
					/* On lower models, currcoef defines TSC freq */
					/* XXX */
					maxcoef = currcoef;
				}

				if (maxcoef)
				{
					if (maxdiv)
						fsbFrequency = ((tscFrequency * 2) / ((maxcoef * 2) + 1));
					else
						fsbFrequency = (tscFrequency / maxcoef);

					if (currdiv)
						cpuFrequency = (fsbFrequency * ((currcoef * 2) + 1) / 2);
					else
						cpuFrequency = (fsbFrequency * currcoef);
					DBG("max: %d%s current: %d%s\n", maxcoef, maxdiv ? ".5" : "",currcoef, currdiv ? ".5" : "");
				}
			}
		}
	}
	else if((cpu_vendor == 0x68747541 /* AMD */) && (cpu_family == 0x0f))
	{
		if(cpu_extfamily == 0x00 /* K8 */)
		{
			msr = rdmsr64(K8_FIDVID_STATUS);
			currcoef = (msr & 0x3f) / 2 + 4;
			currdiv = (msr & 0x01) * 2;
		}
		else if(cpu_extfamily >= 0x01 /* K10+ */)
		{
			msr = rdmsr64(K10_COFVID_STATUS);
			if(cpu_extfamily == 0x01 /* K10 */)
				currcoef = (msr & 0x3f) + 0x10;
			else /* K11+ */
				currcoef = (msr & 0x3f) + 0x08;
			currdiv = (2 << ((msr >> 6) & 0x07));
		}

		if (currcoef)
		{
			if (currdiv)
			{
				fsbFrequency = ((tscFrequency * currdiv) / currcoef);
				DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
			}
			else
			{
				fsbFrequency = (tscFrequency / currcoef);
				DBG("%d\n", currcoef);
			}
			fsbFrequency = (tscFrequency / currcoef);
			cpuFrequency = tscFrequency;
		}
	}

	if (!fsbFrequency)
	{
		fsbFrequency = (DEFAULT_FSB * 1000);
		cpuFrequency = tscFrequency;
		DBG("0 ! using the default value for FSB !\n");
	}

	DBG("TSC Frequency:  %dMHz\n", tscFrequency / 1000000);
	DBG("CPU Frequency:  %dMHz\n", cpuFrequency / 1000000);
	DBG("FSB Frequency:  %dMHz\n", fsbFrequency / 1000000);
	DBG("Press [Enter] to continue..\n");
#if DEBUG_FREQ
	while (getc() != 0x0d) ;
#endif
}
