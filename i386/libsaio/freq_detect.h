/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 */

#ifndef __LIBSAIO_FREQ_DETECT_H
#define __LIBSAIO_FREQ_DETECT_H

#include "libsaio.h"
#ifndef DEBUG_FREQ
#define DEBUG_FREQ 0
#endif

#if DEBUG_FREQ
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

/* Decimal powers: */
#define kilo (1000ULL)
#define Mega (kilo * kilo)
#define Giga (kilo * Mega)
#define Tera (kilo * Giga)
#define Peta (kilo * Tera)

#define bit(n)			(1ULL << (n))
#define bitmask(h,l)	((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)	(((x) & bitmask(h,l)) >> l)

#define	IA32_PERF_STATUS	0x198
#define MSR_FLEX_RATIO		0x194
#define	MSR_PLATFORM_INFO	0xCE
#define K8_FIDVID_STATUS	0xC0010042
#define K10_COFVID_STATUS	0xC0010071

#define DEFAULT_FSB			100000          /* for now, hardcoding 100MHz for old CPUs */

// DFE: This constant comes from older xnu:
#define CLKNUM		1193182		/* formerly 1193167 */

// DFE: These two constants come from Linux except CLOCK_TICK_RATE replaced with CLKNUM
#define CALIBRATE_TIME_MSEC 30 /* 30 msecs */
#define CALIBRATE_LATCH	\
	((CLKNUM * CALIBRATE_TIME_MSEC + 1000/2)/1000)

extern uint64_t tscFrequency;
extern uint64_t fsbFrequency;
extern uint64_t cpuFrequency;

void calculate_freq(void);

static inline uint64_t rdtsc64(void)
{
	uint64_t ret;
	__asm__ volatile("rdtsc" : "=A" (ret));
	return ret;
}

static inline uint64_t rdmsr64(uint32_t msr)
{
    uint64_t ret;
    __asm__ volatile("rdmsr" : "=A" (ret) : "c" (msr));
    return ret;
}

static inline void do_cpuid(uint32_t selector, uint32_t *data)
{
	asm volatile ("cpuid"
				  : "=a" (data[0]),
				  "=b" (data[1]),
				  "=c" (data[2]),
				  "=d" (data[3])
				  : "a" (selector)
				  );
}

#endif /* !__LIBSAIO_FREQ_DETECT_H */
