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
#include "spd.h"
#include "dram_controllers.h"

#ifndef DEBUG_PLATFORM
#define DEBUG_PLATFORM 0
#endif

#if DEBUG_PLATFORM
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

PlatformInfo_t    Platform;
pci_dt_t * dram_controller_dev = NULL;

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
void scan_mem()
{
	static bool done = false;
	if (done) {
		return;
	}

	/* our code only works on Intel chipsets so make sure here */
	if (pci_config_read16(PCIADDR(0, 0x00, 0), 0x00) != 0x8086) {
		bootInfo->memDetect = false;
	} else {
		bootInfo->memDetect = true;
	}
	/* manually */
	getBoolForKey(kUseMemDetect, &bootInfo->memDetect, &bootInfo->chameleonConfig);

	if (bootInfo->memDetect) {
		if (dram_controller_dev != NULL) {
			scan_dram_controller(dram_controller_dev); // Rek: pci dev ram controller direct and fully informative scan ...
		}
		scan_spd(&Platform);
	}
	done = true;
}

/*
 * Scan platform hardware information, called by the main entry point (common_boot() ) 
 * _before_ bootConfig xml parsing settings are loaded
*/
void scan_platform(void)
{
	memset(&Platform, 0, sizeof(Platform));
	build_pci_dt();
	scan_cpu(&Platform);
	//scan_mem(); Rek: called after pci devs init in fake_efi now ...
}
