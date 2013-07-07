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

#include "libsaio.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "dram_controllers.h"

#ifndef DEBUG_DRAM
#define DEBUG_DRAM 0
#endif

#if DEBUG_DRAM
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif

/*
 * Initialise memory controller functions
 */

// Setup P35 Memory Controller
static void setup_p35(pci_dt_t *dram_dev)
{
	uint32_t dev0;
	
	// Activate MMR I/O
	dev0 = pci_config_read32(dram_dev->dev.addr, 0x48);
	if (!(dev0 & 0x1))
	{
		pci_config_write8(dram_dev->dev.addr, 0x48, (dev0 | 1));
	}
}

int nhm_bus = 0x3F;

// Setup Nehalem Integrated Memory Controller
static void setup_nhm(pci_dt_t *dram_dev)
{
	static long possible_nhm_bus[] = {0xFF, 0x7F, 0x3F};
	unsigned long did, vid;
	int i;

	// Nehalem supports Scrubbing
	// First, locate the PCI bus where the MCH is located
	for(i = 0; i < sizeof(possible_nhm_bus); i++)
	{
		vid = pci_config_read16(PCIADDR(possible_nhm_bus[i], 3, 4), PCI_VENDOR_ID);
		did = pci_config_read16(PCIADDR(possible_nhm_bus[i], 3, 4), PCI_DEVICE_ID);
		vid &= 0xFFFF;
		did &= 0xFF00;
		
		if(vid == 0x8086 && did >= 0x2C00)
		{
			nhm_bus = possible_nhm_bus[i]; 
		}
	}
}

/*
 * Retrieve memory controller fsb functions
 */


// Get i965 Memory Speed
static void get_fsb_i965(pci_dt_t *dram_dev)
{
	uint32_t dev0, mch_ratio, mch_cfg, mch_fsb;

	long *ptr;
	
	// Find Ratio
	dev0 = pci_config_read32(dram_dev->dev.addr, 0x48);
	dev0 &= 0xFFFFC000;
	ptr = (long*)(dev0 + 0xC00);
	mch_cfg = *ptr & 0xFFFF;
	
	mch_ratio = 100000;
	
	switch (mch_cfg & 7)
	{
		case 0: mch_fsb = 1066; break;
		case 1: mch_fsb =  533; break;
		default: 
		case 2: mch_fsb =  800; break;
		case 3: mch_fsb =  667; break;		
		case 4: mch_fsb = 1333; break;
		case 6: mch_fsb = 1600; break;					
	}
	
	DBG("mch_fsb %d\n", mch_fsb);
	
	switch (mch_fsb)
	{
		case 533:
		switch ((mch_cfg >> 4) & 7)
		{
			case 1:	mch_ratio = 200000; break;
			case 2:	mch_ratio = 250000; break;
			case 3:	mch_ratio = 300000; break;
		}
		break;
			
		default:
		case 800:
		switch ((mch_cfg >> 4) & 7)
		{
			case 0:	mch_ratio = 100000; break;
			case 1:	mch_ratio = 125000; break;
			case 2:	mch_ratio = 166667; break; // 1.666666667
			case 3:	mch_ratio = 200000; break;
			case 4:	mch_ratio = 266667; break; // 2.666666667
			case 5:	mch_ratio = 333333; break; // 3.333333333
		}
		break;
			
		case 1066:
		switch ((mch_cfg >> 4) & 7)
		{
			case 1:	mch_ratio = 100000; break;
			case 2:	mch_ratio = 125000; break;
			case 3:	mch_ratio = 150000; break;
			case 4:	mch_ratio = 200000; break;
			case 5:	mch_ratio = 250000; break;
		}
		break;
			
		case 1333:
		switch ((mch_cfg >> 4) & 7)
		{
			case 2:	mch_ratio = 100000; break;
			case 3:	mch_ratio = 120000; break;
			case 4:	mch_ratio = 160000; break;
			case 5:	mch_ratio = 200000; break;
		}
		break;
			
		case 1600:
		switch ((mch_cfg >> 4) & 7)
		{
			case 3:	mch_ratio = 100000; break;
			case 4:	mch_ratio = 133333; break; // 1.333333333
			case 5:	mch_ratio = 150000; break;
			case 6:	mch_ratio = 200000; break;
		}
		break;
	}
	
	DBG("mch_ratio %d\n", mch_ratio);

	// Compute RAM Frequency
	Platform.RAM.Frequency = (Platform.CPU.FSBFrequency * mch_ratio) / 100000;
	
	DBG("ram_fsb %d\n", Platform.RAM.Frequency);

}

// Get i965m Memory Speed
static void get_fsb_im965(pci_dt_t *dram_dev)
{
	uint32_t dev0, mch_ratio, mch_cfg, mch_fsb;

	long *ptr;
	
	// Find Ratio
	dev0 = pci_config_read32(dram_dev->dev.addr, 0x48);
	dev0 &= 0xFFFFC000;
	ptr = (long*)(dev0 + 0xC00);
	mch_cfg = *ptr & 0xFFFF;
	
	mch_ratio = 100000;
	
	switch (mch_cfg & 7)
	{
		case 1: mch_fsb = 533; break;
		default: 
		case 2:	mch_fsb = 800; break;
		case 3:	mch_fsb = 667; break;				
		case 6:	mch_fsb = 1066; break;			
	}
	
	switch (mch_fsb)
	{
		case 533:
			switch ((mch_cfg >> 4) & 7)
			{
				case 1:	mch_ratio = 125000; break;
				case 2:	mch_ratio = 150000; break;
				case 3:	mch_ratio = 200000; break;
			}
			break;
			
		case 667:
			switch ((mch_cfg >> 4)& 7)
			{
				case 1:	mch_ratio = 100000; break;
				case 2:	mch_ratio = 120000; break;
				case 3:	mch_ratio = 160000; break;
				case 4:	mch_ratio = 200000; break;
				case 5:	mch_ratio = 240000; break;
			}
			break;
			
		default:
		case 800:
			switch ((mch_cfg >> 4) & 7)
			{
				case 1:	mch_ratio =  83333; break; // 0.833333333
				case 2:	mch_ratio = 100000; break;
				case 3:	mch_ratio = 133333; break; // 1.333333333
				case 4:	mch_ratio = 166667; break; // 1.666666667
				case 5:	mch_ratio = 200000; break;
			}
			break;
		case 1066:
			switch ((mch_cfg >> 4)&7)
			{
				case 5:	mch_ratio = 150000; break;
				case 6:	mch_ratio = 200000; break;
			}
			
	}
	
	// Compute RAM Frequency
	Platform.RAM.Frequency = (Platform.CPU.FSBFrequency * mch_ratio) / 100000;
}


// Get iCore7 Memory Speed
static void get_fsb_nhm(pci_dt_t *dram_dev)
{
	uint32_t mch_ratio, mc_dimm_clk_ratio;
	
	// Get the clock ratio
	mc_dimm_clk_ratio = pci_config_read16(PCIADDR(nhm_bus, 3, 4), 0x54 );
	mch_ratio = (mc_dimm_clk_ratio & 0x1F);
	
	// Compute RAM Frequency
	Platform.RAM.Frequency = Platform.CPU.FSBFrequency * mch_ratio / 2;
}

/*
 * Retrieve memory controller info functions
 */

// Get i965 Memory Timings
static void get_timings_i965(pci_dt_t *dram_dev)
{ 
	// Thanks for CDH optis
	uint32_t dev0, c0ckectrl, c1ckectrl, offset;
	uint32_t ODT_Control_Register, Precharge_Register, ACT_Register, Read_Register, Misc_Register;

	long *ptr;
	
	// Read MMR Base Address
	dev0 = pci_config_read32(dram_dev->dev.addr, 0x48);
	dev0 &= 0xFFFFC000;
	
	ptr = (long*)(dev0 + 0x260);
	c0ckectrl = *ptr & 0xFFFFFFFF;	
	
	ptr = (long*)(dev0 + 0x660);
	c1ckectrl = *ptr & 0xFFFFFFFF;
	
	// If DIMM 0 not populated, check DIMM 1
	((c0ckectrl) >> 20 & 0xF) ? (offset = 0) : (offset = 0x400);
	
	ptr = (long*)(dev0 + offset + 0x29C);
	ODT_Control_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x250);	
	Precharge_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x252);
	ACT_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x258);
	Read_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x244);
	Misc_Register = *ptr & 0xFFFFFFFF;
	
	// 965 Series only support DDR2
	Platform.RAM.Type = SMB_MEM_TYPE_DDR2;
	
	// CAS Latency (tCAS)
	Platform.RAM.CAS = ((ODT_Control_Register >> 17) & 7) + 3;
	
	// RAS-To-CAS (tRCD)
	Platform.RAM.TRC = (Read_Register >> 16) & 0xF;
	
	// RAS Precharge (tRP)
	Platform.RAM.TRP = (ACT_Register >> 13) & 0xF;
	
	// RAS Active to precharge (tRAS)
	Platform.RAM.RAS = (Precharge_Register >> 11) & 0x1F;
	
	if ((c0ckectrl >> 20 & 0xF) && (c1ckectrl >> 20 & 0xF))
		Platform.RAM.Channels = SMB_MEM_CHANNEL_DUAL;
	else
		Platform.RAM.Channels = SMB_MEM_CHANNEL_SINGLE;
}

// Get im965 Memory Timings
static void get_timings_im965(pci_dt_t *dram_dev)
{
	// Thanks for CDH optis
	uint32_t dev0, c0ckectrl, c1ckectrl, offset, ODT_Control_Register, Precharge_Register;
	long *ptr;
	
	// Read MMR Base Address
	dev0 = pci_config_read32(dram_dev->dev.addr, 0x48);
	dev0 &= 0xFFFFC000;
	
	ptr = (long*)(dev0 + 0x1200);
	c0ckectrl = *ptr & 0xFFFFFFFF;	
	
	ptr = (long*)(dev0 + 0x1300);
	c1ckectrl = *ptr & 0xFFFFFFFF;
	
	// If DIMM 0 not populated, check DIMM 1
	((c0ckectrl) >> 20 & 0xF) ? (offset = 0) : (offset = 0x100);
	
	ptr = (long*)(dev0 + offset + 0x121C);
	ODT_Control_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x1214);	
	Precharge_Register = *ptr & 0xFFFFFFFF;
	
	// Series only support DDR2
	Platform.RAM.Type = SMB_MEM_TYPE_DDR2;
	
	// CAS Latency (tCAS)
	Platform.RAM.CAS = ((ODT_Control_Register >> 23) & 7) + 3;
	
	// RAS-To-CAS (tRCD)
	Platform.RAM.TRC = ((Precharge_Register >> 5) & 7) + 2;
	
	// RAS Precharge (tRP)
	Platform.RAM.TRP= (Precharge_Register & 7) + 2;
	
	// RAS Active to precharge (tRAS)
	Platform.RAM.RAS = (Precharge_Register >> 21) & 0x1F;
	
	if ((c0ckectrl >> 20 & 0xF) && (c1ckectrl >> 20 & 0xF)) 
		Platform.RAM.Channels = SMB_MEM_CHANNEL_DUAL;
	else
		Platform.RAM.Channels = SMB_MEM_CHANNEL_SINGLE;
}

// Get P35 Memory Timings
static void get_timings_p35(pci_dt_t *dram_dev)
{ 
	// Thanks for CDH optis
	unsigned long dev0, Memory_Check, c0ckectrl, c1ckectrl, offset;
	unsigned long ODT_Control_Register, Precharge_Register, ACT_Register, Read_Register, Misc_Register;
	long *ptr;
	
	//Device_ID = pci_config_read16(dram_dev->dev.addr, 0x02);
	//Device_ID &= 0xFFFF;
	
	// Now, read MMR Base Address
	dev0 = pci_config_read32(dram_dev->dev.addr, 0x48);
	dev0 &= 0xFFFFC000;
	
	ptr = (long*)(dev0 + 0x260);
	c0ckectrl = *ptr & 0xFFFFFFFF;	
	
	ptr = (long*)(dev0 + 0x660);
	c1ckectrl = *ptr & 0xFFFFFFFF;
	
	// If DIMM 0 not populated, check DIMM 1
	((c0ckectrl) >> 20 & 0xF) ? (offset = 0) : (offset = 0x400);
	
	ptr = (long*)(dev0 + offset + 0x265);
	ODT_Control_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x25D);	
	Precharge_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x252);
	ACT_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x258);
	Read_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x244);
	Misc_Register = *ptr & 0xFFFFFFFF;
	
	ptr = (long*)(dev0 + offset + 0x1E8);
	Memory_Check = *ptr & 0xFFFFFFFF;	
	
	// On P45, check 1A8
	if(dram_dev->device_id > 0x2E00) {
		ptr = (long*)(dev0 + offset + 0x1A8);
		Memory_Check = *ptr & 0xFFFFFFFF;	
		Memory_Check >>= 2;
		Memory_Check &= 1;
		Memory_Check = !Memory_Check;
	} else {
		ptr = (long*)(dev0 + offset + 0x1E8);
		Memory_Check = *ptr & 0xFFFFFFFF;		
	}
	
	// Determine DDR-II or DDR-III
	if (Memory_Check & 1)
		Platform.RAM.Type = SMB_MEM_TYPE_DDR2;
	else
		Platform.RAM.Type = SMB_MEM_TYPE_DDR3;

	// CAS Latency (tCAS)
	if(dram_dev->device_id > 0x2E00)
		Platform.RAM.CAS = ((ODT_Control_Register >> 8) & 0x3F) - 6;
	else
		Platform.RAM.CAS = ((ODT_Control_Register >> 8) & 0x3F) - 9;

	// RAS-To-CAS (tRCD)
	Platform.RAM.TRC = (Read_Register >> 17) & 0xF;
	
	// RAS Precharge (tRP)
	Platform.RAM.TRP = (ACT_Register >> 13) & 0xF;
	
	// RAS Active to precharge (tRAS)
	Platform.RAM.RAS = Precharge_Register & 0x3F;
	
	// Channel configuration
	if (((c0ckectrl >> 20) & 0xF) && ((c1ckectrl >> 20) & 0xF)) 
		Platform.RAM.Channels = SMB_MEM_CHANNEL_DUAL;
	else
		Platform.RAM.Channels = SMB_MEM_CHANNEL_SINGLE;
}

// Get Nehalem Memory Timings
static void get_timings_nhm(pci_dt_t *dram_dev)
{
	unsigned long mc_channel_bank_timing, mc_control, mc_channel_mrs_value;
	int fvc_bn = 4;
	
	// Find which channels are populated
	mc_control = pci_config_read16(PCIADDR(nhm_bus, 3, 0), 0x48);
	mc_control = (mc_control >> 8) & 0x7;
	
	// DDR-III
	Platform.RAM.Type = SMB_MEM_TYPE_DDR3;
	
	// Get the first valid channel
	if(mc_control & 1)
		fvc_bn = 4; 
	else if(mc_control & 2)
		fvc_bn = 5; 
	else if(mc_control & 7) 
		fvc_bn = 6; 

	// Now, detect timings
	mc_channel_bank_timing = pci_config_read32(PCIADDR(nhm_bus, fvc_bn, 0), 0x88);
	mc_channel_mrs_value = pci_config_read32(PCIADDR(nhm_bus, fvc_bn, 0), 0x70);
	
	// CAS Latency (tCAS)
	Platform.RAM.CAS = ((mc_channel_mrs_value >> 4) & 0xF ) + 4;
	
	// RAS-To-CAS (tRCD)
	Platform.RAM.TRC = (mc_channel_bank_timing >> 9) & 0xF; 
	
	// RAS Active to precharge (tRAS)
	Platform.RAM.RAS = (mc_channel_bank_timing >> 4) & 0x1F; 
	
	// RAS Precharge (tRP)
	Platform.RAM.TRP = mc_channel_bank_timing & 0xF;
	
	// Single , Dual or Triple Channels
	if (mc_control == 1 || mc_control == 2 || mc_control == 4 )
		Platform.RAM.Channels = SMB_MEM_CHANNEL_SINGLE;
	else if (mc_control == 7)
		Platform.RAM.Channels = SMB_MEM_CHANNEL_TRIPLE;
	else
		Platform.RAM.Channels = SMB_MEM_CHANNEL_DUAL;
}

static struct mem_controller_t dram_controllers[] = {

	// Default unknown chipset
	{ 0, 0, "",	NULL, NULL, NULL },

	// Intel
	{ 0x8086, 0x7190, "VMWare",	NULL, NULL, NULL },

	{ 0x8086, 0x1A30, "i845",	NULL, NULL, NULL },
	
	{ 0x8086, 0x2970, "i946PL/GZ",		setup_p35, get_fsb_i965,	get_timings_i965	},
	{ 0x8086, 0x2990, "Q963/Q965",		setup_p35, get_fsb_i965,	get_timings_i965	},
	{ 0x8086, 0x29A0, "P965/G965",		setup_p35, get_fsb_i965,	get_timings_i965	},

	{ 0x8086, 0x2A00, "GM965/GL960",	setup_p35, get_fsb_im965,	get_timings_im965	},
	{ 0x8086, 0x2A10, "GME965/GLE960",	setup_p35, get_fsb_im965,	get_timings_im965	},
	{ 0x8086, 0x2A40, "PM/GM45/47",		setup_p35, get_fsb_im965,	get_timings_im965	},

	{ 0x8086, 0x29B0, "Q35",			setup_p35, get_fsb_i965,	get_timings_p35		},
	{ 0x8086, 0x29C0, "P35/G33",		setup_p35, get_fsb_i965,	get_timings_p35		},
	{ 0x8086, 0x29D0, "Q33",			setup_p35, get_fsb_i965,	get_timings_p35		},
	{ 0x8086, 0x29E0, "X38/X48",		setup_p35, get_fsb_i965,	get_timings_p35		},
	{ 0x8086, 0x2E00, "Eaglelake",		setup_p35, get_fsb_i965,	get_timings_p35		},
	{ 0x8086, 0x2E10, "Q45/Q43",		setup_p35, get_fsb_i965,	get_timings_p35		},
	{ 0x8086, 0x2E20, "P45/G45",		setup_p35, get_fsb_i965,	get_timings_p35		},
	{ 0x8086, 0x2E30, "G41",			setup_p35, get_fsb_i965,	get_timings_p35		},

	{ 0x8086, 0xD131, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
	{ 0x8086, 0xD132, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
	{ 0x8086, 0x3400, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
	{ 0x8086, 0x3401, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
	{ 0x8086, 0x3402, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
	{ 0x8086, 0x3403, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
	{ 0x8086, 0x3404, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
	{ 0x8086, 0x3405, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
	{ 0x8086, 0x3406, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
	{ 0x8086, 0x3407, "NHM IMC",		setup_nhm, get_fsb_nhm,		get_timings_nhm		},
};

static const char *memory_channel_types[] =
{
	"Unknown", "Single", "Dual", "Triple"
};			

void scan_dram_controller(pci_dt_t *dram_dev)
{
	int i;
	for(i = 1; i < sizeof(dram_controllers) / sizeof(dram_controllers[0]); i++)
	if ((dram_controllers[i].vendor == dram_dev->vendor_id) 
				&& (dram_controllers[i].device == dram_dev->device_id))
		{
			verbose("%s%s DRAM Controller [%4x:%4x] at %02x:%02x.%x\n", 
						(dram_dev->vendor_id == 0x8086) ? "Intel " : "" ,
						dram_controllers[i].name, dram_dev->vendor_id, dram_dev->device_id,
						dram_dev->dev.bits.bus, dram_dev->dev.bits.dev, dram_dev->dev.bits.func);
			
			if (dram_controllers[i].initialise != NULL)
				dram_controllers[i].initialise(dram_dev);

			if (dram_controllers[i].poll_timings != NULL)
				dram_controllers[i].poll_timings(dram_dev);

			if (dram_controllers[i].poll_speed != NULL)
				dram_controllers[i].poll_speed(dram_dev);

			verbose("Frequency detected: %d MHz (%d) %s Channel \n\tCAS:%d tRC:%d tRP:%d RAS:%d (%d-%d-%d-%d)\n", 
						(uint32_t)Platform.RAM.Frequency / 1000000,
						(uint32_t)Platform.RAM.Frequency / 500000,
						memory_channel_types[Platform.RAM.Channels]
					,Platform.RAM.CAS, Platform.RAM.TRC, Platform.RAM.TRP, Platform.RAM.RAS
					,Platform.RAM.CAS, Platform.RAM.TRC, Platform.RAM.TRP, Platform.RAM.RAS
					);
//			getchar();		
		}
}
