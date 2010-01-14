/*
 *
 * Copyright 2008 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "pci.h"

uint32_t pci_config_read(uint32_t pci_addr, uint8_t reg, uint8_t bytes)
{
	uint32_t data = -1;

	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);

	switch (bytes)
	{
		case 1:
			data = inb(PCI_DATA_REG + (reg & 3));
			break;
		case 2:
			data = inw(PCI_DATA_REG + (reg & 2));
			break;
		case 4:
			data = inl(PCI_DATA_REG);
			break;
	}

	return data;
}

void pci_config_write(uint32_t pci_addr, uint8_t reg, uint32_t data, uint8_t bytes)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);

	switch (bytes)
	{
		case 1:
			outb(PCI_DATA_REG + (reg & 3), data);
			break;
		case 2:
			outw(PCI_DATA_REG + (reg & 2), data);
			break;
		case 4:
			outl(PCI_DATA_REG, data);
			break;
	}
}

pci_dt_t *root_pci_dev;

void scan_pci_bus(pci_dt_t *start, uint8_t bus)
{
	uint8_t dev, func, secondary_bus, header_type;
	uint32_t id, pci_addr;
	pci_dt_t *new;
	pci_dt_t **current = &start->children;
	
	for (dev = 0; dev < 32; dev++)
		for (func = 0; func < 8; func++)
		{
			pci_addr = PCIADDR(bus, dev, func);
			id = pci_config_read32(pci_addr, PCI_VENDOR_ID);
			if (!id || id == 0xffffffff)
				continue;

			new = (pci_dt_t*)malloc(sizeof(pci_dt_t));
			if (!new)
				return;
			memset(new, 0, sizeof(pci_dt_t));
			
			new->dev.addr	= pci_addr;
			new->vendor_id	= id & 0xffff;
			new->device_id	= (id >> 16) & 0xffff;
			new->class_id	= pci_config_read16(pci_addr, PCI_CLASS_DEVICE);
			new->parent		= start;

			header_type = pci_config_read8(pci_addr, PCI_HEADER_TYPE);
			switch (header_type & 0x7f)
			{
				case PCI_HEADER_TYPE_BRIDGE:
				case PCI_HEADER_TYPE_CARDBUS:
					secondary_bus = pci_config_read8(pci_addr, PCI_SECONDARY_BUS);
					if (secondary_bus != 0)
						scan_pci_bus(new, secondary_bus);
					break;
			}

			*current = new;
			current = &new->next;

			if ((func == 0) && ((header_type & 0x80) == 0))
				break;
		}
}

void enable_pci_devs(void)
{
	uint16_t id;
	uint32_t rcba, *fd;

	id = pci_config_read16(PCIADDR(0, 0x00, 0), 0x00);
	/* make sure we're on Intel chipset */
	if (id != 0x8086)
		return;
	rcba = pci_config_read32(PCIADDR(0, 0x1f, 0), 0xf0) & ~1;
	fd = (uint32_t *)(rcba + 0x3418);
	/* set SMBus Disable (SD) to 0 */
	*fd &= ~0x8;
	/* and all devices? */
	//*fd = 0x1;
}

void build_pci_dt(void)
{
	root_pci_dev = malloc(sizeof(pci_dt_t));
	
	if (!root_pci_dev)
		return;
	
	bzero(root_pci_dev, sizeof(pci_dt_t));
	enable_pci_devs();
	scan_pci_bus(root_pci_dev, 0);
}

char dev_path[80];

char *get_pci_dev_path(pci_dt_t *pci_dt)
{
	pci_dt_t *current, *end;
	char tmp[30];

	dev_path[0] = 0;
	end = root_pci_dev;

	while (end != pci_dt)
	{
		current = pci_dt;
		while (current->parent != end)
			current = current->parent;			
		end = current;

		sprintf(tmp, "%s/Pci(0x%x,0x%x)",
			(current->parent == root_pci_dev) ? "PciRoot(0x0)" : "",
			current->dev.bits.dev, current->dev.bits.func);
		strcat(dev_path, tmp);
	}

	return dev_path;
}

void dump_pci_dt(pci_dt_t *pci_dt)
{
	pci_dt_t *current = pci_dt;

	while (current)
	{
		printf("%02x:%02x.%x [%04x] [%04x:%04x] :: %s\n", 
			current->dev.bits.bus, current->dev.bits.dev, current->dev.bits.func, 
			current->class_id, current->vendor_id, current->device_id, 
			get_pci_dev_path(current));
		dump_pci_dt(current->children);
		current = current->next;
	}
}


void lspci(const char *booterParam)
{
	if(bootArgs->Video.v_display == VGA_TEXT_MODE)
	{ 
		setActiveDisplayPage(1);
		clearScreenRows(0, 24);
		setCursorPosition(0, 0, 1);
	}
		
	dump_pci_dt(root_pci_dev->children);
	
	printf("(Press a key to continue...)");
	getc();
	
	if(bootArgs->Video.v_display == VGA_TEXT_MODE)
		setActiveDisplayPage(0);
}

int check_vga_nvidia(pci_dt_t *pci_dt)
{
	pci_dt_t *current = pci_dt;
	while (current)
	{
		if(current->vendor_id == PCI_CLASS_DISPLAY_VGA)
			if(current->class_id == PCI_VENDOR_ID_NVIDIA)
				return 1;
		
		check_vga_nvidia(current->children);
		current = current->next;
	}
	return 0;
}

