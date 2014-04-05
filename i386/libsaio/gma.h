/*
 * Copyright 2013 Intel Corporation
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 Original patch by Nawcom
 http://forum.voodooprojects.org/index.php/topic,1029.0.html
 
 Original Intel HDx000 code from valv
 Intel HD4xx and HD5xx code by ErmaC http://www.insanelymac.com/forum/topic/288241-intel-hd4000-inject-aaplig-platform-id/
 */

#ifndef __LIBSAIO_GMA_H
#define __LIBSAIO_GMA_H

bool setup_gma_devprop(pci_dt_t *gma_dev);

struct intel_gfx_info_t;
typedef struct{
    uint32_t	model;
	char		*label_info;
}intel_gfx_info_t;

#define REG8(reg)	((volatile uint8_t *)regs)[(reg)]
#define REG16(reg)	((volatile uint16_t *)regs)[(reg) >> 1]
#define REG32(reg)	((volatile uint32_t *)regs)[(reg) >> 2]

/****************************************************************************
 * Miscellanious defines
 ****************************************************************************/

/* Intel gfx Controller models */
#define GFX_MODEL_CONSTRUCT(vendor, model) (((uint32_t)(model) << 16) | ((vendor##_VENDORID) & 0xffff))

/* Intel */
#define INTEL_NAME          "Intel"
#define HD_GRAPHICS         "HD Graphics"
#define HD_GRAPHICS_2000    "HD Graphics 2000"
#define HD_GRAPHICS_2500    "HD Graphics 2500"
#define HD_GRAPHICS_3000    "HD Graphics 3000"
#define HD_GRAPHICS_4000    "HD Graphics 4000"
#define HD_GRAPHICS_4600    "HD Graphics 4600"
#define HD_GRAPHICS_5000    "HD Graphics 5000"
#define IRIS_5100           "Iris(TM) Graphics 5100"
#define IRIS_5200           "Iris(TM) Pro Graphics 5200"
#define INTEL_VENDORID		0x8086

/* http://cgit.freedesktop.org/xorg/driver/xf86-video-intel/tree/src/intel_driver.h */
/* http://people.redhat.com/agk/patches/linux/patches-3.6/git-update1.patch */
            
#define GMA_I810                   GFX_MODEL_CONSTRUCT(INTEL, 0x7121)
#define GMA_I810_DC100             GFX_MODEL_CONSTRUCT(INTEL, 0x7123)
#define GMA_I810_E                 GFX_MODEL_CONSTRUCT(INTEL, 0x7125)
#define GMA_I815                   GFX_MODEL_CONSTRUCT(INTEL, 0x1132)
/* ==================================== */

#define GMA_I830_M                 GFX_MODEL_CONSTRUCT(INTEL, 0x3577)
#define GMA_845_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2562)
#define GMA_I854                   GFX_MODEL_CONSTRUCT(INTEL, 0x358E)
#define GMA_I855_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x3582)
#define GMA_I865_G                 GFX_MODEL_CONSTRUCT(INTEL, 0x2572)
/* ==================================== */

#define GMA_I915_G                 GFX_MODEL_CONSTRUCT(INTEL, 0x2582) // GMA 915
#define GMA_I915_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x2592) // GMA 915
#define GMA_E7221_G                GFX_MODEL_CONSTRUCT(INTEL, 0x258A)
#define GMA_I945_G                 GFX_MODEL_CONSTRUCT(INTEL, 0x2772) // Desktop GMA950
//#define GMA_82945G                 GFX_MODEL_CONSTRUCT(INTEL, 2776) // Desktop GMA950
//#define GMA_82915G                 GFX_MODEL_CONSTRUCT(INTEL, 2782) // GMA 915
//#define GMA_038000                 GFX_MODEL_CONSTRUCT(INTEL, 2792) // Mobile GMA915
#define GMA_I945_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x27A2) // Mobile GMA950
#define GMA_I945_GME               GFX_MODEL_CONSTRUCT(INTEL, 0x27AE) // Mobile GMA950
//#define GMA_945GM               GFX_MODEL_CONSTRUCT(INTEL, 27A6) // Mobile GMA950
//#define GMA_PINEVIEW_M_HB             GFX_MODEL_CONSTRUCT(INTEL, 0xA010)
#define GMA_PINEVIEW_M             GFX_MODEL_CONSTRUCT(INTEL, 0xA011) // Mobile GMA3150
#define GMA_GMA3150_M              GFX_MODEL_CONSTRUCT(INTEL, 0xA012) // Mobile GMA3150
//#define GMA_PINEVIEW_HB             GFX_MODEL_CONSTRUCT(INTEL, 0xA000)
#define GMA_PINEVIEW_G             GFX_MODEL_CONSTRUCT(INTEL, 0xA001) // Mobile GMA3150
#define GMA_GMA3150_D              GFX_MODEL_CONSTRUCT(INTEL, 0xA002) // Desktop GMA3150
#define GMA_Q35_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x29B2)
#define GMA_G33_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x29C2) // Desktop GMA3100
// 29C3 // Desktop GMA3100
#define GMA_Q33_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x29D2)
/* ==================================== */

#define GMA_G35_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2982)
#define GMA_I965_Q                 GFX_MODEL_CONSTRUCT(INTEL, 0x2992)
#define GMA_I965_G                 GFX_MODEL_CONSTRUCT(INTEL, 0x29A2)
#define GMA_I946_GZ                GFX_MODEL_CONSTRUCT(INTEL, 0x2972)
#define GMA_I965_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x2A02) // GMAX3100
#define GMA_I965_GME               GFX_MODEL_CONSTRUCT(INTEL, 0x2A12) // GMAX3100
#define GMA_GM45_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x2A42) // GMAX3100
//#define GMA_GM45_GM2                GFX_MODEL_CONSTRUCT(INTEL, 0x2A43) // GMAX3100
#define GMA_G45_E_G                GFX_MODEL_CONSTRUCT(INTEL, 0x2E02)
#define GMA_G45_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2E22)
#define GMA_Q45_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2E12)
#define GMA_G41_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2E32)
#define GMA_B43_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2E42)
#define GMA_B43_G1                 GFX_MODEL_CONSTRUCT(INTEL, 0x2E92)

#define GMA_IRONLAKE_D_G           GFX_MODEL_CONSTRUCT(INTEL, 0x0042) // HD2000
#define GMA_IRONLAKE_M_G           GFX_MODEL_CONSTRUCT(INTEL, 0x0046) // HD2000
/*
#define GMA_IRONLAKE_D_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0040)
#define GMA_IRONLAKE_D2_HB         GFX_MODEL_CONSTRUCT(INTEL, 0x0069)
#define GMA_IRONLAKE_M_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0044)
#define GMA_IRONLAKE_MA_HB         GFX_MODEL_CONSTRUCT(INTEL, 0x0062)
#define GMA_IRONLAKE_MC2_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x006a)
*/
// 004A // HD2000
/* ==================================== */

/* ========== Sandy Bridge ============ */
//#define GMA_SANDYBRIDGE_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0100) /* Desktop */
#define GMA_SANDYBRIDGE_GT1        GFX_MODEL_CONSTRUCT(INTEL, 0x0102) // HD Graphics 2000
//#define GMA_SANDYBRIDGE_M_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0104) /* Mobile */
#define GMA_SANDYBRIDGE_GT2        GFX_MODEL_CONSTRUCT(INTEL, 0x0112) // HD Graphics 3000
#define GMA_SANDYBRIDGE_GT2_PLUS	GFX_MODEL_CONSTRUCT(INTEL, 0x0122) // HD Graphics 3000
#define GMA_SANDYBRIDGE_M_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0106) // HD Graphics 2000 Mobile
#define GMA_SANDYBRIDGE_M_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0116) // HD Graphics 3000 Mobile
#define GMA_SANDYBRIDGE_M_GT2_PLUS	GFX_MODEL_CONSTRUCT(INTEL, 0x0126) // HD Graphics 3000 Mobile
//#define GMA_SANDYBRIDGE_S_HB     GFX_MODEL_CONSTRUCT(INTEL, 0x0108) /* Server */
#define GMA_SANDYBRIDGE_S_GT       GFX_MODEL_CONSTRUCT(INTEL, 0x010A) // HD Graphics
// 010B // ??
// 010E // ??
/* ==================================== */

/* ========== Ivy Bridge ============== */
//#define GMA_IVYBRIDGE_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0150)  /* Desktop */
//#define GMA_IVYBRIDGE_M_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0154)  /* Mobile */
#define GMA_IVYBRIDGE_M_GT1        GFX_MODEL_CONSTRUCT(INTEL, 0x0156) // HD Graphics 2500 Mobile
#define GMA_IVYBRIDGE_M_GT2        GFX_MODEL_CONSTRUCT(INTEL, 0x0166) // HD Graphics 4000 Mobile
#define GMA_IVYBRIDGE_D_GT1        GFX_MODEL_CONSTRUCT(INTEL, 0x0152) // HD Graphics 2500
#define GMA_IVYBRIDGE_D_GT2        GFX_MODEL_CONSTRUCT(INTEL, 0x0162) // HD Graphics 4000
//#define GMA_IVYBRIDGE_S_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0158) /* Server */
#define GMA_IVYBRIDGE_S_GT1        GFX_MODEL_CONSTRUCT(INTEL, 0x015A) // HD Graphics 4000
#define GMA_IVYBRIDGE_S_GT2        GFX_MODEL_CONSTRUCT(INTEL, 0x016A) // HD Graphics P4000
#define GMA_IVYBRIDGE_S_GT3        GFX_MODEL_CONSTRUCT(INTEL, 0x015E) // Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller
#define GMA_IVYBRIDGE_S_GT4        GFX_MODEL_CONSTRUCT(INTEL, 0x0172) // HD Graphics 2500 Mobile // Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller
#define GMA_IVYBRIDGE_S_GT5        GFX_MODEL_CONSTRUCT(INTEL, 0x0176) // HD Graphics 2500 Mobile // 3rd Gen Core processor Graphics Controller
/* ==================================== */

//#define GMA_VALLEYVIEW_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0F00) /* VLV1 */
//#define GMA_VALLEYVIEW_IG          GFX_MODEL_CONSTRUCT(INTEL, 0x0F30)

/* ============ Haswell =============== */
// 0090 // AppleIntelHD5000Graphics.kext
// 0091 // AppleIntelHD5000Graphics.kext
// 0092 // AppleIntelHD5000Graphics.kext
//#define GMA_HASWELL_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0400) /* Desktop */
#define GMA_HASWELL_D_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x0402) //
#define GMA_HASWELL_D_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x0412) // AppleIntelHD5000Graphics.kext
#define GMA_HASWELL_D_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x0422) //
//#define GMA_HASWELL_M_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0404) /* Mobile */
#define GMA_HASWELL_M_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x0406) // AppleIntelHD5000Graphics.kext
#define GMA_HASWELL_M_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x0416) // AppleIntelHD5000Graphics.kext
#define GMA_HASWELL_M_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x0426) // AppleIntelHD5000Graphics.kext
#define GMA_HASWELL_S_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x040A) //
//#define GMA_HASWELL_S_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0408) /* Server */
#define GMA_HASWELL_S_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x041A) //
#define GMA_HASWELL_S_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x042A) //
#define GMA_HASWELL_B_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x040B)
#define GMA_HASWELL_B_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x041B)
#define GMA_HASWELL_B_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x042B)
#define GMA_HASWELL_E_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x040E)
#define GMA_HASWELL_E_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x041E)
#define GMA_HASWELL_E_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x042E)

#define GMA_HASWELL_ULT_D_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A02)
#define GMA_HASWELL_ULT_D_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A12)
#define GMA_HASWELL_ULT_D_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A22) //
#define GMA_HASWELL_ULT_M_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A06) //
#define GMA_HASWELL_ULT_M_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A16) // AppleIntelHD5000Graphics.kext
#define GMA_HASWELL_ULT_M_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A26) // AppleIntelHD5000Graphics.kext
#define GMA_HASWELL_ULT_S_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A0A)
#define GMA_HASWELL_ULT_S_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A1A)
#define GMA_HASWELL_ULT_S_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A2A)
#define GMA_HASWELL_ULT_B_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A0B)
#define GMA_HASWELL_ULT_B_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A1B)
#define GMA_HASWELL_ULT_B_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A2B)
#define GMA_HASWELL_ULT_E_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A0E) //
#define GMA_HASWELL_ULT_E_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A1E) // AppleIntelHD5000Graphics.kext
#define GMA_HASWELL_ULT_E_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A2E) // AppleIntelHD5000Graphics.kext

//#define GMA_HASWELL_SDV_D_GT1_IG GFX_MODEL_CONSTRUCT(INTEL, 0C02)
//#define GMA_HASWELL_E_HB         GFX_MODEL_CONSTRUCT(INTEL, 0C04)
//#define GMA_HASWELL_SDV_M_GT1_IG GFX_MODEL_CONSTRUCT(INTEL, 0C06) // AppleIntelHD5000Graphics.kext
//#define GMA_HASWELL_SDV_D_GT2_IG GFX_MODEL_CONSTRUCT(INTEL, 0C12)
//#define GMA_HASWELL_SDV_M_GT2_IG GFX_MODEL_CONSTRUCT(INTEL, 0C16) // AppleIntelHD5000Graphics.kext
//#define GMA_HASWELL_SDV_D_GT2_PLUS_IG GFX_MODEL_CONSTRUCT(INTEL, 0C22) // AppleIntelHD5000Graphics.kext
//#define GMA_HASWELL_SDV_M_GT2_PLUS_IG GFX_MODEL_CONSTRUCT(INTEL, 0C26) // AppleIntelHD5000Graphics.kext
//#define GMA_HASWELL_SDV_S_GT1_IG	GFX_MODEL_CONSTRUCT(INTEL, 0x0C0A)
//#define GMA_HASWELL_SDV_S_GT2_IG	GFX_MODEL_CONSTRUCT(INTEL, 0x0C1A)
//#define GMA_HASWELL_SDV_S_GT2_PLUS_IG	GFX_MODEL_CONSTRUCT(INTEL, 0x0C2A)

#define GMA_HASWELL_CRW_D_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D02)
#define GMA_HASWELL_CRW_D_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D12) //
#define GMA_HASWELL_CRW_D_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D22) //
//#define GMA_HASWELL_CRW_D_GT2_PLUS_IG	GFX_MODEL_CONSTRUCT(INTEL, 0x0D32)
#define GMA_HASWELL_CRW_M_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D06)
#define GMA_HASWELL_CRW_M_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D16) //
#define GMA_HASWELL_CRW_M_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D26) // AppleIntelHD5000Graphics.kext
#define GMA_HASWELL_CRW_S_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D0A)
#define GMA_HASWELL_CRW_S_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D1A)
#define GMA_HASWELL_CRW_S_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D2A)
#define GMA_HASWELL_CRW_B_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D0B)
#define GMA_HASWELL_CRW_B_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D1B)
#define GMA_HASWELL_CRW_B_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D2B)
#define GMA_HASWELL_CRW_E_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D0E)
#define GMA_HASWELL_CRW_E_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D1E)
#define GMA_HASWELL_CRW_E_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D2E)
#define GMA_HASWELL_CRW_M_GT2_PLUS_IG    GFX_MODEL_CONSTRUCT(INTEL, 0x0D36)
//#define GMA_HASWELL_CRW_S_GT2_PLUS_IG    GFX_MODEL_CONSTRUCT(INTEL, 0x0D3A)

/* END */

#endif /* !__LIBSAIO_GMA_H */
