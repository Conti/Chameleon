/*
 * A very simple SMBIOS Table decoder, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "libsaio.h"
#include "smbios.h"
// Bungo:
#include "boot.h"
#include "bootstruct.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)	msglog(x)
#endif

extern char *getSMBStringForField(SMBStructHeader *structHeader, uint8_t field);
// Bungo:
#define         NotSpecifiedStr     "Not Specified"  // no string
#define         OutOfSpecStr        "<OUT OF SPEC>"  // value out of smbios spec. range
#define         PrivateStr          "** PRIVATE **"  // masking private data
#define         neverMask           false

static bool     privateData         = true;
static SMBByte      minorVersion;   // SMBIOS rev. minor
static SMBByte      majorVersion;   // SMBIOS rev. major
static SMBByte      bcdRevisionLo;  // DMI rev. minor
static SMBByte      bcdRevisionHi;  // DMI rev. major

/*====
 7.2.2
 ===*/
static const char *SMBWakeUpTypes[] =  // Bungo: strings for wake-up type (Table Type 1 - System Information)
{
	"Reserved",          /* 00h */
	"Other",             /* 01h */
	"Unknown",           /* 02h */
	"APM Timer",         /* 03h */
	"Modem Ring",        /* 04h */
	"LAN Remote",        /* 05h */
	"Power Switch",      /* 06h */
	"PCI PME#",          /* 07h */
	"AC Power Restored"  /* 08h */
};

/*====
 7.3.2
 ===*/
static const char *SMBBaseBoardTypes[] =  // Bungo: strings for base board type (Table Type 2 - Base Board Information)
{
	"Unknown",                  /* 01h */
	"Other",                    /* 02h */
	"Server Blade",             /* 03h */
	"Connectivity Switch",      /* 04h */
	"System Management Module", /* 05h */
	"Processor Module",         /* 06h */
	"I/O Module",               /* 07h */
	"Memory Module",            /* 08h */
	"Daughter Board",           /* 09h */
	"Motherboard",              /* 0Ah */
	"Processor+Memory Module",  /* 0Bh */
	"Processor+I/O Module",     /* 0Ch */
	"Interconnect Board"        /* 0Dh */
};

 /*===
 7.4.1
 ===*/
static const char *SMBChassisTypes[] =  // Bungo: strings for chassis type (Table Type 3 - Chassis Information)
{
	"Other",                /* 01h */
	"Unknown",              /* 02h */
	"Desktop",              /* 03h */
	"Low Profile Desktop",  /* 04h */
	"Pizza Box",            /* 05h */
	"Mini Tower",           /* 06h */
	"Tower",                /* 07h */
	"Portable",             /* 08h */
	"Laptop",               /* 09h */
	"Notebook",             /* 0Ah */
	"Hand Held",            /* 0Bh */
	"Docking Station",      /* 0Ch */
	"All in One",           /* 0Dh */
	"Sub Notebook",         /* 0Eh */
	"Space-saving",         /* 0Fh */
	"Lunch Box",		/* 10h */
	"Main Server Chassis",	/* 11h */
	"Expansion Chassis",	/* 12h */
	"SubChassis",		/* 13h */
	"Bus Expansion Chassis",/* 14h */
	"Peripheral Chassis",	/* 15h */
	"RAID Chassis",		/* 16h */
	"Rack Mount Chassis",   /* 17h */
	"Sealed-case PC",	/* 18h */
	"Multi-system Chassis", /* 19h */
	"Compact PCI",		/* 1Ah */
	"Advanced TCA",		/* 1Bh */
	"Blade",		/* 1Ch */ // An SMBIOS implementation for a Blade would contain a Type 3 Chassis structure
	"Blade Enclosing"	/* 1Dh */ // A Blade Enclosure is a specialized chassis that contains a set of Blades.
};

/*====
 7.5.1
 ===*/
static const char *SMBProcessorTypes[] =  // Bungo: strings for processor type (Table Type 4 - Processor Information)
{
	"Other",                /* 01h */
	"Unknown",              /* 02h */
	"Central Processor",    /* 03h */
	"Math Processor",       /* 04h */
	"DSP Processor",        /* 05h */
	"Video Processor"       /* 06h */
};

/*====
 7.5.5
 ===*/
static const char *SMBProcessorUpgrades[] =  // ErmaC: strings for processor upgrade (Table Type 4 - Processor Information)
{
	"Other",                /* 01h */
	"Unknown",              /* 02h */
	"Daughter Board",
	"ZIF Socket",
	"Replaceable Piggy Back",
	"None",
	"LIF Socket",
	"Slot 1",
	"Slot 2",
	"370-pin Socket",
	"Slot A",
	"Slot M",
	"Socket 423",
	"Socket A (Socket 462)",
	"Socket 478",
	"Socket 754",
	"Socket 940",
	"Socket 939",
	"Socket mPGA604",
	"Socket LGA771",
	"Socket LGA775",
	"Socket S1",
	"Socket AM2",
	"Socket F (1207)",
	"Socket LGA1366",
	"Socket G34",
	"Socket AM3",
	"Socket C32",
	"Socket LGA1156",
	"Socket LGA1567",
	"Socket PGA988A",
	"Socket BGA1288",
	"Socket rPGA988B",
	"Socket BGA1023",
	"Socket BGA1224",
	"Socket BGA1155",
	"Socket LGA1356",
	"Socket LGA2011",
	"Socket FS1",
	"Socket FS2",
	"Socket FM1",
	"Socket FM2",
	"Socket LGA2011-3",
	"Socket LGA1356-3"              /* 2Ch */
};

/*=====
 7.18.2
 ====*/
static const char *
SMBMemoryDeviceTypes[] =
{
	"RAM",          /* 00h  Undefined */
	"RAM",          /* 01h  Other */
	"RAM",          /* 02h  Unknown */
	"DRAM",         /* 03h  DRAM */
	"EDRAM",        /* 04h  EDRAM */
	"VRAM",         /* 05h  VRAM */
	"SRAM",         /* 06h  SRAM */
	"RAM",          /* 07h  RAM */
	"ROM",          /* 08h  ROM */
	"FLASH",        /* 09h  FLASH */
	"EEPROM",       /* 0Ah  EEPROM */
	"FEPROM",       /* 0Bh  FEPROM */
	"EPROM",        /* 0Ch  EPROM */
	"CDRAM",        /* 0Dh  CDRAM */
	"3DRAM",        /* 0Eh  3DRAM */
	"SDRAM",        /* 0Fh  SDRAM */
	"SGRAM",        /* 10h  SGRAM */
	"RDRAM",        /* 11h  RDRAM */
	"DDR SDRAM",    /* 12h  DDR */
	"DDR2 SDRAM",   /* 13h  DDR2 */
	"DDR2 FB-DIMM", /* 14h  DDR2 FB-DIMM */
	"RAM",		/* 15h  unused */
	"RAM",		/* 16h  unused */
	"RAM",		/* 17h  unused */
	"DDR3",		/* 18h  DDR3, chosen in [5776134] */
	"FBD2"		/* 19h  FBD2 */
};

static const int kSMBMemoryDeviceTypeCount = sizeof(SMBMemoryDeviceTypes)   /
                            sizeof(SMBMemoryDeviceTypes[0]);

// Bungo: fixes random string readout if null in smbios to "Not Specified" as dmidecode displays
char *SMBStringForField(SMBStructHeader *structHeader, uint8_t field, const bool mask)
{
	char  *str = NULL;
	str = getSMBStringForField(structHeader, field);
	if (!field) {
		str = NotSpecifiedStr;
	}
	else if (mask) {
		str = PrivateStr;
	}

	return str;
};

void printHeader(SMBStructHeader *structHeader)
{
	DBG("Handle: 0x%04x, DMI type %d, %d bytes\n", structHeader->handle, structHeader->type, structHeader->length);
}

//-------------------------------------------------------------------------------------------------------------------------
// BIOS Information (Type 0)
//-------------------------------------------------------------------------------------------------------------------------
void decodeBIOSInformation(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	DBG("BIOS Information\n");
	DBG("\tVendor: %s\n", SMBStringForField(structHeader, ((SMBBIOSInformation *)structHeader)->vendor, neverMask));
	DBG("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBBIOSInformation *)structHeader)->version, neverMask));
	DBG("\tRelease Date: %s\n", SMBStringForField(structHeader, ((SMBBIOSInformation *)structHeader)->releaseDate, neverMask));
// Address:
// Runtime Size:
// ROM Size:
// DBG("\tSupported BIOS functions: (0x%llX) %s\n", ((SMBBIOSInformation *)structHeader)->characteristics, SMBBIOSInfoChar0[((SMBBIOSInformation *)structHeader)->characteristics]);
	DBG("\tBIOS Revision: %d.%d\n", ((SMBBIOSInformation *)structHeader)->releaseMajor, ((SMBBIOSInformation *)structHeader)->releaseMinor);
// Firmware Major Release
// Firmware Minor Release
// SMBByte    characteristicsExt1;
// SMBByte    characteristicsExt2;
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// System Information (Type 1)
//-------------------------------------------------------------------------------------------------------------------------
void decodeSystemInformation(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	DBG("System Information\n");
	DBG("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->manufacturer, neverMask));
	DBG("\tProduct Name: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->productName, neverMask));
	DBG("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->version, neverMask));
	DBG("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->serialNumber, privateData));
	uint8_t *uuid = ((SMBSystemInformation *)structHeader)->uuid;
	if (privateData) {
		DBG("\tUUID: %s\n", PrivateStr);
	} else {
		DBG("\tUUID: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02x%02X-%02X%02X%02X%02X%02X%02X\n",
			uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
			uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	}
	if (((SMBSystemInformation *)structHeader)->wakeupReason > 8) {
		DBG("\tWake-up Type: %s\n", OutOfSpecStr);
	} else {
		DBG("\tWake-up Type: %s\n", SMBWakeUpTypes[((SMBSystemInformation *)structHeader)->wakeupReason]);
	}
	DBG("\tSKU Number: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->skuNumber, neverMask)); // System SKU#
	DBG("\tFamily: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->family, neverMask));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Base Board (or Module) Information (Type 2)
//-------------------------------------------------------------------------------------------------------------------------
void decodeBaseBoard(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	DBG("Base Board Information\n");
	DBG("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->manufacturer, neverMask));
	DBG("\tProduct Name: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->product, neverMask));
	DBG("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->version, neverMask));
	DBG("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->serialNumber, privateData));
	DBG("\tAsset Tag: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->assetTag, neverMask));
// Feature Flags (BYTE)
	DBG("\tLocation In Chassis: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->locationInChassis, neverMask)); // Part Component
// Chassis Handle (WORD)
	if ((((SMBBaseBoard *)structHeader)->boardType < kSMBBaseBoardUnknown) || (((SMBBaseBoard *)structHeader)->boardType > kSMBBaseBoardInterconnect)) {
		DBG("\tType: %s\n", OutOfSpecStr);
	} else {
		DBG("\tType: %s\n", SMBBaseBoardTypes[(((SMBBaseBoard *)structHeader)->boardType - 1)]);
	}
// Number of Contained Object Handles (n) (BYTE)
// Contained Object Handles n(WORDs)
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// System Enclosure or Chassis (Type 3)
//-------------------------------------------------------------------------------------------------------------------------
void decodeSystemEnclosure(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	DBG("Chassis Information\n");
	DBG("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBSystemEnclosure *)structHeader)->manufacturer, neverMask));
	if ((((SMBSystemEnclosure *)structHeader)->chassisType < kSMBchassisOther) || (((SMBSystemEnclosure *)structHeader)->chassisType > kSMBchassisBladeEnclosing)) {
		DBG("\tType: %s\n", OutOfSpecStr);
	} else {
		DBG("\tType: %s\n", SMBChassisTypes[(((SMBSystemEnclosure *)structHeader)->chassisType - 1)]);
	}
// Lock:
	DBG("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBSystemEnclosure *)structHeader)->version, neverMask));
	DBG("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBSystemEnclosure *)structHeader)->serialNumber, privateData));
	DBG("\tAsset Tag: %s\n", SMBStringForField(structHeader, ((SMBSystemEnclosure *)structHeader)->assetTag, neverMask));
// Boot-up State:
// Power Supply State
// Thermal State
// Security Status:
// OEM Information:
// Height;
// Number Of Power Cords: Cords;
// Contained Elements: ElementsCount;
// SKU Number:
// ElementLen;
// Elements[1];         // open array of ElementsCount*ElementLen BYTEs
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Processor Information (Type 4)
//-------------------------------------------------------------------------------------------------------------------------
void decodeProcessorInformation(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	DBG("Processor Information\n");
	DBG("\tSocket Designation: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->socketDesignation, neverMask));
	if ((((SMBProcessorInformation *)structHeader)->processorType < kSMBprocessorTypeOther) || (((SMBProcessorInformation *)structHeader)->processorType > kSMBprocessorTypeGPU)) {
		DBG("\tType: %s\n", OutOfSpecStr);
	} else {
		DBG("\tType: %s\n", SMBProcessorTypes[((SMBProcessorInformation *)structHeader)->processorType - 1]);
	}
	DBG("\tFamily: 0x%X\n", ((SMBProcessorInformation *)structHeader)->processorFamily);
	DBG("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->manufacturer, neverMask));
	DBG("\tID: 0x%llX\n", ((SMBProcessorInformation *)structHeader)->processorID);
//	DBG("\tSignature: Type %u, Family %u, Model %u, Stepping %u\n", (eax >> 12) & 0x3, ((eax >> 20) & 0xFF) + ((eax >> 8) & 0x0F), ((eax >> 12) & 0xF0) + ((eax >> 4) & 0x0F), eax & 0xF);
// Flags:
	DBG("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->processorVersion, neverMask));
//	DBG("\tVoltage: 0.%xV\n", ((SMBProcessorInformation *)structHeader)->voltage);
	DBG("\tExternal Clock: %d MHz\n", ((SMBProcessorInformation *)structHeader)->externalClock);
	DBG("\tMax Speed: %d MHz\n", ((SMBProcessorInformation *)structHeader)->maximumClock);
	DBG("\tCurrent Speed: %d MHz\n", ((SMBProcessorInformation *)structHeader)->currentClock);
// Status: Populated/Unpopulated
	if ((((SMBProcessorInformation *)structHeader)->processorUpgrade < 1) || (((SMBProcessorInformation *)structHeader)->processorUpgrade > 0x2C)) {
		DBG("\tUpgrade: %s\n", OutOfSpecStr);
	} else {
		DBG("\tUpgrade: %s\n", SMBProcessorUpgrades[((SMBProcessorInformation *)structHeader)->processorUpgrade - 1]);
	}
// L1 Cache Handle:
// L2 Cache Handle:
// L3 Cache Handle:
	DBG("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->serialNumber, privateData));
	DBG("\tAsset Tag: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->assetTag, neverMask));
	DBG("\tPart Number: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->partNumber, neverMask));
	if(((SMBProcessorInformation *)structHeader)->coreCount != 0) {
		DBG("\tCore Count: %d\n", ((SMBProcessorInformation *)structHeader)->coreCount);}
	if(((SMBProcessorInformation *)structHeader)->coreEnabled != 0) {
		DBG("\tCore Enabled: %d\n", ((SMBProcessorInformation *)structHeader)->coreEnabled);}
	if(((SMBProcessorInformation *)structHeader)->threadCount != 0) {
		DBG("\tThread Count: %d\n", ((SMBProcessorInformation *)structHeader)->threadCount);
	}
// Characteristics:
//	DBG("\tProcessor Family 2: %d\n", ((SMBProcessorInformation *)structHeader)->processorFamily2);
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Memory Module Information (Type 6)
//-------------------------------------------------------------------------------------------------------------------------
//void decodeMemoryModule(SMBStructHeader *structHeader)
//{
//	DBG("Memory Module Information\n");
//	DBG("\tSocket Designation: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->socketDesignation));
//	DBG("\tBank Connections: Type: %d\n", structHeader->bankConnections);
//	DBG("\tCurrent Speed: %X\n", structHeader->currentSpeed);
//	DBG("\tType: %llX\n", structHeader->currentMemoryType);
//	DBG("\tInstalled Size: %d\n", structHeader->installedSize);
//	DBG("\tEnabled Size: %d\n", structHeader->enabledSize);
//	DBG("\tError Status: %x\n", structHeader->errorStatus);
//	DBG("\n");
//}

//-------------------------------------------------------------------------------------------------------------------------
// OEM Strings (Type 11)
//-------------------------------------------------------------------------------------------------------------------------
void decodeSMBOEMStrings(SMBStructHeader *structHeader)
{
	char *stringPtr = (char *)structHeader + structHeader->length;
	printHeader(structHeader);
	DBG("OEM Strings\n");
	SMBByte i;
	for (i = 1; i <= ((SMBOEMStrings *)structHeader)->count; i++) {
		DBG("\tString %d: %s\n", i, stringPtr);
		stringPtr = stringPtr + strlen(stringPtr) + 1;
	}
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// MemoryDevice (Type 17)
//-------------------------------------------------------------------------------------------------------------------------
void decodeMemoryDevice(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	DBG("Memory Device\n");
// Aray Handle
	DBG("\tError Information Handle: 0x%x\n", ((SMBMemoryDevice *)structHeader)->errorHandle);
// Total Width:
// Data Width:
// Size:
// Form Factor:
// Set:
	DBG("\tLocator: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->deviceLocator, neverMask));
	DBG("\tBank Locator: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->bankLocator, neverMask));
	if (((SMBMemoryDevice *)structHeader)->memoryType > kSMBMemoryDeviceTypeCount) {
		DBG("\tMemory Type: %s\n", OutOfSpecStr);
	} else {
		DBG("\tMemory Type: %s\n", SMBMemoryDeviceTypes[((SMBMemoryDevice *)structHeader)->memoryType]);
	}
// Type Detail:
	DBG("\tSpeed: %d MHz\n", ((SMBMemoryDevice *)structHeader)->memorySpeed);
	DBG("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->manufacturer, neverMask));
	DBG("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->serialNumber, privateData));
	DBG("\tAsset Tag: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->assetTag, neverMask));
	DBG("\tPart Number: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->partNumber, neverMask));
// Rank:
// Configured Clock Speed:
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific  (Type 131)
//-------------------------------------------------------------------------------------------------------------------------
void decodeOemProcessorType(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	DBG("Apple specific Processor Type\n");
	DBG("\tCpu-type: 0x%x\n", ((SMBOemProcessorType *)structHeader)->ProcessorType);
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific  (Type 132)
//-------------------------------------------------------------------------------------------------------------------------
void decodeOemProcessorBusSpeed(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	DBG("Apple specific Processor Interconnect Speed\n");
	DBG("\tQPI = %d MT/s\n", ((SMBOemProcessorBusSpeed *)structHeader)->ProcessorBusSpeed);
	DBG("\n");
}

// Info for the Table Above: dmi 2.7+ https://wiki.debian.org/InstallingDebianOn/Thinkpad/T42/lenny?action=AttachFile&do=get&target=dmidecode.Lenny_Thinkpad_T42_2373.txt
//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific  (Type 133)
//-------------------------------------------------------------------------------------------------------------------------
//void decodeOemPlatformFeature(SMBStructHeader *structHeader)
//{
//	printHeader(structHeader);
//	DBG("Apple specific Platform Feature\n");
//	DBG("\t%s\n", ((SMBOemPlatformFeature *)structHeader)->PlatformFeature);
//	DBG("\n");
//}

//-------------------------------------------------------------------------------------------------------------------------
// Specific  (Type 134)
//-------------------------------------------------------------------------------------------------------------------------
//void decodeOem(SMBStructHeader *structHeader)
//{
//	printHeader(structHeader);
//	DBG("Apple specific Feature\n");
//	DBG("\t%s\n", ((SMBOemPlatformFeature *)structHeader)->Feature);
//	DBG("\n");
//}

//-------------------------------------------------------------------------------------------------------------------------


void decodeSMBIOSTable(SMBEntryPoint *eps)
{
	uint8_t *ptr = (uint8_t *)eps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)ptr;

	minorVersion = eps->minorVersion;
	majorVersion = eps->majorVersion;
	bcdRevisionHi = eps->dmi.bcdRevision >> 4;
	bcdRevisionLo = eps->dmi.bcdRevision & 0x0F;

	getBoolForKey(kPrivateData, &privateData, &bootInfo->chameleonConfig);  // Bungo: chek if mask some data

	DBG("\n");
	DBG("SMBIOS rev.: %d.%d, DMI rev.: %d.%d\n", majorVersion, minorVersion, bcdRevisionHi, bcdRevisionLo);
	DBG("\n");
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		switch (structHeader->type)
		{
			case kSMBTypeBIOSInformation: // Type 0
				decodeBIOSInformation(structHeader);
				break;

			case kSMBTypeSystemInformation: // Type 1
				decodeSystemInformation(structHeader);
				break;

			case kSMBTypeBaseBoard: // Type 2
				decodeBaseBoard(structHeader);
				break;

			case kSMBTypeSystemEnclosure: // Type 3
				decodeSystemEnclosure(structHeader);
				break;

			case kSMBTypeProcessorInformation: // Type 4
				decodeProcessorInformation(structHeader);
				break;

			//case kSMBTypeMemoryModule: // Type 6
			//	decodeMemoryModule(structHeader);
			//	break;

			//case kSMBTypeSystemSlot: // Type 9
			//	decodeSMBTypeSystemSlot(structHeader);
			//	break;

			case kSMBOEMStrings: // Type 11
				decodeSMBOEMStrings(structHeader);
				break;

			case kSMBTypeMemoryDevice: // Type 17
				decodeMemoryDevice(structHeader);
				break;

			//kSMBTypeMemoryArrayMappedAddress: // Type 19
			//	break;

			/* Skip all Apple Specific Structures */
			// case kSMBTypeFirmwareVolume: // Type 128
			// case kSMBTypeMemorySPD: // Type 130
			//	break;

			case kSMBTypeOemProcessorType: // Type 131
				decodeOemProcessorType(structHeader);
				break;

			case kSMBTypeOemProcessorBusSpeed: // Type 132
				decodeOemProcessorBusSpeed(structHeader);
				break;

			//kSMBTypeOemPlatformFeature: // Type 133
			//	decodeOemPlatformFeature(structHeader);
			//	break;

			case kSMBTypeEndOfTable: // Type 127
				DBG("Handle 0x%04x, DMI type %d, %d  bytes\n", structHeader->handle, structHeader->type, structHeader->length);
				DBG("End of Table\n");
				break;

			default:
				break;
		}

		ptr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0) {
			ptr += 2;
		}

		structHeader = (SMBStructHeader *)ptr;
	}
	DBG("\n");
}

