/*
 *	HDA injector / Audio Enabler
 *
 *	Copyright (C) 2012	Chameleon Team
 *	Edit by Fabio (ErmaC)
 *
 *	HDA injector is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	HDA injector is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Alternatively you can choose to comply with APSL
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
 ******************************************************************************
 * http://www.leidinger.net/FreeBSD/dox/dev_sound/html/df/d54/hdac_8c_source.html
 *
 * Copyright (c) 2006 Stephane E. Potvin <sepotvin@videotron.ca>
 * Copyright (c) 2006 Ariff Abdullah <ariff@FreeBSD.org>
 * Copyright (c) 2008-2012 Alexander Motin <mav@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Intel High Definition Audio (Controller) driver for FreeBSD.
 *
 ******************************************************************************/

#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "pci_root.h"
#include "platform.h"
#include "device_inject.h"
#include "hda.h"
#include "aml_generator.h"

#ifndef DEBUG_HDA
#define DEBUG_HDA 0
#endif

#if DEBUG_HDA
#define DBG(x...)  verbose(x)
#else
#define DBG(x...)
#endif

extern uint32_t devices_number;

const char *hda_slot_name[]		=	{ "AAPL,slot-name", "Built In" };

uint8_t default_HDEF_layout_id[]		=	{0x0C, 0x00, 0x00, 0x00};
#define HDEF_LEN ( sizeof(default_HDEF_layout_id) / sizeof(uint8_t) )
uint8_t default_HDAU_layout_id[]		=	{0x01, 0x00, 0x00, 0x00};
#define HDAU_LEN ( sizeof(default_HDAU_layout_id) / sizeof(uint8_t) )
uint8_t connector_type_value[]          =	{0x00, 0x08, 0x00, 0x00};

/* Structures */

static hda_controller_devices know_hda_controller[] = {
	//8086  Intel Corporation
	{ HDA_INTEL_HASWELL,     "Haswell" },
	{ HDA_INTEL_CRYSTAL,     "Crystal Well" },
	{ HDA_INTEL_CPT,     "6 Series/C200 Series Chipset Family" },
	{ HDA_INTEL_PATSBURG,"C600/X79 series chipset" },
	{ HDA_INTEL_PPT1,    "7 Series/C210 Series Chipset Family" },
	{ HDA_INTEL_82801F,  "82801FB/FBM/FR/FW/FRW (ICH6 Family)" },
	{ HDA_INTEL_63XXESB, "631x/631xESB/632xESB" },
	{ HDA_INTEL_82801G,  "NM10/ICH7 Family" },
	{ HDA_INTEL_82801H,  "82801H (ICH8 Family)" },
	{ HDA_INTEL_82801I,  "82801I (ICH9 Family)" },
	{ HDA_INTEL_82801JI, "82801JI (ICH10 Family)" },
	{ HDA_INTEL_82801JD, "82801JD/DO (ICH10 Family) " },
	{ HDA_INTEL_PCH,     "5 Series/3400 Series" },
	{ HDA_INTEL_PCH2,    "5 Series/3400 Series" },
	{ HDA_INTEL_SCH,     "System Controller Hub (SCH Poulsbo)" },
	{ HDA_INTEL_LPT1,     "Lynx Point" },
	{ HDA_INTEL_LPT2,     "Lynx Point" },
	{ HDA_INTEL_LYNX,     "Lynx Point-LP" },
	{ HDA_INTEL_LYNX2,     "Lynx Point-LP" },
	//10de  NVIDIA Corporation
	{ HDA_NVIDIA_MCP51,  "MCP51" },
	{ HDA_NVIDIA_MCP55,  "MCP55" },
	{ HDA_NVIDIA_MCP61_1, "MCP61" },
	{ HDA_NVIDIA_MCP61_2, "MCP61" },
	{ HDA_NVIDIA_MCP65_1, "MCP65" },
	{ HDA_NVIDIA_MCP65_2, "MCP65" },
	{ HDA_NVIDIA_MCP67_1, "MCP67" },
	{ HDA_NVIDIA_MCP67_2, "MCP67" },
	{ HDA_NVIDIA_MCP73_1, "MCP73" },
	{ HDA_NVIDIA_MCP73_2, "MCP73" },
	{ HDA_NVIDIA_MCP78_1, "MCP78" },
	{ HDA_NVIDIA_MCP78_2, "MCP78" },
	{ HDA_NVIDIA_MCP78_3, "MCP78" },
	{ HDA_NVIDIA_MCP78_4, "MCP78" },
	{ HDA_NVIDIA_MCP79_1, "MCP79" },
	{ HDA_NVIDIA_MCP79_2, "MCP79" },
	{ HDA_NVIDIA_MCP79_3, "MCP79" },
	{ HDA_NVIDIA_MCP79_4, "MCP79" },
	{ HDA_NVIDIA_MCP89_1, "MCP89" },
	{ HDA_NVIDIA_MCP89_2, "MCP89" },
	{ HDA_NVIDIA_MCP89_3, "MCP89" },
	{ HDA_NVIDIA_MCP89_4, "MCP89" },
	{ HDA_NVIDIA_0BE2,   "(0x0be2)" },
	{ HDA_NVIDIA_0BE3,   "(0x0be3)" },
	{ HDA_NVIDIA_0BE4,   "(0x0be4)" },
	{ HDA_NVIDIA_GT100,  "GT100" },
	{ HDA_NVIDIA_GT104,  "GT104" },
	{ HDA_NVIDIA_GT106,  "GT106" },
	{ HDA_NVIDIA_GT108,  "GT108" },
	{ HDA_NVIDIA_GT116,  "GT116" },
	{ HDA_NVIDIA_GF119,  "GF119" },
	{ HDA_NVIDIA_GF110,  "GF110" },
	{ HDA_NVIDIA_GF114,  "GF114" }, // HDMi
	{ HDA_NVIDIA_GK110,  "GK110" },
	{ HDA_NVIDIA_GK106,  "GK106" },
	{ HDA_NVIDIA_GK107,  "GK107" },
	{ HDA_NVIDIA_GK104,  "GK104" },
	//1002  Advanced Micro Devices [AMD] nee ATI Technologies Inc
	{ HDA_ATI_SB450,     "IXP SB4x0" },
	{ HDA_ATI_SB600,     "SB600" },
	{ HDA_ATI_RS600,     "RS600" },
	{ HDA_ATI_RS690,     "RS690" },
	{ HDA_ATI_RS780,     "RS780" },
	{ HDA_ATI_RS880,     "RS880" },
	{ HDA_ATI_TRINITY,   "Trinity" },
	{ HDA_ATI_R600,      "R600" },
	{ HDA_ATI_RV610,     "RV610" },
	{ HDA_ATI_RV620,     "RV620" },
	{ HDA_ATI_RV630,     "RV630" },
	{ HDA_ATI_RV635,     "RV635" },
	{ HDA_ATI_RV710,     "RV710" },
	{ HDA_ATI_RV730,     "RV730" },
	{ HDA_ATI_RV740,     "RV740" },
	{ HDA_ATI_RV770,     "RV770" },
	{ HDA_ATI_RV810,     "RV810" },
	{ HDA_ATI_RV830,     "RV830" },
	{ HDA_ATI_RV840,     "RV840" },
	{ HDA_ATI_RV870,     "RV870" },
	{ HDA_ATI_RV910,     "Caicos" },
	{ HDA_ATI_RV930,     "RV930" },
	{ HDA_ATI_RV940,     "RV940" },
	{ HDA_ATI_RV970,     "RV970" },
	{ HDA_ATI_R1000,     "Tahiti XT" }, // HDMi
	{ HDA_ATI_VERDE,     "Cape Verde" }, // HDMi
	//17f3  RDC Semiconductor, Inc.
	{ HDA_RDC_M3010,     "M3010" },
	//1106  VIA Technologies, Inc.
	{ HDA_VIA_VT82XX,    "VT8251/8237A" },
	//1039  Silicon Integrated Systems [SiS]
	{ HDA_SIS_966,       "966" },
	//10b9  ULi Electronics Inc.(Split off ALi Corporation in 2003)
	{ HDA_ULI_M5461,     "M5461" },
	/* Unknown */
	{ HDA_INTEL_ALL,  "Unknown Intel device" },
	{ HDA_NVIDIA_ALL, "Unknown NVIDIA device" },
	{ HDA_ATI_ALL,    "Unknown ATI device" },
	{ HDA_VIA_ALL,    "Unknown VIA device" },
	{ HDA_SIS_ALL,    "Unknown SiS device" },
	{ HDA_ULI_ALL,    "Unknown ULI device" },
};
#define HDAC_DEVICES_LEN (sizeof(know_hda_controller) / sizeof(know_hda_controller[0]))

/* CODECs */

// ErmaC: TODO build function to probe the codecID
/*
static hdacc_codecs know_codecs[] = {
    { HDA_CODEC_CS4206, 0,          "Cirrus Logic CS4206" },
    { HDA_CODEC_CS4207, 0,          "Cirrus Logic CS4207" },
    { HDA_CODEC_CS4210, 0,          "Cirrus Logic CS4210" },
    { HDA_CODEC_ALC221, 0,          "Realtek ALC221" },
    { HDA_CODEC_ALC260, 0,          "Realtek ALC260" },
    { HDA_CODEC_ALC262, 0,          "Realtek ALC262" },
    { HDA_CODEC_ALC267, 0,          "Realtek ALC267" },
    { HDA_CODEC_ALC268, 0,          "Realtek ALC268" },
    { HDA_CODEC_ALC269, 0,          "Realtek ALC269" },
    { HDA_CODEC_ALC270, 0,          "Realtek ALC270" },
    { HDA_CODEC_ALC272, 0,          "Realtek ALC272" },
    { HDA_CODEC_ALC273, 0,          "Realtek ALC273" },
    { HDA_CODEC_ALC275, 0,          "Realtek ALC275" },
    { HDA_CODEC_ALC276, 0,          "Realtek ALC276" },
    { HDA_CODEC_ALC660, 0,          "Realtek ALC660-VD" },
    { HDA_CODEC_ALC662, 0x0002,     "Realtek ALC662 rev2" },
    { HDA_CODEC_ALC662, 0,          "Realtek ALC662" },
    { HDA_CODEC_ALC663, 0,          "Realtek ALC663" },
    { HDA_CODEC_ALC665, 0,          "Realtek ALC665" },
    { HDA_CODEC_ALC670, 0,          "Realtek ALC670" },
    { HDA_CODEC_ALC680, 0,          "Realtek ALC680" },
    { HDA_CODEC_ALC861, 0x0340,     "Realtek ALC660" },
    { HDA_CODEC_ALC861, 0,          "Realtek ALC861" },
    { HDA_CODEC_ALC861VD, 0,        "Realtek ALC861-VD" },
    { HDA_CODEC_ALC880, 0,          "Realtek ALC880" },
    { HDA_CODEC_ALC882, 0,          "Realtek ALC882" },
    { HDA_CODEC_ALC883, 0,          "Realtek ALC883" },
    { HDA_CODEC_ALC885, 0x0101,     "Realtek ALC889A" },
    { HDA_CODEC_ALC885, 0x0103,     "Realtek ALC889A" },
    { HDA_CODEC_ALC885, 0,          "Realtek ALC885" },
    { HDA_CODEC_ALC887, 0,          "Realtek ALC887" },
    { HDA_CODEC_ALC888, 0x0101,     "Realtek ALC1200" },
    { HDA_CODEC_ALC888, 0,          "Realtek ALC888" },
    { HDA_CODEC_ALC889, 0,          "Realtek ALC889" },
    { HDA_CODEC_ALC892, 0,          "Realtek ALC892" },
    { HDA_CODEC_ALC898, 0,          "Realtek ALC898" },
    { HDA_CODEC_ALC899, 0,          "Realtek ALC899" },
    { HDA_CODEC_AD1882, 0,          "Analog Devices AD1882" },
    { HDA_CODEC_AD1882A, 0,         "Analog Devices AD1882A" },
    { HDA_CODEC_AD1883, 0,          "Analog Devices AD1883" },
    { HDA_CODEC_AD1884, 0,          "Analog Devices AD1884" },
    { HDA_CODEC_AD1884A, 0,         "Analog Devices AD1884A" },
    { HDA_CODEC_AD1981HD, 0,        "Analog Devices AD1981HD" },
    { HDA_CODEC_AD1983, 0,          "Analog Devices AD1983" },
    { HDA_CODEC_AD1984, 0,          "Analog Devices AD1984" },
    { HDA_CODEC_AD1984A, 0,         "Analog Devices AD1984A" },
    { HDA_CODEC_AD1984B, 0,         "Analog Devices AD1984B" },
    { HDA_CODEC_AD1986A, 0,         "Analog Devices AD1986A" },
    { HDA_CODEC_AD1987, 0,          "Analog Devices AD1987" },
    { HDA_CODEC_AD1988, 0,          "Analog Devices AD1988A" },
    { HDA_CODEC_AD1988B, 0,         "Analog Devices AD1988B" },
    { HDA_CODEC_AD1989A, 0,         "Analog Devices AD1989A" },
    { HDA_CODEC_AD1989B, 0,         "Analog Devices AD1989B" },
    { HDA_CODEC_CA0110, 0,          "Creative CA0110-IBG" },
    { HDA_CODEC_CA0110_2, 0,        "Creative CA0110-IBG" },
    { HDA_CODEC_CA0132, 0,          "Creative CA0132" },
    { HDA_CODEC_SB0880, 0,          "Creative SB0880 X-Fi" },
    { HDA_CODEC_CMI9880, 0,         "CMedia CMI9880" },
    { HDA_CODEC_CMI98802, 0,        "CMedia CMI9880" },
    { HDA_CODEC_CXD9872RDK, 0,      "Sigmatel CXD9872RD/K" },
    { HDA_CODEC_CXD9872AKD, 0,      "Sigmatel CXD9872AKD" },
    { HDA_CODEC_STAC9200D, 0,       "Sigmatel STAC9200D" },
    { HDA_CODEC_STAC9204X, 0,       "Sigmatel STAC9204X" },
    { HDA_CODEC_STAC9204D, 0,       "Sigmatel STAC9204D" },
    { HDA_CODEC_STAC9205X, 0,       "Sigmatel STAC9205X" },
    { HDA_CODEC_STAC9205D, 0,       "Sigmatel STAC9205D" },
    { HDA_CODEC_STAC9220, 0,        "Sigmatel STAC9220" },
    { HDA_CODEC_STAC9220_A1, 0,     "Sigmatel STAC9220_A1" },
    { HDA_CODEC_STAC9220_A2, 0,     "Sigmatel STAC9220_A2" },
    { HDA_CODEC_STAC9221, 0,        "Sigmatel STAC9221" },
    { HDA_CODEC_STAC9221_A2, 0,     "Sigmatel STAC9221_A2" },
    { HDA_CODEC_STAC9221D, 0,       "Sigmatel STAC9221D" },
    { HDA_CODEC_STAC922XD, 0,       "Sigmatel STAC9220D/9223D" },
    { HDA_CODEC_STAC9227X, 0,       "Sigmatel STAC9227X" },
    { HDA_CODEC_STAC9227D, 0,       "Sigmatel STAC9227D" },
    { HDA_CODEC_STAC9228X, 0,       "Sigmatel STAC9228X" },
    { HDA_CODEC_STAC9228D, 0,       "Sigmatel STAC9228D" },
    { HDA_CODEC_STAC9229X, 0,       "Sigmatel STAC9229X" },
    { HDA_CODEC_STAC9229D, 0,       "Sigmatel STAC9229D" },
    { HDA_CODEC_STAC9230X, 0,       "Sigmatel STAC9230X" },
    { HDA_CODEC_STAC9230D, 0,       "Sigmatel STAC9230D" },
    { HDA_CODEC_STAC9250, 0,        "Sigmatel STAC9250" },
    { HDA_CODEC_STAC9251, 0,        "Sigmatel STAC9251" },
    { HDA_CODEC_STAC9255, 0,        "Sigmatel STAC9255" },
    { HDA_CODEC_STAC9255D, 0,       "Sigmatel STAC9255D" },
    { HDA_CODEC_STAC9254, 0,        "Sigmatel STAC9254" },
    { HDA_CODEC_STAC9254D, 0,       "Sigmatel STAC9254D" },
    { HDA_CODEC_STAC9271X, 0,       "Sigmatel STAC9271X" },
    { HDA_CODEC_STAC9271D, 0,       "Sigmatel STAC9271D" },
    { HDA_CODEC_STAC9272X, 0,       "Sigmatel STAC9272X" },
    { HDA_CODEC_STAC9272D, 0,       "Sigmatel STAC9272D" },
    { HDA_CODEC_STAC9273X, 0,       "Sigmatel STAC9273X" },
    { HDA_CODEC_STAC9273D, 0,       "Sigmatel STAC9273D" },
    { HDA_CODEC_STAC9274, 0,        "Sigmatel STAC9274" },
    { HDA_CODEC_STAC9274D, 0,       "Sigmatel STAC9274D" },
    { HDA_CODEC_STAC9274X5NH, 0,    "Sigmatel STAC9274X5NH" },
    { HDA_CODEC_STAC9274D5NH, 0,    "Sigmatel STAC9274D5NH" },
    { HDA_CODEC_STAC9872AK, 0,      "Sigmatel STAC9872AK" },
    { HDA_CODEC_IDT92HD005, 0,      "IDT 92HD005" },
    { HDA_CODEC_IDT92HD005D, 0,     "IDT 92HD005D" },
    { HDA_CODEC_IDT92HD206X, 0,     "IDT 92HD206X" },
    { HDA_CODEC_IDT92HD206D, 0,     "IDT 92HD206D" },
    { HDA_CODEC_IDT92HD66B1X5, 0,   "IDT 92HD66B1X5" },
    { HDA_CODEC_IDT92HD66B2X5, 0,   "IDT 92HD66B2X5" },
    { HDA_CODEC_IDT92HD66B3X5, 0,   "IDT 92HD66B3X5" },
    { HDA_CODEC_IDT92HD66C1X5, 0,   "IDT 92HD66C1X5" },
    { HDA_CODEC_IDT92HD66C2X5, 0,   "IDT 92HD66C2X5" },
    { HDA_CODEC_IDT92HD66C3X5, 0,   "IDT 92HD66C3X5" },
    { HDA_CODEC_IDT92HD66B1X3, 0,   "IDT 92HD66B1X3" },
    { HDA_CODEC_IDT92HD66B2X3, 0,   "IDT 92HD66B2X3" },
    { HDA_CODEC_IDT92HD66B3X3, 0,   "IDT 92HD66B3X3" },
    { HDA_CODEC_IDT92HD66C1X3, 0,   "IDT 92HD66C1X3" },
    { HDA_CODEC_IDT92HD66C2X3, 0,   "IDT 92HD66C2X3" },
    { HDA_CODEC_IDT92HD66C3_65, 0,  "IDT 92HD66C3_65" },
    { HDA_CODEC_IDT92HD700X, 0,     "IDT 92HD700X" },
    { HDA_CODEC_IDT92HD700D, 0,     "IDT 92HD700D" },
    { HDA_CODEC_IDT92HD71B5, 0,     "IDT 92HD71B5" },
    { HDA_CODEC_IDT92HD71B5_2, 0,   "IDT 92HD71B5" },
    { HDA_CODEC_IDT92HD71B6, 0,     "IDT 92HD71B6" },
    { HDA_CODEC_IDT92HD71B6_2, 0,   "IDT 92HD71B6" },
    { HDA_CODEC_IDT92HD71B7, 0,     "IDT 92HD71B7" },
    { HDA_CODEC_IDT92HD71B7_2, 0,   "IDT 92HD71B7" },
    { HDA_CODEC_IDT92HD71B8, 0,     "IDT 92HD71B8" },
    { HDA_CODEC_IDT92HD71B8_2, 0,   "IDT 92HD71B8" },
    { HDA_CODEC_IDT92HD73C1, 0,     "IDT 92HD73C1" },
    { HDA_CODEC_IDT92HD73D1, 0,     "IDT 92HD73D1" },
    { HDA_CODEC_IDT92HD73E1, 0,     "IDT 92HD73E1" },
    { HDA_CODEC_IDT92HD75B3, 0,     "IDT 92HD75B3" },
    { HDA_CODEC_IDT92HD75BX, 0,     "IDT 92HD75BX" },
    { HDA_CODEC_IDT92HD81B1C, 0,    "IDT 92HD81B1C" },
    { HDA_CODEC_IDT92HD81B1X, 0,    "IDT 92HD81B1X" },
    { HDA_CODEC_IDT92HD83C1C, 0,    "IDT 92HD83C1C" },
    { HDA_CODEC_IDT92HD83C1X, 0,    "IDT 92HD83C1X" },
    { HDA_CODEC_IDT92HD87B1_3, 0,   "IDT 92HD87B1/3" },
    { HDA_CODEC_IDT92HD87B2_4, 0,   "IDT 92HD87B2/4" },
    { HDA_CODEC_IDT92HD89C3, 0,     "IDT 92HD89C3" },
    { HDA_CODEC_IDT92HD89C2, 0,     "IDT 92HD89C2" },
    { HDA_CODEC_IDT92HD89C1, 0,     "IDT 92HD89C1" },
    { HDA_CODEC_IDT92HD89B3, 0,     "IDT 92HD89B3" },
    { HDA_CODEC_IDT92HD89B2, 0,     "IDT 92HD89B2" },
    { HDA_CODEC_IDT92HD89B1, 0,     "IDT 92HD89B1" },
    { HDA_CODEC_IDT92HD89E3, 0,     "IDT 92HD89E3" },
    { HDA_CODEC_IDT92HD89E2, 0,     "IDT 92HD89E2" },
    { HDA_CODEC_IDT92HD89E1, 0,     "IDT 92HD89E1" },
    { HDA_CODEC_IDT92HD89D3, 0,     "IDT 92HD89D3" },
    { HDA_CODEC_IDT92HD89D2, 0,     "IDT 92HD89D2" },
    { HDA_CODEC_IDT92HD89D1, 0,     "IDT 92HD89D1" },
    { HDA_CODEC_IDT92HD89F3, 0,     "IDT 92HD89F3" },
    { HDA_CODEC_IDT92HD89F2, 0,     "IDT 92HD89F2" },
    { HDA_CODEC_IDT92HD89F1, 0,     "IDT 92HD89F1" },
    { HDA_CODEC_IDT92HD90BXX, 0,    "IDT 92HD90BXX" },
    { HDA_CODEC_IDT92HD91BXX, 0,    "IDT 92HD91BXX" },
    { HDA_CODEC_IDT92HD93BXX, 0,    "IDT 92HD93BXX" },
    { HDA_CODEC_IDT92HD98BXX, 0,    "IDT 92HD98BXX" },
    { HDA_CODEC_IDT92HD99BXX, 0,    "IDT 92HD99BXX" },
    { HDA_CODEC_CX20549, 0,         "Conexant CX20549 (Venice)" },
    { HDA_CODEC_CX20551, 0,         "Conexant CX20551 (Waikiki)" },
    { HDA_CODEC_CX20561, 0,         "Conexant CX20561 (Hermosa)" },
    { HDA_CODEC_CX20582, 0,         "Conexant CX20582 (Pebble)" },
    { HDA_CODEC_CX20583, 0,         "Conexant CX20583 (Pebble HSF)" },
    { HDA_CODEC_CX20584, 0,         "Conexant CX20584" },
    { HDA_CODEC_CX20585, 0,         "Conexant CX20585" },
    { HDA_CODEC_CX20588, 0,         "Conexant CX20588" },
    { HDA_CODEC_CX20590, 0,         "Conexant CX20590" },
    { HDA_CODEC_CX20631, 0,         "Conexant CX20631" },
    { HDA_CODEC_CX20632, 0,         "Conexant CX20632" },
    { HDA_CODEC_CX20641, 0,         "Conexant CX20641" },
    { HDA_CODEC_CX20642, 0,         "Conexant CX20642" },
    { HDA_CODEC_CX20651, 0,         "Conexant CX20651" },
    { HDA_CODEC_CX20652, 0,         "Conexant CX20652" },
    { HDA_CODEC_CX20664, 0,         "Conexant CX20664" },
    { HDA_CODEC_CX20665, 0,         "Conexant CX20665" },
    { HDA_CODEC_VT1708_8, 0,        "VIA VT1708_8" },
    { HDA_CODEC_VT1708_9, 0,        "VIA VT1708_9" },
    { HDA_CODEC_VT1708_A, 0,        "VIA VT1708_A" },
    { HDA_CODEC_VT1708_B, 0,        "VIA VT1708_B" },
    { HDA_CODEC_VT1709_0, 0,        "VIA VT1709_0" },
    { HDA_CODEC_VT1709_1, 0,        "VIA VT1709_1" },
    { HDA_CODEC_VT1709_2, 0,        "VIA VT1709_2" },
    { HDA_CODEC_VT1709_3, 0,        "VIA VT1709_3" },
    { HDA_CODEC_VT1709_4, 0,        "VIA VT1709_4" },
    { HDA_CODEC_VT1709_5, 0,        "VIA VT1709_5" },
    { HDA_CODEC_VT1709_6, 0,        "VIA VT1709_6" },
    { HDA_CODEC_VT1709_7, 0,        "VIA VT1709_7" },
    { HDA_CODEC_VT1708B_0, 0,       "VIA VT1708B_0" },
    { HDA_CODEC_VT1708B_1, 0,       "VIA VT1708B_1" },
    { HDA_CODEC_VT1708B_2, 0,       "VIA VT1708B_2" },
    { HDA_CODEC_VT1708B_3, 0,       "VIA VT1708B_3" },
    { HDA_CODEC_VT1708B_4, 0,       "VIA VT1708B_4" },
    { HDA_CODEC_VT1708B_5, 0,       "VIA VT1708B_5" },
    { HDA_CODEC_VT1708B_6, 0,       "VIA VT1708B_6" },
    { HDA_CODEC_VT1708B_7, 0,       "VIA VT1708B_7" },
    { HDA_CODEC_VT1708S_0, 0,       "VIA VT1708S_0" },
    { HDA_CODEC_VT1708S_1, 0,       "VIA VT1708S_1" },
    { HDA_CODEC_VT1708S_2, 0,       "VIA VT1708S_2" },
    { HDA_CODEC_VT1708S_3, 0,       "VIA VT1708S_3" },
    { HDA_CODEC_VT1708S_4, 0,       "VIA VT1708S_4" },
    { HDA_CODEC_VT1708S_5, 0,       "VIA VT1708S_5" },
    { HDA_CODEC_VT1708S_6, 0,       "VIA VT1708S_6" },
    { HDA_CODEC_VT1708S_7, 0,       "VIA VT1708S_7" },
    { HDA_CODEC_VT1702_0, 0,        "VIA VT1702_0" },
    { HDA_CODEC_VT1702_1, 0,        "VIA VT1702_1" },
    { HDA_CODEC_VT1702_2, 0,        "VIA VT1702_2" },
    { HDA_CODEC_VT1702_3, 0,        "VIA VT1702_3" },
    { HDA_CODEC_VT1702_4, 0,        "VIA VT1702_4" },
    { HDA_CODEC_VT1702_5, 0,        "VIA VT1702_5" },
    { HDA_CODEC_VT1702_6, 0,        "VIA VT1702_6" },
    { HDA_CODEC_VT1702_7, 0,        "VIA VT1702_7" },
    { HDA_CODEC_VT1716S_0, 0,       "VIA VT1716S_0" },
    { HDA_CODEC_VT1716S_1, 0,       "VIA VT1716S_1" },
    { HDA_CODEC_VT1718S_0, 0,       "VIA VT1718S_0" },
    { HDA_CODEC_VT1718S_1, 0,       "VIA VT1718S_1" },
    { HDA_CODEC_VT1802_0, 0,        "VIA VT1802_0" },
    { HDA_CODEC_VT1802_1, 0,        "VIA VT1802_1" },
    { HDA_CODEC_VT1812, 0,          "VIA VT1812" },
    { HDA_CODEC_VT1818S, 0,         "VIA VT1818S" },
    { HDA_CODEC_VT1828S, 0,         "VIA VT1828S" },
    { HDA_CODEC_VT2002P_0, 0,       "VIA VT2002P_0" },
    { HDA_CODEC_VT2002P_1, 0,       "VIA VT2002P_1" },
    { HDA_CODEC_VT2020, 0,          "VIA VT2020" },
    { HDA_CODEC_ATIRS600_1, 0,      "ATI RS600" },
    { HDA_CODEC_ATIRS600_2, 0,      "ATI RS600" },
    { HDA_CODEC_ATIRS690, 0,        "ATI RS690/780" },
    { HDA_CODEC_ATIR6XX, 0,         "ATI R6xx" },
    { HDA_CODEC_NVIDIAMCP67, 0,     "NVIDIA MCP67" },
    { HDA_CODEC_NVIDIAMCP73, 0,     "NVIDIA MCP73" },
    { HDA_CODEC_NVIDIAMCP78, 0,     "NVIDIA MCP78" },
    { HDA_CODEC_NVIDIAMCP78_2, 0,   "NVIDIA MCP78" },
    { HDA_CODEC_NVIDIAMCP78_3, 0,   "NVIDIA MCP78" },
    { HDA_CODEC_NVIDIAMCP78_4, 0,   "NVIDIA MCP78" },
    { HDA_CODEC_NVIDIAMCP7A, 0,     "NVIDIA MCP7A" },
    { HDA_CODEC_NVIDIAGT220, 0,     "NVIDIA GT220" },
    { HDA_CODEC_NVIDIAGT21X, 0,     "NVIDIA GT21x" },
    { HDA_CODEC_NVIDIAMCP89, 0,     "NVIDIA MCP89" },
    { HDA_CODEC_NVIDIAGT240, 0,     "NVIDIA GT240" },
    { HDA_CODEC_NVIDIAGTS450, 0,    "NVIDIA GTS450" },
    { HDA_CODEC_NVIDIAGT440, 0,     "NVIDIA GT440" },
    { HDA_CODEC_NVIDIAGTX550, 0,    "NVIDIA GTX550" },
    { HDA_CODEC_NVIDIAGTX570, 0,    "NVIDIA GTX570" },
    { HDA_CODEC_INTELIP, 0,         "Intel Ibex Peak" },
    { HDA_CODEC_INTELBL, 0,         "Intel Bearlake" },
    { HDA_CODEC_INTELCA, 0,         "Intel Cantiga" },
    { HDA_CODEC_INTELEL, 0,         "Intel Eaglelake" },
    { HDA_CODEC_INTELIP2, 0,        "Intel Ibex Peak" },
    { HDA_CODEC_INTELCPT, 0,        "Intel Cougar Point" },
    { HDA_CODEC_INTELPPT, 0,        "Intel Panther Point" },
    { HDA_CODEC_INTELCL, 0,         "Intel Crestline" },
    { HDA_CODEC_SII1390, 0,         "Silicon Image SiI1390" },
    { HDA_CODEC_SII1392, 0,         "Silicon Image SiI1392" },
    // Unknown CODECs
    { HDA_CODEC_ADXXXX, 0,          "Analog Devices" },
    { HDA_CODEC_AGEREXXXX, 0,       "Lucent/Agere Systems" },
    { HDA_CODEC_ALCXXXX, 0,         "Realtek" },
    { HDA_CODEC_ATIXXXX, 0,         "ATI" },
    { HDA_CODEC_CAXXXX, 0,          "Creative" },
    { HDA_CODEC_CMIXXXX, 0,         "CMedia" },
    { HDA_CODEC_CMIXXXX2, 0,        "CMedia" },
    { HDA_CODEC_CSXXXX, 0,          "Cirrus Logic" },
    { HDA_CODEC_CXXXXX, 0,          "Conexant" },
    { HDA_CODEC_CHXXXX, 0,          "Chrontel" },
    { HDA_CODEC_IDTXXXX, 0,         "IDT" },
    { HDA_CODEC_INTELXXXX, 0,       "Intel" },
    { HDA_CODEC_MOTOXXXX, 0,        "Motorola" },
    { HDA_CODEC_NVIDIAXXXX, 0,      "NVIDIA" },
    { HDA_CODEC_SIIXXXX, 0,         "Silicon Image" },
    { HDA_CODEC_STACXXXX, 0,        "Sigmatel" },
    { HDA_CODEC_VTXXXX, 0,          "VIA" },
};
#define HDACC_CODECS_LEN        (sizeof(know_codecs) / sizeof(know_codecs[0]))
*/

/*****************
 * Device Methods
 *****************/

/* get HDA device name */
static char *get_hda_controller_name(uint16_t controller_device_id, uint16_t controller_vendor_id)
{
	int i;
	static char desc[128];

	for (i = 0; i < HDAC_DEVICES_LEN; i++)
	{
		if (know_hda_controller[i].model == ((controller_device_id << 16) | controller_vendor_id))
		{
			if(controller_vendor_id == INTEL_VENDORID){
				sprintf(desc, "Intel %s Hight Definition Audio Controller", know_hda_controller[i].desc);
				desc[sizeof(desc) - 1] = '\0';
			} else if (controller_vendor_id == NVIDIA_VENDORID) {
				sprintf(desc, "nVidia %s HDA Controller (HDMi)", know_hda_controller[i].desc);
				desc[sizeof(desc) - 1] = '\0';
			} else if (controller_vendor_id == ATI_VENDORID) {
				sprintf(desc, "ATI %s HDA Controller (HDMi)", know_hda_controller[i].desc);
				desc[sizeof(desc) - 1] = '\0';
			} else if (controller_vendor_id == RDC_VENDORID) {
				sprintf(desc, "RDC %s Hight Definition Audio Controller", know_hda_controller[i].desc);
				desc[sizeof(desc) - 1] = '\0';
			} else if (controller_vendor_id == VIA_VENDORID) {
				sprintf(desc, "VIA %s HDA Controller", know_hda_controller[i].desc);
				desc[sizeof(desc) - 1] = '\0';
			} else if (controller_vendor_id == SIS_VENDORID) {
				sprintf(desc, "SiS %s HDA Controller", know_hda_controller[i].desc);
				desc[sizeof(desc) - 1] = '\0';
			} else if (controller_vendor_id == ULI_VENDORID) {
				sprintf(desc, "ULI %s HDA Controller", know_hda_controller[i].desc);
				desc[sizeof(desc) - 1] = '\0';
			}
			return desc;
		}
	}
	sprintf(desc, "Unknown HD Audio device");
	desc[sizeof(desc) - 1] = '\0';
	return desc;
}

static int devprop_add_hda_template(struct DevPropDevice *device)
{
	if (!device)
	{
		return 0;
	}
	devices_number++;

	return 1;
}

bool setup_hda_devprop(pci_dt_t *hda_dev)
{
	struct		DevPropDevice	*device = NULL;
	char		*devicepath = NULL;
	char		*controller_name = NULL;
	int         len;
	uint8_t		BuiltIn = 0x00;
	uint16_t	controller_vendor_id = hda_dev->vendor_id;
	uint16_t	controller_device_id = hda_dev->device_id;
	const char	*value;

	devicepath = get_pci_dev_path(hda_dev);
	controller_name = get_hda_controller_name(controller_device_id, controller_vendor_id);

	if (!string)
	{
		string = devprop_create_string();
		if (!string)
		{
			return 0;
		}
	}

	if (!devicepath)
	{
		return 0;
	}

	device = devprop_add_device(string, devicepath);
	if (!device)
	{
		return 0;
	}
	devprop_add_hda_template(device);

	verbose("\n--------------------------------\n");
	verbose("- AUDIO DEVICE INFO -\n");
	verbose("--------------------------------\n");

	switch ((controller_device_id << 16) | controller_vendor_id) {

	/***********************************************************************
	* The above case are intended as for HDEF device at address 0x001B0000
	***********************************************************************/
		case HDA_INTEL_HASWELL:
		case HDA_INTEL_CRYSTAL:
		case HDA_INTEL_CPT:
		case HDA_INTEL_PATSBURG:
        case HDA_INTEL_PPT1:
		case HDA_INTEL_82801F:
		case HDA_INTEL_63XXESB:
		case HDA_INTEL_82801G:
		case HDA_INTEL_82801H:
        case HDA_INTEL_82801I:
		case HDA_INTEL_82801JI:
		case HDA_INTEL_82801JD:
		case HDA_INTEL_PCH:
		case HDA_INTEL_PCH2:
        case HDA_INTEL_SCH:
		case HDA_INTEL_LPT1:
		case HDA_INTEL_LPT2:
		case HDA_INTEL_LYNX:
		case HDA_INTEL_LYNX2:
	/* if the key value kHDEFLayoutID as a value set that value, if not will assign a default layout */
	if (getValueForKey(kHDEFLayoutID, &value, &len, &bootInfo->chameleonConfig) && len == HDEF_LEN * 2)
	{
		uint8_t new_HDEF_layout_id[HDEF_LEN];
		if (hex2bin(value, new_HDEF_layout_id, HDEF_LEN) == 0)	{
			memcpy(default_HDEF_layout_id, new_HDEF_layout_id, HDEF_LEN);
			verbose("Using user supplied HDEF layout-id: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", 
				default_HDEF_layout_id[0], default_HDEF_layout_id[1], default_HDEF_layout_id[2], default_HDEF_layout_id[3]);
		}
	}
	else
	{
		verbose("Using default HDEF layout-id: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                    default_HDEF_layout_id[0], default_HDEF_layout_id[1], default_HDEF_layout_id[2], default_HDEF_layout_id[3]);
	}
	devprop_add_value(device, "layout-id", default_HDEF_layout_id, HDEF_LEN);
	devprop_add_value(device, "built-in", &BuiltIn, 1);
	devprop_add_value(device, "hda-gfx", (uint8_t *)"onboard-1", 10);
	break;

	/****************************************************************************************************************
	* The above case are intended as for HDAU (NVIDIA) device onboard audio for GFX card with Audio controller HDMi
	****************************************************************************************************************/
	case HDA_NVIDIA_GK107:
	case HDA_NVIDIA_GF114:
	case HDA_NVIDIA_GK106:
	case HDA_NVIDIA_GK104:
	case HDA_NVIDIA_GF110:
	case HDA_NVIDIA_GF119:
        case HDA_NVIDIA_GT116:
	case HDA_NVIDIA_GT104:
	case HDA_NVIDIA_GT108:
	case HDA_NVIDIA_GT106:
	case HDA_NVIDIA_GT100:
        case HDA_NVIDIA_0BE4:
	case HDA_NVIDIA_0BE3:
	case HDA_NVIDIA_0BE2:

	/* if the key value kHDAULayoutID as a value set that value, if not will assign a default layout */
	if (getValueForKey(kHDAULayoutID, &value, &len, &bootInfo->chameleonConfig) && len == HDAU_LEN * 2)
	{
		uint8_t new_HDAU_layout_id[HDAU_LEN];
		if (hex2bin(value, new_HDAU_layout_id, HDAU_LEN) == 0)
		{
			memcpy(default_HDAU_layout_id, new_HDAU_layout_id, HDAU_LEN);
			verbose("Using user supplied HDAU layout-id: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                            default_HDAU_layout_id[0], default_HDAU_layout_id[1], default_HDAU_layout_id[2], default_HDAU_layout_id[3]);
		}
	}
	else
	{
		verbose("Using default HDAU layout-id: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                           default_HDAU_layout_id[0], default_HDAU_layout_id[1], default_HDAU_layout_id[2], default_HDAU_layout_id[3]);
	}

            devprop_add_value(device, "layout-id", default_HDAU_layout_id, HDAU_LEN); /*FIX ME*/
            devprop_add_value(device, "@0,connector-type", connector_type_value, 4);
            devprop_add_value(device, "@1,connector-type", connector_type_value, 4);
            devprop_add_value(device, "hda-gfx", (uint8_t *)"onboard-2", 10);
            devprop_add_value(device, "built-in", &BuiltIn, 1);
            break;

	/*************************************************************************************************************
	* The above case are intended as for HDAU (ATi) device onboard audio for GFX card with Audio controller HDMi
	*************************************************************************************************************/
	case HDA_ATI_SB450:
	case HDA_ATI_SB600:
	case HDA_ATI_RS600:
	case HDA_ATI_RS690:
	case HDA_ATI_RS780:
	case HDA_ATI_R600:
	case HDA_ATI_RV630:
	case HDA_ATI_RV610:
	case HDA_ATI_RV670:
	case HDA_ATI_RV635:
	case HDA_ATI_RV620:
	case HDA_ATI_RV770:
	case HDA_ATI_RV730:
	case HDA_ATI_RV710:
	case HDA_ATI_RV740:
	case HDA_ATI_RV870:
	case HDA_ATI_RV840:
	case HDA_ATI_RV830:
	case HDA_ATI_RV810:
	case HDA_ATI_RV970:
	case HDA_ATI_RV940:
	case HDA_ATI_RV930:
	case HDA_ATI_RV910:
	case HDA_ATI_R1000:
	case HDA_ATI_VERDE:

            /* if the key value kHDAULayoutID as a value set that value, if not will assign a default layout */
            if (getValueForKey(kHDAULayoutID, &value, &len, &bootInfo->chameleonConfig) && len == HDAU_LEN * 2)
            {
                uint8_t new_HDAU_layout_id[HDAU_LEN];
                if (hex2bin(value, new_HDAU_layout_id, HDAU_LEN) == 0)
                {
                    memcpy(default_HDAU_layout_id, new_HDAU_layout_id, HDAU_LEN);
                    verbose("Using user supplied HDAU layout-id: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                            default_HDAU_layout_id[0], default_HDAU_layout_id[1], default_HDAU_layout_id[2], default_HDAU_layout_id[3]);
                }
			}
			else
			{
				verbose("Using default HDAU layout-id: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                            default_HDAU_layout_id[0], default_HDAU_layout_id[1], default_HDAU_layout_id[2], default_HDAU_layout_id[3]);
            }

            devprop_add_value(device, "layout-id", default_HDAU_layout_id, HDAU_LEN); /*FIX ME*/
            devprop_add_value(device, "hda-gfx", (uint8_t *)"onboard-2", 10);
            devprop_add_value(device, "built-in", &BuiltIn, 1);
            break;

        default:
            break;
	}

	verbose("Class code: [%04x]\nModel name: %s [%04x:%04x] (rev %02x)\nSubsystem: [%04x:%04x]\n%s\ndevice number: %d\n",
	hda_dev->class_id, controller_name, hda_dev->vendor_id, hda_dev->device_id, hda_dev->revision_id,
	hda_dev->subsys_id.subsys.vendor_id, hda_dev->subsys_id.subsys.device_id, devicepath, devices_number);

	verbose("--------------------------------\n");

	stringdata = malloc(sizeof(uint8_t) * string->length);
	memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
	stringlength = string->length;

	return true;
}
