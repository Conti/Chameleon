/*
 *  platform.h
 *
 */

#ifndef __LIBSAIO_PLATFORM_H
#define __LIBSAIO_PLATFORM_H

#include "libsaio.h"

#define bit(n)		(1ULL << (n))
#define bitmask(h,l)	((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)	(((x) & bitmask(h,l)) >> l)

extern void scan_platform();

/* SMBIOS Memory Types */ 
#define     SMB_MEM_TYPE_UNDEFINED	0 
#define     SMB_MEM_TYPE_OTHER		1 
#define     SMB_MEM_TYPE_UNKNOWN	2 
#define     SMB_MEM_TYPE_DRAM		3 
#define     SMB_MEM_TYPE_EDRAM		4 
#define     SMB_MEM_TYPE_VRAM		5 
#define     SMB_MEM_TYPE_SRAM		6 
#define     SMB_MEM_TYPE_RAM		7 
#define     SMB_MEM_TYPE_ROM		8 
#define     SMB_MEM_TYPE_FLASH		9 
#define     SMB_MEM_TYPE_EEPROM		10 
#define     SMB_MEM_TYPE_FEPROM		11 
#define     SMB_MEM_TYPE_EPROM		12 
#define     SMB_MEM_TYPE_CDRAM		13 
#define     SMB_MEM_TYPE_3DRAM		14 
#define     SMB_MEM_TYPE_SDRAM		15 
#define     SMB_MEM_TYPE_SGRAM		16 
#define     SMB_MEM_TYPE_RDRAM		17 
#define     SMB_MEM_TYPE_DDR		18 
#define     SMB_MEM_TYPE_DDR2		19 
#define     SMB_MEM_TYPE_FBDIMM		20 
#define     SMB_MEM_TYPE_DDR3		24			// Supported in 10.5.6+ AppleSMBIOS

/* Memory Configuration Types */ 
#define     SMB_MEM_CHANNEL_UNKNOWN	0 
#define     SMB_MEM_CHANNEL_SINGLE	1 
#define     SMB_MEM_CHANNEL_DUAL	2 
#define     SMB_MEM_CHANNEL_TRIPLE	3 

/* Maximum number of ram slots */
#define		MAX_RAM_SLOTS			16

typedef struct _RamSlotInfo_t {
	bool		InUse;							// Module Present
	uint32_t	ModuleSize;						// Size of Module in MB
	char		*spd;							// SPD Dump
} RamSlotInfo_t;

typedef struct _PlatformInfo_t {
	bool	Mobile;								// Mobile Platform
	bool	x86_64;								// 64 Bit Capable
	struct PCI {
		uint8_t			NoDevices;				// No of PCI devices
	} PCI;
	struct CPU {
		uint32_t		Vendor;					// Vendor
		uint32_t		Model;					// Model
		uint32_t		ExtModel;				// Extended Model
		uint32_t		Family;					// Family
		uint32_t		ExtFamily;				// Extended Family
		uint8_t			NoCores;				// No Cores per Package
		uint8_t			NoThreads;				// Threads per Package
		uint8_t			MaxCoef;				// Max Multiplier
		uint8_t			MaxDiv;
		uint8_t			CurrCoef;				// Current Multiplier
		uint8_t			CurrDiv;
		float			MaxRatio;				
		float			CurrRatio;				
		uint64_t		TSCFrequency;			// TSC Frequency Hz
		uint64_t		FSBFrequency;			// FSB Frequency Hz
		uint64_t		CPUFrequency;			// CPU Frequency Hz
		bool			Mobile;					// Mobile CPU
		uint32_t		BrandString[16];		// 48 Byte Branding String
	} CPU;
	struct RAM {
		uint64_t		Frequency;				// Ram Frequency
		uint32_t		Divider;				// Memory divider
		float			CAS;					// CAS 1/2/2.5/3/4/5/6/7
		uint8_t			TRC;					
		uint8_t			TRP;
		uint8_t			RAS;
		uint8_t			Channels;				// Channel Configuration Single,Dual or Triple
		uint8_t			NoSlots;				// Maximum no of slots available
		uint8_t			Type;					// Standard SMBIOS v2.5 Memory Type
		char			*BrandString;			// Branding String Memory Controller
		RamSlotInfo_t	DIMM[MAX_RAM_SLOTS];	// Information about each slot
	} RAM;
} PlatformInfo_t;

extern PlatformInfo_t Platform;

#endif /* !__LIBSAIO_PLATFORM_H */

