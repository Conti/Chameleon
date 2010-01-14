/*
 *
 *
 */

#include "libsaio.h"
#include "pci.h"
#include "platform.h"
#include "spd.h"

#ifndef DEBUG_SPD
#define DEBUG_SPD 0
#endif

#if DEBUG_SPD
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

static const char *spd_memory_types[] =
{
	"RAM",          /* 00h  Undefined */
	"FPM",          /* 01h  FPM */
	"EDO",          /* 02h  EDO */
	"",				/* 03h  PIPELINE NIBBLE */
	"SDRAM",        /* 04h  SDRAM */
	"",				/* 05h  MULTIPLEXED ROM */
	"DDR SGRAM",	/* 06h  SGRAM DDR */
	"DDR SDRAM",	/* 07h  SDRAM DDR */
	"DDR2 SDRAM",   /* 08h  SDRAM DDR 2 */
	"",				/* 09h  Undefined */
	"",				/* 0Ah  Undefined */
	"DDR3 SDRAM",   /* 0Bh  SDRAM DDR 3 */
};

#define rdtsc(low,high) \
__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

#define SMBHSTSTS 0
#define SMBHSTCNT 2
#define SMBHSTCMD 3
#define SMBHSTADD 4
#define SMBHSTDAT 5

unsigned char smb_read_byte_intel(uint32_t base, uint8_t adr, uint8_t cmd)
{
	int l1, h1, l2, h2;
    unsigned long long t;
	
    outb(base + SMBHSTSTS, 0x1f);					// reset SMBus Controller
    outb(base + SMBHSTDAT, 0xff);
	
    while( inb(base + SMBHSTSTS) & 0x01);			// wait until ready
	
    outb(base + SMBHSTCMD, cmd);
    outb(base + SMBHSTADD, (adr << 1) | 0x01 );
    outb(base + SMBHSTCNT, 0x48 );
	
    rdtsc(l1, h1);
	
 	while (!( inb(base + SMBHSTSTS) & 0x02))		// wait til command finished
	{	
		rdtsc(l2, h2);
		t = ((h2 - h1) * 0xffffffff + (l2 - l1)) / (Platform.CPU.TSCFrequency / 40);
		if (t > 10)
			break;									// break after 10ms
    }
    return inb(base + SMBHSTDAT);
}

static void read_smb_intel(pci_dt_t *smbus_dev)
{
	int i, x;

	uint8_t		spd_size;
	uint32_t	base;
	
	RamSlotInfo_t	*slot;
	
	base = pci_config_read16(smbus_dev->dev.addr, 0x20) & 0xFFFE;
	
	// Search MAX_RAM_SLOTS slots
	for (i = 0; i < 6; i++)
	{
		slot = &Platform.RAM.DIMM[i];

		spd_size = smb_read_byte_intel(base, 0x50 + i, 0);

		// Check spd is present
		if (spd_size != 0xff)
		{
			slot->InUse = YES;

			slot->spd = malloc(spd_size);
			if (slot->spd)
			{
				bzero(slot->spd, spd_size);
				
				// Copy spd data into buffer
				for (x = 0; x < spd_size; x++)
					slot->spd[x] = smb_read_byte_intel(base, 0x50 + i, x);
			
				switch (slot->spd[SPD_MEMORY_TYPE])
				{
					case SPD_MEMORY_TYPE_SDRAM_DDR2:

						slot->ModuleSize = ((1 << (slot->spd[SPD_NUM_ROWS] & 0x0f) + (slot->spd[SPD_NUM_COLUMNS] & 0x0f) - 17) * 
								((slot->spd[SPD_NUM_DIMM_BANKS] & 0x7) + 1) * slot->spd[SPD_NUM_BANKS_PER_SDRAM]);
						break;
			
					case SPD_MEMORY_TYPE_SDRAM_DDR3:
						
						slot->ModuleSize = ((slot->spd[4] & 0x0f) + 28 ) + ((slot->spd[8] & 0x7)  + 3 );
						slot->ModuleSize -= (slot->spd[7] & 0x7) + 25;
						slot->ModuleSize = ((1 << slot->ModuleSize) * (((slot->spd[7] >> 3) & 0x1f) + 1));
						
						break;
				}
			}

			verbose(" slot %d - %dMB %s SPD %d bytes at %x\n", i, slot->ModuleSize, 
					spd_memory_types[(uint8_t)slot->spd[SPD_MEMORY_TYPE]],
					spd_size, slot->spd);
		}
	}
		
}

static struct smbus_controllers_t smbus_controllers[] = {

	{0x8086, 0x5032, "EP80579", read_smb_intel },
	{0x8086, 0x269B, "ESB2",    read_smb_intel },
	{0x8086, 0x25A4, "6300ESB", read_smb_intel },
	{0x8086, 0x24C3, "ICH4",    read_smb_intel },
	{0x8086, 0x24D3, "ICH5",    read_smb_intel },
	{0x8086, 0x266A, "ICH6",    read_smb_intel },
	{0x8086, 0x27DA, "ICH7",    read_smb_intel },
	{0x8086, 0x283E, "ICH8",    read_smb_intel },
	{0x8086, 0x2930, "ICH9",    read_smb_intel },	
	{0x8086, 0x3A30, "ICH10R",  read_smb_intel },
	{0x8086, 0x3A60, "ICH10B",  read_smb_intel },
	{0x8086, 0x3B30, "P55",     read_smb_intel },

};

void scan_smbus_controller(pci_dt_t *smbus_dev)
{
	int	i;

	for( i = 1; i <  sizeof(smbus_controllers) / sizeof(smbus_controllers[0]); i++ )
		if (( smbus_controllers[i].vendor == smbus_dev->vendor_id) 
			&& ( smbus_controllers[i].device == smbus_dev->device_id))
		{
			verbose("%s%s SMBus Controller [%4x:%4x] at %02x:%02x.%x\n", 
				   (smbus_dev->vendor_id == 0x8086) ? "Intel(R) " : "",
				   smbus_controllers[i].name,
				   smbus_dev->vendor_id, smbus_dev->device_id,
				   smbus_dev->dev.bits.bus, smbus_dev->dev.bits.dev, smbus_dev->dev.bits.func);
			
			smbus_controllers[i].read_smb(smbus_dev);
			
		}
	
}

