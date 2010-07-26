/*
 * dram controller access and scan from the pci host controller
 * Integrated and adapted for chameleon 2.0 RC5 by Rekursor from bs0d work
 * original source comes from:
 *
 * memtest86
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady, cbrady@sgi.com
 * ----------------------------------------------------
 * MemTest86+ V4.00 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.canardpc.com - http://www.memtest.org
 */

#ifndef __LIBSAIO_DRAM_CONTROLLERS_H
#define __LIBSAIO_DRAM_CONTROLLERS_H

#include "libsaio.h"

void scan_dram_controller();

struct mem_controller_t {
	uint16_t vendor;
	uint16_t device;
	char *name;
	void (*initialise)(pci_dt_t *dram_dev);
	void (*poll_speed)(pci_dt_t *dram_dev);
	void (*poll_timings)(pci_dt_t *dram_dev);
};

#endif /* !__LIBSAIO_DRAM_CONTROLLERS_H */
