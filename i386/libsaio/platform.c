/*
 *  platform.c
 *
 * AsereBLN: cleanup
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "cpu.h"
#include "mem.h"
#include "spd.h"

#ifndef DEBUG_PLATFORM
#define DEBUG_PLATFORM 0
#endif

#if DEBUG_PLATFORM
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

PlatformInfo_t    Platform;

/** Return if a CPU feature specified by feature is activated (true) or not (false)  */
bool platformCPUFeature(uint32_t feature)
{
	if (Platform.CPU.Features & feature) {
		return true;
	} else {
		return false;
	}
}

/** scan mem for memory autodection purpose */
void scan_mem() {
    static bool done = false;
    if (done) return;

    bool useAutodetection = true;
    getBoolForKey(kUseMemDetect, &useAutodetection, &bootInfo->bootConfig);

    if (useAutodetection) {
        scan_memory(&Platform); // unfortunately still necesary for some comp where spd cant read correct speed
        scan_spd(&Platform);
    }
    done = true;
}

/** 
    Scan platform hardware information, called by the main entry point (common_boot() ) 
    _before_ bootConfig xml parsing settings are loaded
*/
void scan_platform(void)
{
	memset(&Platform, 0, sizeof(Platform));
	build_pci_dt();
	scan_cpu(&Platform);
	// It's working after some changes in strdup
	scan_mem();
}
