/*
 * spd.c - serial presence detect memory information
 *
 * Originally restored from pcefi10.5 by netkas
 * Dynamic mem detection original impl. by Rekursor
 * System profiler fix and other fixes by Mozodojo.
 */

#include "libsaio.h"
#include "pci.h"
#include "platform.h"
#include "spd.h"
#include "cpu.h"
#include "saio_internal.h"
#include "bootstruct.h"
#include "memvendors.h"

#ifndef DEBUG_SPD
#define DEBUG_SPD 0
#endif

#if DEBUG_SPD
#define DBG(x...)	printf(x)
#else
#define DBG(x...)	msglog(x)
#endif

static const char *spd_memory_types[] =
{
	"RAM",          /* 00h  Undefined */
	"FPM",          /* 01h  FPM */
	"EDO",          /* 02h  EDO */
	"",		/* 03h  PIPELINE NIBBLE */
	"SDRAM",        /* 04h  SDRAM */
	"",		/* 05h  MULTIPLEXED ROM */
	"DDR SGRAM",	/* 06h  SGRAM DDR */
	"DDR SDRAM",	/* 07h  SDRAM DDR */
	"DDR2 SDRAM",   /* 08h  SDRAM DDR 2 */
	"",		/* 09h  Undefined */
	"",		/* 0Ah  Undefined */
	"DDR3 SDRAM"	/* 0Bh  SDRAM DDR 3 */
};

#define UNKNOWN_MEM_TYPE 2
static uint8_t spd_mem_to_smbios[] =
{
	UNKNOWN_MEM_TYPE,		/* 00h  Undefined */
	UNKNOWN_MEM_TYPE,		/* 01h  FPM */
	UNKNOWN_MEM_TYPE,		/* 02h  EDO */
	UNKNOWN_MEM_TYPE,		/* 03h  PIPELINE NIBBLE */
	SMB_MEM_TYPE_SDRAM,		/* 04h  SDRAM */
	SMB_MEM_TYPE_ROM,		/* 05h  MULTIPLEXED ROM */
	SMB_MEM_TYPE_SGRAM,		/* 06h  SGRAM DDR */
	SMB_MEM_TYPE_DDR,		/* 07h  SDRAM DDR */
	SMB_MEM_TYPE_DDR2,		/* 08h  SDRAM DDR 2 */
	UNKNOWN_MEM_TYPE,		/* 09h  Undefined */
	UNKNOWN_MEM_TYPE,		/* 0Ah  Undefined */
	SMB_MEM_TYPE_DDR3		/* 0Bh  SDRAM DDR 3 */
};
#define SPD_TO_SMBIOS_SIZE (sizeof(spd_mem_to_smbios)/sizeof(uint8_t))

#define rdtsc(low,high) \
__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

#define SMBHSTSTS 0
#define SMBHSTCNT 2
#define SMBHSTCMD 3
#define SMBHSTADD 4
#define SMBHSTDAT 5
#define SBMBLKDAT 7

/** Read one byte from the intel i2c, used for reading SPD on intel chipsets only. */

unsigned char smb_read_byte_intel(uint32_t base, uint8_t adr, uint8_t cmd)
{
	int l1, h1, l2, h2;
	unsigned long long t;

	outb(base + SMBHSTSTS, 0x1f);	// reset SMBus Controller
	outb(base + SMBHSTDAT, 0xff);

	rdtsc(l1, h1);
	while ( inb(base + SMBHSTSTS) & 0x01)    // wait until read
	{
		rdtsc(l2, h2);
		t = ((h2 - h1) * 0xffffffff + (l2 - l1)) / (Platform.CPU.TSCFrequency / 100);
		if (t > 5)
		{
			return 0xFF;	// break
		}
	}

	outb(base + SMBHSTCMD, cmd);
	outb(base + SMBHSTADD, (adr << 1) | 0x01 );
	outb(base + SMBHSTCNT, 0x48 );

	rdtsc(l1, h1);

	while (!( inb(base + SMBHSTSTS) & 0x02))	// wait til command finished
	{
		rdtsc(l2, h2);
		t = ((h2 - h1) * 0xffffffff + (l2 - l1)) / (Platform.CPU.TSCFrequency / 100);
		if (t > 5)
		{
			break;	// break after 5ms
		}
	}
	return inb(base + SMBHSTDAT);
}

/* SPD i2c read optimization: prefetch only what we need, read non prefetcheable bytes on the fly */
#define READ_SPD(spd, base, slot, x) spd[x] = smb_read_byte_intel(base, 0x50 + slot, x)

int spd_indexes[] = {
	SPD_MEMORY_TYPE,
	SPD_DDR3_MEMORY_BANK,
	SPD_DDR3_MEMORY_CODE,
	SPD_NUM_ROWS,
	SPD_NUM_COLUMNS,
	SPD_NUM_DIMM_BANKS,
	SPD_NUM_BANKS_PER_SDRAM,
	4,7,8,9,12,64, /* TODO: give names to these values */
	95,96,97,98, 122,123,124,125 /* UIS */
};
#define SPD_INDEXES_SIZE (sizeof(spd_indexes) / sizeof(int))

/** Read from spd *used* values only*/
static void init_spd(char * spd, uint32_t base, int slot)
{
	int i;
	for (i=0; i< SPD_INDEXES_SIZE; i++) {
		READ_SPD(spd, base, slot, spd_indexes[i]);
	}
}

/** Get Vendor Name from spd, 2 cases handled DDR3 and DDR2, 
    have different formats, always return a valid ptr.*/
const char * getVendorName(RamSlotInfo_t* slot, uint32_t base, int slot_num)
{
	uint8_t bank = 0;
	uint8_t code = 0;
	int i = 0;
	uint8_t * spd = (uint8_t *) slot->spd;

	if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR3)
	{ // DDR3
		bank = (spd[SPD_DDR3_MEMORY_BANK] & 0x07f); // constructors like Patriot use b7=1
		code = spd[SPD_DDR3_MEMORY_CODE];
		for (i=0; i < VEN_MAP_SIZE; i++) {
			if (bank==vendorMap[i].bank && code==vendorMap[i].code) {
				return vendorMap[i].name;
			}
		}
	} else if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR2 || spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR) {
		if(spd[64]==0x7f) {
			for (i=64; i<72 && spd[i]==0x7f;i++) {
				bank++;
				READ_SPD(spd, base, slot_num, (uint8_t)(i+1)); // prefetch next spd byte to read for next loop
			}
			READ_SPD(spd, base, slot_num,(uint8_t)i);
			code = spd[i];
		} else {
			code = spd[64]; 
			bank = 0;
		}
		for (i=0; i < VEN_MAP_SIZE; i++) {
			if (bank==vendorMap[i].bank && code==vendorMap[i].code) {
				return vendorMap[i].name;
			}
		}
	}
	/* OK there is no vendor id here lets try to match the partnum if it exists */
	if (strstr(slot->PartNo,"GU332") == slot->PartNo) { // Unifosa fingerprint
		return "Unifosa";
	}
	return "NoName";
}

/* Get Default Memory Module Speed (no overclocking handled) */
int getDDRspeedMhz(const char * spd)
{
	if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR3) {
		switch(spd[12]) {
			case 0x0f:
				return 1066;
			case 0x0c:
				return 1333;
			case 0x0a:
				return 1600;
			case 0x14:
			default:
				return 800;
		}
	} else if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR2 || spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR) {
		switch(spd[9]) {
			case 0x50:
				return 400;
			case 0x3d:
				return 533;
			case 0x30:
				return 667;
			case 0x25:
			default:
				return 800;
			case 0x1E:
				return 1066;
		}
	}
	return  800; // default freq for unknown types
}

#define SMST(a) ((uint8_t)((spd[a] & 0xf0) >> 4))
#define SLST(a) ((uint8_t)(spd[a] & 0x0f))

/* Get DDR3 or DDR2 serial number, 0 most of the times, always return a valid ptr */
const char *getDDRSerial(const char* spd)
{
	static char asciiSerial[16];

	if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR3) // DDR3
	{
		snprintf(asciiSerial, sizeof(asciiSerial), "%X%X%X%X%X%X%X%X", SMST(122) /*& 0x7*/, SLST(122), SMST(123), SLST(123), SMST(124), SLST(124), SMST(125), SLST(125));
	}
	else if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR2 || spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR) // DDR2 or DDR
	{
		snprintf(asciiSerial, sizeof(asciiSerial), "%X%X%X%X%X%X%X%X", SMST(95) /*& 0x7*/, SLST(95), SMST(96), SLST(96), SMST(97), SLST(97), SMST(98), SLST(98));
	} else {
		sprintf(asciiSerial, "000000000000000");
	}

	return strdup(asciiSerial);
}

/* Get DDR3 or DDR2 Part Number, always return a valid ptr */
const char * getDDRPartNum(char* spd, uint32_t base, int slot)
{
	static char asciiPartNo[32];
	int i, start=0, index = 0;

	if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR3) {
		start = 128;
	} else if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR2 || spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR) {
		start = 73;
	}
	
	// Check that the spd part name is zero terminated and that it is ascii:
	bzero(asciiPartNo, sizeof(asciiPartNo));
	char c;
	for (i=start; i < start + sizeof(asciiPartNo); i++) {
		READ_SPD(spd, base, slot, i); // only read once the corresponding model part (ddr3 or ddr2)
		c = spd[i];
		if (isalpha(c) || isdigit(c) || ispunct(c)) {
			// It seems that System Profiler likes only letters and digits...
			asciiPartNo[index++] = c;
		} else if (!isascii(c)) {
			break;
		}
	}

	return strdup(asciiPartNo);
}

int mapping []= {0,2,1,3,4,6,5,7,8,10,9,11};


/* Read from smbus the SPD content and interpret it for detecting memory attributes */
static void read_smb_intel(pci_dt_t *smbus_dev) {
	int        i, speed;
	uint8_t    spd_size, spd_type;
	uint32_t   base, mmio, hostc;
//	bool       dump = false;
	RamSlotInfo_t*  slot;

	uint16_t cmd = pci_config_read16(smbus_dev->dev.addr, 0x04);
	DBG("SMBus CmdReg: 0x%x\n", cmd);
	pci_config_write16(smbus_dev->dev.addr, 0x04, cmd | 1);

	mmio = pci_config_read32(smbus_dev->dev.addr, 0x10);// & ~0x0f;
	base = pci_config_read16(smbus_dev->dev.addr, 0x20) & 0xFFFE;
	hostc = pci_config_read8(smbus_dev->dev.addr, 0x40);
	verbose("Scanning SMBus [%04x:%04x], mmio: 0x%x, ioport: 0x%x, hostc: 0x%x\n", 
	smbus_dev->vendor_id, smbus_dev->device_id, mmio, base, hostc);

	//Azi: no use for this!
	//  getBoolForKey("DumpSPD", &dump, &bootInfo->chameleonConfig);
	// needed at least for laptops
	bool fullBanks = Platform.DMI.MemoryModules == Platform.DMI.CntMemorySlots;

	char spdbuf[MAX_SPD_SIZE];
	// Search MAX_RAM_SLOTS slots
	for (i = 0; i <  MAX_RAM_SLOTS; i++) {
		slot = &Platform.RAM.DIMM[i];
		spd_size = smb_read_byte_intel(base, 0x50 + i, 0);
		DBG("SPD[0] (size): %d @0x%x\n", spd_size, 0x50 + i);
		// Check spd is present
		if (spd_size && (spd_size != 0xff)) {

			slot->spd = spdbuf;
			slot->InUse = true;

			bzero(slot->spd, spd_size);

			// Copy spd data into buffer

			//for (x = 0; x < spd_size; x++) slot->spd[x] = smb_read_byte_intel(base, 0x50 + i, x);
			init_spd(slot->spd, base, i);

			switch (slot->spd[SPD_MEMORY_TYPE]) {
				case SPD_MEMORY_TYPE_SDRAM_DDR:

				slot->ModuleSize = (((1 << ((slot->spd[SPD_NUM_ROWS] & 0x0f)
                                      + (slot->spd[SPD_NUM_COLUMNS] & 0x0f) - 17)) *
                                     ((slot->spd[SPD_NUM_DIMM_BANKS] & 0x7) + 1) * 
                                      slot->spd[SPD_NUM_BANKS_PER_SDRAM])/3)*2;
				break;

				case SPD_MEMORY_TYPE_SDRAM_DDR2:

					slot->ModuleSize = ((1 << ((slot->spd[SPD_NUM_ROWS] & 0x0f) + (slot->spd[SPD_NUM_COLUMNS] & 0x0f) - 17)) *
							((slot->spd[SPD_NUM_DIMM_BANKS] & 0x7) + 1) * slot->spd[SPD_NUM_BANKS_PER_SDRAM]);
				break;

				case SPD_MEMORY_TYPE_SDRAM_DDR3:

					slot->ModuleSize = ((slot->spd[4] & 0x0f) + 28 ) + ((slot->spd[8] & 0x7)  + 3 );
					slot->ModuleSize -= (slot->spd[7] & 0x7) + 25;
					slot->ModuleSize = ((1 << slot->ModuleSize) * (((slot->spd[7] >> 3) & 0x1f) + 1));

				break;
			}

			spd_type = (slot->spd[SPD_MEMORY_TYPE] < ((char) 12) ? slot->spd[SPD_MEMORY_TYPE] : 0);
			slot->Type = spd_mem_to_smbios[spd_type];
			if (slot->Type == UNKNOWN_MEM_TYPE) {
				continue;
			}
			slot->PartNo = getDDRPartNum(slot->spd, base, i);
			slot->Vendor = getVendorName(slot, base, i);
			slot->SerialNo = getDDRSerial(slot->spd);

			// determine spd speed
			speed = getDDRspeedMhz(slot->spd);
			if (slot->Frequency < speed) {
				slot->Frequency = speed;
			}

			// pci memory controller if available, is more reliable
			if (Platform.RAM.Frequency > 0) {
				uint32_t freq = (uint32_t)Platform.RAM.Frequency / 500000;
				// now round off special cases
				uint32_t fmod100 = freq %100;
				switch(fmod100) {
					case  1:	freq--;	break;
					case 32:	freq++;	break;
					case 65:	freq++; break;
					case 98:	freq+=2;break;
					case 99:	freq++; break;
				}
				slot->Frequency = freq;
			}

			verbose("Slot: %d Type %d %dMB (%s) %dMHz Vendor=%s\n      PartNo=%s SerialNo=%s\n", 
                       i,
                       (int)slot->Type,
                       slot->ModuleSize, 
                       spd_memory_types[spd_type],
                       slot->Frequency,
                       slot->Vendor,
                       slot->PartNo,
                       slot->SerialNo);
                       slot->InUse = true;
		}

		// laptops sometimes show slot 0 and 2 with slot 1 empty when only 2 slots are presents so:
		Platform.DMI.DIMM[i]= 
        	(uint32_t)((i>0 && Platform.RAM.DIMM[1].InUse==false && fullBanks && Platform.DMI.CntMemorySlots == 2) ?
        	mapping[i] : i); // for laptops case, mapping setup would need to be more generic than this
		slot->spd = NULL;

	} // for
}

static struct smbus_controllers_t smbus_controllers[] = {

	{0x8086, 0x1C22, "6 Series",                                read_smb_intel },
	{0x8086, 0x1D22, "C600/X79 Series",                         read_smb_intel },
	{0x8086, 0x1D70, "C600/X79 Series",                         read_smb_intel },
	{0x8086, 0x1D71, "C608/C606/X79 Series",                    read_smb_intel },
	{0x8086, 0x1D72, "C608",                                    read_smb_intel },
	{0x8086, 0x1E22, "7 Series/C210 Series",                    read_smb_intel },
	{0x8086, 0x2330, "DH89xxCC",                                read_smb_intel },
	{0x8086, 0x2413, "82801AA",                                 read_smb_intel },
	{0x8086, 0x2423, "82801BA/BAM",                             read_smb_intel },
	{0x8086, 0x2443, "82801BA/BAM",                             read_smb_intel },
	{0x8086, 0x2483, "82801CA/CAM",                             read_smb_intel },
	{0x8086, 0x24C3, "82801DB/DBL/DBM (ICH4/ICH4-L/ICH4-M)",    read_smb_intel },
	{0x8086, 0x24D3, "82801EB/ER (ICH5/ICH5R)",                 read_smb_intel },
	{0x8086, 0x25A4, "6300ESB",                                 read_smb_intel },
	{0x8086, 0x266A, "82801FB/FBM/FR/FW/FRW (ICH6 Family)",     read_smb_intel },
	{0x8086, 0x269B, "631xESB/632xESB/3100",                    read_smb_intel },
	{0x8086, 0x27DA, "N10/ICH 7 Family",                        read_smb_intel },
	{0x8086, 0x283E, "82801H (ICH8 Family) ",                   read_smb_intel },
	{0x8086, 0x2930, "82801I (ICH9 Family)",                    read_smb_intel },
	{0x8086, 0x3A30, "82801JI (ICH10 Family)",                  read_smb_intel },
	{0x8086, 0x3A60, "82801JD/DO (ICH10 Family)",               read_smb_intel },
	{0x8086, 0x3B30, "5 Series/3400 Series",                    read_smb_intel },
	{0x8086, 0x5032, "EP80579",                                 read_smb_intel },
	{0x8086, 0x8C22, "8 Series/C220",                           read_smb_intel },
	{0x8086, 0x9C22, "Lynx Point-LP",                           read_smb_intel }

};

// initial call : pci_dt = root_pci_dev;
// find_and_read_smbus_controller(root_pci_dev);
bool find_and_read_smbus_controller(pci_dt_t* pci_dt)
{
	pci_dt_t	*current = pci_dt;
	int i;

	while (current) {
#if 0
		printf("%02x:%02x.%x [%04x] [%04x:%04x] :: %s\n", 
		current->dev.bits.bus, current->dev.bits.dev, current->dev.bits.func, 
		current->class_id, current->vendor_id, current->device_id, 
		get_pci_dev_path(current));
#endif
		for ( i = 0; i <  sizeof(smbus_controllers) / sizeof(smbus_controllers[0]); i++ ) {
			if (current->vendor_id == smbus_controllers[i].vendor && current->device_id == smbus_controllers[i].device) {
				smbus_controllers[i].read_smb(current); // read smb
				return true;
			}
		}
		find_and_read_smbus_controller(current->children);
		current = current->next;
	}
	return false; // not found
}

void scan_spd(PlatformInfo_t *p)
{
	find_and_read_smbus_controller(root_pci_dev);
}

