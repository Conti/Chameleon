/*
 * SMBIOS Table Patcher, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */


#include "boot.h"
#include "bootstruct.h"
#include "smbios_getters.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)	msglog(x)
#endif

#define SMBPlist			&bootInfo->smbiosConfig
/* ASSUMPTION: 16KB should be enough for the whole thing */
#define SMB_ALLOC_SIZE	16384


//-------------------------------------------------------------------------------------------------------------------------
// SMBIOS Plist Keys
//-------------------------------------------------------------------------------------------------------------------------
/* BIOS Information */
#define kSMBBIOSInformationVendorKey				"SMbiosvendor"
#define kSMBBIOSInformationVersionKey				"SMbiosversion"
#define kSMBBIOSInformationReleaseDateKey			"SMbiosdate"

/* System Information */
#define kSMBSystemInformationManufacturerKey		"SMmanufacturer"
#define kSMBSystemInformationProductNameKey			"SMproductname"
#define kSMBSystemInformationVersionKey				"SMsystemversion"
#define kSMBSystemInformationSerialNumberKey		"SMserial"
#define kSMBSystemInformationFamilyKey				"SMfamily"

/* Base Board */
#define kSMBBaseBoardManufacturerKey				"SMboardmanufacturer"
#define kSMBBaseBoardProductKey						"SMboardproduct"

/* Processor Information */
#define kSMBProcessorInformationExternalClockKey	"SMexternalclock"
#define kSMBProcessorInformationMaximumClockKey		"SMmaximalclock"

/* Memory Device */
#define kSMBMemoryDeviceDeviceLocatorKey			"SMmemdevloc"
#define kSMBMemoryDeviceBankLocatorKey				"SMmembankloc"
#define kSMBMemoryDeviceMemoryTypeKey				"SMmemtype"
#define kSMBMemoryDeviceMemorySpeedKey				"SMmemspeed"
#define kSMBMemoryDeviceManufacturerKey				"SMmemmanufacturer"
#define kSMBMemoryDeviceSerialNumberKey				"SMmemserial"
#define kSMBMemoryDevicePartNumberKey				"SMmempart"

/* Apple Specific */
#define kSMBOemProcessorTypeKey						"SMcputype"
#define kSMBOemProcessorBusSpeedKey					"SMbusspeed"

//-------------------------------------------------------------------------------------------------------------------------
// Default SMBIOS Data
//-------------------------------------------------------------------------------------------------------------------------
/* Rewrite: use a struct */

#define kDefaultVendorManufacturer					"Apple Inc."
#define kDefaultBIOSReleaseDate						"11/06/2009"
#define kDefaultSerialNumber						"SOMESRLNMBR"
#define kDefaultBoardProduct						"Mac-F4208DC8"
#define kDefaultSystemVersion						"1.0"

// defaults for a Mac mini 
#define kDefaultMacminiFamily						"Macmini"
#define kDefaultMacmini								"Macmini2,1"
#define kDefaultMacminiBIOSVersion					"    MM21.88Z.009A.B00.0903051113"

// defaults for a MacBook
#define kDefaultMacBookFamily						"MacBook"
#define kDefaultMacBook								"MacBook4,1"
#define kDefaultMacBookBIOSVersion					"    MB41.88Z.0073.B00.0903051113"

// defaults for a MacBook Pro
#define kDefaultMacBookProFamily					"MacBookPro"
#define kDefaultMacBookPro							"MacBookPro4,1"
#define kDefaultMacBookProBIOSVersion				"    MBP41.88Z.0073.B00.0903051113"

// defaults for an iMac
#define kDefaultiMacFamily							"iMac"
#define kDefaultiMac								"iMac8,1"
#define kDefaultiMacBIOSVersion						"    IM81.88Z.00C1.B00.0903051113"
// defaults for an iMac11,1 core i3/i5/i7
#define kDefaultiMacNehalem							"iMac11,1"
#define kDefaultiMacNehalemBIOSVersion				"    IM111.88Z.0034.B00.0903051113"

// defaults for a Mac Pro
#define kDefaultMacProFamily						"MacPro"
#define kDefaultMacPro								"MacPro3,1"
#define kDefaultMacProBIOSVersion					"    MP31.88Z.006C.B05.0903051113"
// defaults for a Mac Pro 4,1 core i7/Xeon
#define kDefaultMacProNehalem						"MacPro4,1"
#define kDefaultMacProNehalemBIOSVersion			"    MP41.88Z.0081.B04.0903051113"
// defaults for a Mac Pro 5,1 core i7/Xeon
#define kDefaultMacProWestmere						"MacPro5,1"
#define kDefaultMacProWestmereBIOSVersion			"    MP51.88Z.007F.B00.1008031144"
#define kDefaulMacProWestmereBIOSReleaseDate		"08/03/10"
//-------------------------------------------------------------------------------------------------------------------------


#define getFieldOffset(struct, field)	((uint8_t)(uint32_t)&(((struct *)0)->field))

typedef struct {
	SMBStructHeader *orig;
	SMBStructHeader *new;
} SMBStructPtrs;

struct {
	char *vendor;
	char *version;
	char *releaseDate;
} defaultBIOSInfo;

struct {
	char *manufacturer;
	char *productName;
	char *version;
	char *serialNumber;
	char *family;
} defaultSystemInfo;

struct {
	char *manufacturer;
	char *product;
} defaultBaseBoard;


typedef struct {
	uint8_t			type;
	SMBValueType	valueType;
	uint8_t			fieldOffset;
	char			*keyString;
	bool			(*getSMBValue)(returnType *);
	char			**defaultValue;
} SMBValueSetter;

SMBValueSetter SMBSetters[] =                           
{                                                           
	//-------------------------------------------------------------------------------------------------------------------------
	// BIOSInformation
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeBIOSInformation,	kSMBString,	getFieldOffset(SMBBIOSInformation, vendor),			kSMBBIOSInformationVendorKey,		
		NULL,	&defaultBIOSInfo.vendor			},

	{kSMBTypeBIOSInformation,	kSMBString,	getFieldOffset(SMBBIOSInformation, version),		kSMBBIOSInformationVersionKey,		
		NULL,	&defaultBIOSInfo.version		},

	{kSMBTypeBIOSInformation,	kSMBString,	getFieldOffset(SMBBIOSInformation, releaseDate),	kSMBBIOSInformationReleaseDateKey,	
		NULL,	&defaultBIOSInfo.releaseDate	},

	//-------------------------------------------------------------------------------------------------------------------------
	// SystemInformation
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, manufacturer),	kSMBSystemInformationManufacturerKey,	
		NULL,	&defaultSystemInfo.manufacturer	},

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, productName),	kSMBSystemInformationProductNameKey,	
		NULL,	&defaultSystemInfo.productName	},

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, version),		kSMBSystemInformationVersionKey,		
		NULL,	&defaultSystemInfo.version		},

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, serialNumber),	kSMBSystemInformationSerialNumberKey,	
		NULL,	&defaultSystemInfo.serialNumber	},

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, skuNumber),	NULL,									
		NULL,	NULL							},

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, family),		kSMBSystemInformationFamilyKey,			
		NULL,	&defaultSystemInfo.family		},


	//-------------------------------------------------------------------------------------------------------------------------
	// BaseBoard
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, manufacturer),			kSMBBaseBoardManufacturerKey,	
		NULL,	&defaultBaseBoard.manufacturer	},

	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, product),				kSMBBaseBoardProductKey,		
		NULL,	&defaultBaseBoard.product		},

    {kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, version),				NULL,	NULL,	NULL},

    {kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, serialNumber),			NULL,	NULL,	NULL},

    {kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, assetTagNumber),		NULL,	NULL,	NULL},

    {kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, locationInChassis),	NULL,	NULL,	NULL},


	//-------------------------------------------------------------------------------------------------------------------------
	// ProcessorInformation
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, socketDesignation),	NULL,	NULL,	NULL},

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, manufacturer),		NULL,	NULL,	NULL},

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, processorVersion),	NULL,	NULL,	NULL},

	{kSMBTypeProcessorInformation,	kSMBWord,	getFieldOffset(SMBProcessorInformation, externalClock),		kSMBProcessorInformationExternalClockKey,	
		getProcessorInformationExternalClock,	NULL},

	{kSMBTypeProcessorInformation,	kSMBWord,	getFieldOffset(SMBProcessorInformation, maximumClock),		kSMBProcessorInformationMaximumClockKey,	
		getProcessorInformationMaximumClock,	NULL},

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, serialNumber),		NULL,	NULL,	NULL},

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, assetTag),			NULL,	NULL,	NULL},

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, partNumber),		NULL,	NULL,	NULL},

	//-------------------------------------------------------------------------------------------------------------------------
	// Memory Device
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, deviceLocator),	kSMBMemoryDeviceDeviceLocatorKey,	
		NULL,							NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, bankLocator),	kSMBMemoryDeviceBankLocatorKey,		
		NULL,							NULL},

	{kSMBTypeMemoryDevice,	kSMBByte,	getFieldOffset(SMBMemoryDevice, memoryType),	kSMBMemoryDeviceMemoryTypeKey,		
		getSMBMemoryDeviceMemoryType,	NULL},

	{kSMBTypeMemoryDevice,	kSMBWord,	getFieldOffset(SMBMemoryDevice, memorySpeed),	kSMBMemoryDeviceMemorySpeedKey,		
		getSMBMemoryDeviceMemorySpeed,	NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, manufacturer),	kSMBMemoryDeviceManufacturerKey,	
		getSMBMemoryDeviceManufacturer,	NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, serialNumber),	kSMBMemoryDeviceSerialNumberKey,	
		getSMBMemoryDeviceSerialNumber,	NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, assetTag),		NULL,	NULL,	NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, partNumber),	kSMBMemoryDevicePartNumberKey,		
		getSMBMemoryDevicePartNumber,	NULL},


	//-------------------------------------------------------------------------------------------------------------------------
	// Apple Specific
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeOemProcessorType,		kSMBWord,	getFieldOffset(SMBOemProcessorType, ProcessorType),			kSMBOemProcessorTypeKey,		
		getSMBOemProcessorType,			NULL},

	{kSMBTypeOemProcessorBusSpeed,	kSMBWord,	getFieldOffset(SMBOemProcessorBusSpeed, ProcessorBusSpeed),	kSMBOemProcessorBusSpeedKey,	
		getSMBOemProcessorBusSpeed,		NULL}
};

int numOfSetters = sizeof(SMBSetters) / sizeof(SMBValueSetter);


SMBEntryPoint *origeps	= 0;
SMBEntryPoint *neweps	= 0;

static uint8_t stringIndex;	// increament when a string is added and set the field value accordingly
static uint8_t stringsSize;	// add string size

static SMBWord tableLength		= 0;
static SMBWord handle			= 0;
static SMBWord maxStructSize	= 0;
static SMBWord structureCount	= 0;

/* Rewrite this function */
void setDefaultSMBData(void)
{
	defaultBIOSInfo.vendor			= kDefaultVendorManufacturer;
	defaultBIOSInfo.releaseDate		= kDefaultBIOSReleaseDate;

	defaultSystemInfo.manufacturer	= kDefaultVendorManufacturer;
	defaultSystemInfo.version		= kDefaultSystemVersion;
	defaultSystemInfo.serialNumber	= kDefaultSerialNumber;

	defaultBaseBoard.manufacturer	= kDefaultVendorManufacturer;
	defaultBaseBoard.product		= kDefaultBoardProduct;

	if (platformCPUFeature(CPU_FEATURE_MOBILE))
	{
		if (Platform.CPU.NoCores > 1)
		{
			defaultBIOSInfo.version			= kDefaultMacBookProBIOSVersion;
			defaultSystemInfo.productName	= kDefaultMacBookPro;
			defaultSystemInfo.family		= kDefaultMacBookProFamily;
		}
		else
		{
			defaultBIOSInfo.version			= kDefaultMacBookBIOSVersion;
			defaultSystemInfo.productName	= kDefaultMacBook;
			defaultSystemInfo.family		= kDefaultMacBookFamily;
		}
	}
	else
	{
		switch (Platform.CPU.NoCores) 
		{
			case 1: 
				defaultBIOSInfo.version			= kDefaultMacminiBIOSVersion;
				defaultSystemInfo.productName	= kDefaultMacmini;
				defaultSystemInfo.family		= kDefaultMacminiFamily;
				break;

			case 2:
				defaultBIOSInfo.version			= kDefaultiMacBIOSVersion;
				defaultSystemInfo.productName	= kDefaultiMac;
				defaultSystemInfo.family		= kDefaultiMacFamily;
				break;
			default:
			{
				switch (Platform.CPU.Family) 
				{
					case 0x06:
					{
						switch (Platform.CPU.Model)
						{
							case CPU_MODEL_FIELDS:		// Intel Core i5, i7 LGA1156 (45nm)
							case CPU_MODEL_DALES:		// Intel Core i5, i7 LGA1156 (45nm) ???
							case CPU_MODEL_DALES_32NM:	// Intel Core i3, i5, i7 LGA1156 (32nm) (Clarkdale, Arrandale)
							case 0x19:					// Intel Core i5 650 @3.20 Ghz 
								defaultBIOSInfo.version			= kDefaultiMacNehalemBIOSVersion;
								defaultSystemInfo.productName	= kDefaultiMacNehalem;
								defaultSystemInfo.family		= kDefaultiMacFamily;
								break;

							case CPU_MODEL_NEHALEM: 
							case CPU_MODEL_NEHALEM_EX:
								defaultBIOSInfo.version			= kDefaultMacProNehalemBIOSVersion;
								defaultSystemInfo.productName	= kDefaultMacProNehalem;
								defaultSystemInfo.family		= kDefaultMacProFamily;
								break;

							case CPU_MODEL_WESTMERE: 
							case CPU_MODEL_WESTMERE_EX:
								defaultBIOSInfo.version			= kDefaultMacProWestmereBIOSVersion;
								defaultBIOSInfo.releaseDate		= kDefaulMacProWestmereBIOSReleaseDate;
								defaultSystemInfo.productName	= kDefaultMacProWestmere;
								defaultSystemInfo.family		= kDefaultMacProFamily;
								break;

							default:
								defaultBIOSInfo.version			= kDefaultMacProBIOSVersion;
								defaultSystemInfo.productName	= kDefaultMacPro;
								defaultSystemInfo.family		= kDefaultMacProFamily;
								break;
						}
						break;
					}
					default:
						defaultBIOSInfo.version			= kDefaultMacProBIOSVersion;
						defaultSystemInfo.productName	= kDefaultMacPro;
						defaultSystemInfo.family		= kDefaultMacProFamily;
						break;
				}
				break;
			}
		}
	}
}

/* Used for SM*_N smbios.plist keys */
bool getSMBValueForKey(SMBStructHeader *structHeader, const char *keyString, const char **string, returnType *value)
{
	static int idx = -1;
	static int current = -1;
	int len;
	char key[24];

	if (current != structHeader->handle)
	{
		idx++;
		current = structHeader->handle;
	}

	sprintf(key, "%s%d", keyString, idx);

	if (value)
		if (getIntForKey(key, (int *)&(value->dword), SMBPlist))
			return true;
	else
		if (getValueForKey(key, string, &len, SMBPlist))
			return true;
	return false;
}

char *getSMBStringForField(SMBStructHeader *structHeader, uint8_t field)
{
	uint8_t *stringPtr = (uint8_t *)structHeader + structHeader->length;

	if (!field)
		return (char *)0;

	for (field--; field != 0 && strlen((char *)stringPtr) > 0; 
		field--, stringPtr = (uint8_t *)((uint32_t)stringPtr + strlen((char *)stringPtr) + 1));

	return (char *)stringPtr;
}

void setSMBStringForField(SMBStructHeader *structHeader, const char *string, uint8_t *field)
{
	int strSize;

	if (!field)
		return;
	if (!string)
	{
		*field = 0;
		return;
	}

	strSize = strlen(string);

	// remove any spaces found at the end
	while (string[strSize - 1] == ' ')
		strSize--;

	memcpy((uint8_t *)structHeader + structHeader->length + stringsSize, string, strSize);
	*field = stringIndex;

	stringIndex++;
	stringsSize += strSize + 1;
}

bool setSMBValue(SMBStructPtrs *structPtr, int idx, returnType *value)
{
	const char *string = 0;
	int len;

	if (numOfSetters <= idx)
		return false;

	switch (SMBSetters[idx].valueType)
	{
		case kSMBString:
			if (SMBSetters[idx].keyString)
			{
				if (getValueForKey(SMBSetters[idx].keyString, &string, &len, SMBPlist))
					break;
				else
					if (structPtr->orig->type == kSMBTypeMemoryDevice)	// MemoryDevice only
						if (getSMBValueForKey(structPtr->orig, SMBSetters[idx].keyString, &string, NULL))
							break;
			}
			if (SMBSetters[idx].getSMBValue)
				if (SMBSetters[idx].getSMBValue((returnType *)&string))
					break;
			if ((SMBSetters[idx].defaultValue) && *(SMBSetters[idx].defaultValue))
			{
				string = *(SMBSetters[idx].defaultValue);
				break;
			}
			string = getSMBStringForField(structPtr->orig, *(uint8_t *)value);
			break;

		case kSMBByte:
		case kSMBWord:
		case kSMBDWord:
		//case kSMBQWord:
			if (SMBSetters[idx].keyString)
			{
				if (getIntForKey(SMBSetters[idx].keyString, (int *)&(value->dword), SMBPlist))
					return true;
				else
					if (structPtr->orig->type == kSMBTypeMemoryDevice)	// MemoryDevice only
						if (getSMBValueForKey(structPtr->orig, SMBSetters[idx].keyString, NULL, value))
							return true;
			}
			if (SMBSetters[idx].getSMBValue)
				if (SMBSetters[idx].getSMBValue(value))
					return true;
#if 0
			if (*(SMBSetters[idx].defaultValue))
			{
				value->dword = *(uint32_t *)(SMBSetters[idx].defaultValue);
				return true;
			}
#endif
			break;
	}

	if (SMBSetters[idx].valueType == kSMBString && string)
		setSMBStringForField(structPtr->new, string, &value->byte);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific
//-------------------------------------------------------------------------------------------------------------------------
void addSMBFirmwareVolume(SMBStructPtrs *structPtr)
{
	return;
}

void addSMBMemorySPD(SMBStructPtrs *structPtr)
{
	/* SPD data from Platform.RAM.spd */
	return;
}

void addSMBOemProcessorType(SMBStructPtrs *structPtr)
{
	SMBOemProcessorType *p = (SMBOemProcessorType *)structPtr->new;

	p->header.type		= kSMBTypeOemProcessorType;
	p->header.length	= sizeof(SMBOemProcessorType);
	p->header.handle	= handle++;

	setSMBValue(structPtr, numOfSetters - 2 , (returnType *)&(p->ProcessorType));

	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + sizeof(SMBOemProcessorType) + 2);
	tableLength += sizeof(SMBOemProcessorType) + 2;
	structureCount++;
}

void addSMBOemProcessorBusSpeed(SMBStructPtrs *structPtr)
{
	SMBOemProcessorBusSpeed *p = (SMBOemProcessorBusSpeed *)structPtr->new;

	p->header.type		= kSMBTypeOemProcessorBusSpeed;
	p->header.length	= sizeof(SMBOemProcessorBusSpeed);
	p->header.handle	= handle++;

	setSMBValue(structPtr, numOfSetters -1, (returnType *)&(p->ProcessorBusSpeed));

	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + sizeof(SMBOemProcessorBusSpeed) + 2);
	tableLength += sizeof(SMBOemProcessorBusSpeed) + 2;
	structureCount++;
}

//-------------------------------------------------------------------------------------------------------------------------
// EndOfTable
//-------------------------------------------------------------------------------------------------------------------------
void addSMBEndOfTable(SMBStructPtrs *structPtr)
{
	structPtr->new->type	= kSMBTypeEndOfTable;
	structPtr->new->length	= sizeof(SMBStructHeader);
	structPtr->new->handle	= handle++;

	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + sizeof(SMBStructHeader) + 2);
	tableLength += sizeof(SMBStructHeader) + 2;
	structureCount++;
}

void setSMBStruct(SMBStructPtrs *structPtr)
{
	bool setterFound = false;

	uint8_t *ptr;
	SMBWord structSize;
	int i;

	stringIndex = 1;
	stringsSize = 0;

	if (handle < structPtr->orig->handle)
		handle = structPtr->orig->handle;

	memcpy((void *)structPtr->new, structPtr->orig, structPtr->orig->length);

	for (i = 0; i < numOfSetters; i++)
		if (structPtr->orig->type == SMBSetters[i].type)
		{
			setterFound = true;
			setSMBValue(structPtr, i, (returnType *)((uint8_t *)structPtr->new + SMBSetters[i].fieldOffset));
		}

	if (setterFound)
	{
		ptr = (uint8_t *)structPtr->new + structPtr->orig->length;
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0)
			ptr += 2;

		structSize = ptr - (uint8_t *)structPtr->new;
	}
	else
	{
		ptr = (uint8_t *)structPtr->orig + structPtr->orig->length;
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0)
			ptr += 2;
		
		structSize = ptr - (uint8_t *)structPtr->orig;
		memcpy((void *)structPtr->new, structPtr->orig, structSize);
	}

	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + structSize);

	tableLength += structSize;

	if (structSize >  maxStructSize)
		maxStructSize = structSize;

	structureCount++;
}

void setupNewSMBIOSTable(SMBEntryPoint *eps, SMBStructPtrs *structPtr)
{
	uint8_t *ptr = (uint8_t *)eps->dmi.tableAddress;
	structPtr->orig = (SMBStructHeader *)ptr;

	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structPtr->orig + sizeof(SMBStructHeader)));)
	{
		switch (structPtr->orig->type)
		{
			/* Skip all Apple Specific Structures */
			case kSMBTypeFirmwareVolume:
			case kSMBTypeMemorySPD:
			case kSMBTypeOemProcessorType:
			case kSMBTypeOemProcessorBusSpeed:
				/* And this one too, to be added at the end */
			case kSMBTypeEndOfTable:
				break;

			default:
				/* Add */
				setSMBStruct(structPtr);
				break;
		}

		ptr = (uint8_t *)((uint32_t)structPtr->orig + structPtr->orig->length);
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0)
			ptr += 2;

		structPtr->orig = (SMBStructHeader *)ptr;
	}

	addSMBFirmwareVolume(structPtr);
	addSMBMemorySPD(structPtr);
	addSMBOemProcessorType(structPtr);
	addSMBOemProcessorBusSpeed(structPtr);

	addSMBEndOfTable(structPtr);
}

void setupSMBIOSTable(void)
{
	SMBStructPtrs *structPtr;
	uint8_t *buffer;
	bool setSMB = true;

	if (!origeps)
		return;

	neweps = origeps;

	structPtr = (SMBStructPtrs *)malloc(sizeof(SMBStructPtrs));
	if (!structPtr)
		return;
	
	buffer = malloc(SMB_ALLOC_SIZE);
	if (!buffer)
		return;

	bzero(buffer, SMB_ALLOC_SIZE);
	structPtr->new = (SMBStructHeader *)buffer;

	getBoolForKey(kSMBIOSdefaults, &setSMB, &bootInfo->bootConfig);
	if (setSMB)
		setDefaultSMBData();

	setupNewSMBIOSTable(origeps, structPtr);

	neweps = (SMBEntryPoint *)AllocateKernelMemory(sizeof(SMBEntryPoint));
	if (!neweps)
		return;
	bzero(neweps, sizeof(SMBEntryPoint));

	neweps->anchor[0]			= '_';
	neweps->anchor[1]			= 'S';
	neweps->anchor[2]			= 'M';
	neweps->anchor[3]			= '_';
	neweps->entryPointLength	= sizeof(SMBEntryPoint);
	neweps->majorVersion		= 2;
	neweps->minorVersion		= 4;
	neweps->maxStructureSize	= maxStructSize;
	neweps->entryPointRevision	= 0;

	neweps->dmi.anchor[0]		= '_';
	neweps->dmi.anchor[1]		= 'D';
	neweps->dmi.anchor[2]		= 'M';
	neweps->dmi.anchor[3]		= 'I';
	neweps->dmi.anchor[4]		= '_';
	neweps->dmi.tableLength		= tableLength;
	neweps->dmi.tableAddress	= AllocateKernelMemory(tableLength);
	neweps->dmi.structureCount	= structureCount;
	neweps->dmi.bcdRevision		= 0x24;

	if (!neweps->dmi.tableAddress)
		return;

	memcpy((void *)neweps->dmi.tableAddress, buffer, tableLength);

	neweps->dmi.checksum		= 0;
	neweps->dmi.checksum		= 0x100 - checksum8(&neweps->dmi, sizeof(DMIEntryPoint));

	neweps->checksum			= 0;
	neweps->checksum			= 0x100 - checksum8(neweps, sizeof(SMBEntryPoint));

	free(buffer);
	decodeSMBIOSTable(neweps);
}

void *getSmbios(int which)
{
	switch (which)
	{
		case SMBIOS_ORIGINAL:
			if (!origeps)
				origeps = getAddressOfSmbiosTable();
			return origeps;
		case SMBIOS_PATCHED:
			return neweps;
	}

	return 0;
}

/* Collect any information needed later */
void readSMBIOSInfo(SMBEntryPoint *eps)
{
	uint8_t *structPtr = (uint8_t *)eps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)structPtr;

	int dimmnbr = 0;
	Platform.DMI.MaxMemorySlots	= 0;
	Platform.DMI.CntMemorySlots	= 0;
	Platform.DMI.MemoryModules	= 0;

	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		switch (structHeader->type)
		{
			case kSMBTypeSystemInformation:
				Platform.UUID = ((SMBSystemInformation *)structHeader)->uuid;
				break;

			case kSMBTypePhysicalMemoryArray:
				Platform.DMI.MaxMemorySlots += ((SMBPhysicalMemoryArray *)structHeader)->numMemoryDevices;
				break;

			case kSMBTypeMemoryDevice:
				Platform.DMI.CntMemorySlots++;
        		if (((SMBMemoryDevice *)structHeader)->memorySize != 0)
					Platform.DMI.MemoryModules++;
        		if (((SMBMemoryDevice *)structHeader)->memorySpeed > 0)
					Platform.RAM.DIMM[dimmnbr].Frequency = ((SMBMemoryDevice *)structHeader)->memorySpeed;
				dimmnbr++;
				break;
		}

		structPtr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)structPtr)[0] != 0; structPtr++);

		if (((uint16_t *)structPtr)[0] == 0)
			structPtr += 2;

		structHeader = (SMBStructHeader *)structPtr;
	}
}

