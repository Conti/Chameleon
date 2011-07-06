/*
 *  usb.c
 *  
 *
 *  Created by mackerintel on 12/20/08.
 *  Copyright 2008 mackerintel. All rights reserved.
 *
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"

#ifndef DEBUG_USB
#define DEBUG_USB 0
#endif

#if DEBUG_USB
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif


struct pciList
{
	pci_dt_t* pciDev;
	struct pciList* next;
};

struct pciList* usbList = NULL;

int legacy_off (pci_dt_t *pci_dev);
int ehci_acquire (pci_dt_t *pci_dev);
int uhci_reset (pci_dt_t *pci_dev);

// Add usb device to the list
void notify_usb_dev(pci_dt_t *pci_dev)
{
	struct pciList* current = usbList;
	if(!usbList)
	{
		usbList = (struct pciList*)malloc(sizeof(struct pciList));
		usbList->next = NULL;
		usbList->pciDev = pci_dev;
		
	}
	else
	{
		while(current != NULL && current->next != NULL)
		{
			current = current->next;
		}
		current->next = (struct pciList*)malloc(sizeof(struct pciList));
		current = current->next;
		
		current->pciDev = pci_dev;
		current->next = NULL;
	}
}

// Loop through the list and call the apropriate patch function
int usb_loop()
{
	int retVal = 1;
	bool fix_ehci, fix_uhci, fix_usb, fix_legacy;
	fix_ehci = fix_uhci = fix_usb = fix_legacy = false;
	
	if (getBoolForKey(kUSBBusFix, &fix_usb, &bootInfo->chameleonConfig))
	{
		fix_ehci = fix_uhci = fix_legacy = fix_usb;	// Disable all if none set
	}
	else 
	{
		getBoolForKey(kEHCIacquire, &fix_ehci, &bootInfo->chameleonConfig);
		getBoolForKey(kUHCIreset, &fix_uhci, &bootInfo->chameleonConfig);
		getBoolForKey(kLegacyOff, &fix_legacy, &bootInfo->chameleonConfig);
	}
	
	struct pciList* current = usbList;
	
	while(current)
	{
		switch (pci_config_read8(current->pciDev->dev.addr, PCI_CLASS_PROG))
		{
			// EHCI
			case 0x20:
		    	if(fix_ehci)   retVal &= ehci_acquire(current->pciDev);
		    	if(fix_legacy) retVal &= legacy_off(current->pciDev);
				
				break;
				
			// UHCI
			case 0x00:
				if (fix_uhci) retVal &= uhci_reset(current->pciDev);

				break;
		}
		
		current = current->next;
	}
	return retVal;
}

int legacy_off (pci_dt_t *pci_dev)
{
	// Set usb legacy off modification by Signal64
	// NOTE: This *must* be called after the last file is loaded from the drive in the event that we are booting form usb.
	// NOTE2: This should be called after any getc()/getchar() call. (aka, after the Wait=y keyworkd is used)
	// AKA: Make this run immediatly before the kernel is called
	uint32_t	capaddr, opaddr;  		
	uint8_t		eecp;			
	uint32_t	usbcmd, usbsts, usbintr;			
	uint32_t	usblegsup, usblegctlsts;		
	
	int isOSowned;
	int isBIOSowned;
	
	verbose("Setting Legacy USB Off on controller [%04x:%04x] at %02x:%2x.%x\n", 
			pci_dev->vendor_id, pci_dev->device_id,
			pci_dev->dev.bits.bus, pci_dev->dev.bits.dev, pci_dev->dev.bits.func);
	
	
	// capaddr = Capability Registers = dev.addr + offset stored in dev.addr + 0x10 (USBBASE)
	capaddr = pci_config_read32(pci_dev->dev.addr, 0x10);	
	
	// opaddr = Operational Registers = capaddr + offset (8bit CAPLENGTH in Capability Registers + offset 0)
	opaddr = capaddr + *((unsigned char*)(capaddr)); 		
	
	// eecp = EHCI Extended Capabilities offset = capaddr HCCPARAMS bits 15:8
	eecp=*((unsigned char*)(capaddr + 9));
	
	DBG("capaddr=%x opaddr=%x eecp=%x\n", capaddr, opaddr, eecp);
	
	usbcmd = *((unsigned int*)(opaddr));			// Command Register
	usbsts = *((unsigned int*)(opaddr + 4));		// Status Register
	usbintr = *((unsigned int*)(opaddr + 8));		// Interrupt Enable Register
	
	DBG("usbcmd=%08x usbsts=%08x usbintr=%08x\n", usbcmd, usbsts, usbintr);
	
	// read PCI Config 32bit USBLEGSUP (eecp+0) 
	usblegsup = pci_config_read32(pci_dev->dev.addr, eecp);
	
	// informational only
	isBIOSowned = !!((usblegsup) & (1 << (16)));
	isOSowned = !!((usblegsup) & (1 << (24)));
	
	// read PCI Config 32bit USBLEGCTLSTS (eecp+4) 
	usblegctlsts = pci_config_read32(pci_dev->dev.addr, eecp + 4);
	
	DBG("usblegsup=%08x isOSowned=%d isBIOSowned=%d usblegctlsts=%08x\n", usblegsup, isOSowned, isBIOSowned, usblegctlsts);
	
	// Reset registers to Legacy OFF
	DBG("Clearing USBLEGCTLSTS\n");
	pci_config_write32(pci_dev->dev.addr, eecp + 4, 0);	//usblegctlsts
	
	// if delay value is in milliseconds it doesn't appear to work. 
	// setting value to anything up to 65535 does not add the expected delay here.
	delay(100);
	
	usbcmd = *((unsigned int*)(opaddr));
	usbsts = *((unsigned int*)(opaddr + 4));
	usbintr = *((unsigned int*)(opaddr + 8));
	
	DBG("usbcmd=%08x usbsts=%08x usbintr=%08x\n", usbcmd, usbsts, usbintr);
	
	DBG("Clearing Registers\n");
	
	// clear registers to default
	usbcmd = (usbcmd & 0xffffff00);
	*((unsigned int*)(opaddr)) = usbcmd;
	*((unsigned int*)(opaddr + 8)) = 0;					//usbintr - clear interrupt registers
	*((unsigned int*)(opaddr + 4)) = 0x1000;			//usbsts - clear status registers 	
	pci_config_write32(pci_dev->dev.addr, eecp, 1);		//usblegsup
	
	// get the results
	usbcmd = *((unsigned int*)(opaddr));
	usbsts = *((unsigned int*)(opaddr + 4));
	usbintr = *((unsigned int*)(opaddr + 8));
	
	DBG("usbcmd=%08x usbsts=%08x usbintr=%08x\n", usbcmd, usbsts, usbintr);
	
	// read 32bit USBLEGSUP (eecp+0) 
	usblegsup = pci_config_read32(pci_dev->dev.addr, eecp);
	
	// informational only
	isBIOSowned = !!((usblegsup) & (1 << (16)));
	isOSowned = !!((usblegsup) & (1 << (24)));
	
	// read 32bit USBLEGCTLSTS (eecp+4) 
	usblegctlsts = pci_config_read32(pci_dev->dev.addr, eecp + 4);
	
	DBG("usblegsup=%08x isOSowned=%d isBIOSowned=%d usblegctlsts=%08x\n", usblegsup, isOSowned, isBIOSowned, usblegctlsts);
	
	verbose("Legacy USB Off Done\n");	
	return 1;
}

int ehci_acquire (pci_dt_t *pci_dev)
{
	int		j, k;
	uint32_t	base;
	uint8_t		eecp;
	uint8_t		legacy[8];
	bool		isOwnershipConflict;	
	bool		alwaysHardBIOSReset;

	alwaysHardBIOSReset = false;	
	if (!getBoolForKey(kEHCIhard, &alwaysHardBIOSReset, &bootInfo->chameleonConfig)) {
		alwaysHardBIOSReset = true;
	}

	pci_config_write16(pci_dev->dev.addr, 0x04, 0x0002);
	base = pci_config_read32(pci_dev->dev.addr, 0x10);

	verbose("EHCI controller [%04x:%04x] at %02x:%2x.%x DMA @%x\n", 
		pci_dev->vendor_id, pci_dev->device_id,
		pci_dev->dev.bits.bus, pci_dev->dev.bits.dev, pci_dev->dev.bits.func, 
		base);

	if (*((unsigned char*)base) < 0xc)
	{
		DBG("Config space too small: no legacy implementation\n");
		return 1;
	}
	eecp = *((unsigned char*)(base + 9));
	if (!eecp) {
		DBG("No extended capabilities: no legacy implementation\n");
		return 1;
	}

	DBG("eecp=%x\n",eecp);

	// bad way to do it
	// pci_conf_write(pci_dev->dev.addr, eecp, 4, 0x01000001);
	for (j = 0; j < 8; j++) {
		legacy[j] = pci_config_read8(pci_dev->dev.addr, eecp + j);
		DBG("%02x ", legacy[j]);
	}
	DBG("\n");

	//Real Job: based on orByte's AppleUSBEHCI.cpp
	//We try soft reset first - some systems hang on reboot with hard reset
	// Definitely needed during reboot on 10.4.6

	isOwnershipConflict = ((legacy[3] & 1 !=  0) && (legacy[2] & 1 !=  0));
	if (!alwaysHardBIOSReset && isOwnershipConflict) {
		DBG("EHCI - Ownership conflict - attempting soft reset ...\n");
		DBG("EHCI - toggle OS Ownership to 0\n");
		pci_config_write8(pci_dev->dev.addr, eecp + 3, 0);
		for (k = 0; k < 25; k++) {
			for (j = 0; j < 8; j++) {
				legacy[j] = pci_config_read8(pci_dev->dev.addr, eecp + j);
			}
			if (legacy[3] == 0) {
				break;
			}
			delay(10);
		}
	}	

	DBG("Found USBLEGSUP_ID - value %x:%x - writing OSOwned\n", legacy[3],legacy[2]);
	pci_config_write8(pci_dev->dev.addr, eecp + 3, 1);

	// wait for kEHCI_USBLEGSUP_BIOSOwned bit to clear
	for (k = 0; k < 25; k++) {
		for (j = 0;j < 8; j++) {
			legacy[j] = pci_config_read8(pci_dev->dev.addr, eecp + j);
		}
		DBG ("%x:%x,",legacy[3],legacy[2]);
		if (legacy[2] == 0) {
			break;
		}
		delay(10);
	}

	for (j = 0;j < 8; j++) {
		legacy[j] = pci_config_read8(pci_dev->dev.addr, eecp + j);
	}
	isOwnershipConflict = ((legacy[2]) != 0);
	if (isOwnershipConflict) {
		// Soft reset has failed. Assume SMI being ignored
		// Hard reset
		// Force Clear BIOS BIT
		DBG("EHCI - Ownership conflict - attempting hard reset ...\n");			
		DBG ("%x:%x\n",legacy[3],legacy[2]);
		DBG("EHCI - Force BIOS Ownership to 0\n");

		pci_config_write8(pci_dev->dev.addr, eecp + 2, 0);
		for (k = 0; k < 25; k++) {
			for (j = 0; j < 8; j++) {
				legacy[j] = pci_config_read8(pci_dev->dev.addr, eecp + j);
			}
			DBG ("%x:%x,",legacy[3],legacy[2]);

			if ((legacy[2]) == 0) {
				break;
			}
			delay(10);	
		}		
		// Disable further SMI events
		for (j = 4; j < 8; j++) {
			pci_config_write8(pci_dev->dev.addr, eecp + j, 0);
		}
	}

	for (j = 0; j < 8; j++) {
		legacy[j] = pci_config_read8(pci_dev->dev.addr, eecp + j);
	}

	DBG ("%x:%x\n",legacy[3],legacy[2]);

	// Final Ownership Resolution Check...
	if (legacy[2] & 1) {					
		DBG("EHCI controller unable to take control from BIOS\n");
		return 0;
	}

	DBG("EHCI Acquire OS Ownership done\n");	
	return 1;
}

int uhci_reset (pci_dt_t *pci_dev)
{
	uint32_t base, port_base;
	
	base = pci_config_read32(pci_dev->dev.addr, 0x20);
	port_base = (base >> 5) & 0x07ff;

	verbose("UHCI controller [%04x:%04x] at %02x:%2x.%x base %x(%x)\n", 
		pci_dev->vendor_id, pci_dev->device_id,
		pci_dev->dev.bits.bus, pci_dev->dev.bits.dev, pci_dev->dev.bits.func, 
		port_base, base);
	
	pci_config_write16(pci_dev->dev.addr, 0xc0, 0x8f00);

	outw (port_base, 0x0002);
	delay(10);
	outw (port_base+4,0);
	delay(10);
	outw (port_base,0);
	return 1;
}
