/*
 *  platform.h
 *  AsereBLN: reworked and extended
 *
 */

#ifndef __LIBSAIO_PLATFORM_H
#define __LIBSAIO_PLATFORM_H

//#include "libsaio.h"

extern bool platformCPUFeature(uint32_t);
extern void scan_platform(void);
extern void dumpPhysAddr(const char * title, void * a, int len);

/* CPUID Vendor */
#define CPUID_VENDOR_INTEL      0x756E6547
#define CPUID_VENDOR_AMD        0x68747541

/* CPUID index into cpuid_raw */
#define CPUID_0				0
#define CPUID_1				1
#define CPUID_2				2
#define CPUID_3				3
#define CPUID_4				4
#define CPUID_5				5
#define CPUID_6				6
#define CPUID_80			7
#define CPUID_81			8
#define CPUID_88			9
#define CPUID_MAX			10

#define CPU_MODEL_PRESCOTT		0x03			// Celeron D, Pentium 4 (90nm)
#define CPU_MODEL_NOCONA		0x04			// Xeon Nocona, Irwindale (90nm)
#define CPU_MODEL_PRESLER		0x06			// Pentium 4, Pentium D (65nm)
#define CPU_MODEL_PENTIUM_M		0x09			// Banias
#define CPU_MODEL_DOTHAN		0x0D			// Dothan
#define CPU_MODEL_YONAH			0x0E			// Sossaman, Yonah
#define CPU_MODEL_MEROM			0x0F			// Allendale, Conroe, Kentsfield, Woodcrest, Clovertown, Tigerton, Merom
#define CPU_MODEL_CONROE		0x0F			// 
#define CPU_MODEL_CELERON		0x16			// 
#define CPU_MODEL_PENRYN		0x17			// Wolfdale, Yorkfield, Harpertown, Penryn
#define CPU_MODEL_WOLFDALE		0x17			// 
#define CPU_MODEL_NEHALEM		0x1A			// Bloomfield. Nehalem-EP, Nehalem-WS, Gainestown
#define CPU_MODEL_ATOM			0x1C			// Pineview
#define CPU_MODEL_XEON_MP		0x1D			// MP 7400
#define CPU_MODEL_FIELDS		0x1E			// Lynnfield, Clarksfield, Jasper Forest
#define CPU_MODEL_DALES			0x1F			// Havendale, Auburndale
#define CPU_MODEL_DALES_32NM		0x25			// Clarkdale, Arrandale
#define CPU_MODEL_ATOM_SAN		0x26			// Lincroft
#define CPU_MODEL_LINCROFT		0x27			// 
#define CPU_MODEL_SANDYBRIDGE		0x2A			// Sandy Bridge
#define CPU_MODEL_WESTMERE		0x2C			// Gulftown, Westmere-EP, Westmere-WS
#define CPU_MODEL_JAKETOWN		0x2D			// Sandy Bridge-E, Sandy Bridge-EP
#define CPU_MODEL_NEHALEM_EX		0x2E			// Beckton
#define CPU_MODEL_WESTMERE_EX		0x2F			// Westmere-EX
#define CPU_MODEL_ATOM_2000		0x36			// Cedarview
#define CPU_MODEL_IVYBRIDGE		0x3A			// Ivy Bridge
#define CPU_MODEL_HASWELL		0x3C			// Haswell DT
#define CPU_MODEL_IVYBRIDGE_XEON	0x3E			// Ivy Bridge Xeon
#define CPU_MODEL_HASWELL_SVR		0x3F			// Haswell MB
//#define CPU_MODEL_HASWELL_H		0x??			// Haswell H
#define CPU_MODEL_HASWELL_ULT		0x45			// Haswell ULT
#define CPU_MODEL_CRYSTALWELL		0x46			// Haswell ULX

/* CPU Features */
#define CPU_FEATURE_MMX			0x00000001		// MMX Instruction Set
#define CPU_FEATURE_SSE			0x00000002		// SSE Instruction Set
#define CPU_FEATURE_SSE2		0x00000004		// SSE2 Instruction Set
#define CPU_FEATURE_SSE3		0x00000008		// SSE3 Instruction Set
#define CPU_FEATURE_SSE41		0x00000010		// SSE41 Instruction Set
#define CPU_FEATURE_SSE42		0x00000020		// SSE42 Instruction Set
#define CPU_FEATURE_EM64T		0x00000040		// 64Bit Support
#define CPU_FEATURE_HTT			0x00000080		// HyperThreading
#define CPU_FEATURE_MOBILE		0x00000100		// Mobile CPU
#define CPU_FEATURE_MSR			0x00000200		// MSR Support

/* SMBIOS Memory Types */ 
#define SMB_MEM_TYPE_UNDEFINED	0
#define SMB_MEM_TYPE_OTHER		1
#define SMB_MEM_TYPE_UNKNOWN	2
#define SMB_MEM_TYPE_DRAM		3
#define SMB_MEM_TYPE_EDRAM		4
#define SMB_MEM_TYPE_VRAM		5
#define SMB_MEM_TYPE_SRAM		6
#define SMB_MEM_TYPE_RAM		7
#define SMB_MEM_TYPE_ROM		8
#define SMB_MEM_TYPE_FLASH		9
#define SMB_MEM_TYPE_EEPROM		10
#define SMB_MEM_TYPE_FEPROM		11
#define SMB_MEM_TYPE_EPROM		12
#define SMB_MEM_TYPE_CDRAM		13
#define SMB_MEM_TYPE_3DRAM		14
#define SMB_MEM_TYPE_SDRAM		15
#define SMB_MEM_TYPE_SGRAM		16
#define SMB_MEM_TYPE_RDRAM		17
#define SMB_MEM_TYPE_DDR		18
#define SMB_MEM_TYPE_DDR2		19
#define SMB_MEM_TYPE_FBDIMM		20
#define SMB_MEM_TYPE_DDR3		24			// Supported in 10.5.6+ AppleSMBIOS

/* Memory Configuration Types */ 
#define SMB_MEM_CHANNEL_UNKNOWN		0
#define SMB_MEM_CHANNEL_SINGLE		1
#define SMB_MEM_CHANNEL_DUAL		2
#define SMB_MEM_CHANNEL_TRIPLE		3

/* Maximum number of ram slots */
#define MAX_RAM_SLOTS			8
#define RAM_SLOT_ENUMERATOR		{0, 2, 4, 1, 3, 5, 6, 8, 10, 7, 9, 11}

/* Maximum number of SPD bytes */
#define MAX_SPD_SIZE			256

/* Size of SMBIOS UUID in bytes */
#define UUID_LEN			16

typedef struct _RamSlotInfo_t
{
	uint32_t		ModuleSize;					// Size of Module in MB
	uint32_t		Frequency;					// in Mhz
	const char*		Vendor;
	const char*		PartNo;
	const char*		SerialNo;
	char*			spd;						// SPD Dump
	bool			InUse;
	uint8_t			Type;
	uint8_t			BankConnections;			// table type 6, see (3.3.7)
	uint8_t			BankConnCnt;
} RamSlotInfo_t;

//==============================================================================

typedef struct _PlatformInfo_t {
	struct CPU {
		uint32_t		Features;				// CPU Features like MMX, SSE2, VT, MobileCPU
		uint32_t		Vendor;					// Vendor
		uint32_t		Signature;				// Processor Signature
		uint32_t		Stepping;				// Stepping
		//uint16_t		Type;					// Type
		uint32_t		Model;					// Model
		uint32_t		ExtModel;				// Extended Model
		uint32_t		Family;					// Family
		uint32_t		ExtFamily;				// Extended Family
		uint32_t		NoCores;				// No Cores per Package
		uint32_t		NoThreads;				// Threads per Package
		uint8_t			MaxCoef;				// Max Multiplier
		uint8_t			MaxDiv;					// Min Multiplier
		uint8_t			CurrCoef;				// Current Multiplier
		uint8_t			CurrDiv;
		uint64_t		TSCFrequency;				// TSC Frequency Hz
		uint64_t		FSBFrequency;				// FSB Frequency Hz
		uint64_t		CPUFrequency;				// CPU Frequency Hz
		uint32_t		MaxRatio;				// Max Bus Ratio
		uint32_t		MinRatio;				// Min Bus Ratio
		char			BrandString[48];			// 48 Byte Branding String
		uint32_t		CPUID[CPUID_MAX][4];			// CPUID 0..4, 80..81 Raw Values
	} CPU;

	struct RAM {
		uint64_t		Frequency;				// Ram Frequency
		uint32_t		Divider;				// Memory divider
		uint8_t			CAS;					// CAS 1/2/2.5/3/4/5/6/7
		uint8_t			TRC;					
		uint8_t			TRP;
		uint8_t			RAS;
		uint8_t			Channels;				// Channel Configuration Single,Dual or Triple
		uint8_t			NoSlots;				// Maximum no of slots available
		uint8_t			Type;					// Standard SMBIOS v2.5 Memory Type
		RamSlotInfo_t	DIMM[MAX_RAM_SLOTS];	// Information about each slot
	} RAM;

	struct DMI {
		int			MaxMemorySlots;		// number of memory slots populated by SMBIOS
		int			CntMemorySlots;		// number of memory slots counted
		int			MemoryModules;		// number of memory modules installed
		int			DIMM[MAX_RAM_SLOTS];	// Information and SPD mapping for each slot
	} DMI;

	uint8_t				Type;			// System Type: 1=Desktop, 2=Portable... according ACPI2.0 (FACP: PM_Profile)
	uint8_t				*UUID;
} PlatformInfo_t;

extern PlatformInfo_t Platform;

#endif /* !__LIBSAIO_PLATFORM_H */
