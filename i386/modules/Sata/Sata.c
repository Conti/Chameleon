/*
 * Copyright (c) 2011,2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "bootstruct.h"
#include "pci.h"
#include "device_inject.h"
#include "platform.h"

#ifndef DEBUG_SATA
#define DEBUG_SATA 0
#endif

#if DEBUG_SATA
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif

void SATA_hook(void* arg1, void* arg2, void* arg3, void* arg4);

uint8_t default_SATA_ID[]= {
	0x81, 0x26, 0x00, 0x00
};
#define SATA_ID_LEN ( sizeof(default_SATA_ID) / sizeof(uint8_t) )

void SATA_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	pci_dt_t* current = arg1;
	struct DevPropDevice		*device = NULL;
	char *devicepath = NULL;
	
	if (current && current->class_id == PCI_CLASS_STORAGE_SATA)
	{        
        if (!string)
        {
            string = devprop_create_string();
            if (!string) return;
        }
		devicepath = get_pci_dev_path(current);
        if (!devicepath) return;
		
        device = devprop_add_device(string, devicepath);
        if (!device) return;
        
        devprop_add_value(device, "device-id", default_SATA_ID, SATA_ID_LEN);
        
        verbose("SATA device : [%04x:%04x :: %04x], changed to ICH6 ESB2 \n",  
                current->vendor_id, current->device_id,current->class_id);
		
	}	
	
}

void Sata_start(void);
void Sata_start(void)
{
    register_hook_callback("PCIDevice", &SATA_hook);
}

