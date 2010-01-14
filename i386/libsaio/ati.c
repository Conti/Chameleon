/*
 *  ATI injector
 *
 *  Copyright (C) 2009  Jasmin Fazlic, iNDi, netkas
 *
 *  ATI injector is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ATI driver and injector is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ATI injector.  If not, see <http://www.gnu.org/licenses/>.
 */ 
/*
 * Alternatively you can choose to comply with APSL
 */
 
 
#include "libsaio.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "device_inject.h"
#include "ati.h"

#ifndef DEBUG_ATI
#define DEBUG_ATI 0
#endif

#if DEBUG_ATI
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

#define MAX_NUM_DCB_ENTRIES 16

#define TYPE_GROUPED 0xff

extern uint32_t devices_number;

const char *ati_compatible_0[]	=	{ "@0,compatible",	"ATY,%s"		};
const char *ati_compatible_1[]	=	{ "@1,compatible",	"ATY,%s"		};
const char *ati_device_type_0[]	=	{ "@0,device_type",	"display"			};
const char *ati_device_type_1[]	=	{ "@1,device_type",	"display"			};
const char *ati_device_type[]	=	{ "device_type",	"ATY,%sParent"		};
const char *ati_name_0[]		=	{ "@0,name",		"ATY,%s"	};
const char *ati_name_1[]		=	{ "@1,name",		"ATY,%s"	};
const char *ati_name[]			=	{ "name",	"ATY,%sParent"		};
const char *ati_efidisplay_0[]	=	{ "@0,ATY,EFIDisplay",		"TMDSB"	};
struct ati_data_key ati_connector_type_0	=	{ 0x04, "@0,connector-type", 		{0x00, 0x04, 0x00, 0x00} };
struct ati_data_key ati_connector_type_1	=	{ 0x04, "@1,connector-type",		{0x04, 0x00, 0x00, 0x00}};
struct ati_data_key ati_display_con_fl_type_0	=	{ 0x04, "@0,display-connect-flags",		{0x00, 0x00, 0x04, 0x00}};
const char *ati_display_type_0[]	=	{ "@0,display-type",	"LCD"			};
const char *ati_display_type_1[]	=	{ "@1,display-type",	"NONE"			};
struct ati_data_key ati_aux_power_conn		=	{ 0x04, "AAPL,aux-power-connected",		{0x01, 0x00, 0x00, 0x00}};
struct ati_data_key ati_backlight_ctrl		=	{ 0x04, "AAPL,backlight-control",		{0x00, 0x00, 0x00, 0x00}};
//const char *ati_slot_name[]			=	{ "AAPL,slot-name",		"Slot-1"};
struct ati_data_key ati_aapl01_coher		=	{ 0x04, "AAPL01,Coherency",		{0x01, 0x00, 0x00, 0x00}};
const char *ati_card_no[]			=	{ "ATY,Card#",			"109-B77101-00"};
const char *ati_copyright[]			=	{ "ATY,Copyright",		"Copyright AMD Inc. All Rights Reserved. 2005-2009"};
const char *ati_efi_compile_d[]		=	{ "ATY,EFICompileDate",		"Jan 26 2009"};
struct ati_data_key ati_efi_disp_conf		=	{ 0x08, "ATY,EFIDispConfig",		{0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}};
struct ati_data_key ati_efi_drv_type		=	{ 0x01, "ATY,EFIDriverType",		{0x02}};
struct ati_data_key ati_efi_enbl_mode		=	{ 0x01, "ATY,EFIEnabledMode",		{0x01}};
struct ati_data_key ati_efi_init_stat		=	{ 0x04, "ATY,EFIHWInitStatus",		{0x00, 0x00, 0x00, 0x00}};
struct ati_data_key ati_efi_orientation		=	{ 0x02, "ATY,EFIOrientation",		{0x02, 0x00}};
const char *ati_efi_version[]		=	{ "ATY,EFIVersion",		"01.00.318"};
const char *ati_efi_versionB[]		=	{ "ATY,EFIVersionB",		"113-SBSJ1G04-00R-02"};
const char *ati_efi_versionE[]		=	{ "ATY,EFIVersionE",		"113-B7710A-318"};
struct ati_data_key ati_mclk		=	{ 0x04, "ATY,MCLK",		{0x70, 0x2e, 0x11, 0x00}};
struct ati_data_key ati_mem_rev_id		=	{ 0x02, "ATY,MemRevisionID",		{0x03, 0x00}};
struct ati_data_key ati_mem_vend_id		=	{ 0x02, "ATY,MemVendorID",		{0x02, 0x00}};
const char *ati_mrt[]		=	{ "ATY,MRT",		" "};
const char *ati_romno[]		=	{ "ATY,Rom#",		"113-B7710C-176"};
struct ati_data_key ati_sclk		=	{ 0x04, "ATY,SCLK",		{0x28, 0xdb, 0x0b, 0x00}};
struct ati_data_key ati_vendor_id	=	{ 0x02, "ATY,VendorID",		{0x02, 0x10}};
struct ati_data_key ati_platform_info = { 0x80, "ATY,PlatformInfo", {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
struct ati_data_key ati_mvad	=	{ 0x40, "MVAD",		{0x3f, 0x5c, 0x82, 0x02, 0xff, 0x90, 0x00, 0x54, 0x60, 0x00, 0xac, 0x10, 0xa0, 0x17, 0x00, 0x03, 0xb0, 0x68, 0x00, 0x0a, 0xa0, 0x0a, 0x30, 0x00, 0x20, 0x00, 0x40, 0x06, 0x6e, 0x06, 0x03, 0x00, 0x06, 0x00, 0x40, 0x06, 0x00, 0x0a, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x10, 0x06, 0x92, 0x20, 0x00, 0x03}};
struct ati_data_key ati_saved_config	=	{ 0x100, "saved-config",		{0x3f, 0x5c, 0x82, 0x02, 0xff, 0x90, 0x00, 0x54, 0x60, 0x00, 0xac, 0x10, 0xa0, 0x17, 0x00, 0x03, 0xb0, 0x68, 0x00, 0x0a, 0xa0, 0x0a, 0x30, 0x00, 0x20, 0x00, 0x40, 0x06, 0x6e, 0x06, 0x03, 0x00, 0x06, 0x00, 0x40, 0x06, 0x00, 0x0a, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x10, 0x06, 0x92, 0x20, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xee, 0x02, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x31, 0x30, 0x50, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x32, 0x32, 0x32, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
///non 48xx keys
const char *ati_efidisplay_0_n4[]	=	{ "@0,ATY,EFIDisplay",		"TMDSA"	};
struct ati_data_key ati_connector_type_0_n4	=	{ 0x04, "@0,connector-type", 		{0x04, 0x00, 0x00, 0x00} };
struct ati_data_key ati_connector_type_1_n4	=	{ 0x04, "@1,connector-type",		{0x00, 0x02, 0x00, 0x00}};
struct ati_data_key ati_aapl_emc_disp_list_n4	=	{ 0x40, "AAPL,EMC-Display-List",		{0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x10, 0x00, 0x00, 0x1b, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x10, 0x00, 0x00, 0x1c, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x10, 0x00, 0x00, 0x21, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
struct ati_data_key ati_fb_offset_n4		=	{ 0x08, "ATY,FrameBufferOffset",		{0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00}};
struct ati_data_key ati_hwgpio_n4			=	{ 0x04, "ATY,HWGPIO",		{0x23, 0xa8, 0x48, 0x00}};
struct ati_data_key ati_iospace_offset_n4	=	{ 0x08, "ATY,IOSpaceOffset",		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00}};
struct ati_data_key ati_mclk_n4				=	{ 0x04, "ATY,MCLK",		{0x00, 0x35, 0x0c, 0x00}};
struct ati_data_key ati_sclk_n4				=	{ 0x04, "ATY,SCLK",		{0x60, 0xae, 0x0a, 0x00}};
struct ati_data_key ati_refclk_n4			=	{ 0x04, "ATY,RefCLK",	{0x8c, 0x0a, 0x00, 0x00}};
struct ati_data_key ati_regspace_offset_n4	=	{ 0x08, "ATY,RegisterSpaceOffset",		{0x00, 0x00, 0x00, 0x00, 0x90, 0xa2, 0x00, 0x00}};
struct ati_data_key ati_vram_memsize_0	=	{ 0x08, "@0,VRAM,memsize",		{0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00}};
struct ati_data_key ati_vram_memsize_1	=	{ 0x08, "@1,VRAM,memsize",		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
struct ati_data_key ati_aapl_blackscr_prefs_0_n4	=	{ 0x04, "AAPL00,blackscreen-preferences",		{0x00, 0x00, 0x00, 0x00}};
struct ati_data_key ati_aapl_blackscr_prefs_1_n4	=	{ 0x04, "AAPL01,blackscreen-preferences",		{0x00, 0x00, 0x00, 0x00}};
struct ati_data_key ati_swgpio_info_n4			=	{ 0x04, "ATY,SWGPIO Info",		{0x00, 0x48, 0xa8, 0x23}};
struct ati_data_key ati_efi_orientation_n4		=	{ 0x01, "ATY,EFIOrientation",		{0x08}};
struct ati_data_key ati_mvad_n4					=	{ 0x100, "MVAD",		{0x3e, 0x5c, 0x82, 0x00, 0xff, 0x90, 0x00, 0xf6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x3c, 0x80, 0x07, 0x20, 0x08, 0x30, 0x00, 0x20, 0x00, 0xb0, 0x04, 0xd3, 0x04, 0x03, 0x00, 0x06, 0x00, 0xb0, 0x04, 0x80, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x90, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x31, 0x30, 0x50, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x32, 0x32, 0x32, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
struct ati_data_key ati_saved_config_n4			=	{ 0x100, "saved-config",		{0x3e, 0x5c, 0x82, 0x00, 0xff, 0x90, 0x00, 0xf6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x3c, 0x80, 0x07, 0x20, 0x08, 0x30, 0x00, 0x20, 0x00, 0xb0, 0x04, 0xd3, 0x04, 0x03, 0x00, 0x06, 0x00, 0xb0, 0x04, 0x80, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x90, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x31, 0x30, 0x50, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x32, 0x32, 0x32, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
//const char *ati_slot_name_n4[]			=	{ "AAPL,slot-name",		"Slot-2"};




struct pcir_s {
	uint32_t signature;
	uint16_t vid;
	uint16_t devid;
};

// Known cards as of 2008/08/26
static struct ati_chipsets_t ATIKnownChipsets[] = {
	{ 0x00000000, "Unknown" },
		{ 0x10029589,  "ATI Radeon 2600 Series"}  ,
		{ 0x10029588,  "ATI Radeon 2600 Series"}  ,
		{ 0x100294C3,  "ATI Radeon 2400 Series"}  ,
		{ 0x100294C4,  "ATI Radeon 2400 Series"}  ,
		{ 0x100294C6,  "ATI Radeon 2400 Series"}  ,
		{ 0x10029400,  "ATI Radeon 2900 Series"}  ,
		{ 0x10029405,  "ATI Radeon 2900GT Series"}  ,
		{ 0x10029581,  "ATI Radeon 2600 Series"}  ,
		{ 0x10029583,  "ATI Radeon 2600 Series"}  ,
		{ 0x10029586,  "ATI Radeon 2600 Series"}  ,
		{ 0x10029587,  "ATI Radeon 2600 Series"}  ,
		{ 0x100294C9,  "ATI Radeon 2400 Series"}  ,
		{ 0x10029501,  "ATI Radeon 3800 Series"}  ,
		{ 0x10029505,  "ATI Radeon 3800 Series"}  ,
		{ 0x10029515,  "ATI Radeon 3800 Series"}  ,
		{ 0x10029507,  "ATI Radeon 3800 Series"}  ,
		{ 0x10029500,  "ATI Radeon 3800 Series"}  ,
		{ 0x1002950F,  "ATI Radeon 3800X2 Series"}  ,
		{ 0x100295C5,  "ATI Radeon 3400 Series"}  ,
		{ 0x100295C7,  "ATI Radeon 3400 Series"}  ,
		{ 0x100295C0,  "ATI Radeon 3400 Series"}  ,
		{ 0x10029596,  "ATI Radeon 3600 Series"}  ,
		{ 0x10029590,  "ATI Radeon 3600 Series"}  ,
		{ 0x10029599,  "ATI Radeon 3600 Series"}  ,
		{ 0x10029597,  "ATI Radeon 3600 Series"}  ,
		{ 0x10029598,  "ATI Radeon 3600 Series"}  ,
		{ 0x10029442,  "ATI Radeon 4850 Series"}  ,
		{ 0x10029440,  "ATI Radeon 4870 Series"}  ,
		{ 0x1002944C,  "ATI Radeon 4830 Series"}  ,
		{ 0x10029460,  "ATI Radeon 4890 Series"}  ,
		{ 0x10029462,  "ATI Radeon 4890 Series"}  ,
		{ 0x10029441,  "ATI Radeon 4870X2 Series"}  ,
		{ 0x10029443,  "ATI Radeon 4850X2 Series"}  ,
		{ 0x10029444,  "ATI Radeon 4800 Series"}  ,
		{ 0x10029446,  "ATI Radeon 4800 Series"}  ,
		{ 0x1002944E,  "ATI Radeon 4730 Series"}  ,
		{ 0x10029450,  "ATI Radeon 4800 Series"}  ,
		{ 0x10029452,  "ATI Radeon 4800 Series"}  ,		
		{ 0x10029456,  "ATI Radeon 4800 Series"}  ,		
		{ 0x1002944A,  "ATI Radeon 4800 Mobility Series"}  ,
		{ 0x1002945A,  "ATI Radeon 4800 Mobility Series"}  ,
		{ 0x1002945B,  "ATI Radeon 4800 Mobility Series"}  ,
		{ 0x1002944B,  "ATI Radeon 4800 Mobility Series"}  ,
		{ 0x10029490,  "ATI Radeon 4600 Series"}  ,
		{ 0x10029498,  "ATI Radeon 4600 Series"}  ,
		{ 0x1002949E,  "ATI Radeon 4600 Series"}  ,
		{ 0x10029480,  "ATI Radeon 4600 Series"}  ,
		{ 0x10029488,  "ATI Radeon 4600 Series"}  ,
		{ 0x10029540,  "ATI Radeon 4500 Series"}  ,
		{ 0x10029541,  "ATI Radeon 4500 Series"}  ,
		{ 0x1002954E,  "ATI Radeon 4500 Series"}  ,
		{ 0x10029552,  "ATI Radeon 4300 Mobility Series"}  ,
		{ 0x10029553,  "ATI Radeon 4500 Mobility Series"}  ,
		{ 0x1002954F,  "ATI Radeon 4300 Series"},
};

static struct ati_chipsets_t ATIKnownFramebuffers[] = {
		{ 0x00000000, "Megalodon" },
		{ 0x10029589,  "Lamna"}  ,
		{ 0x10029588,  "Lamna"}  ,
		{ 0x100294C3,  "Iago"}  ,
		{ 0x100294C4,  "Iago"}  ,
		{ 0x100294C6,  "Iago"}  ,
		{ 0x10029400,  "Franklin"}  ,
		{ 0x10029405,  "Franklin"}  ,
		{ 0x10029581,  "Hypoprion"}  ,
		{ 0x10029583,  "Hypoprion"}  ,
		{ 0x10029586,  "Hypoprion"}  ,
		{ 0x10029587,  "Hypoprion"}  ,
		{ 0x100294C9,  "Iago"}  ,
		{ 0x10029501,  "Megalodon"}  ,
		{ 0x10029505,  "Megalodon"}  ,
		{ 0x10029515,  "Megalodon"}  ,
		{ 0x10029507,  "Megalodon"}  ,
		{ 0x10029500,  "Megalodon"}  ,
		{ 0x1002950F,  "Triakis"}  ,
		{ 0x100295C5,  "Iago"}  ,
		{ 0x100295C7,  "Iago"}  ,
		{ 0x100295C0,  "Iago"}  ,
		{ 0x10029596,  "Megalodon"}  ,
		{ 0x10029590,  "Megalodon"}  ,
		{ 0x10029599,  "Megalodon"}  ,
		{ 0x10029597,  "Megalodon"}  ,
		{ 0x10029598,  "Megalodon"}  ,
		{ 0x10029442,  "Motmot"}  ,
		{ 0x10029440,  "Motmot"}  ,
		{ 0x1002944C,  "Motmot"}  ,
		{ 0x10029460,  "Motmot"}  ,
		{ 0x10029462,  "Motmot"}  ,
		{ 0x10029441,  "Motmot"}  ,
		{ 0x10029443,  "Motmot"}  ,
		{ 0x10029444,  "Motmot"}  ,
		{ 0x10029446,  "Motmot"}  ,
		{ 0x1002944E,  "Motmot"}  ,
		{ 0x10029450,  "Motmot"}  ,
		{ 0x10029452,  "Motmot"}  ,		
		{ 0x10029456,  "Motmot"}  ,		
		{ 0x1002944A,  "Motmot"}  ,
		{ 0x1002945A,  "Motmot"}  ,
		{ 0x1002945B,  "Motmot"}  ,
		{ 0x1002944B,  "Motmot"}  ,
		{ 0x10029490,  "Motmot"}  ,
		{ 0x10029498,  "Motmot"}  ,
		{ 0x1002949E,  "Motmot"}  ,
		{ 0x10029480,  "Motmot"}  ,
		{ 0x10029488,  "Motmot"}  ,
		{ 0x10029540,  "Motmot"}  ,
		{ 0x10029541,  "Motmot"}  ,
		{ 0x1002954E,  "Motmot"}  ,
		{ 0x10029552,  "Motmot"}  ,
		{ 0x10029553,  "Motmot"}  ,
		{ 0x1002954F,  "Motmot"}  ,
};

unsigned int accessROM(pci_dt_t *ati_dev, unsigned int mode)
{
	uint32_t	bar[7];
	volatile uint8_t *regs;

	bar[2] = pci_config_read32(ati_dev->dev.addr, 0x18 );
	regs = (uint8_t *) (bar[2] & ~0x0f);

if(mode)
{
	if(mode != 1) return 0xe00002c7;
	REG32W(0x179c, 0x00080000);
	REG32W(0x1798, 0x00080721);
	REG32W(0x17a0, 0x00080621);
	REG32W(0x1600, 0x14030300);
	REG32W(0x1798, 0x21);
	REG32W(0x17a0, 0x21);
	REG32W(0x179c, 0x00);
	REG32W(0x17a0, 0x21);
	REG32W(0x1798, 0x21);
	REG32W(0x1798, 0x21);
} else {
	REG32W(0x1600, 0x14030302);	
	REG32W(0x1798, 0x21);
	REG32W(0x17a0, 0x21);
	REG32W(0x179c, 0x00080000);
	REG32W(0x17a0, 0x00080621);
	REG32W(0x1798, 0x00080721);
	REG32W(0x1798, 0x21);
	REG32W(0x17a0, 0x21);
	REG32W(0x179c, 0x00);
	REG32W(0x1604, 0x0400e9fc);
	REG32W(0x161c, 0x00);
	REG32W(0x1620, 0x9f);
	REG32W(0x1618, 0x00040004);
	REG32W(0x161c, 0x00);
	REG32W(0x1604, 0xe9fc);
	REG32W(0x179c, 0x00080000);
	REG32W(0x1798, 0x00080721);
	REG32W(0x17a0, 0x00080621);
	REG32W(0x1798, 0x21);
	REG32W(0x17a0, 0x21);
	REG32W(0x179c, 0x00);
}
return 0;
}

unsigned char * readAtomBIOS(pci_dt_t *ati_dev)
{
	uint32_t	bar[7];
	unsigned int * BIOSBase = NULL;
	unsigned int counter;
	volatile uint8_t *regs;

	bar[2] = pci_config_read32(ati_dev->dev.addr, 0x18 );
	regs = (uint8_t *) (bar[2] & ~0x0f);
	accessROM(ati_dev, 0);
	REG32W(0xa8, 0);
	REG32R(0xac);
	REG32W(0xa8, 0);
	REG32R(0xac);	
	
	BIOSBase = malloc(0x10000);
	if(BIOSBase)
	{
		REG32W(0xa8, 0);
		BIOSBase[0] = REG32R(0xac);
		counter = 4;
		do
		{
			REG32W(0xa8, counter);
			BIOSBase[counter/4] = REG32R(0xac);
			counter +=4;
		}
		while(counter != 0x10000);
	}
	accessROM((pci_dt_t *)regs, 1);

	if ((* (UInt16 *)BIOSBase) != 0xAA55) {
		printf("Wrong BIOS signature: %04x\n", (* (UInt16 *)BIOSBase));
		return 0;
	}
	return (unsigned char *)BIOSBase;
}

#define R5XX_CONFIG_MEMSIZE            0x00F8
#define R6XX_CONFIG_MEMSIZE            0x5428

uint32_t getvramsizekb(pci_dt_t *ati_dev)
{
	uint32_t	bar[7];
	volatile uint8_t *regs;
	uint32_t	RamSize = 0;

	bar[2] = pci_config_read32(ati_dev->dev.addr, 0x18 );
	regs = (uint8_t *) (bar[2] & ~0x0f);
	if (ati_dev->device_id < 0x9400)
	RamSize = (REG32R(R5XX_CONFIG_MEMSIZE)) >> 10;
	else
	RamSize = (REG32R(R6XX_CONFIG_MEMSIZE)) >> 10;
	
	return RamSize;
}

#define AVIVO_D1CRTC_CONTROL 0x6080
#define AVIVO_CRTC_EN (1<<0)
#define AVIVO_D2CRTC_CONTROL 0x6880

bool
radeon_card_posted(pci_dt_t *ati_dev)
{
	// if devid matches biosimage(from legacy) devid - posted card, fails with X2/crossfire cards.
/*	char *biosimage = 0xC0000;
	
	if ((uint8_t)biosimage[0] == 0x55 && (uint8_t)biosimage[1] == 0xaa)
	{
		struct  pci_rom_pci_header_t *rom_pci_header;   
		rom_pci_header = (struct pci_rom_pci_header_t*)(biosimage + (uint8_t)biosimage[24] + (uint8_t)biosimage[25]*256);
	
    	if (rom_pci_header->signature == 0x52494350)
		{
     		if (rom_pci_header->device == ati_dev->device_id)
			{
				return TRUE;
				printf("Card was POSTed\n");
			}
		}
	}
	return FALSE;
	printf("Card was not POSTed\n");
	*/
 //fails yet
	uint32_t	bar[7];
	volatile uint8_t *regs;

	bar[2] = pci_config_read32(ati_dev->dev.addr, 0x18 );
	regs = (uint8_t *) (bar[2] & ~0x0f);
	
    uint32_t val;
	
		val = REG32R(AVIVO_D1CRTC_CONTROL) | REG32R(AVIVO_D2CRTC_CONTROL);
		if (val & AVIVO_CRTC_EN)
			return TRUE;

    return FALSE;

}

uint32_t load_ati_bios_file(char *filename, char *buffer)
{
	int		fd, size;
	char	dirspec[128];

	// Check Extra on booting partition
	sprintf(dirspec, "/Extra/%s", filename);
	fd = open(dirspec, 0);
	if (fd < 0)
	{	
		// Fall back to booter partition
		sprintf(dirspec, "bt(0,0)/Extra/%s", filename);
		fd=open (dirspec, 0);
		if (fd < 0)
			return 0;
	}

	size = read(fd, buffer, file_size (fd));
	close (fd);
	return  size;
}

char *get_ati_model(uint32_t id) {
	int i=0;
	for(i = 0; i <  (sizeof(ATIKnownChipsets) / sizeof(ATIKnownChipsets[0])); i++) {
		if(ATIKnownChipsets[i].device == id)
			return ATIKnownChipsets[i].name;
	}
	return ATIKnownChipsets[0].name;
}

char *get_ati_fb(uint32_t id) {
	int i=0;
	for(i = 0; i <  (sizeof(ATIKnownFramebuffers) / sizeof(ATIKnownFramebuffers[0])); i++) {
		if(ATIKnownFramebuffers[i].device == id)
			return ATIKnownFramebuffers[i].name;
	}
	return ATIKnownFramebuffers[0].name;
}

int devprop_add_iopciconfigspace(struct DevPropDevice *device, pci_dt_t *ati_dev)
{
	if(!device)
		return 0;
	if(!ati_dev)
		return 0;
	printf("dumping pci config space, 256 bytes\n");
	int i;
	uint8_t *config_space;
	config_space = malloc(256);
	for(i=0; i<=255; i++)
		config_space[i] = pci_config_read8( ati_dev->dev.addr, i);
	devprop_add_value(device, "ATY,PCIConfigSpace", config_space, 256);
	free(config_space);
	return 1;
}

int devprop_add_ati_template_4xxx(struct DevPropDevice *device)
{
	if(!device)
		return 0;
	
//	if(!DP_ADD_TEMP_VAL(device, ati_compatible_0))
//		return 0;
//	if(!DP_ADD_TEMP_VAL(device, ati_compatible_1))
//		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_device_type_0))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_device_type_1))
		return 0;
//	if(!DP_ADD_TEMP_VAL(device, ati_device_type))
//		return 0;
//	if(!DP_ADD_TEMP_VAL(device, ati_name_0))
//		return 0;
//	if(!DP_ADD_TEMP_VAL(device, ati_name_1))
//		return 0;
//	if(!DP_ADD_TEMP_VAL(device, ati_name))
//		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efidisplay_0))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_display_type_0))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_display_type_1))
		return 0;
//	if(!DP_ADD_TEMP_VAL(device, ati_slot_name))
//		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_card_no))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_copyright))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efi_compile_d))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efi_version))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efi_versionB))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efi_versionE))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_mrt))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_romno))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_name_1))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_connector_type_0))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_connector_type_1))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_display_con_fl_type_0))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_aux_power_conn))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_backlight_ctrl))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_aapl01_coher))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_efi_disp_conf))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_efi_drv_type))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_efi_enbl_mode))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_efi_init_stat))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_efi_orientation))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_mclk))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_mem_rev_id))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_mem_vend_id))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_sclk))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_vendor_id))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_platform_info))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_mvad))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_saved_config))
		return 0;
	return 1;
}

int devprop_add_ati_template(struct DevPropDevice *device)
{
	if(!device)
		return 0;

	if(!DP_ADD_TEMP_VAL_DATA(device, ati_platform_info))
		return 0;	
	if(!DP_ADD_TEMP_VAL(device, ati_device_type_0))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_device_type_1))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efidisplay_0_n4))
		return 0;
//	if(!DP_ADD_TEMP_VAL(device, ati_slot_name_n4))
//		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_card_no))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_copyright))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efi_compile_d))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efi_version))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efi_versionB))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_efi_versionE))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_mrt))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_romno))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, ati_name_1))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_connector_type_0_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_connector_type_1_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_aux_power_conn))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_backlight_ctrl))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_aapl01_coher))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_efi_drv_type))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_efi_enbl_mode))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_mem_rev_id))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_mem_vend_id))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_vendor_id))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_aapl_emc_disp_list_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_fb_offset_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_hwgpio_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_iospace_offset_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_mclk_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_sclk_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_refclk_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_regspace_offset_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_efi_orientation_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_aapl_blackscr_prefs_0_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_aapl_blackscr_prefs_1_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_swgpio_info_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_efi_orientation_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_mvad_n4))
		return 0;
	if(!DP_ADD_TEMP_VAL_DATA(device, ati_saved_config_n4))
		return 0;
	return 1;
}


bool setup_ati_devprop(pci_dt_t *ati_dev)
{
//	int			len;
	char		*devicepath;
//	volatile uint8_t *regs;
//	uint32_t	bar[7];
	
	char		*model;
	char		*framebuffer;
	char		tmpString[64];
	char		*rom=0;
	uint32_t	rom_size=0;
	char		*biosimage = 0;
	uint32_t		biosimage_size = 0;
	uint8_t		toFree = 0;
	char 		romfilename[32];
	const char *val;
    int len1;
	uint32_t	vram_size=0;
	uint8_t		cmd=0;
	
	devicepath = get_pci_dev_path(ati_dev);

	cmd = pci_config_read8( ati_dev->dev.addr, 4);
	verbose("old pci command - %x\n", cmd);
	if( cmd == 0) {
	    pci_config_write8(ati_dev->dev.addr, 4, 6);	
	    cmd = pci_config_read8( ati_dev->dev.addr, 4);
	    verbose("new pci command - %x\n", cmd);
	};

	model = get_ati_model((ati_dev->vendor_id << 16) | ati_dev->device_id);
	
	framebuffer = get_ati_fb((ati_dev->vendor_id << 16) | ati_dev->device_id);
	if (!string)
		string = devprop_create_string();

	struct DevPropDevice *device = malloc(sizeof(struct DevPropDevice));
	device = devprop_add_device(string, devicepath);
	
	if(!device)
	{
		printf("Failed initializing dev-prop string dev-entry, press any key...\n");
		getc();
		return false;
	}

	/* FIXME: for primary graphics card only */
	uint32_t boot_display = 0x00000001;
	if(radeon_card_posted(ati_dev)) boot_display=0x01;
		else boot_display=0x00;
	verbose("boot display - %x\n", boot_display);
	devprop_add_value(device, "@0,AAPL,boot-display", (uint8_t*)&boot_display, 4);

	if(framebuffer[0] == 'M' && framebuffer[1] == 'o' && framebuffer[2] == 't') //faster than strcmp ;)
		devprop_add_ati_template_4xxx(device);
	else
	{
		devprop_add_ati_template(device);
		vram_size = getvramsizekb(ati_dev) * 1024;
		if ((vram_size > 0x80000000) || (vram_size == 0)) vram_size = 0x10000000; //vram reported wrong, defaulting to 256 mb
		devprop_add_value(device, "VRAM,totalsize", (uint8_t*)&vram_size, 4);
		ati_vram_memsize_0.data[6] = (vram_size >> 16) & 0xFF; //4,5 are 0x00 anyway
		ati_vram_memsize_0.data[7] = (vram_size >> 24) & 0xFF;		
		ati_vram_memsize_1.data[6] = (vram_size >> 16) & 0xFF; //4,5 are 0x00 anyway
		ati_vram_memsize_1.data[7] = (vram_size >> 24) & 0xFF;
		DP_ADD_TEMP_VAL_DATA(device, ati_vram_memsize_0);
		DP_ADD_TEMP_VAL_DATA(device, ati_vram_memsize_1);
		devprop_add_iopciconfigspace(device, ati_dev);		
	}
	devprop_add_value(device, "model", (uint8_t*)model, (strlen(model) + 1));
	devprop_add_value(device, "ATY,DeviceID", (uint8_t*)&ati_dev->device_id, 2);
	//fb setup

	char tmp[10];                                                                                                                                                                                 

	sprintf(tmp, "Slot-%x",devices_number);
	devprop_add_value(device, "AAPL,slot-name", (uint8_t*)tmp, strlen(tmp));
	devices_number++;
	
	sprintf(tmpString, ati_compatible_0[1], framebuffer);
	devprop_add_value(device, (char *) ati_compatible_0[0], (uint8_t *)tmpString, strlen(tmpString)+1);
	
	sprintf(tmpString, ati_compatible_1[1], framebuffer);
	devprop_add_value(device, (char *) ati_compatible_1[0], (uint8_t *)tmpString, strlen(tmpString)+1);
	
	sprintf(tmpString, ati_device_type[1], framebuffer);
	devprop_add_value(device, (char *) ati_device_type[0], (uint8_t *)tmpString, strlen(tmpString)+1);
	
	sprintf(tmpString, ati_name[1], framebuffer);
	devprop_add_value(device, (char *) ati_name[0], (uint8_t *)tmpString, strlen(tmpString)+1);
	
	sprintf(tmpString, ati_name_0[1], framebuffer);
	devprop_add_value(device, (char *) ati_name_0[0], (uint8_t *)tmpString, strlen(tmpString)+1);
	
	sprintf(tmpString, ati_name_1[1], framebuffer);
	devprop_add_value(device, (char *) ati_name_1[0], (uint8_t *)tmpString, strlen(tmpString)+1);
	//code for loading bios from file, in form - rom_devid_vendif.rom
	if (getValueForKey("-useatirom", &val, &len1, &bootInfo->bootConfig))
	{
		sprintf(romfilename, "ati_%04x_%04x.rom", (uint16_t)ati_dev->device_id, (uint16_t)ati_dev->vendor_id);
		verbose("looking for file /Extra/%s\n", romfilename);
		rom = malloc(0x20000);
		rom_size = load_ati_bios_file((char *)romfilename, (char *)rom);
		if(rom_size > 0x10000) rom_size = 0x10000; //we dont need rest anyway;
		if(rom_size == 0) printf("file not found\n");
	};
	{
		if(rom_size == 0)
		{
			if(boot_display) // no custom rom
				biosimage = 0; //try to dump from legacy space, otherwise can result in 100% fan speed
			else
				biosimage = (char *)readAtomBIOS(ati_dev); //readAtomBios result in bug on some cards (100% fan speed and black screen),
											   //not using it for posted card, rading from legacy space instead
		} else {
			biosimage = rom; //going custom rom way
			verbose("Using rom %s\n", romfilename);
		}
		if(biosimage == 0)
		{
			biosimage = (char *)0xC0000;
			toFree = 0;
			verbose("Not going to use bios image file\n");
		} else toFree = 1;

		if ((uint8_t)biosimage[0] == 0x55 && (uint8_t)biosimage[1] == 0xaa)
		{
			printf("Found bios image\n");
			biosimage_size = (uint8_t)biosimage[2] * 512;
		
			struct  pci_rom_pci_header_t *rom_pci_header;   
			rom_pci_header = (struct pci_rom_pci_header_t*)(biosimage + (uint8_t)biosimage[24] + (uint8_t)biosimage[25]*256);
		
        	if (rom_pci_header->signature == 0x52494350)
			{
         		if (rom_pci_header->device != ati_dev->device_id)
				{
					verbose("Bios image (%x) doesnt match card (%x), ignoring\n", rom_pci_header->device, ati_dev->device_id);
				} else {
					if(toFree)
						verbose("Adding binimage to card %x from mmio space with size %x\n", ati_dev->device_id, biosimage_size);
					else
						verbose("Adding binimage to card %x from legacy space with size %x\n", ati_dev->device_id, biosimage_size);
					devprop_add_value(device, "ATY,bin_image", (uint8_t*) biosimage, biosimage_size);
				}
			} else verbose("Wrong pci header signature %x\n", rom_pci_header->signature);
			
		} else verbose("Bios image not found at  %x, content %x %x\n", biosimage, (uint8_t)biosimage[0], (uint8_t)biosimage[1]);
		if(toFree) free(biosimage);
	}
	stringdata = malloc(sizeof(uint8_t) * string->length);
	if(!stringdata)
	{
		printf("no stringdata press a key...\n");
		getc();
		return false;
	}
	
	memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
	stringlength = string->length;

	return true;
}
