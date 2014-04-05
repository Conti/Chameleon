/*
 * SMBIOS Table Patcher, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */


#include "boot.h"
#include "bootstruct.h"
#include "smbios_getters.h"
// Bungo
#include "convert.h"

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

/* =======================
 BIOS Information (Type 0)
 ========================= */
#define kSMBBIOSInformationVendorKey		        "SMbiosvendor"         // Apple Inc.
#define kSMBBIOSInformationVersionKey		        "SMbiosversion"        // MP31.88Z.006C.B05.0802291410
#define kSMBBIOSInformationReleaseDateKey	        "SMbiosdate"           // 02/29/08
// Bungo
#define kSMBBIOSInformationReleaseKey               "SMbiosrelease"        // BIOS Revision
// example: BIOS Revision: 1.23 --> 2 bytes: Major=0x01, Minor=0x17 --> after swap: 0x1701hex = 5889dec (SMBIOS_spec_DSP0134_2.7.1)

/* =========================
 System Information (Type 1)
 =========================== */
#define kSMBSystemInformationManufacturerKey        "SMmanufacturer"       // Apple Inc.
#define kSMBSystemInformationProductNameKey         "SMproductname"        // MacPro3,1
#define kSMBSystemInformationVersionKey             "SMsystemversion"      // 1.0
#define kSMBSystemInformationSerialNumberKey        "SMserial"             // Serial number
//Bungo
#define kSMBSystemInformationUUIDKey                "SMsystemuuid"         // ioreg -rd1 -c IOPlatformExpertDevice | awk '/IOPlatformUUID/ { split($0, line, "\""); printf("%s\n", line[4]); }'
#define kSMBSystemInformationSKUNumberKey           "SMskunumber"          // System SKU#

#define kSMBSystemInformationFamilyKey              "SMfamily"             // MacPro

/* =========================================
 Base Board (or Module) Information (Type 2)
 =========================================== */
#define kSMBBaseBoardManufacturerKey                "SMboardmanufacturer"  // Apple Inc.
#define kSMBBaseBoardProductKey                     "SMboardproduct"       // Mac-F2268DC8
// Bungo
#define kSMBBaseBoardVersionKey                     "SMboardversion"       // MacPro3,1
#define kSMBBaseBoardSerialNumberKey                "SMboardserial"        // C02140302D5DMT31M
#define kSMBBaseBoardAssetTagKey                    "SMboardassettag"      // Base Board Asset Tag# Bungo: renamed folowing convention
#define kSMBBaseBoardLocationInChassisKey           "SMboardlocation"      // Part Component

// ErmaC BoardType 0x0a(10) or 0x0b(11) MacPro Family
#define kSMBBaseBoardTypeKey                        "SMboardtype"          // 10 (Motherboard) all model, 11 (Processor+Memory Module) MacPro

// Bungo
/* =======================
 System Enclosure (Type 3)
 ========================= */
#define kSMBSystemEnclosureManufacturerKey          "SMchassismanufacturer" // Apple Inc.
#define kSMBSystemEnclosureTypeKey                  "SMchassistype"         // 7 Desktop
#define kSMBSystemEnclosureVersionKey               "SMchassisversion"      // Mac-F42C88C8
#define kSMBSystemEnclosureSerialNumberKey          "SMchassisserial"       // Serial number
#define kSMBSystemEnclosureAssetTagKey              "SMchassisassettag"     // Pro-Enclosure  Bungo: renamed folowing convention

/* ============================
 Processor Information (Type 4)
 ============================== */
// Bungo
#define kSMBProcessorInformationSocketKey           "SMcpusocket"
#define kSMBProcessorInformationManufacturerKey     "SMcpumanufacturer"
#define kSMBProcessorInformationVersionKey          "SMcpuversion"
//
#define kSMBProcessorInformationExternalClockKey	"SMexternalclock"
#define kSMBProcessorInformationMaximumClockKey		"SMmaximalclock"
// Bungo
#define kSMBProcessorInformationCurrentClockKey     "SMcurrentclock"
#define kSMBProcessorInformationUpgradeKey          "SMcpuupgrade"
#define kSMBProcessorInformationSerialNumberKey     "SMcpuserial"
#define kSMBProcessorInformationAssetTagKey         "SMcpuassettag" // Bungo: renamed folowing convention
#define kSMBProcessorInformationPartNumberKey       "SMcpupartnumber"

/* =====================
 Memory Device (Type 17)
 ======================= */
#define kSMBMemoryDeviceDeviceLocatorKey            "SMmemdevloc"           //
#define kSMBMemoryDeviceBankLocatorKey              "SMmembankloc"          //
#define kSMBMemoryDeviceMemoryTypeKey               "SMmemtype"             //
#define kSMBMemoryDeviceMemorySpeedKey              "SMmemspeed"            //
#define kSMBMemoryDeviceManufacturerKey             "SMmemmanufacturer"     //
#define kSMBMemoryDeviceSerialNumberKey             "SMmemserial"           //
#define kSMBMemoryDevicePartNumberKey               "SMmempart"             //
// Bungo:
#define kSMBMemoryDeviceAssetTagKey                 "SMmemassettag"         //

/* ===========================================
 Memory SPD Data   (Apple Specific - Type 130)
 ============================================= */

/* ============================================
 OEM Processor Type (Apple Specific - Type 131)
 ============================================== */
#define kSMBOemProcessorTypeKey                     "SMoemcputype" // Bungo: renamed from SMcputype

/* =================================================
 OEM Processor Bus Speed (Apple Specific - Type 132)
 =================================================== */
#define kSMBOemProcessorBusSpeedKey                 "SMoemcpubusspeed" // Bungo: renamed from SMbusspeed

/* ==============================================
 OEM Platform Feature (Apple Specific - Type 133)
 ================================================ */
//#define kSMBOemPlatformFeatureKey

/* ==================================================*/
#define getFieldOffset(struct, field)	((uint8_t)(uint32_t)&(((struct *)0)->field))

typedef struct
{
	SMBStructHeader *orig;
	SMBStructHeader *new;
} SMBStructPtrs;

/* =======================
 BIOS Information (Type 0)
 ========================= */
typedef struct
{
	char *vendor;
	char *version;
	char *releaseDate;
	uint16_t release;     // Bungo
} defaultBIOSInfo_t;

defaultBIOSInfo_t defaultBIOSInfo;

/* =========================
 System Information (Type 1)
 =========================== */
typedef struct
{
	char *manufacturer;
	char *productName;
	char *version;
	char *serialNumber;
	char *skuNumber;	// ErmaC
	char *family;
} defaultSystemInfo_t;

defaultSystemInfo_t defaultSystemInfo;

/* =========================================
 Base Board (or Module) Information (Type 2)
 =========================================== */
typedef struct
{
	char *manufacturer;
	char *product;
	char *version;			// Bungo
	char *serialNumber;		// ErmaC
	char *assetTag;			// ErmaC Bungo: renamed folowing convention
	char *locationInChassis;	// ErmaC
	uint8_t boardType;		// ErmaC
} defaultBaseBoard_t;

defaultBaseBoard_t defaultBaseBoard;

// Bungo
typedef struct {
	char		*manufacturer;
	uint8_t		chassisType;
	char		*version;
	char		*serialNumber;
	char		*assetTag;   // Bungo: renamed folowing convention
	//char		*skuNumber;
} defaultChassis_t;

defaultChassis_t defaultChassis;

typedef struct
{
	uint8_t			type;
	SMBValueType		valueType;
	uint8_t			fieldOffset;
	char			*keyString;
	bool			(*getSMBValue)(returnType *);
	char			**defaultValue;
} SMBValueSetter;

SMBValueSetter SMBSetters[] = 
{
	/* =======================
	 BIOS Information (Type 0)
	 ========================= */
	{ kSMBTypeBIOSInformation, kSMBString, getFieldOffset(SMBBIOSInformation, vendor),
		kSMBBIOSInformationVendorKey, NULL, &defaultBIOSInfo.vendor }, // SMbiosvendor - Apple Inc.

	{ kSMBTypeBIOSInformation, kSMBString, getFieldOffset(SMBBIOSInformation, version),
		kSMBBIOSInformationVersionKey, NULL, &defaultBIOSInfo.version }, // SMbiosversion - MP31.88Z.006C.B05.0802291410

	{ kSMBTypeBIOSInformation, kSMBString, getFieldOffset(SMBBIOSInformation, releaseDate),
		kSMBBIOSInformationReleaseDateKey, NULL, &defaultBIOSInfo.releaseDate }, // SMbiosdate - 02/29/08

	// Bungo
	{ kSMBTypeBIOSInformation, kSMBWord, getFieldOffset(SMBBIOSInformation, releaseMajor),
		kSMBBIOSInformationReleaseKey, NULL,	(char **)&defaultBIOSInfo.release }, // SMbiosrelease - 0.1 (256)

	/* =========================
	 System Information (Type 1)
	 =========================== */
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, manufacturer),
		kSMBSystemInformationManufacturerKey, NULL,	&defaultSystemInfo.manufacturer	}, // SMmanufacturer - Apple Inc.

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, productName),
		kSMBSystemInformationProductNameKey, NULL, &defaultSystemInfo.productName }, // SMproductname - MacPro3,1

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, version),
		kSMBSystemInformationVersionKey, NULL, &defaultSystemInfo.version }, // SMsystemversion - 1.0

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, serialNumber),
		kSMBSystemInformationSerialNumberKey, NULL, &defaultSystemInfo.serialNumber }, // SMserial - Serial number
	/* Bungo:
	{kSMBTypeSystemInformation,	kSMBByte,	getFieldOffset(SMBSystemInformation, uuid),
		kSMBSystemInformationUUIDKey, NULL, NULL}, // SMsystemuuid

	{kSMBTypeSystemInformation,	kSMBByte,	getFieldOffset(SMBSystemInformation, wakeupReason),
		NULL, NULL, NULL}, // reason for system wakeup
	*/

	// Bungo
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, skuNumber),
		kSMBSystemInformationSKUNumberKey, NULL, &defaultSystemInfo.skuNumber}, // SMskunumber - System SKU#

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, family),
		kSMBSystemInformationFamilyKey,	NULL,	&defaultSystemInfo.family}, // SMfamily - MacPro


	/* =========================================
	 Base Board (or Module) Information (Type 2)
	 =========================================== */
	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, manufacturer),
		kSMBBaseBoardManufacturerKey, NULL, &defaultBaseBoard.manufacturer }, // SMboardmanufacturer - Apple Inc.

	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, product),
		kSMBBaseBoardProductKey, NULL, &defaultBaseBoard.product }, // SMboardproduct - Mac-F2268DC8

	// Bungo
	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, version),
		kSMBBaseBoardVersionKey, NULL, &defaultBaseBoard.version }, // SMboardversion - MacPro3,1

	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, serialNumber),
		kSMBBaseBoardSerialNumberKey, NULL, &defaultBaseBoard.serialNumber }, // SMboardserial - C02140302D5DMT31M

	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, assetTag),
		kSMBBaseBoardAssetTagKey, NULL, &defaultBaseBoard.assetTag }, // SMboardassetag - Base Board Asset Tag#

	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, locationInChassis),
		kSMBBaseBoardLocationInChassisKey, NULL, &defaultBaseBoard.locationInChassis }, // SMboardlocation - Part Component

	{kSMBTypeBaseBoard,	kSMBByte,	getFieldOffset(SMBBaseBoard, boardType),
		kSMBBaseBoardTypeKey,	NULL, (char **)&defaultBaseBoard.boardType }, // SMboardtype - 10 (Motherboard) all model, 11 (Processor+Memory Module) MacPro

/*	{kSMBTypeBaseBoard,	kSMBByte, getFieldOffset(SMBBaseBoard, numberOfContainedHandles),
		NULL , NULL, NULL }, // numberOfContainedHandles = 0
*/
	//

	// Bungo
	/* =======================
	 System Enclosure (Type 3)
	 ========================= */
	{kSMBTypeSystemEnclosure,	kSMBString,	getFieldOffset(SMBSystemEnclosure, manufacturer),
		kSMBSystemEnclosureManufacturerKey, NULL,	&defaultChassis.manufacturer }, // SMchassismanufacturer - Apple Inc.

	{kSMBTypeSystemEnclosure, kSMBByte,	getFieldOffset(SMBSystemEnclosure, chassisType),
		kSMBSystemEnclosureTypeKey, NULL, (char **)&defaultChassis.chassisType	}, // SMchassistype - 7

	{kSMBTypeSystemEnclosure, kSMBString, getFieldOffset(SMBSystemEnclosure, version),
		kSMBSystemEnclosureVersionKey, NULL, &defaultChassis.version }, // SMchassisversion - Mac-F42C88C8

	{kSMBTypeSystemEnclosure, kSMBString, getFieldOffset(SMBSystemEnclosure, serialNumber),
		kSMBSystemEnclosureSerialNumberKey, NULL, &defaultChassis.serialNumber }, // SMchassisserial

	{kSMBTypeSystemEnclosure, kSMBString, getFieldOffset(SMBSystemEnclosure, assetTag),
		kSMBSystemEnclosureAssetTagKey, NULL, &defaultChassis.assetTag }, // SMchassisassettag - Pro Enclosure

	/*
	{kSMBTypeSystemEnclosure, kSMBString, getFieldOffset(SMBSystemEnclosure, skuNumber),
		NULL, NULL, &defaultChassis.skuNumber },
	*/

	/* ============================
	 Processor Information (Type 4)
	 ============================== */
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, socketDesignation),
		kSMBProcessorInformationSocketKey, NULL, NULL}, // SMcpusocket -

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, manufacturer),
		kSMBProcessorInformationManufacturerKey, NULL, NULL}, // SMcpumanufacturer - Intel(R) Corporation

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, processorVersion),
		kSMBProcessorInformationVersionKey, NULL, NULL}, // SMcpuversion

	{kSMBTypeProcessorInformation,	kSMBWord, getFieldOffset(SMBProcessorInformation, externalClock),
		kSMBProcessorInformationExternalClockKey, getProcessorInformationExternalClock,	NULL}, // SMcpuexternalclock

	{kSMBTypeProcessorInformation,	kSMBWord, getFieldOffset(SMBProcessorInformation, maximumClock),
		kSMBProcessorInformationMaximumClockKey, getProcessorInformationMaximumClock,	NULL}, // SMcpumaximumclock
	// Bungo
	{kSMBTypeProcessorInformation,	kSMBWord,	getFieldOffset(SMBProcessorInformation, currentClock),
		kSMBProcessorInformationCurrentClockKey, NULL, NULL}, // SMcpucurrentclock

	{kSMBTypeProcessorInformation,	kSMBByte,	getFieldOffset(SMBProcessorInformation, processorUpgrade),
		kSMBProcessorInformationUpgradeKey, NULL, NULL}, // SMcpuupgrade
	//
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, serialNumber),
		kSMBProcessorInformationSerialNumberKey, NULL, NULL},

	// Bungo
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, assetTag),
		kSMBProcessorInformationAssetTagKey, NULL, NULL}, // SMcpuassettag
	//
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, partNumber),
		kSMBProcessorInformationPartNumberKey, NULL, NULL},

	/* =====================
	 Memory Device (Type 17)
	 ======================= */
	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, deviceLocator),
		kSMBMemoryDeviceDeviceLocatorKey, NULL, NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, bankLocator),
		kSMBMemoryDeviceBankLocatorKey, NULL, NULL},

	{kSMBTypeMemoryDevice,	kSMBByte,	getFieldOffset(SMBMemoryDevice, memoryType),
		kSMBMemoryDeviceMemoryTypeKey, getSMBMemoryDeviceMemoryType,	NULL},

	{kSMBTypeMemoryDevice,	kSMBWord,	getFieldOffset(SMBMemoryDevice, memorySpeed),
		kSMBMemoryDeviceMemorySpeedKey, getSMBMemoryDeviceMemorySpeed,	NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, manufacturer),
		kSMBMemoryDeviceManufacturerKey, getSMBMemoryDeviceManufacturer, NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, serialNumber),
		kSMBMemoryDeviceSerialNumberKey, getSMBMemoryDeviceSerialNumber, NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, assetTag),
		kSMBMemoryDeviceAssetTagKey, NULL, NULL},

	{kSMBTypeMemoryDevice,	kSMBWord,	getFieldOffset(SMBMemoryDevice, errorHandle),
		NULL, getSMBMemoryDeviceMemoryErrorHandle, NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, partNumber),
		kSMBMemoryDevicePartNumberKey, getSMBMemoryDevicePartNumber, NULL},

	//-------------------------------------------------------------------------------------------------------------------------
	// Apple Specific
	//-------------------------------------------------------------------------------------------------------------------------
	// OEM Processor Type (Apple Specific - Type 131)
	{kSMBTypeOemProcessorType,		kSMBWord,	getFieldOffset(SMBOemProcessorType, ProcessorType),			kSMBOemProcessorTypeKey,		
		getSMBOemProcessorType,			NULL},

	// OEM Processor Bus Speed (Apple Specific - Type 132)
	{kSMBTypeOemProcessorBusSpeed,	kSMBWord,	getFieldOffset(SMBOemProcessorBusSpeed, ProcessorBusSpeed),	kSMBOemProcessorBusSpeedKey,	
		getSMBOemProcessorBusSpeed,		NULL}

	// OEM Platform Feature (Apple Specific - Type 133)
	/*
	{kSMBTypeOemPlatformFeature,	kSMBWord,	getFieldOffset(SMBOemPlatformFeature, PlatformFeature),	kSMBOemPlatformFeatureKey,
		getSMBOemPlatformFeature,		NULL}
	*/
};

int numOfSetters = sizeof(SMBSetters) / sizeof(SMBValueSetter);


SMBEntryPoint *origeps			= 0;
SMBEntryPoint *neweps			= 0;

static uint8_t stringIndex;	// increament when a string is added and set the field value accordingly
static uint8_t stringsSize;	// add string size

static SMBWord tableLength	= 0;
static SMBWord handle		= 0;
static SMBWord maxStructSize	= 0;
static SMBWord structureCount	= 0;

//-------------------------------------------------------------------------------------------------------------------------
// Default SMBIOS Data
//-------------------------------------------------------------------------------------------------------------------------
/* Rewrite: use a struct */

// Bungo: suggest to not mixing data from different Mac models, use real Mac SMBIOS dumps

#define kDefaultVendorManufacturer			"Apple Inc."
//#define kDefaultBIOSReleaseDate			"11/06/2009"
#define kDefaultSerialNumber				"SOMESRLNMBR"
//Bungo
#define kDefaultSkuNumber				"Default SKU#"
#define kDefaultAssetTag				"Default Asset Tag#"
//#define kDefaultBoardType				"10" // 0xA
//#define kDefaultBoardProcessorType			"11" // 0xB
#define kDefaultSystemVersion				"1.0"
#define kDefaultBIOSRelease				256 // 256 = 0x0100 -> swap bytes: 0x0001 -> Release: 0.1 (see SMBIOS spec. table Type 0)
//#define kDefaultLocatioInChassis			"Part Component"
//#define KDefaultBoardSerialNumber			"C02140302D5DMT31M" // new C07019501PLDCVHAD - C02032101R5DC771H

//=========== Mac mini ===========
#define kDefaultMacMiniFamily				"Mac mini"
//#define kDefaultMacMiniBoardAssetTagNumber		"Mini-Aluminum"

#define kDefaultMacMini					"Macmini2,1"
#define kDefaultMacMiniBIOSVersion			"    MM21.88Z.009A.B00.0706281359"
#define kDefaultMacMiniBIOSReleaseDate			"06/28/07"
#define kDefaultMacMiniBoardProduct			"Mac-F4208EAA"

// MacMini5,1 Mac-8ED6AF5B48C039E1 - MM51.88Z.0077.B0F.1110201309
// MacMini5,2 Mac-4BC72D62AD45599E
// MacMini5,3
//#define kDefaultMacMini				"Macmini5,3"
//#define kDefaultMacMiniBIOSVersion			"    MM51.88Z.0077.B10.1201241549"
//#define kDefaultMacMiniBoardProduct			"Mac-F65AE981FFA204ED"
//#define kDefaultMacMiniBIOSReleaseDate		"01/24/2012"

// MacMini 6,1 - Mac-F65AE981FFA204ED
// MacMini 6,2
//#define kDefaultMacMini62				"Macmini6,2"
//#define kDefaultMacMini62BIOSVersion			"    MM61.88Z.0106.B00.1208091121"
//#define kDefaultMacMini62BoardProduct			"Mac-F65AE981FFA204ED"
//#define kDefaultMacMini62BIOSReleaseDate		"10/14/2012"

//=========== MacBook ===========
#define kDefaultMacBookFamily				"MacBook"
//#define kDefaultMacBookBoardAssetTagNumber		"MacBook-Black"

#define kDefaultMacBook					"MacBook4,1"
#define kDefaultMacBookBIOSVersion			"    MB41.88Z.00C1.B00.0802091535"
#define kDefaultMacBookBIOSReleaseDate			"02/09/08"
#define kDefaultMacBookBoardProduct			"Mac-F22788A9"

//=========== MacBookAir ===========
#define kDefaultMacBookAirFamily			"MacBook Air"

// MacBookAir4,1 - Mac-C08A6BB70A942AC2
// MacBookAir4,2 - Mac-742912EFDBEE19B3
#define kDefaultMacBookAir				"MacBookAir5,2"
#define kDefaultMacBookAirBIOSVersion			"    MBA51.88Z.00EF.B00.1205221442"
#define kDefaultMacBookAirBIOSReleaseDate		"05/10/12"
#define kDefaultMacBookBoardAirProduct			"Mac-2E6FAB96566FE58C"

// MacBookAir6,1 - Mac-35C1E88140C3E6CF - MBA61.88Z.0099.B04.1309271229
// MacBookAir6,2 - Mac-7DF21CB3ED6977E5 - MBA62.88Z.00EF.B00.1205221442

//=========== MacBookPro ===========
#define kDefaultMacBookProFamily			"MacBook Pro"
//#define kDefaultMacBookProBoardAssetTagNumber		"MacBook-Aluminum"

#define kDefaultMacBookPro				"MacBookPro4,1"
#define kDefaultMacBookProBIOSVersion			"    MBP41.88Z.00C1.B03.0802271651"
#define kDefaultMacBookProBIOSReleaseDate		"02/27/08"
#define kDefaultMacBookProBoardProduct			"Mac-F42C89C8"

//#define kDefaultMacBookPro				"MacBookPro8,1"
//#define kDefaultMacBookProBIOSVersion			"    MBP81.88Z.0047.B24.1110141131"
//#define kDefaultMacBookProBoardProduct		"Mac-94245B3640C91C81"
//#define kDefaultMacBookProBIOSReleaseDate		"10/14/11"

// MacBookPro8,2 - Mac_94245A3940C91C80
// MacBookPro8,3 - Mac-942459F5819B171B

// MacBookPro10,2
//#define kDefaultMacBookProIvy				"MacBookPro10,2"
//#define kDefaultMacBookProIvyBIOSVersion		"    MBP102.88Z.0106.B01.1208311637"
//#define kDefaultMacBookProIvyBoardProduct		"Mac-AFD8A9D944EA4843"
//#define kDefaultMacBookProIvyBIOSReleaseDate		"10/02/2012"

// MacBookPro11,2 - Mac-3CBD00234E554E41 - MBP112.88Z.0138.B03.1310291227
// MacBookPro11,3 - Mac-2BD1B31983FE1663 - MBP112.88Z.0138.B02.1310181745

//=========== iMac ===========
#define kDefaultiMacFamily				"iMac"
//#define kDefaultiMacBoardAssetTagNumber		"iMac-Aluminum"

#define kDefaultiMac					"iMac8,1"
#define kDefaultiMacBIOSVersion				"    IM81.88Z.00C1.B00.0903051113"
#define kDefaultiMacBIOSReleaseDate			"02/09/08"
#define kDefaultiMacBoardProduct			"Mac-F227BEC8"

// iMac10,1
// iMac11,1 core i3/i5/i7
#define kDefaultiMacNehalem				"iMac11,1"
#define kDefaultiMacNehalemBIOSVersion			"    IM111.88Z.0034.B02.1003171314"
#define kDefaultiMacNehalemBIOSReleaseDate		"03/30/10"
#define kDefaultiMacNehalemBoardProduct			"Mac-F2268DAE"
// iMac11,2
// iMac11,3

// iMac12,1
#define kDefaultiMacSandy				"iMac12,1"
#define kDefaultiMacSandyBIOSVersion			"    IM121.88Z.0047.B00.1102091756"
#define kDefaultiMacSandyBIOSReleaseDate		"01/02/08"
#define kDefaultiMacSandyBoardProduct			"Mac-942B5BF58194151B"

// iMac12,2 Mac-942B59F58194171B
//#define kDefaultiMacSandy				"iMac12,2"
//#define kDefaultiMacSandyBIOSVersion			"    IM121.88Z.0047.B1D.1110171110"
//#define kDefaultiMacSandyBIOSReleaseDate		"10/17/11"
//#define kDefaultiMacSandyBoardProduct			"Mac-942B59F58194171B"

// iMac13,1
// Bios: IM131.88Z.010A.B05.1211151146
// Data: 11/15/2012
// Board: Mac-00BE6ED71E35EB86

// iMac13,2
//#define kDefaultiMacIvy				"iMac13,2"
//#define kDefaultiMacIvyBIOSVersion			"    IM131.88Z.00CE.B00.1203281326"
//#define kDefaultiMacIvyBIOSReleaseDate		"03/28/2012"
//#define kDefaultiMacIvyBoardProduct			"Mac-FC02E91DDD3FA6A4"

//=========== MacPro ===========
#define kDefaultMacProFamily				"Mac Pro"
//#define KDefauktMacProBoardAssetTagNumber		"Pro-Enclosure"
//#define kDefaultMacProBoardType			"0xB" // 11

#define kDefaultMacPro					"MacPro3,1"
#define kDefaultMacProBIOSVersion			"    MP31.88Z.006C.B05.0903051113"
#define kDefaultMacProBIOSReleaseDate			"08/03/2010"
//#define kDefaultMacProSystemVersion			"1.3"
#define kDefaultMacProBoardProduct			"Mac-F42C88C8"
//#define KDefaultMacProBoardSerialNumber		"J593902RA4MFE"

// Mac Pro 4,1 core i7/Xeon
#define kDefaultMacProNehalem				"MacPro4,1"
#define kDefaultMacProNehalemBIOSVersion		"    MP41.88Z.0081.B07.0910130729"
#define kDefaultMacProNehalemBIOSReleaseDate		"10/13/09"
//#define kDefaultMacProNehalemSystemVersion		"1.4"
#define kDefaultMacProNehalemBoardProduct		"Mac-F221BEC8"
//#define KDefaultMacProNehalemBoardSerialNumber	"J593004RB1LUE"

// Mac Pro 5,1 core i7/Xeon
#define kDefaultMacProWestmere				"MacPro5,1"
#define kDefaultMacProWestmereBIOSVersion		"    MP51.88Z.007F.B03.1010071432"
#define kDefaultMacProWestmereBIOSReleaseDate		"10/07/10"
//#define kDefaultMacProWestmereSystemVersion		"1.2"
#define kDefaultMacProWestmereBoardProduct		"Mac-F221BEC8"
//#define KDefaultMacProWestmereBoardSerialNumber	"J522700H7BH8C"

// Mac Pro 6,1
#define kDefaultMacProHaswell				"MacPro6,1"
#define kDefaultMacProHaswellBIOSVersion		"    MP61.88Z.0116.B04.1312061508"
#define kDefaultMacProHaswellBIOSReleaseDate		"12/06/2013"
//#define kDefaultMacProHaswellSystemVersion		"1.?"
#define kDefaultMacProHaswellBoardProduct		"Mac-F60DEB81FF30ACF6"
//#define KDefaultMacProHaswellBoardSerialNumber	"?????????????"

//#define KDefaultBoardSerialNumber			"C02140302D5DMT31M" // new C07019501PLDCVHAD - C02032101R5DC771H
// J593902RA4MFE 3,1
// J5031046RCZJA 5,1
// J521101A5CZJC 3,1
// J593004RB1LUE MacPro4,1
// J513401PZBH8C 5,1
// J590802LC4ACB 3,1
// J594900AH1LUE 4,1
// J512500HZBH8C 5,1
// J522700H7BH8C MacPro5,1

/* ============================================ */

bool   useSMBIOSdefaults        = true;  // Bungo

SMBByte PlatformType			= 3;  // Bungo: same as Platfom.Type in platform.h

/* Rewrite this function */
void setDefaultSMBData(void)  // Bungo: setting data from real Macs
{
	defaultBIOSInfo.vendor              = kDefaultVendorManufacturer;
	defaultBIOSInfo.release             = kDefaultBIOSRelease;    // Bungo

	defaultSystemInfo.manufacturer      = kDefaultVendorManufacturer;
	defaultSystemInfo.version           = kDefaultSystemVersion;
	defaultSystemInfo.serialNumber      = kDefaultSerialNumber;
	defaultSystemInfo.skuNumber         = kDefaultSkuNumber;      // Bungo

	defaultBaseBoard.manufacturer       = kDefaultVendorManufacturer;
	defaultBaseBoard.serialNumber       = kDefaultSerialNumber;
	defaultBaseBoard.assetTag           = kDefaultAssetTag;

	defaultChassis.manufacturer         = kDefaultVendorManufacturer;
	defaultChassis.serialNumber         = kDefaultSerialNumber;
	defaultChassis.assetTag             = kDefaultAssetTag;
//	defaultChassis.skuNumber            = kDefaultSkuNumber;

	// if (platformCPUFeature(CPU_FEATURE_MOBILE)) Bungo: doesn't recognise correctly, need fixing
	if (PlatformType == 2)  // this method works but it's a substitute
	{
		if (Platform.CPU.NoCores > 1) {
			defaultSystemInfo.productName    = kDefaultMacBookPro;
			defaultBIOSInfo.version          = kDefaultMacBookProBIOSVersion;
			defaultBIOSInfo.releaseDate      = kDefaultMacBookProBIOSReleaseDate;
			defaultSystemInfo.family         = kDefaultMacBookProFamily;
			defaultBaseBoard.product         = kDefaultMacBookProBoardProduct;
			defaultBaseBoard.boardType       = kSMBBaseBoardMotherboard;
			defaultChassis.chassisType       = kSMBchassisUnknown;
		} else {
			defaultSystemInfo.productName    = kDefaultMacBook;
			defaultBIOSInfo.version          = kDefaultMacBookBIOSVersion;
			defaultBIOSInfo.releaseDate      = kDefaultMacBookBIOSReleaseDate;
			defaultSystemInfo.family         = kDefaultMacBookFamily;
			defaultBaseBoard.product         = kDefaultMacBookBoardProduct;
			defaultBaseBoard.boardType       = kSMBBaseBoardMotherboard;
			defaultChassis.chassisType       = kSMBchassisUnknown;
		}
	} else {
		switch (Platform.CPU.NoCores)
		{
			case 1:
				defaultBIOSInfo.version         = kDefaultMacMiniBIOSVersion;
				defaultBIOSInfo.releaseDate     = kDefaultMacMiniBIOSReleaseDate;
				defaultSystemInfo.productName   = kDefaultMacMini;
				defaultSystemInfo.family        = kDefaultMacMiniFamily;
				defaultBaseBoard.product        = kDefaultMacMiniBoardProduct;
				defaultBaseBoard.boardType      = kSMBBaseBoardUnknown;
				defaultChassis.chassisType      = kSMBchassisLPDesktop;
				break;

			case 2:
				defaultBIOSInfo.version         = kDefaultiMacBIOSVersion;
				defaultBIOSInfo.releaseDate     = kDefaultiMacBIOSReleaseDate;
				defaultSystemInfo.productName   = kDefaultiMac;
				defaultSystemInfo.family        = kDefaultiMacFamily;
				defaultBaseBoard.product        = kDefaultiMacBoardProduct;
				defaultBaseBoard.boardType      = kSMBBaseBoardMotherboard;
				defaultChassis.chassisType      = kSMBchassisAllInOne;
				break;
			default:
			{
				switch (Platform.CPU.Family)
				{
					case 0x06:
					{
						switch (Platform.CPU.Model)
						{
							case CPU_MODEL_FIELDS:			// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
							case CPU_MODEL_DALES:
							case CPU_MODEL_DALES_32NM:		// Intel Core i3, i5 LGA1156 (32nm)
								defaultBIOSInfo.version			= kDefaultiMacNehalemBIOSVersion;
								defaultBIOSInfo.releaseDate		= kDefaultiMacNehalemBIOSReleaseDate;
								defaultSystemInfo.productName	= kDefaultiMacNehalem;
								defaultSystemInfo.family		= kDefaultiMacFamily;
								defaultBaseBoard.product        = kDefaultiMacNehalemBoardProduct;
								defaultBaseBoard.boardType      = kSMBBaseBoardMotherboard;
								defaultChassis.chassisType      = kSMBchassisAllInOne;
								break;

							case CPU_MODEL_SANDYBRIDGE:			// Intel Core i3, i5, i7 LGA1155 (32nm)
							case CPU_MODEL_IVYBRIDGE:			// Intel Core i3, i5, i7 LGA1155 (22nm)
							case CPU_MODEL_IVYBRIDGE_XEON:
								defaultBIOSInfo.version         = kDefaultiMacSandyBIOSVersion;
								defaultBIOSInfo.releaseDate     = kDefaultiMacSandyBIOSReleaseDate;
								defaultSystemInfo.productName	= kDefaultiMacSandy;
								defaultSystemInfo.family        = kDefaultiMacFamily;
								defaultBaseBoard.product        = kDefaultiMacSandyBoardProduct;
								defaultBaseBoard.boardType      = kSMBBaseBoardMotherboard;
								defaultChassis.chassisType      = kSMBchassisAllInOne;
								break;

							case CPU_MODEL_NEHALEM:			// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
							case CPU_MODEL_NEHALEM_EX:		// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65x
								defaultBIOSInfo.version		= kDefaultMacProNehalemBIOSVersion;
								defaultBIOSInfo.releaseDate	= kDefaultMacProNehalemBIOSReleaseDate;
								defaultSystemInfo.productName	= kDefaultMacProNehalem;
								defaultSystemInfo.family	= kDefaultMacProFamily;
								defaultBaseBoard.product        = kDefaultMacProNehalemBoardProduct;
								defaultBaseBoard.boardType      = kSMBBaseBoardProcessorMemoryModule;
								defaultChassis.chassisType      = kSMBchassisTower;
								break;

							case CPU_MODEL_WESTMERE:		// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
							case CPU_MODEL_WESTMERE_EX:		// Intel Xeon E7
							case CPU_MODEL_JAKETOWN:		// Intel Core i7, Xeon E5 LGA2011 (32nm)
								defaultBIOSInfo.version		= kDefaultMacProWestmereBIOSVersion;
								defaultBIOSInfo.releaseDate	= kDefaultMacProWestmereBIOSReleaseDate;
								defaultSystemInfo.productName	= kDefaultMacProWestmere;
								defaultSystemInfo.family	= kDefaultMacProFamily;
								defaultBaseBoard.product        = kDefaultMacProWestmereBoardProduct;
								defaultBaseBoard.boardType      = kSMBBaseBoardProcessorMemoryModule;
								defaultChassis.chassisType      = kSMBchassisTower;
								break;

							default:
								defaultBIOSInfo.version		= kDefaultMacProBIOSVersion;
								defaultBIOSInfo.releaseDate	= kDefaultMacProBIOSReleaseDate;
								defaultSystemInfo.productName	= kDefaultMacPro;
								defaultSystemInfo.family	= kDefaultMacProFamily;
								defaultBaseBoard.product        = kDefaultMacProBoardProduct;
								defaultBaseBoard.boardType      = kSMBBaseBoardMotherboard;
								defaultChassis.chassisType      = kSMBchassisUnknown;
								break;
						}
						break;
					}
					default:
						defaultBIOSInfo.version		= kDefaultMacProBIOSVersion;
						defaultBIOSInfo.releaseDate	= kDefaultMacProBIOSReleaseDate;
						defaultSystemInfo.productName	= kDefaultMacPro;
						defaultSystemInfo.family	= kDefaultMacProFamily;
						defaultBaseBoard.product        = kDefaultMacProBoardProduct;
						defaultBaseBoard.boardType      = kSMBBaseBoardMotherboard;
						defaultChassis.chassisType      = kSMBchassisUnknown;
						break;
				}
				break;
			}
		}
	}
}

/* Used for SM*n smbios.plist keys */
bool getSMBValueForKey(SMBStructHeader *structHeader, const char *keyString, const char **string, returnType *value)
{
	static int idx = -1;
	static int current = -1;
	int len;
	char key[24];

	if (current != structHeader->handle) {
		idx++;
		current = structHeader->handle;
	}

	sprintf(key, "%s%d", keyString, idx);

	if (value) {
		if (getIntForKey(key, (int *)&(value->dword), SMBPlist)) {
			return true;
		}
	} else {
		if (getValueForKey(key, string, &len, SMBPlist)) {
			return true;
		}
	}
	return false;
}

char *getSMBStringForField(SMBStructHeader *structHeader, uint8_t field)
{
	uint8_t *stringPtr = (uint8_t *)structHeader + structHeader->length;

	if (!field) {
		return NULL;
	}

	for (field--; field != 0 && strlen((char *)stringPtr) > 0;
		field--, stringPtr = (uint8_t *)((uint32_t)stringPtr + strlen((char *)stringPtr) + 1));

	return (char *)stringPtr;
}

void setSMBStringForField(SMBStructHeader *structHeader, const char *string, uint8_t *field)
{
	int strSize;

	if (!field) {
		return;
	}

	if (!string) {
		*field = 0;
		return;
	}

	strSize = strlen(string);

	// remove any spaces found at the end but only in MemoryDevice
	if (structHeader->type == kSMBTypeMemoryDevice) {
		while ((strSize != 0) && (string[strSize - 1] == ' ')) {
			strSize--;
		}
	}

	if (strSize == 0) {
		*field = 0;
		return;
	}

	memcpy((uint8_t *)structHeader + structHeader->length + stringsSize, string, strSize);
	*field = stringIndex;

	stringIndex++;
	stringsSize += strSize + 1;
}

bool setSMBValue(SMBStructPtrs *structPtr, int idx, returnType *value)
{
	const char *string = 0;
	int len;
	bool parsed;
	int val;

	if (numOfSetters <= idx) {
		return false;
	}

	switch (SMBSetters[idx].valueType) {
		case kSMBString:
		{
			if (SMBSetters[idx].keyString)
			{
				if (getValueForKey(SMBSetters[idx].keyString, &string, &len, SMBPlist))
				{
					break;
				} else {
					if (structPtr->orig->type == kSMBTypeMemoryDevice)	// MemoryDevice only
					{
						if (getSMBValueForKey(structPtr->orig, SMBSetters[idx].keyString, &string, NULL))
						{
							break;
						}
					}
				}

			}
			if (SMBSetters[idx].getSMBValue) {
				if (SMBSetters[idx].getSMBValue((returnType *)&string)) {
					break;
				}
			}
			// if ((SMBSetters[idx].defaultValue) && *(SMBSetters[idx].defaultValue))  Bungo
			if (useSMBIOSdefaults && SMBSetters[idx].defaultValue && *(SMBSetters[idx].defaultValue)) {
				string = *(SMBSetters[idx].defaultValue);
				break;
			}
			string = getSMBStringForField(structPtr->orig, *(uint8_t *)value);
			break;
		}
		case kSMBByte:
		case kSMBWord:
		case kSMBDWord:
		//case kSMBQWord:
			if (SMBSetters[idx].keyString) 	{
				parsed = getIntForKey(SMBSetters[idx].keyString, &val, SMBPlist);
				if (!parsed)
				{
					if (structPtr->orig->type == kSMBTypeMemoryDevice) { // MemoryDevice only
						parsed = getSMBValueForKey(structPtr->orig, SMBSetters[idx].keyString, NULL, (returnType *)&val);
					}
				}
				if (parsed) {
					switch (SMBSetters[idx].valueType) {
						case kSMBByte:
							value->byte = (uint8_t)val;
							break;
						case kSMBWord:
							value->word = (uint16_t)val;
							break;
						//case kSMBQWord:
						//	value->qword = (uint64_t)val;
						//	break;
						case kSMBDWord:
						default:
							value->dword = (uint32_t)val;
							break;
					}
					return true;
				}
			}

			if (SMBSetters[idx].getSMBValue) {
				if (SMBSetters[idx].getSMBValue(value)) {
					return true;
				}
			}
// #if 0  Bungo: enables code below
			// if (*(SMBSetters[idx].defaultValue))  Bungo
			if (useSMBIOSdefaults && SMBSetters[idx].defaultValue && *(SMBSetters[idx].defaultValue)) {
			// value->dword = *(uint32_t *)(SMBSetters[idx].defaultValue);  Bungo
			switch (SMBSetters[idx].valueType) {
				case kSMBByte:
					value->byte = *(uint8_t *)(SMBSetters[idx].defaultValue);
					break;
				case kSMBWord:
					value->word = *(uint16_t *)(SMBSetters[idx].defaultValue);
					break;
				//case kSMBQWord:
				//	value->qword = *(uint32_t *)(SMBSetters[idx].defaultValue);
				//	break;
				case kSMBDWord:
				default:
					value->dword = *(uint32_t *)(SMBSetters[idx].defaultValue);
					break;
			}
			return true;
		}
// #endif  Bungo
		break;
	}

	// if (SMBSetters[idx].valueType == kSMBString && string)  Bungo: use null string too -> "Not Specified"
	if ((SMBSetters[idx].valueType == kSMBString) && string) {
		setSMBStringForField(structPtr->new, string, &value->byte);
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific
//-------------------------------------------------------------------------------------------------------------------------

/* ===========================================
 Firmware Volume   (Apple Specific - Type 128)
 ============================================= */
void addSMBFirmwareVolume(SMBStructPtrs *structPtr)
{
	return;
}

/* ===========================================
 Memory SPD Data   (Apple Specific - Type 130)
 ============================================= */
void addSMBMemorySPD(SMBStructPtrs *structPtr)
{
	/* SPD data from Platform.RAM.spd */
	return;
}

/* ============================================
 OEM Processor Type (Apple Specific - Type 131)
 ============================================== */
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

/* =================================================
 OEM Processor Bus Speed (Apple Specific - Type 132)
 =================================================== */
void addSMBOemProcessorBusSpeed(SMBStructPtrs *structPtr)
{
	SMBOemProcessorBusSpeed *p = (SMBOemProcessorBusSpeed *)structPtr->new;

	switch (Platform.CPU.Family) 
	{
		case 0x06:
		{
			switch (Platform.CPU.Model)
			{
				case 0x19:			// Intel Core i5 650 @3.20 Ghz
				case CPU_MODEL_FIELDS:		// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
				case CPU_MODEL_DALES:
				case CPU_MODEL_DALES_32NM:	// Intel Core i3, i5 LGA1156 (32nm)
				case CPU_MODEL_NEHALEM:		// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
				case CPU_MODEL_NEHALEM_EX:	// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65x
				case CPU_MODEL_WESTMERE:	// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
				case CPU_MODEL_WESTMERE_EX:	// Intel Xeon E7
				case CPU_MODEL_SANDYBRIDGE:	// Intel Core i3, i5, i7 LGA1155 (32nm)
				case CPU_MODEL_IVYBRIDGE:	// Intel Core i3, i5, i7 LGA1155 (22nm)
				case CPU_MODEL_IVYBRIDGE_XEON:
				case CPU_MODEL_JAKETOWN:	// Intel Core i7, Xeon E5 LGA2011 (32nm)
				case CPU_MODEL_HASWELL:
				case CPU_MODEL_HASWELL_SVR:
				case CPU_MODEL_HASWELL_ULT:
				case CPU_MODEL_CRYSTALWELL:

					break;

				default:
					return;
			}
		}
	}

	p->header.type		= kSMBTypeOemProcessorBusSpeed;
	p->header.length	= sizeof(SMBOemProcessorBusSpeed);
	p->header.handle	= handle++;

	setSMBValue(structPtr, numOfSetters -1, (returnType *)&(p->ProcessorBusSpeed));

	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + sizeof(SMBOemProcessorBusSpeed) + 2);
	tableLength += sizeof(SMBOemProcessorBusSpeed) + 2;
	structureCount++;
}

/* ==============================================
 OEM Platform Feature (Apple Specific - Type 133)
 ================================================ */
 /*void addSMBOemPlatformFeature(SMBStructPtrs *structPtr) { }*/

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

	/* Bungo: not needed because of tables lengths fix in next lines
	// http://forge.voodooprojects.org/p/chameleon/issues/361/
	bool forceFullMemInfo = false;

	if (structPtr->orig->type == kSMBTypeMemoryDevice) {
		getBoolForKey(kMemFullInfo, &forceFullMemInfo, &bootInfo->chameleonConfig);
		if (forceFullMemInfo) {
			structPtr->orig->length = 27;
		}
	}*/

	stringIndex = 1;
	stringsSize = 0;

	if (handle < structPtr->orig->handle) {
		handle = structPtr->orig->handle;
	}
	// Bungo: fix unsuported tables lengths from original smbios: extend smaller or truncate bigger - we use SMBIOS rev. 2.4 like Apple uses
	switch (structPtr->orig->type) {
		case kSMBTypeBIOSInformation:
			structSize = sizeof(SMBBIOSInformation);
			break;
		case kSMBTypeSystemInformation:
			structSize = sizeof(SMBSystemInformation);
			break;
		case kSMBTypeBaseBoard:
			structSize = sizeof(SMBBaseBoard);
			break;
		case kSMBTypeSystemEnclosure:
			structSize = sizeof(SMBSystemEnclosure);
			break;
		case kSMBTypeProcessorInformation:
			structSize = sizeof(SMBProcessorInformation);
			break;
		case kSMBTypeMemoryDevice:
			structSize = sizeof(SMBMemoryDevice);
			break;
		default:
			structSize = structPtr->orig->length; // don't change if not to patch
			break;
	}

	// memcpy((void *)structPtr->new, structPtr->orig, structPtr->orig->length);
	if (structPtr->orig->length <= structSize) {
		memcpy((void *)structPtr->new, structPtr->orig, structPtr->orig->length);
	} else {
		memcpy((void *)structPtr->new, structPtr->orig, structSize);
	}

	structPtr->new->length = structSize;

	for (i = 0; i < numOfSetters; i++) {
		// Bungo:
		//if ((structPtr->orig->type == SMBSetters[i].type) && (SMBSetters[i].fieldOffset < structPtr->orig->length)) {
		if ((structPtr->orig->type == SMBSetters[i].type) && (SMBSetters[i].fieldOffset < structSize)) {
			setterFound = true;
			setSMBValue(structPtr, i, (returnType *)((uint8_t *)structPtr->new + SMBSetters[i].fieldOffset));
		}
	}

	if (setterFound) {
		// Bungo:
		// ptr = (uint8_t *)structPtr->new + structPtr->orig->length;
		ptr = (uint8_t *)structPtr->new + structPtr->new->length;
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0) {
			ptr += 2;
		}
		structSize = ptr - (uint8_t *)structPtr->new;
	} else {
		ptr = (uint8_t *)structPtr->orig + structPtr->orig->length;
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0) {
			ptr += 2;
		}

		structSize = ptr - (uint8_t *)structPtr->orig;
		memcpy((void *)structPtr->new, structPtr->orig, structSize);
	}

	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + structSize);

	tableLength += structSize;

	if (structSize > maxStructSize) {
		maxStructSize = structSize;
	}

	structureCount++;
}

void setupNewSMBIOSTable(SMBEntryPoint *eps, SMBStructPtrs *structPtr)
{
	uint8_t *ptr = (uint8_t *)eps->dmi.tableAddress;
	structPtr->orig = (SMBStructHeader *)ptr;

	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structPtr->orig + sizeof(SMBStructHeader)));) {
		switch (structPtr->orig->type) {
			/* Skip all Apple Specific Structures */
			case kSMBTypeFirmwareVolume:
			case kSMBTypeMemorySPD:
			case kSMBTypeOemProcessorType:
			case kSMBTypeOemProcessorBusSpeed:
				/* And this one too, to be added at the end */
			case kSMBTypeEndOfTable:
				break;

			default:
			{
				/* Add */
				setSMBStruct(structPtr);
				break;
			}
		}

		ptr = (uint8_t *)((uint32_t)structPtr->orig + structPtr->orig->length);
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0) {
			ptr += 2;
		}

		structPtr->orig = (SMBStructHeader *)ptr;
	}

	addSMBFirmwareVolume(structPtr);
	addSMBMemorySPD(structPtr);
	addSMBOemProcessorType(structPtr);
	addSMBOemProcessorBusSpeed(structPtr);

	addSMBEndOfTable(structPtr);
}

// Bungo: does fix system uuid in SMBIOS (and EFI) instead of in EFI only
uint8_t *FixSystemUUID()
{
	uint8_t *ptr = (uint8_t *)neweps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)ptr;
	int i, isZero, isOnes;
	uint8_t FixedUUID[UUID_LEN] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
	const char *sysId = getStringForKey(kSMBSystemInformationUUIDKey, SMBPlist);
	uint8_t *ret = (uint8_t *)getUUIDFromString(sysId);

	for (;(structHeader->type != kSMBTypeSystemInformation);) // find System Information Table (Type 1) in patched SMBIOS
	{
		ptr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0) {
			ptr += 2;
		}

		structHeader = (SMBStructHeader *)ptr;
	}

	ptr = ((SMBSystemInformation *)structHeader)->uuid;

	if (!sysId || !ret) { // no or bad custom UUID,...
		sysId = 0;
		ret = Platform.UUID; // ...try bios dmi system uuid extraction
	}

	for (i=0, isZero=1, isOnes=1; i<UUID_LEN; i++) // check if empty or setable, means: no uuid present
	{
		if (ret[i] != 0x00) {
			isZero = 0;
		}

		if (ret[i] != 0xff) {
			isOnes = 0;
		}
	}

	if (isZero || isOnes)  { // if empty or setable...
		verbose("No UUID present in SMBIOS System Information Table\n");
		ret = FixedUUID; // ...set a fixed value for system-id = 000102030405060708090A0B0C0D0E0F
	}

	memcpy(ptr, ret, UUID_LEN); // fix uuid in the table
	return ptr;
}  // Bungo: end fix

void setupSMBIOSTable(void)
{
	SMBStructPtrs *structPtr;
	uint8_t *buffer;
	// bool setSMB = true; Bungo: now we use useSMBIOSdefaults

	if (!origeps) {
		return;
	}

	neweps = origeps;

	structPtr = (SMBStructPtrs *)malloc(sizeof(SMBStructPtrs));
	if (!structPtr) {
		return;
	}
	
	buffer = (uint8_t *)malloc(SMB_ALLOC_SIZE);
	if (!buffer) {
		free(structPtr);
		return;
	}

	bzero(buffer, SMB_ALLOC_SIZE);
	structPtr->new = (SMBStructHeader *)buffer;

	// getBoolForKey(kSMBIOSdefaults, &setSMB, &bootInfo->chameleonConfig);  Bungo
	getBoolForKey(kSMBIOSdefaults, &useSMBIOSdefaults, &bootInfo->chameleonConfig);
	// if (setSMB)  Bungo
	setDefaultSMBData();

	setupNewSMBIOSTable(origeps, structPtr);

	neweps = (SMBEntryPoint *)AllocateKernelMemory(sizeof(SMBEntryPoint));
	if (!neweps) {
		free(buffer);
		free(structPtr);
		return;
	}
	bzero(neweps, sizeof(SMBEntryPoint));

	neweps->anchor[0]			= '_';
	neweps->anchor[1]			= 'S';
	neweps->anchor[2]			= 'M';
	neweps->anchor[3]			= '_';
	neweps->entryPointLength	= sizeof(SMBEntryPoint);
	neweps->majorVersion		= 2;                    // Bungo:
	neweps->minorVersion		= 4;                    // Here we're using 2.4 SMBIOS rev. as real Macs
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
	neweps->dmi.bcdRevision		= 0x24;                 // ... and 2.4 DMI rev. as real Macs

	if (!neweps->dmi.tableAddress) {
		free(buffer);
		free(structPtr);
		return;
	}

	memcpy((void *)neweps->dmi.tableAddress, buffer, tableLength);

	Platform.UUID = FixSystemUUID(); // Bungo: fix System UUID

	neweps->dmi.checksum		= 0;
	neweps->dmi.checksum		= 0x100 - checksum8(&neweps->dmi, sizeof(DMIEntryPoint));

	neweps->checksum		= 0;
	neweps->checksum		= 0x100 - checksum8(neweps, sizeof(SMBEntryPoint));

	free(buffer);
	free(structPtr);

	decodeSMBIOSTable(neweps);

	DBG("SMBIOS orig was = %x\n", origeps);
	DBG("SMBIOS new is = %x\n", neweps);
}

void *getSmbios(int which)
{
	switch (which) {
		case SMBIOS_ORIGINAL:
			if (!origeps) {
				origeps = getAddressOfSmbiosTable();
			}
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
	Platform.DMI.MaxMemorySlots	= 0;	// number of memory slots polulated by SMBIOS
	Platform.DMI.CntMemorySlots	= 0;	// number of memory slots counted
	Platform.DMI.MemoryModules	= 0;

	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		switch (structHeader->type)
		{
			case kSMBTypeSystemInformation:
				Platform.UUID = ((SMBSystemInformation *)structHeader)->uuid; // get factory system uuid
				break;

			case kSMBTypeSystemEnclosure: // Bungo: determine platform type
				switch (((SMBSystemEnclosure *)structHeader)->chassisType) {
					case kSMBchassisDesktop:
					case kSMBchassisLPDesktop:
					case kSMBchassisAllInOne:
					case kSMBchassisLunchBox:
						PlatformType = 1; // desktop (iMac, MacMini)
						break;
					case kSMBchassisPortable:
					case kSMBchassisLaptop:
					case kSMBchassisNotebook:
					case kSMBchassisHandHeld:
					case kSMBchassisSubNotebook:
						PlatformType = 2; // notebook (Mac Books)
					    break;
					default:
						PlatformType = 3; // workstation (Mac Pro, Xserve)
						break;
				}
				break;
				//
			case kSMBTypePhysicalMemoryArray:
				Platform.DMI.MaxMemorySlots += ((SMBPhysicalMemoryArray *)structHeader)->numMemoryDevices;
				break;

			case kSMBTypeMemoryDevice:
				Platform.DMI.CntMemorySlots++;
				if (((SMBMemoryDevice *)structHeader)->memorySize != 0)	{
					Platform.DMI.MemoryModules++;
				}
				if (((SMBMemoryDevice *)structHeader)->memorySpeed > 0)	{
					Platform.RAM.DIMM[dimmnbr].Frequency = ((SMBMemoryDevice *)structHeader)->memorySpeed;
				}
				dimmnbr++;
				break;
			default:
				break;
		}

		structPtr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)structPtr)[0] != 0; structPtr++);

		if (((uint16_t *)structPtr)[0] == 0) {
			structPtr += 2;
		}

		structHeader = (SMBStructHeader *)structPtr;
	}
}
