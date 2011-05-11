/*
 * A very simple SMBIOS Table decoder, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "libsaio.h"
#include "smbios.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)	msglog(x)
#endif


static SMBWord minorVersion;

extern char *getSMBStringForField(SMBStructHeader *structHeader, uint8_t field);

//-------------------------------------------------------------------------------------------------------------------------
// BIOSInformation
//-------------------------------------------------------------------------------------------------------------------------
void decodeBIOSInformation(SMBBIOSInformation *structHeader)
{
	DBG("BIOSInformation:\n");
	DBG("\tvendor: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->vendor));
	DBG("\tversion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\treleaseDate: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->releaseDate));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// SystemInformation
//-------------------------------------------------------------------------------------------------------------------------
void decodeSystemInformation(SMBSystemInformation *structHeader)
{
	DBG("SystemInformation:\n");
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tproductName: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->productName));
	DBG("\tversion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));

	if (minorVersion < 1 || structHeader->header.length < 25)
		return;
	uint8_t *uuid = structHeader->uuid;
	DBG("\tuuid: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02x%02X-%02X%02X%02X%02X%02X%02X\n",
		uuid[0], uuid[1], uuid[2], uuid[3],  
		uuid[4], uuid[5], 
		uuid[6], uuid[7], 
		uuid[8], uuid[9], 
		uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	DBG("\twakeupReason: 0x%x\n", structHeader->wakeupReason);

	if (minorVersion < 4 || structHeader->header.length < 27)
		return;
	DBG("\tskuNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->skuNumber));
	DBG("\tfamily: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->family));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// BaseBoard
//-------------------------------------------------------------------------------------------------------------------------
void decodeBaseBoard(SMBBaseBoard *structHeader)
{
	DBG("BaseBoard:\n");
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tproduct: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->product));
	DBG("\tversion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tassetTagNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTagNumber));
	DBG("\tlocationInChassis: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->locationInChassis));
	DBG("\tboardType: 0x%X\n", structHeader->boardType);
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// SystemEnclosure
//-------------------------------------------------------------------------------------------------------------------------
void decodeSystemEnclosure(SMBSystemEnclosure *structHeader)
{
	DBG("SystemEnclosure:\n");
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\ttype: %d\n", structHeader->type);
	DBG("\tversion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tassetTagNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTagNumber));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// ProcessorInformation
//-------------------------------------------------------------------------------------------------------------------------
void decodeProcessorInformation(SMBProcessorInformation *structHeader)
{
	DBG("ProcessorInformation:\n");
	DBG("\tsocketDesignation: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->socketDesignation));
	DBG("\tprocessorType: %d\n", structHeader->processorType);
	DBG("\tprocessorFamily: 0x%X\n", structHeader->processorFamily);
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tprocessorID: 0x%llX\n", structHeader->processorID);
	DBG("\tprocessorVersion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->processorVersion));
	DBG("\texternalClock: %dMHz\n", structHeader->externalClock);
	DBG("\tmaximumClock: %dMHz\n", structHeader->maximumClock);
	DBG("\tcurrentClock: %dMHz\n", structHeader->currentClock);

	if (minorVersion < 3 || structHeader->header.length < 35)
		return;
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tassetTag: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag));
	DBG("\tpartNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->partNumber));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// MemoryDevice
//-------------------------------------------------------------------------------------------------------------------------
void decodeMemoryDevice(SMBMemoryDevice *structHeader)
{
	DBG("MemoryDevice:\n");
	DBG("\tdeviceLocator: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->deviceLocator));
	DBG("\tbankLocator: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->bankLocator));
	DBG("\tmemoryType: %s\n", SMBMemoryDeviceTypes[structHeader->memoryType]);

	if (minorVersion < 3 || structHeader->header.length < 27)
		return;
	DBG("\tmemorySpeed: %dMHz\n", structHeader->memorySpeed);
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tassetTag: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag));
	DBG("\tpartNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->partNumber));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific
//-------------------------------------------------------------------------------------------------------------------------
void decodeOemProcessorType(SMBOemProcessorType *structHeader)
{
	DBG("AppleProcessorType:\n");
	DBG("\tProcessorType: 0x%x\n", ((SMBOemProcessorType *)structHeader)->ProcessorType);
	DBG("\n");
}

void decodeOemProcessorBusSpeed(SMBOemProcessorBusSpeed *structHeader)
{
	DBG("AppleProcessorBusSpeed:\n");
	DBG("\tProcessorBusSpeed (QPI): %d.%dGT/s\n", 
			((SMBOemProcessorBusSpeed *)structHeader)->ProcessorBusSpeed / 1000, 
			(((SMBOemProcessorBusSpeed *)structHeader)->ProcessorBusSpeed / 100) % 10);
	DBG("\n");
}
//-------------------------------------------------------------------------------------------------------------------------


void decodeSMBIOSTable(SMBEntryPoint *eps)
{
	uint8_t *ptr = (uint8_t *)eps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)ptr;

	minorVersion = eps->minorVersion;

	DBG("\n");
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
				structHeader->type, structHeader->length, structHeader->handle);

		switch (structHeader->type)
		{
			case kSMBTypeBIOSInformation:
				decodeBIOSInformation((SMBBIOSInformation *)structHeader);
				break;

			case kSMBTypeSystemInformation:
				decodeSystemInformation((SMBSystemInformation *)structHeader);
				break;

			case kSMBTypeBaseBoard:
				decodeBaseBoard((SMBBaseBoard *)structHeader);
				break;

			case kSMBTypeSystemEnclosure:
				decodeSystemEnclosure((SMBSystemEnclosure *)structHeader);
				break;

			case kSMBTypeProcessorInformation:
				decodeProcessorInformation((SMBProcessorInformation *)structHeader);
				break;

			case kSMBTypeMemoryDevice:
				decodeMemoryDevice((SMBMemoryDevice *)structHeader);
				break;

			/* Skip all Apple Specific Structures */
			case kSMBTypeFirmwareVolume:
			case kSMBTypeMemorySPD:
				break;

			case kSMBTypeOemProcessorType:
				decodeOemProcessorType((SMBOemProcessorType *)structHeader);
				break;

			case kSMBTypeOemProcessorBusSpeed:
				decodeOemProcessorBusSpeed((SMBOemProcessorBusSpeed *)structHeader);
				break;

			case kSMBTypeEndOfTable:
				/* Skip, to be added at the end */
				break;

			default:
				break;
		}

		ptr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0)
			ptr += 2;

		structHeader = (SMBStructHeader *)ptr;
	}
	DBG("\n");
}

