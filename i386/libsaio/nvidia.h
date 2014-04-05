/*
 *	NVidia injector
 *
 *	Copyright (C) 2009	Jasmin Fazlic, iNDi
 *
 *	NVidia injector modified by Fabio (ErmaC) on May 2012,
 *	for allow the cosmetics injection also based on SubVendorID and SubDeviceID.
 *
 *	NVidia injector is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	NVidia driver and injector is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with NVidia injector.	 If not, see <http://www.gnu.org/licenses/>.
 *
 *	Alternatively you can choose to comply with APSL
 *
 *	DCB-Table parsing is based on software (nouveau driver) originally distributed under following license:
 *
 *
 *	Copyright 2005-2006 Erik Waling
 *	Copyright 2006 Stephane Marchesin
 *	Copyright 2007-2009 Stuart Bennett
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a
 *	copy of this software and associated documentation files (the "Software"),
 *	to deal in the Software without restriction, including without limitation
 *	the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *	and/or sell copies of the Software, and to permit persons to whom the
 *	Software is furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in
 *	all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *	THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *	OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#ifndef __LIBSAIO_NVIDIA_H
#define __LIBSAIO_NVIDIA_H

bool setup_nvidia_devprop(pci_dt_t *nvda_dev);

struct nvidia_pci_info_t;
typedef struct {
	uint32_t    device; // VendorID + DeviceID
	char        *name;
} nvidia_pci_info_t;

struct nvidia_card_info_t;
typedef struct {
	uint32_t    device; // VendorID + DeviceID
	uint32_t    subdev; // SubdeviceID + SubvendorID
	char        *name;
	//bool        kEnableHDMIAudio   //	HDMi
	//VRAM
} nvidia_card_info_t;

#define DCB_MAX_NUM_ENTRIES 16
#define DCB_MAX_NUM_I2C_ENTRIES 16
#define DCB_MAX_NUM_GPIO_ENTRIES 32
#define DCB_MAX_NUM_CONNECTOR_ENTRIES 16
#define DCB_LOC_ON_CHIP 0

struct bios {
	uint16_t	signature;		/* 0x55AA */
	uint8_t		size;			/* Size in multiples of 512 */
};

#define NV_PMC_OFFSET							0x00000000
#define NV_PMC_SIZE                                                     0x00001000 // 0x2ffff
#define NV_PDISPLAY_OFFSET						0x610000
#define NV_PDISPLAY_SIZE						0x10000

#define NV_PROM_OFFSET							0x00300000
#define NV_PROM_SIZE							0x00010000
#define NV_PRAMIN_OFFSET						0x00700000
#define NV_PRAMIN_SIZE							0x00100000
#define NV04_PFB_FIFO_DATA						0x0010020c
#define NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_MASK				0xfff00000
#define NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_SHIFT				20
#define NVC0_MEM_CTRLR_RAM_AMOUNT					0x0010f20c
#define NVC0_MEM_CTRLR_COUNT						0x00121c74

#define NV_PBUS_PCI_NV_19						0x0000184C
#define NV_PBUS_PCI_NV_20						0x00001850
#define NV_PBUS_PCI_NV_20_ROM_SHADOW_DISABLED	(0 << 0)
#define NV_PBUS_PCI_NV_20_ROM_SHADOW_ENABLED	(1 << 0)

#define REG8(reg)  ((volatile uint8_t *)regs)[(reg)]
#define REG16(reg)  ((volatile uint16_t *)regs)[(reg) >> 1]
#define REG32(reg)  ((volatile uint32_t *)regs)[(reg) >> 2]

#define NV_ARCH_03              0x03
#define NV_ARCH_04              0x04
#define NV_ARCH_10              0x10
#define NV_ARCH_20              0x20
#define NV_ARCH_30              0x30
#define NV_ARCH_40              0x40
#define NV_ARCH_50              0x50
#define NV_ARCH_C0              0xC0
#define NV_ARCH_D0              0xD0
#define NV_ARCH_E0              0xE0

#define CHIPSET_NV03            0x0010
#define CHIPSET_NV04            0x0020
#define CHIPSET_NV10            0x0100
#define CHIPSET_NV11            0x0110
#define CHIPSET_NV15            0x0150
#define CHIPSET_NV17            0x0170
#define CHIPSET_NV18            0x0180
#define CHIPSET_NFORCE          0x01A0
#define CHIPSET_NFORCE2         0x01F0
#define CHIPSET_NV20            0x0200
#define CHIPSET_NV25            0x0250
#define CHIPSET_NV28            0x0280
#define CHIPSET_NV30            0x0300
#define CHIPSET_NV31            0x0310
#define CHIPSET_NV34            0x0320
#define CHIPSET_NV35            0x0330
#define CHIPSET_NV36            0x0340
#define CHIPSET_NV40            0x0040
#define CHIPSET_NV41            0x00C0
#define CHIPSET_NV43            0x0140
#define CHIPSET_NV44            0x0160
#define CHIPSET_NV44A           0x0220
#define CHIPSET_NV45            0x0210
#define CHIPSET_NV50            0x0190
#define CHIPSET_NV84            0x0400
#define CHIPSET_MISC_BRIDGED    0x00F0
#define CHIPSET_G70             0x0090
#define CHIPSET_G71             0x0290
#define CHIPSET_G72             0x01D0
#define CHIPSET_G73             0x0390

// integrated GeForces (6100, 6150)
#define CHIPSET_C51             0x0240

// variant of C51, seems based on a G70 design
#define CHIPSET_C512            0x03D0
#define CHIPSET_G73_BRIDGED     0x02E0

#endif /* !__LIBSAIO_NVIDIA_H */
