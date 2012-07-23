/*
 * Copyright 2008 mackerintel
 */

/*
 * Copyright (c) 2011 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#ifndef __LIBSAIO_ACPI_CODEC_H
#define __LIBSAIO_ACPI_CODEC_H

#include "libsaio.h"
#include "efi.h"

#define OEMOPT_TEMPLATE		"oemXXXX"
#define OEMOPT_SIZE			sizeof(OEMOPT_TEMPLATE)

#define kOEMDSDT			"oemDSDT"			/* acpi_codec.c */
#define kOEMFADT			"oemFADT"			/* acpi_codec.c */
#define kOEMAPIC			"oemAPIC"			/* acpi_codec.c */

#define KIntelFADT			"IntelFADTSpec"		/* acpi_codec.c */

#define kSTRIPAPIC          "StripAPICTable"    /* acpi_codec.c */

#define kRestartFix			"RestartFix"        /* acpi_codec.c */
#define KResetType			"ResetType"			/* acpi_codec.c */ 
#define PCI_RESET_TYPE			0	// (default)
#define KEYBOARD_RESET_TYPE		1

#define kGeneratePStates	"GeneratePStates"	/* acpi_codec.c */
#define kGenerateCStates	"GenerateCStates"	/* acpi_codec.c */
#define kGenerateTStates	"GenerateTStates"	/* acpi_codec.c */

#define kMaxRatio			"MaxBusRatio"		/* acpi_codec.c */
#define kMinRatio			"MinBusRatio"		/* acpi_codec.c */
#define	kSpeedstep			"EnableSpeedStep"	/* acpi_codec.c */
#define kEnableC2State      "EnableC2State"     /* acpi_codec.c */
#define kEnableC3State      "EnableC3State"     /* acpi_codec.c */
#define kEnableC4State		"EnableC4State"		/* acpi_codec.c */
#define kEnableC6State      "EnableC6State"     /* acpi_codec.c */
#define kEnableC7State      "EnableC7State"     /* acpi_codec.c */
#define KEnableMwait		"EnableMwait"       /* acpi_codec.c */
#define KForceSSDT			"ForceGenSSDT"      /* acpi_codec.c */
#define KAcpiCoordType		"PstateCoordination" /* acpi_codec.c */
#define kCoreTurbo			"EnableTurbo"		/* acpi_codec.c */

#define kUpdateACPI			"UpdateACPI"	    /* acpi_codec.c */
//#define kUnsafeACPI			"UnsafeACPI"		/* acpi_codec.c */ // DEPRECATED, replaced by nacpi=0x2000  

#define kOnlySignedAml		"LoadSignedAmlOnly"	    /* acpi_codec.c */

#if UNUSED
#define kGenerateFACS		"GenerateFACS"		/* acpi_codec.c */
#define kOEMFACS			"oemFACS"			/* acpi_codec.c */
#endif

extern int AcpiSetup(void);

struct p_state 
{
	union 
	{
		uint16_t Control;
		struct 
		{
			uint8_t VID;	// Voltage ID
			uint8_t FID;	// Frequency ID
		};
	};
	
	uint8_t		CID;		// Compare ID
	uint32_t	Frequency;
};

#endif /* !__LIBSAIO_ACPI_CODEC_H */
