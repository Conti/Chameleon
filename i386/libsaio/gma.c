/*
	Original patch by Nawcom
	http://forum.voodooprojects.org/index.php/topic,1029.0.html
 
    Original Intel HDx000 code from valv
*/

#include "libsa.h"
#include "saio_internal.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "device_inject.h"
#include "gma.h"
#include "vbe.h"
#include "graphics.h"

#ifndef DEBUG_GMA
#define DEBUG_GMA 0
#endif

#if DEBUG_GMA
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

uint8_t GMAX3100_vals[22][4] = {
	{ 0x01,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x08 },
	{ 0x64,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x08 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x20,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x20,0x03,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x08,0x52,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x3B,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 }
};

uint8_t HD2000_vals[16][4] = {
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x14,0x00,0x00,0x00 },
	{ 0xfa,0x00,0x00,0x00 },
	{ 0x2c,0x01,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x14,0x00,0x00,0x00 },
	{ 0xf4,0x01,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
};

uint8_t HD3000_vals[16][4] = {
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x14,0x00,0x00,0x00 },
	{ 0xfa,0x00,0x00,0x00 },
	{ 0x2c,0x01,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x14,0x00,0x00,0x00 },
	{ 0xf4,0x01,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
};

uint8_t HD2000_tbl_info[18] = {
	0x30,0x44,0x02,0x02,0x02,0x02,0x00,0x00,0x00,
	0x00,0x01,0x02,0x02,0x02,0x00,0x01,0x02,0x02
};
uint8_t HD2000_os_info[20] = {
	0x30,0x49,0x01,0x11,0x11,0x11,0x08,0x00,0x00,0x01,
	0xf0,0x1f,0x01,0x00,0x00,0x00,0x10,0x07,0x00,0x00
};

// The following values came from a Sandy Bridge MacBook Air
uint8_t HD3000_tbl_info[18] = {
	0x30,0x44,0x02,0x02,0x02,0x02,0x00,0x00,0x00,
	0x00,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01
};

// The following values came from a Sandy Bridge MacBook Air
uint8_t HD3000_os_info[20] = {
	0x30,0x49,0x01,0x12,0x12,0x12,0x08,0x00,0x00,0x01,
	0xf0,0x1f,0x01,0x00,0x00,0x00,0x10,0x07,0x00,0x00
};


uint8_t reg_TRUE[]	= { 0x01, 0x00, 0x00, 0x00 };
uint8_t reg_FALSE[] = { 0x00, 0x00, 0x00, 0x00 };

static struct gma_gpu_t KnownGPUS[] = {
	{ 0x00000000, "Unknown"			},
	{ 0x808627A2, "Mobile GMA950"	},
	{ 0x808627AE, "Mobile GMA950"	},
	{ 0x808627A6, "Mobile GMA950"	},
	{ 0x8086A011, "Mobile GMA3150"	},
	{ 0x8086A012, "Mobile GMA3150"	},
	{ 0x80862772, "Desktop GMA950"	},
	{ 0x80862776, "Desktop GMA950"	},
//	{ 0x8086A001, "Desktop GMA3150" },
	{ 0x8086A001, "Mobile GMA3150"	},
	{ 0x8086A002, "Desktop GMA3150" },
	{ 0x80862A02, "GMAX3100"		},
	{ 0x80862A03, "GMAX3100"		},
	{ 0x80862A12, "GMAX3100"		},
	{ 0x80862A13, "GMAX3100"		},
	{ 0x80862A42, "GMAX3100"		},
	{ 0x80862A43, "GMAX3100"		},
	{ 0x80860102, "HD Graphics 2000"			},
	{ 0x80860106, "HD Graphics 2000 Mobile"	},
	{ 0x80860112, "HD Graphics 3000"			},
	{ 0x80860116, "HD Graphics 3000 Mobile"	},
	{ 0x80860122, "HD Graphics 3000"			},
	{ 0x80860126, "HD Graphics 3000 Mobile"	},
	{ 0x80860162, "HD Graphics 4000"          },
	{ 0x80860166, "HD Graphics 4000 Mobile"   },
};

char *get_gma_model(uint32_t id) {
	int i = 0;
	
	for (i = 0; i < (sizeof(KnownGPUS) / sizeof(KnownGPUS[0])); i++)
	{
		if (KnownGPUS[i].device == id)
			return KnownGPUS[i].name;
	}
	return KnownGPUS[0].name;
}

bool setup_gma_devprop(pci_dt_t *gma_dev)
{
	char					*devicepath;
	volatile uint8_t		*regs;
	uint32_t				bar[7];
	char					*model;
	uint8_t BuiltIn =		0x00;
	uint8_t ClassFix[4] =	{ 0x00, 0x00, 0x03, 0x00 };
	unsigned int			device_id;	
	
	devicepath = get_pci_dev_path(gma_dev);
	
	bar[0] = pci_config_read32(gma_dev->dev.addr, 0x10);
	regs = (uint8_t *) (bar[0] & ~0x0f);
	
	model = get_gma_model((gma_dev->vendor_id << 16) | gma_dev->device_id);
	device_id = gma_dev->device_id;
	
	verbose("Intel %s [%04x:%04x] :: %s\n",
			model, gma_dev->vendor_id, gma_dev->device_id, devicepath);
	
	if (!string)
		string = devprop_create_string();
	
	struct DevPropDevice *device = malloc(sizeof(struct DevPropDevice));
	device = devprop_add_device(string, devicepath);
	
	if (!device)
	{
		printf("Failed initializing dev-prop string dev-entry.\n");
		pause();
		return false;
	}
	
	devprop_add_value(device, "model", (uint8_t*)model, (strlen(model) + 1));
	devprop_add_value(device, "device_type", (uint8_t*)"display", 8);	
	
	if ((model == (char *)&"Mobile GMA950")
		|| (model == (char *)&"Mobile GMA3150"))
	{
		devprop_add_value(device, "AAPL,HasPanel", reg_TRUE, 4);
		devprop_add_value(device, "built-in", &BuiltIn, 1);
		devprop_add_value(device, "class-code", ClassFix, 4);
	}
	else if ((model == (char *)&"Desktop GMA950")
			|| (model == (char *)&"Desktop GMA3150"))
	{
		BuiltIn = 0x01;
		devprop_add_value(device, "built-in", &BuiltIn, 1);
		devprop_add_value(device, "class-code", ClassFix, 4);
	}
	else if (model == (char *)&"GMAX3100")
	{
		devprop_add_value(device, "AAPL,HasPanel",					GMAX3100_vals[0], 4);
		devprop_add_value(device, "AAPL,SelfRefreshSupported",		GMAX3100_vals[1], 4);
		devprop_add_value(device, "AAPL,aux-power-connected",		GMAX3100_vals[2], 4);
		devprop_add_value(device, "AAPL,backlight-control",			GMAX3100_vals[3], 4);
		devprop_add_value(device, "AAPL00,blackscreen-preferences", GMAX3100_vals[4], 4);
		devprop_add_value(device, "AAPL01,BacklightIntensity",		GMAX3100_vals[5], 4);
		devprop_add_value(device, "AAPL01,blackscreen-preferences", GMAX3100_vals[6], 4);
		devprop_add_value(device, "AAPL01,DataJustify",				GMAX3100_vals[7], 4);
		devprop_add_value(device, "AAPL01,Depth",					GMAX3100_vals[8], 4);
		devprop_add_value(device, "AAPL01,Dither",					GMAX3100_vals[9], 4);
		devprop_add_value(device, "AAPL01,DualLink",				GMAX3100_vals[10], 4);
		devprop_add_value(device, "AAPL01,Height",					GMAX3100_vals[11], 4);
		devprop_add_value(device, "AAPL01,Interlace",				GMAX3100_vals[12], 4);
		devprop_add_value(device, "AAPL01,Inverter",				GMAX3100_vals[13], 4);
		devprop_add_value(device, "AAPL01,InverterCurrent",			GMAX3100_vals[14], 4);
		devprop_add_value(device, "AAPL01,InverterCurrency",		GMAX3100_vals[15], 4);
		devprop_add_value(device, "AAPL01,LinkFormat",				GMAX3100_vals[16], 4);
		devprop_add_value(device, "AAPL01,LinkType",				GMAX3100_vals[17], 4);
		devprop_add_value(device, "AAPL01,Pipe",					GMAX3100_vals[18], 4);
		devprop_add_value(device, "AAPL01,PixelFormat",				GMAX3100_vals[19], 4);
		devprop_add_value(device, "AAPL01,Refresh",					GMAX3100_vals[20], 4);
		devprop_add_value(device, "AAPL01,Stretch",					GMAX3100_vals[21], 4);
		devprop_add_value(device, "class-code",						ClassFix, 4);
	}
	else if (model == (char *)&"HD Graphics 2000 Mobile")
	{
		devprop_add_value(device, "class-code", ClassFix, 4);
		devprop_add_value(device, "hda-gfx", (uint8_t *)"onboard-1", 10); 
		devprop_add_value(device, "AAPL00,PixelFormat", HD2000_vals[0], 4);
		devprop_add_value(device, "AAPL00,T1", HD2000_vals[1], 4);
		devprop_add_value(device, "AAPL00,T2", HD2000_vals[2], 4);
		devprop_add_value(device, "AAPL00,T3", HD2000_vals[3], 4);
		devprop_add_value(device, "AAPL00,T4", HD2000_vals[4], 4);
		devprop_add_value(device, "AAPL00,T5", HD2000_vals[5], 4);
		devprop_add_value(device, "AAPL00,T6", HD2000_vals[6], 4);
		devprop_add_value(device, "AAPL00,T7", HD2000_vals[7], 4);
		devprop_add_value(device, "AAPL00,LinkType", HD2000_vals[8], 4);
		devprop_add_value(device, "AAPL00,LinkFormat", HD2000_vals[9], 4);
		devprop_add_value(device, "AAPL00,DualLink", HD2000_vals[10], 4);
		devprop_add_value(device, "AAPL00,Dither", HD2000_vals[11], 4);
		devprop_add_value(device, "AAPL00,DataJustify", HD3000_vals[12], 4);
		devprop_add_value(device, "graphic-options", HD2000_vals[13], 4);
		devprop_add_value(device, "AAPL,tbl-info", HD2000_tbl_info, 18);
		devprop_add_value(device, "AAPL,os-info", HD2000_os_info, 20);
	}
	else if (model == (char *)&"HD Graphics 3000 Mobile")
	{
		devprop_add_value(device, "class-code", ClassFix, 4);
		devprop_add_value(device, "hda-gfx", (uint8_t *)"onboard-1", 10); 
		devprop_add_value(device, "AAPL00,PixelFormat", HD3000_vals[0], 4);
		devprop_add_value(device, "AAPL00,T1", HD3000_vals[1], 4);
		devprop_add_value(device, "AAPL00,T2", HD3000_vals[2], 4);
		devprop_add_value(device, "AAPL00,T3", HD3000_vals[3], 4);
		devprop_add_value(device, "AAPL00,T4", HD3000_vals[4], 4);
		devprop_add_value(device, "AAPL00,T5", HD3000_vals[5], 4);
		devprop_add_value(device, "AAPL00,T6", HD3000_vals[6], 4);
		devprop_add_value(device, "AAPL00,T7", HD3000_vals[7], 4);
		devprop_add_value(device, "AAPL00,LinkType", HD3000_vals[8], 4);
		devprop_add_value(device, "AAPL00,LinkFormat", HD3000_vals[9], 4);
		devprop_add_value(device, "AAPL00,DualLink", HD3000_vals[10], 4);
		devprop_add_value(device, "AAPL00,Dither", HD3000_vals[11], 4);
		devprop_add_value(device, "AAPL00,DataJustify", HD3000_vals[12], 4);
		devprop_add_value(device, "graphic-options", HD3000_vals[13], 4);
		devprop_add_value(device, "AAPL,tbl-info", HD3000_tbl_info, 18);
		devprop_add_value(device, "AAPL,os-info", HD3000_os_info, 20);
	}
	else if (model == (char *)&"HD Graphics 2000")
	{
		devprop_add_value(device, "built-in", &BuiltIn, 1);
		devprop_add_value(device, "class-code", ClassFix, 4);
		devprop_add_value(device, "device-id", (uint8_t*)&device_id, sizeof(device_id));
		devprop_add_value(device, "hda-gfx", (uint8_t *)"onboard-1", 10); 
		devprop_add_value(device, "AAPL,tbl-info", HD2000_tbl_info, 18);
		devprop_add_value(device, "AAPL,os-info", HD2000_os_info, 20);
	}
	else if (model == (char *)&"HD Graphics 3000")
	{
		devprop_add_value(device, "built-in", &BuiltIn, 1);
		devprop_add_value(device, "class-code", ClassFix, 4);
		device_id = 0x00000126;											// Inject a valid mobile GPU device id instead of patching kexts
		devprop_add_value(device, "device-id", (uint8_t*)&device_id, sizeof(device_id));
		devprop_add_value(device, "hda-gfx", (uint8_t *)"onboard-1", 10); 
		devprop_add_value(device, "AAPL,tbl-info", HD3000_tbl_info, 18);
		devprop_add_value(device, "AAPL,os-info", HD3000_os_info, 20);
	}
	else if(model == (char*)&"HD Graphics 4000" ||
			model == (char*)&"HD Graphics 4000 Mobile")
	{
		uint32_t ram = (((getVBEVideoRam() + 512) / 1024) + 512) / 1024;
		uint32_t ig_platform_id;
		
		switch (ram)
		{
			case 96:
				ig_platform_id = 0x01660000; // 96mb
				break;
					
			case 64:
				ig_platform_id = 0x01660009; // 64mb
				break;
				
			case 32:					
				ig_platform_id = 0x01620005; // 32mb
				break;
			default:
				printf("Please specify 96, 64, or 32MB RAM for the HD4000 in the bios.\n"
					  "The selected %dMB RAM configuration is not supported for the  HD4000.\n", ram);
				pause();
				return false;	// Exit early before the AAPL,ig-platform-id property is set.
				break;
		}

		devprop_add_value(device, "built-in", &BuiltIn, 1);
		devprop_add_value(device, "class-code", ClassFix, 4);
		devprop_add_value(device, "hda-gfx", (uint8_t *)"onboard-1", 10);
		devprop_add_value(device, "AAPL,ig-platform-id", (uint8_t*)&ig_platform_id, 4);
	}
	
	stringdata = malloc(sizeof(uint8_t) * string->length);
	if (!stringdata)
	{
		printf("No stringdata.\n");
		pause();
		return false;
	}
	
	memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
	stringlength = string->length;
	
	return true;
}
