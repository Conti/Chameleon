#ifndef __LIBSAIO_ACPI_H
#define __LIBSAIO_ACPI_H

#define ACPI_RANGE_START    (0x0E0000)
#define ACPI_RANGE_END      (0x0FFFFF)

#define UINT64_LE_FROM_CHARS(a,b,c,d,e,f,g,h) \
(   ((uint64_t)h << 56) \
|   ((uint64_t)g << 48) \
|   ((uint64_t)f << 40) \
|   ((uint64_t)e << 32) \
|   ((uint64_t)d << 24) \
|   ((uint64_t)c << 16) \
|   ((uint64_t)b <<  8) \
|   ((uint64_t)a <<  0) \
)

#define ACPI_SIGNATURE_UINT64_LE UINT64_LE_FROM_CHARS('R','S','D',' ','P','T','R',' ')

/* Per ACPI 3.0a spec */

// TODO Migrate
struct acpi_2_rsdp {
    char            Signature[8];
    uint8_t         Checksum;
    char            OEMID[6];
    uint8_t         Revision;
    uint32_t        RsdtAddress;
    uint32_t        Length;
    uint64_t        XsdtAddress;
    uint8_t         ExtendedChecksum;
    char            Reserved[3];
} __attribute__((packed));

// TODO Migrate
struct acpi_2_rsdt {
	char            Signature[4];
	uint32_t        Length;
	uint8_t         Revision;
	uint8_t         Checksum;
	char            OEMID[6];
	char            OEMTableId[8];
	uint32_t        OEMRevision;
	uint32_t        CreatorId;
	uint32_t        CreatorRevision;
} __attribute__((packed));

// TODO Migrate
struct acpi_2_xsdt {
	char            Signature[4];
	uint32_t        Length;
	uint8_t         Revision;
	uint8_t         Checksum;
	char            OEMID[6];
	char            OEMTableId[8];
	uint32_t        OEMRevision;
	uint32_t        CreatorId;
	uint32_t        CreatorRevision;
} __attribute__((packed));

// TODO Migrate
struct acpi_2_ssdt {
	char            Signature[4];
	uint32_t        Length;
	uint8_t         Revision;
	uint8_t         Checksum;
	char            OEMID[6];
	char            OEMTableId[8];
	uint32_t        OEMRevision;
	uint32_t        CreatorId;
	uint32_t        CreatorRevision;
} __attribute__((packed));

// TODO Migrate
struct acpi_2_dsdt {
	char            Signature[4];
	uint32_t        Length;
	uint8_t         Revision;
	uint8_t         Checksum;
	char            OEMID[6];
	char            OEMTableId[8];
	uint32_t        OEMRevision;
	uint32_t        CreatorId;
	uint32_t        CreatorRevision;
} __attribute__((packed));

// TODO Migrate
struct acpi_2_fadt {
	char            Signature[4];
	uint32_t        Length;
	uint8_t         Revision;
	uint8_t         Checksum;
	char            OEMID[6];
	char            OEMTableId[8];
	uint32_t        OEMRevision;
	uint32_t        CreatorId;
	uint32_t        CreatorRevision;
	uint32_t        FIRMWARE_CTRL;
	uint32_t        DSDT;
	uint8_t         Model;			// JrCs
	uint8_t         PM_Profile;		// JrCs
	uint16_t		SCI_Interrupt;
	uint32_t		SMI_Command_Port;
	uint8_t			ACPI_Enable;
	uint8_t			ACPI_Disable;
	uint8_t			S4BIOS_Command;
	uint8_t			PState_Control;
	uint32_t		PM1A_Event_Block_Address;
	uint32_t		PM1B_Event_Block_Address;
	uint32_t		PM1A_Control_Block_Address;
	uint32_t		PM1B_Control_Block_Address;
	uint32_t		PM2_Control_Block_Address;
	uint32_t		PM_Timer_Block_Address;
	uint32_t		GPE0_Block_Address;
	uint32_t		GPE1_Block_Address;
	uint8_t			PM1_Event_Block_Length;
	uint8_t			PM1_Control_Block_Length;
	uint8_t			PM2_Control_Block_Length;
	uint8_t			PM_Timer_Block_Length;
	uint8_t			GPE0_Block_Length;
	uint8_t			GPE1_Block_Length;
	uint8_t			GPE1_Base_Offset;
	uint8_t			CST_Support;
	uint16_t		C2_Latency;
	uint16_t		C3_Latency;
	uint16_t		CPU_Cache_Size;
	uint16_t		Cache_Flush_Stride;
	uint8_t			Duty_Cycle_Offset;
	uint8_t			Duty_Cycle_Width;
	uint8_t			RTC_Day_Alarm_Index;
	uint8_t			RTC_Month_Alarm_Index;
	uint8_t			RTC_Century_Index;
	uint16_t		Boot_Flags;
	uint8_t			Reserved0;
/* Begin Asere */
	//Reset Fix
	uint32_t        Flags;
	uint8_t         Reset_SpaceID;
	uint8_t         Reset_BitWidth;
	uint8_t         Reset_BitOffset;
	uint8_t         Reset_AccessWidth;
	uint64_t        Reset_Address;
	uint8_t         Reset_Value;
	uint8_t         Reserved[3];

	uint64_t	    X_FIRMWARE_CTRL;
	uint64_t	    X_DSDT;
/* End Asere */
	/*We absolutely don't care about theese fields*/
	uint8_t		notimp2[96];
} __attribute__((packed));

#endif /* !__LIBSAIO_ACPI_H */
