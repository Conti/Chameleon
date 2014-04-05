/*
 *
 * Copyright 2008 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#ifndef __LIBSAIO_PCI_H
#define __LIBSAIO_PCI_H

/*
 *      31              24           16 15     11 10  8
 *     +---------------------------------------------------------------+
 *     |1|      0      |      BUS      |   DEV   |FUNC |       0       |
 *     +---------------------------------------------------------------+
 */

typedef struct {
	uint32_t		:2;
	uint32_t	reg	:6;
	uint32_t	func	:3;
	uint32_t	dev	:5;
	uint32_t	bus	:8;
	uint32_t		:7;
	uint32_t	eb	:1;
} pci_addr_t;

typedef union {
	pci_addr_t	bits;
	uint32_t	addr;
} pci_dev_t;

typedef struct pci_dt_t {
	uint8_t*	regs;
	pci_dev_t	dev;

	uint16_t	devfn; /* encoded device & function index */
	uint16_t	vendor_id; /* Specifies a vendor ID. The PCI bus configuration code obtains this
                            vendor ID from the vendor ID device register. */
	uint16_t	device_id; /* Specifies a device ID that identifies the specific device. The PCI
                            bus configuration code obtains this device ID from the device ID
                            device register. */

	union {
		struct {
			uint16_t	vendor_id; /* Specifies a subsystem vendor ID. */
			uint16_t	device_id; /* Specifies a subsystem device ID that identifies the specific device. */
		} subsys;
		uint32_t	subsys_id;
	}subsys_id;

	uint8_t progif; /* A read-only register that specifies a register-level programming interface the device has, if it has any at all. */

	uint8_t revision_id; /* PCI revision ID. Specifies a revision identifier for a particular device. Where valid IDs are allocated by the vendor. */

	uint16_t	class_id; /*  Specifies a class code. This member is a data structure that stores information related to the device's class code device register. */

	//uint16_t subclass_id; /* A read-only register that specifies the specific function the device performs. */

	struct pci_dt_t			*parent;
	struct pci_dt_t			*children;
	struct pci_dt_t			*next;
} pci_dt_t; // Info

/* Have pci_addr in the same format as the values written to 0xcf8
 * so register accesses can be made easy. */
#define PCIADDR(bus, dev, func) ((1 << 31) | (bus << 16) | (dev << 11) | (func << 8))
#define PCI_ADDR_REG		0xcf8
#define PCI_DATA_REG		0xcfc

extern pci_dt_t		*root_pci_dev;
extern uint8_t		pci_config_read8(uint32_t, uint8_t);
extern uint16_t		pci_config_read16(uint32_t, uint8_t);
extern uint32_t		pci_config_read32(uint32_t, uint8_t);
extern void		pci_config_write8(uint32_t, uint8_t, uint8_t);
extern void		pci_config_write16(uint32_t, uint8_t, uint16_t);
extern void		pci_config_write32(uint32_t, uint8_t, uint32_t);
extern char		*get_pci_dev_path(pci_dt_t *);
extern void		build_pci_dt(void);
extern void		dump_pci_dt(pci_dt_t *);

/* Option ROM header */
typedef struct {
	uint16_t		signature;		// 0xAA55
	uint8_t			rom_size;
	uint32_t		entry_point;
	uint8_t			reserved[15];
	uint16_t		pci_header_offset;
	uint16_t		expansion_header_offset;
} option_rom_header_t;

/* Option ROM PCI Data Structure */
typedef struct {
	uint32_t		signature;		// ati - 0x52494350, nvidia - 0x50434952, 'PCIR'
	uint16_t		vendor_id;
	uint16_t		device_id;
	uint16_t		vital_product_data_offset;
	uint16_t		structure_length;
	uint8_t			structure_revision;
	uint8_t			class_code[3];
	uint16_t		image_length;
	uint16_t		image_revision;
	uint8_t			code_type;
	uint8_t			indicator;
	uint16_t		reserved;
} option_rom_pci_header_t;

//-----------------------------------------------------------------------------
// added by iNDi

typedef struct {
	uint32_t		signature;		// 0x24506E50 '$PnP'
	uint8_t			revision;		//	1
	uint8_t			length;
	uint16_t		offset;
	uint8_t			checksum;
	uint32_t		identifier;
	uint16_t		manufacturer;
	uint16_t		product;
	uint8_t			class[3];
	uint8_t			indicators;
	uint16_t		boot_vector;
	uint16_t		disconnect_vector;
	uint16_t		bootstrap_vector;
	uint16_t		reserved;
	uint16_t		resource_vector;
} option_rom_pnp_header_t;

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 *
 *      register name                      offset
 *******************************************************/
#define PCI_VENDOR_ID						0x00		/* 16 bits */
#define PCI_DEVICE_ID						0x02		/* 16 bits */
#define PCI_COMMAND							0x04		/* 16 bits */
#define PCI_COMMAND_IO						0x1			/* Enable response in I/O space */
#define PCI_COMMAND_MEMORY					0x2			/* Enable response in Memory space */
#define PCI_COMMAND_MASTER					0x4			/* Enable bus mastering */
#define PCI_COMMAND_SPECIAL					0x8			/* Enable response to special cycles */
#define PCI_COMMAND_INVALIDATE				0x10		/* Use memory write and invalidate */
#define PCI_COMMAND_VGA_PALETTE				0x20		/* Enable palette snooping */
#define PCI_COMMAND_PARITY					0x40		/* Enable parity checking */
#define PCI_COMMAND_WAIT					0x80		/* Enable address/data stepping */
#define PCI_COMMAND_SERR					0x100		/* Enable SERR */
#define PCI_COMMAND_FAST_BACK				0x200		/* Enable back-to-back writes */
#define PCI_COMMAND_DISABLE_INTx			0x400		/* PCIE: Disable INTx interrupts */

#define PCI_STATUS							0x06		/* 16 bits */
#define PCI_STATUS_INTx						0x08		/* PCIE: INTx interrupt pending */
#define PCI_STATUS_CAP_LIST					0x10		/* Support Capability List */
#define PCI_STATUS_66MHZ					0x20		/* Support 66 Mhz PCI 2.1 bus */
#define PCI_STATUS_UDF						0x40		/* Support User Definable Features [obsolete] */
#define PCI_STATUS_FAST_BACK				0x80		/* Accept fast-back to back */
#define PCI_STATUS_PARITY					0x100		/* Detected parity error */
#define PCI_STATUS_DEVSEL_MASK				0x600		/* DEVSEL timing */
#define PCI_STATUS_DEVSEL_FAST				0x000
#define PCI_STATUS_DEVSEL_MEDIUM			0x200
#define PCI_STATUS_DEVSEL_SLOW				0x400
#define PCI_STATUS_SIG_TARGET_ABORT 		0x800		/* Set on target abort */
#define PCI_STATUS_REC_TARGET_ABORT 		0x1000		/* Master ack of " */
#define PCI_STATUS_REC_MASTER_ABORT 		0x2000		/* Set on master abort */
#define PCI_STATUS_SIG_SYSTEM_ERROR 		0x4000		/* Set when we drive SERR */
#define PCI_STATUS_DETECTED_PARITY			0x8000		/* Set on parity error */

#define PCI_CLASS_REVISION					0x08			/* High 24 bits are class, low 8  revision */
#define PCI_CLASS_PROG						0x09			/* Reg. Level Programming Interface know also as PCI_PROG_IF */
#define PCI_CLASS_DEVICE					0x0a			/* Device subclass */
//#define PCI_SUBCLASS_DEVICE				0x0b			/* Device class */

#define PCI_CACHE_LINE_SIZE					0x0c		/* 8 bits */
#define PCI_LATENCY_TIMER					0x0d		/* 8 bits */
#define PCI_HEADER_TYPE						0x0e		/* 8 bits */
#define PCI_HEADER_TYPE_NORMAL				0
#define PCI_HEADER_TYPE_BRIDGE				1
#define PCI_HEADER_TYPE_CARDBUS				2	

#define PCI_BIST							0x0f		/* 8 bits */
#define PCI_BIST_CODE_MASK					0x0f		/* Return result */
#define PCI_BIST_START						0x40		/* 1 to start BIST, 2 secs or less */
#define PCI_BIST_CAPABLE					0x80		/* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back. Only
 * 1 bits are decoded.
 */
#define PCI_BASE_ADDRESS_0					0x10		/* 32 bits */
#define PCI_BASE_ADDRESS_1					0x14		/* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2					0x18		/* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3					0x1c		/* 32 bits */
#define PCI_BASE_ADDRESS_4					0x20		/* 32 bits */
#define PCI_BASE_ADDRESS_5					0x24		/* 32 bits */
#define PCI_BASE_ADDRESS_SPACE				0x01		/* 0 = memory, 1 = I/O */
#define PCI_BASE_ADDRESS_SPACE_IO			0x01
#define PCI_BASE_ADDRESS_SPACE_MEMORY		0x00
#define PCI_BASE_ADDRESS_MEM_TYPE_MASK		0x06
#define PCI_BASE_ADDRESS_MEM_TYPE_32		0x00		/* 32 bit address */
#define PCI_BASE_ADDRESS_MEM_TYPE_1M		0x02		/* Below 1M [obsolete] */
#define PCI_BASE_ADDRESS_MEM_TYPE_64		0x04		/* 64 bit address */
#define PCI_BASE_ADDRESS_MEM_PREFETCH		0x08		/* prefetchable? */
#define PCI_BASE_ADDRESS_MEM_MASK			(~(pciaddr_t)0x0f)
#define PCI_BASE_ADDRESS_IO_MASK			(~(pciaddr_t)0x03)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#define PCI_CARDBUS_CIS						0x28
#define PCI_SUBSYSTEM_VENDOR_ID				0x2c
#define PCI_SUBSYSTEM_ID					0x2e
#define PCI_ROM_ADDRESS						0x30		/* Bits 31..11 are address, 10..1 reserved */
#define PCI_ROM_ADDRESS_ENABLE				0x01
#define PCI_ROM_ADDRESS_MASK				(~(pciaddr_t)0x7ff)

#define PCI_CAPABILITY_LIST					0x34		/* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#define PCI_INTERRUPT_LINE					0x3c		/* 8 bits */
#define PCI_INTERRUPT_PIN					0x3d		/* 8 bits */
#define PCI_MIN_GNT							0x3e		/* 8 bits */
#define PCI_MAX_LAT							0x3f		/* 8 bits */

/* Header type 1 (PCI-to-PCI bridges) */
#define PCI_PRIMARY_BUS						0x18		/* Primary bus number */
#define PCI_SECONDARY_BUS					0x19		/* Secondary bus number */
#define PCI_SUBORDINATE_BUS					0x1a		/* Highest bus number behind the bridge */
#define PCI_SEC_LATENCY_TIMER				0x1b		/* Latency timer for secondary interface */
#define PCI_IO_BASE							0x1c		/* I/O range behind the bridge */
#define PCI_IO_LIMIT						0x1d
#define PCI_IO_RANGE_TYPE_MASK				0x0f		/* I/O bridging type */
#define PCI_IO_RANGE_TYPE_16				0x00
#define PCI_IO_RANGE_TYPE_32				0x01
#define PCI_IO_RANGE_MASK					~0x0f
#define PCI_SEC_STATUS						0x1e		/* Secondary status register */
#define PCI_MEMORY_BASE						0x20		/* Memory range behind */
#define PCI_MEMORY_LIMIT					0x22
#define PCI_MEMORY_RANGE_TYPE_MASK			0x0f
#define PCI_MEMORY_RANGE_MASK				~0x0f
#define PCI_PREF_MEMORY_BASE				0x24		/* Prefetchable memory range behind */
#define PCI_PREF_MEMORY_LIMIT				0x26
#define PCI_PREF_RANGE_TYPE_MASK			0x0f
#define PCI_PREF_RANGE_TYPE_32				0x00
#define PCI_PREF_RANGE_TYPE_64				0x01
#define PCI_PREF_RANGE_MASK					~0x0f
#define PCI_PREF_BASE_UPPER32				0x28		/* Upper half of prefetchable memory range */
#define PCI_PREF_LIMIT_UPPER32				0x2c
#define PCI_IO_BASE_UPPER16					0x30		/* Upper half of I/O addresses */
#define PCI_IO_LIMIT_UPPER16				0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#define PCI_ROM_ADDRESS1					0x38		/* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_BRIDGE_CONTROL					0x3e
#define PCI_BRIDGE_CTL_PARITY				0x01		/* Enable parity detection on secondary interface */
#define PCI_BRIDGE_CTL_SERR					0x02		/* The same for SERR forwarding */
#define PCI_BRIDGE_CTL_NO_ISA				0x04		/* Disable bridging of ISA ports */
#define PCI_BRIDGE_CTL_VGA					0x08		/* Forward VGA addresses */
#define PCI_BRIDGE_CTL_MASTER_ABORT			0x20		/* Report master aborts */
#define PCI_BRIDGE_CTL_BUS_RESET			0x40		/* Secondary bus reset */
#define PCI_BRIDGE_CTL_FAST_BACK			0x80		/* Fast Back2Back enabled on secondary interface */
#define PCI_BRIDGE_CTL_PRI_DISCARD_TIMER 	0x100		/* PCI-X? */
#define PCI_BRIDGE_CTL_SEC_DISCARD_TIMER 	0x200		/* PCI-X? */
#define PCI_BRIDGE_CTL_DISCARD_TIMER_STATUS 0x400		/* PCI-X? */
#define PCI_BRIDGE_CTL_DISCARD_TIMER_SERR_EN 0x800		/* PCI-X? */

/* Header type 2 (CardBus bridges) */
/* 0x14-0x15 reserved */
#define PCI_CB_SEC_STATUS					0x16		/* Secondary status */
#define PCI_CB_PRIMARY_BUS					0x18		/* PCI bus number */
#define PCI_CB_CARD_BUS						0x19		/* CardBus bus number */
#define PCI_CB_SUBORDINATE_BUS				0x1a		/* Subordinate bus number */
#define PCI_CB_LATENCY_TIMER				0x1b		/* CardBus latency timer */
#define PCI_CB_MEMORY_BASE_0				0x1c
#define PCI_CB_MEMORY_LIMIT_0				0x20
#define PCI_CB_MEMORY_BASE_1				0x24
#define PCI_CB_MEMORY_LIMIT_1				0x28
#define PCI_CB_IO_BASE_0					0x2c
#define PCI_CB_IO_BASE_0_HI					0x2e
#define PCI_CB_IO_LIMIT_0					0x30
#define PCI_CB_IO_LIMIT_0_HI				0x32
#define PCI_CB_IO_BASE_1					0x34
#define PCI_CB_IO_BASE_1_HI					0x36
#define PCI_CB_IO_LIMIT_1					0x38
#define PCI_CB_IO_LIMIT_1_HI				0x3a
#define	 PCI_CB_IO_RANGE_MASK				~0x03
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_CB_BRIDGE_CONTROL				0x3e
#define PCI_CB_BRIDGE_CTL_PARITY			0x01		/* Similar to standard bridge control register */
#define PCI_CB_BRIDGE_CTL_SERR				0x02
#define PCI_CB_BRIDGE_CTL_ISA				0x04
#define PCI_CB_BRIDGE_CTL_VGA				0x08
#define PCI_CB_BRIDGE_CTL_MASTER_ABORT		0x20
#define PCI_CB_BRIDGE_CTL_CB_RESET			0x40		/* CardBus reset */
#define PCI_CB_BRIDGE_CTL_16BIT_INT			0x80		/* Enable interrupt for 16-bit cards */
#define PCI_CB_BRIDGE_CTL_PREFETCH_MEM0 	0x100		/* Prefetch enable for both memory regions */
#define PCI_CB_BRIDGE_CTL_PREFETCH_MEM1 	0x200
#define PCI_CB_BRIDGE_CTL_POST_WRITES		0x400
#define PCI_CB_SUBSYSTEM_VENDOR_ID			0x40
#define PCI_CB_SUBSYSTEM_ID					0x42
#define PCI_CB_LEGACY_MODE_BASE				0x44		/* 16-bit PC Card legacy mode base address (ExCa) */
/* 0x48-0x7f reserved */

/* Capability Identification Numbers list */
#define PCI_CAP_LIST_ID						0			/* Capability ID */
#define PCI_CAP_ID_PM						0x01		/* Power Management */
#define PCI_CAP_ID_AGP						0x02		/* Accelerated Graphics Port */
#define PCI_CAP_ID_VPD						0x03		/* Vital Product Data */
#define PCI_CAP_ID_SLOTID					0x04		/* Slot Identification */
#define PCI_CAP_ID_MSI						0x05		/* Message Signaled Interrupts */
#define PCI_CAP_ID_CHSWP					0x06		/* CompactPCI HotSwap */
#define PCI_CAP_ID_PCIX						0x07		/* PCI-X */
#define PCI_CAP_ID_HT						0x08		/* HyperTransport */
#define PCI_CAP_ID_VNDR						0x09		/* Vendor specific */
#define PCI_CAP_ID_DBG						0x0A		/* Debug port */
#define PCI_CAP_ID_CCRC						0x0B		/* CompactPCI Central Resource Control */
#define PCI_CAP_ID_HOTPLUG					0x0C		/* PCI hot-plug */
#define PCI_CAP_ID_SSVID					0x0D		/* Bridge subsystem vendor/device ID */
#define PCI_CAP_ID_AGP3						0x0E		/* AGP 8x */
#define PCI_CAP_ID_SECURE					0x0F		/* Secure device (?) */
#define PCI_CAP_ID_EXP						0x10		/* PCI Express */
#define PCI_CAP_ID_MSIX						0x11		/* MSI-X */
#define PCI_CAP_ID_SATA						0x12		/* Serial-ATA HBA */
#define PCI_CAP_ID_AF						0x13		/* Advanced features of PCI devices integrated in PCIe root cplx */
#define PCI_CAP_LIST_NEXT					1			/* Next capability in the list */
#define PCI_CAP_FLAGS						2			/* Capability defined flags (16 bits) */
#define PCI_CAP_SIZEOF						4

/* Capabilities residing in the
 PCI Express extended configuration space */
#define PCI_EXT_CAP_ID_AER					0x01		/* Advanced Error Reporting */
#define PCI_EXT_CAP_ID_VC					0x02		/* Virtual Channel */
#define PCI_EXT_CAP_ID_DSN					0x03		/* Device Serial Number */
#define PCI_EXT_CAP_ID_PB					0x04		/* Power Budgeting */
#define PCI_EXT_CAP_ID_RCLINK				0x05		/* Root Complex Link Declaration */
#define PCI_EXT_CAP_ID_RCILINK				0x06		/* Root Complex Internal Link Declaration */
#define PCI_EXT_CAP_ID_RCECOLL				0x07		/* Root Complex Event Collector */
#define PCI_EXT_CAP_ID_MFVC					0x08		/* Multi-Function Virtual Channel */
#define PCI_EXT_CAP_ID_RBCB					0x0a		/* Root Bridge Control Block */
#define PCI_EXT_CAP_ID_VNDR					0x0b		/* Vendor specific */
#define PCI_EXT_CAP_ID_ACS					0x0d		/* Access Controls */
#define PCI_EXT_CAP_ID_ARI					0x0e		/* Alternative Routing-ID Interpretation */
#define PCI_EXT_CAP_ID_ATS					0x0f		/* Address Translation Service */
#define PCI_EXT_CAP_ID_SRIOV				0x10		/* Single Root I/O Virtualization */

/* Power Management Registers */
#define PCI_PM_CAP_VER_MASK					0x0007		/* Version (2=PM1.1) */
#define PCI_PM_CAP_PME_CLOCK				0x0008		/* Clock required for PME generation */
#define PCI_PM_CAP_DSI						0x0020		/* Device specific initialization required */
#define PCI_PM_CAP_AUX_C_MASK				0x01c0		/* Maximum aux current required in D3cold */
#define PCI_PM_CAP_D1						0x0200		/* D1 power state support */
#define PCI_PM_CAP_D2						0x0400		/* D2 power state support */
#define PCI_PM_CAP_PME_D0					0x0800		/* PME can be asserted from D0 */
#define PCI_PM_CAP_PME_D1					0x1000		/* PME can be asserted from D1 */
#define PCI_PM_CAP_PME_D2					0x2000		/* PME can be asserted from D2 */
#define PCI_PM_CAP_PME_D3_HOT				0x4000		/* PME can be asserted from D3hot */
#define PCI_PM_CAP_PME_D3_COLD				0x8000		/* PME can be asserted from D3cold */
#define PCI_PM_CTRL							4			/* PM control and status register */
#define PCI_PM_CTRL_STATE_MASK				0x0003		/* Current power state (D0 to D3) */
#define PCI_PM_CTRL_PME_ENABLE				0x0100		/* PME pin enable */
#define PCI_PM_CTRL_DATA_SEL_MASK			0x1e00		/* PM table data index */
#define PCI_PM_CTRL_DATA_SCALE_MASK			0x6000		/* PM table data scaling factor */
#define PCI_PM_CTRL_PME_STATUS				0x8000		/* PME pin status */
#define PCI_PM_PPB_EXTENSIONS				6			/* PPB support extensions */
#define PCI_PM_PPB_B2_B3					0x40		/* If bridge enters D3hot, bus enters: 0=B3, 1=B2 */
#define PCI_PM_BPCC_ENABLE					0x80		/* Secondary bus is power managed */
#define PCI_PM_DATA_REGISTER				7			/* PM table contents read here */
#define PCI_PM_SIZEOF						8

/* AGP registers */
#define PCI_AGP_VERSION						2			/* BCD version number */
#define PCI_AGP_RFU							3			/* Rest of capability flags */
#define PCI_AGP_STATUS						4			/* Status register */
#define PCI_AGP_STATUS_RQ_MASK				0xff000000	/* Maximum number of requests - 1 */
#define PCI_AGP_STATUS_ISOCH				0x10000		/* Isochronous transactions supported */
#define PCI_AGP_STATUS_ARQSZ_MASK			0xe000		/* log2(optimum async req size in bytes) - 4 */
#define PCI_AGP_STATUS_CAL_MASK				0x1c00		/* Calibration cycle timing */
#define PCI_AGP_STATUS_SBA					0x0200		/* Sideband addressing supported */
#define PCI_AGP_STATUS_ITA_COH				0x0100		/* In-aperture accesses always coherent */
#define PCI_AGP_STATUS_GART64				0x0080		/* 64-bit GART entries supported */
#define PCI_AGP_STATUS_HTRANS				0x0040		/* If 0, core logic can xlate host CPU accesses thru aperture */
#define PCI_AGP_STATUS_64BIT				0x0020		/* 64-bit addressing cycles supported */
#define PCI_AGP_STATUS_FW					0x0010		/* Fast write transfers supported */
#define PCI_AGP_STATUS_AGP3					0x0008		/* AGP3 mode supported */
#define PCI_AGP_STATUS_RATE4				0x0004		/* 4x transfer rate supported (RFU in AGP3 mode) */
#define PCI_AGP_STATUS_RATE2				0x0002		/* 2x transfer rate supported (8x in AGP3 mode) */
#define PCI_AGP_STATUS_RATE1				0x0001		/* 1x transfer rate supported (4x in AGP3 mode) */
#define PCI_AGP_COMMAND						8			/* Control register */
#define PCI_AGP_COMMAND_RQ_MASK				0xff000000	/* Master: Maximum number of requests */
#define PCI_AGP_COMMAND_ARQSZ_MASK			0xe000		/* log2(optimum async req size in bytes) - 4 */
#define PCI_AGP_COMMAND_CAL_MASK			0x1c00		/* Calibration cycle timing */
#define PCI_AGP_COMMAND_SBA					0x0200		/* Sideband addressing enabled */
#define PCI_AGP_COMMAND_AGP					0x0100		/* Allow processing of AGP transactions */
#define PCI_AGP_COMMAND_GART64				0x0080		/* 64-bit GART entries enabled */
#define PCI_AGP_COMMAND_64BIT				0x0020		/* Allow generation of 64-bit addr cycles */
#define PCI_AGP_COMMAND_FW					0x0010		/* Enable FW transfers */
#define PCI_AGP_COMMAND_RATE4				0x0004		/* Use 4x rate (RFU in AGP3 mode) */
#define PCI_AGP_COMMAND_RATE2				0x0002		/* Use 2x rate (8x in AGP3 mode) */
#define PCI_AGP_COMMAND_RATE1				0x0001		/* Use 1x rate (4x in AGP3 mode) */
#define PCI_AGP_SIZEOF						12

/* Vital Product Data */
#define PCI_VPD_ADDR						2			/* Address to access (15 bits!) */
#define PCI_VPD_ADDR_MASK					0x7fff		/* Address mask */
#define PCI_VPD_ADDR_F						0x8000		/* Write 0, 1 indicates completion */
#define PCI_VPD_DATA						4			/* 32-bits of data returned here */

/* Slot Identification */
#define PCI_SID_ESR							2			/* Expansion Slot Register */
#define PCI_SID_ESR_NSLOTS					0x1f		/* Number of expansion slots available */
#define PCI_SID_ESR_FIC						0x20		/* First In Chassis Flag */
#define PCI_SID_CHASSIS_NR					3			/* Chassis Number */

/* Message Signaled Interrupts registers */
#define PCI_MSI_FLAGS						2			/* Various flags */
#define PCI_MSI_FLAGS_MASK_BIT				0x100		/* interrupt masking & reporting supported */
#define PCI_MSI_FLAGS_64BIT					0x080		/* 64-bit addresses allowed */
#define PCI_MSI_FLAGS_QSIZE					0x070		/* Message queue size configured */
#define PCI_MSI_FLAGS_QMASK					0x00e		/* Maximum queue size available */
#define PCI_MSI_FLAGS_ENABLE				0x001		/* MSI feature enabled */
#define PCI_MSI_RFU							3			/* Rest of capability flags */
#define PCI_MSI_ADDRESS_LO					4			/* Lower 32 bits */
#define PCI_MSI_ADDRESS_HI					8			/* Upper 32 bits (if PCI_MSI_FLAGS_64BIT set) */
#define PCI_MSI_DATA_32						8			/* 16 bits of data for 32-bit devices */
#define PCI_MSI_DATA_64						12			/* 16 bits of data for 64-bit devices */
#define PCI_MSI_MASK_BIT_32					12			/* per-vector masking for 32-bit devices */
#define PCI_MSI_MASK_BIT_64					16			/* per-vector masking for 64-bit devices */
#define PCI_MSI_PENDING_32					16			/* per-vector interrupt pending for 32-bit devices */
#define PCI_MSI_PENDING_64					20			/* per-vector interrupt pending for 64-bit devices */

/* PCI-X */
#define PCI_PCIX_COMMAND												2 /* Command register offset */
#define PCI_PCIX_COMMAND_DPERE									   0x0001 /* Data Parity Error Recover Enable */
#define PCI_PCIX_COMMAND_ERO									   0x0002 /* Enable Relaxed Ordering */
#define PCI_PCIX_COMMAND_MAX_MEM_READ_BYTE_COUNT				   0x000c /* Maximum Memory Read Byte Count */
#define PCI_PCIX_COMMAND_MAX_OUTSTANDING_SPLIT_TRANS			   0x0070
#define PCI_PCIX_COMMAND_RESERVED									0xf80
#define PCI_PCIX_STATUS													4 /* Status register offset */
#define PCI_PCIX_STATUS_FUNCTION							   0x00000007
#define PCI_PCIX_STATUS_DEVICE								   0x000000f8
#define PCI_PCIX_STATUS_BUS									   0x0000ff00
#define PCI_PCIX_STATUS_64BIT								   0x00010000
#define PCI_PCIX_STATUS_133MHZ								   0x00020000
#define PCI_PCIX_STATUS_SC_DISCARDED						   0x00040000 /* Split Completion Discarded */
#define PCI_PCIX_STATUS_UNEXPECTED_SC						   0x00080000 /* Unexpected Split Completion */
#define PCI_PCIX_STATUS_DEVICE_COMPLEXITY					   0x00100000 /* 0 = simple device, 1 = bridge device */
#define PCI_PCIX_STATUS_DESIGNED_MAX_MEM_READ_BYTE_COUNT	   0x00600000 /* 0 = 512 bytes, 1 = 1024, 2 = 2048, 3 = 4096 */
#define PCI_PCIX_STATUS_DESIGNED_MAX_OUTSTANDING_SPLIT_TRANS   0x03800000
#define PCI_PCIX_STATUS_DESIGNED_MAX_CUMULATIVE_READ_SIZE	   0x1c000000
#define PCI_PCIX_STATUS_RCVD_SC_ERR_MESS					   0x20000000 /* Received Split Completion Error Message */
#define PCI_PCIX_STATUS_266MHZ								   0x40000000 /* 266 MHz capable */
#define PCI_PCIX_STATUS_533MHZ								   0x80000000 /* 533 MHz capable */
#define PCI_PCIX_SIZEOF		4


/* PCI-X Bridges */
#define PCI_PCIX_BRIDGE_SEC_STATUS										2 /* Secondary bus status register offset */
#define PCI_PCIX_BRIDGE_SEC_STATUS_64BIT						   0x0001
#define PCI_PCIX_BRIDGE_SEC_STATUS_133MHZ						   0x0002
#define PCI_PCIX_BRIDGE_SEC_STATUS_SC_DISCARDED					   0x0004 /* Split Completion Discarded on secondary bus */
#define PCI_PCIX_BRIDGE_SEC_STATUS_UNEXPECTED_SC				   0x0008 /* Unexpected Split Completion on secondary bus */
#define PCI_PCIX_BRIDGE_SEC_STATUS_SC_OVERRUN					   0x0010 /* Split Completion Overrun on secondary bus */
#define PCI_PCIX_BRIDGE_SEC_STATUS_SPLIT_REQUEST_DELAYED		   0x0020
#define PCI_PCIX_BRIDGE_SEC_STATUS_CLOCK_FREQ					   0x01c0
#define PCI_PCIX_BRIDGE_SEC_STATUS_RESERVED						   0xfe00
#define PCI_PCIX_BRIDGE_STATUS											4 /* Primary bus status register offset */
#define PCI_PCIX_BRIDGE_STATUS_FUNCTION						   0x00000007
#define PCI_PCIX_BRIDGE_STATUS_DEVICE						   0x000000f8
#define PCI_PCIX_BRIDGE_STATUS_BUS							   0x0000ff00
#define PCI_PCIX_BRIDGE_STATUS_64BIT						   0x00010000
#define PCI_PCIX_BRIDGE_STATUS_133MHZ						   0x00020000
#define PCI_PCIX_BRIDGE_STATUS_SC_DISCARDED					   0x00040000 /* Split Completion Discarded */
#define PCI_PCIX_BRIDGE_STATUS_UNEXPECTED_SC				   0x00080000 /* Unexpected Split Completion */
#define PCI_PCIX_BRIDGE_STATUS_SC_OVERRUN					   0x00100000 /* Split Completion Overrun */
#define PCI_PCIX_BRIDGE_STATUS_SPLIT_REQUEST_DELAYED		   0x00200000
#define PCI_PCIX_BRIDGE_STATUS_RESERVED						   0xffc00000
#define PCI_PCIX_BRIDGE_UPSTREAM_SPLIT_TRANS_CTRL						8 /* Upstream Split Transaction Register offset */
#define PCI_PCIX_BRIDGE_DOWNSTREAM_SPLIT_TRANS_CTRL					   12 /* Downstream Split Transaction Register offset */
#define PCI_PCIX_BRIDGE_STR_CAPACITY						   0x0000ffff
#define PCI_PCIX_BRIDGE_STR_COMMITMENT_LIMIT				   0xffff0000
#define PCI_PCIX_BRIDGE_SIZEOF 12

/* PCI Express */
#define PCI_EXP_FLAGS						0x2			/* Capabilities register */
#define PCI_EXP_FLAGS_VERS					0x000f		/* Capability version */
#define PCI_EXP_FLAGS_TYPE					0x00f0		/* Device/Port type */
#define PCI_EXP_TYPE_ENDPOINT				0x0 		/* Express Endpoint */
#define PCI_EXP_TYPE_LEG_END				0x1 		/* Legacy Endpoint */
#define PCI_EXP_TYPE_ROOT_PORT				0x4 		/* Root Port */
#define PCI_EXP_TYPE_UPSTREAM				0x5 		/* Upstream Port */
#define PCI_EXP_TYPE_DOWNSTREAM				0x6 		/* Downstream Port */
#define PCI_EXP_TYPE_PCI_BRIDGE				0x7 		/* PCI/PCI-X Bridge */
#define PCI_EXP_TYPE_PCIE_BRIDGE			0x8 		/* PCI/PCI-X to PCIE Bridge */
#define PCI_EXP_TYPE_ROOT_INT_EP			0x9 		/* Root Complex Integrated Endpoint */
#define PCI_EXP_TYPE_ROOT_EC				0xa 		/* Root Complex Event Collector */
#define PCI_EXP_FLAGS_SLOT					0x0100		/* Slot implemented */
#define PCI_EXP_FLAGS_IRQ					0x3e00		/* Interrupt message number */
#define PCI_EXP_DEVCAP						0x4 		/* Device capabilities */
#define PCI_EXP_DEVCAP_PAYLOAD				0x07		/* Max_Payload_Size */
#define PCI_EXP_DEVCAP_PHANTOM				0x18		/* Phantom functions */
#define PCI_EXP_DEVCAP_EXT_TAG				0x20		/* Extended tags */
#define PCI_EXP_DEVCAP_L0S					0x1c0		/* L0s Acceptable Latency */
#define PCI_EXP_DEVCAP_L1					0xe00		/* L1 Acceptable Latency */
#define PCI_EXP_DEVCAP_ATN_BUT				0x1000		/* Attention Button Present */
#define PCI_EXP_DEVCAP_ATN_IND				0x2000		/* Attention Indicator Present */
#define PCI_EXP_DEVCAP_PWR_IND				0x4000		/* Power Indicator Present */
#define PCI_EXP_DEVCAP_RBE					0x8000		/* Role-Based Error Reporting */
#define PCI_EXP_DEVCAP_PWR_VAL				0x3fc0000	/* Slot Power Limit Value */
#define PCI_EXP_DEVCAP_PWR_SCL				0xc000000	/* Slot Power Limit Scale */
#define PCI_EXP_DEVCAP_FLRESET				0x10000000	/* Function-Level Reset */
#define PCI_EXP_DEVCTL						0x8			/* Device Control */
#define PCI_EXP_DEVCTL_CERE					0x0001		/* Correctable Error Reporting En. */
#define PCI_EXP_DEVCTL_NFERE				0x0002		/* Non-Fatal Error Reporting Enable */
#define PCI_EXP_DEVCTL_FERE					0x0004		/* Fatal Error Reporting Enable */
#define PCI_EXP_DEVCTL_URRE					0x0008		/* Unsupported Request Reporting En. */
#define PCI_EXP_DEVCTL_RELAXED				0x0010		/* Enable Relaxed Ordering */
#define PCI_EXP_DEVCTL_PAYLOAD				0x00e0		/* Max_Payload_Size */
#define PCI_EXP_DEVCTL_EXT_TAG				0x0100		/* Extended Tag Field Enable */
#define PCI_EXP_DEVCTL_PHANTOM				0x0200		/* Phantom Functions Enable */
#define PCI_EXP_DEVCTL_AUX_PME				0x0400		/* Auxiliary Power PM Enable */
#define PCI_EXP_DEVCTL_NOSNOOP				0x0800		/* Enable No Snoop */
#define PCI_EXP_DEVCTL_READRQ				0x7000		/* Max_Read_Request_Size */
#define PCI_EXP_DEVCTL_BCRE					0x8000		/* Bridge Configuration Retry Enable */
#define PCI_EXP_DEVCTL_FLRESET				0x8000		/* Function-Level Reset [bit shared with BCRE] */
#define PCI_EXP_DEVSTA						0xa 		/* Device Status */
#define PCI_EXP_DEVSTA_CED					0x01		/* Correctable Error Detected */
#define PCI_EXP_DEVSTA_NFED					0x02		/* Non-Fatal Error Detected */
#define PCI_EXP_DEVSTA_FED					0x04		/* Fatal Error Detected */
#define PCI_EXP_DEVSTA_URD					0x08		/* Unsupported Request Detected */
#define PCI_EXP_DEVSTA_AUXPD				0x10		/* AUX Power Detected */
#define PCI_EXP_DEVSTA_TRPND				0x20		/* Transactions Pending */
#define PCI_EXP_LNKCAP						0xc			/* Link Capabilities */
#define PCI_EXP_LNKCAP_SPEED				0x0000f 	/* Maximum Link Speed */
#define PCI_EXP_LNKCAP_WIDTH				0x003f0 	/* Maximum Link Width */
#define PCI_EXP_LNKCAP_ASPM					0x00c00 	/* Active State Power Management */
#define PCI_EXP_LNKCAP_L0S					0x07000 	/* L0s Acceptable Latency */
#define PCI_EXP_LNKCAP_L1					0x38000 	/* L1 Acceptable Latency */
#define PCI_EXP_LNKCAP_CLOCKPM				0x40000 	/* Clock Power Management */
#define PCI_EXP_LNKCAP_SURPRISE				0x80000 	/* Surprise Down Error Reporting */
#define PCI_EXP_LNKCAP_DLLA					0x100000	/* Data Link Layer Active Reporting */
#define PCI_EXP_LNKCAP_LBNC					0x200000	/* Link Bandwidth Notification Capability */
#define PCI_EXP_LNKCAP_PORT					0xff000000	/* Port Number */
#define PCI_EXP_LNKCTL						0x10		/* Link Control */
#define PCI_EXP_LNKCTL_ASPM					0x0003		/* ASPM Control */
#define PCI_EXP_LNKCTL_RCB					0x0008		/* Read Completion Boundary */
#define PCI_EXP_LNKCTL_DISABLE				0x0010		/* Link Disable */
#define PCI_EXP_LNKCTL_RETRAIN				0x0020		/* Retrain Link */
#define PCI_EXP_LNKCTL_CLOCK				0x0040		/* Common Clock Configuration */
#define PCI_EXP_LNKCTL_XSYNCH				0x0080		/* Extended Synch */
#define PCI_EXP_LNKCTL_CLOCKPM				0x0100		/* Clock Power Management */
#define PCI_EXP_LNKCTL_HWAUTWD				0x0200		/* Hardware Autonomous Width Disable */
#define PCI_EXP_LNKCTL_BWMIE				0x0400		/* Bandwidth Mgmt Interrupt Enable */
#define PCI_EXP_LNKCTL_AUTBWIE				0x0800		/* Autonomous Bandwidth Mgmt Interrupt Enable */
#define PCI_EXP_LNKSTA						0x12		/* Link Status */
#define PCI_EXP_LNKSTA_SPEED				0x000f		/* Negotiated Link Speed */
#define PCI_EXP_LNKSTA_WIDTH				0x03f0		/* Negotiated Link Width */
#define PCI_EXP_LNKSTA_TR_ERR				0x0400		/* Training Error (obsolete) */
#define PCI_EXP_LNKSTA_TRAIN				0x0800		/* Link Training */
#define PCI_EXP_LNKSTA_SL_CLK				0x1000		/* Slot Clock Configuration */
#define PCI_EXP_LNKSTA_DL_ACT				0x2000		/* Data Link Layer in DL_Active State */
#define PCI_EXP_LNKSTA_BWMGMT				0x4000		/* Bandwidth Mgmt Status */
#define PCI_EXP_LNKSTA_AUTBW				0x8000		/* Autonomous Bandwidth Mgmt Status */
#define PCI_EXP_SLTCAP						0x14		/* Slot Capabilities */
#define PCI_EXP_SLTCAP_ATNB					0x0001		/* Attention Button Present */
#define PCI_EXP_SLTCAP_PWRC					0x0002		/* Power Controller Present */
#define PCI_EXP_SLTCAP_MRL					0x0004		/* MRL Sensor Present */
#define PCI_EXP_SLTCAP_ATNI					0x0008		/* Attention Indicator Present */
#define PCI_EXP_SLTCAP_PWRI					0x0010		/* Power Indicator Present */
#define PCI_EXP_SLTCAP_HPS					0x0020		/* Hot-Plug Surprise */
#define PCI_EXP_SLTCAP_HPC					0x0040		/* Hot-Plug Capable */
#define PCI_EXP_SLTCAP_PWR_VAL				0x00007f80	/* Slot Power Limit Value */
#define PCI_EXP_SLTCAP_PWR_SCL				0x00018000	/* Slot Power Limit Scale */
#define PCI_EXP_SLTCAP_INTERLOCK			0x020000	/* Electromechanical Interlock Present */
#define PCI_EXP_SLTCAP_NOCMDCOMP			0x040000	/* No Command Completed Support */
#define PCI_EXP_SLTCAP_PSN					0xfff80000	/* Physical Slot Number */
#define PCI_EXP_SLTCTL						0x18		/* Slot Control */
#define PCI_EXP_SLTCTL_ATNB					0x0001		/* Attention Button Pressed Enable */
#define PCI_EXP_SLTCTL_PWRF					0x0002		/* Power Fault Detected Enable */
#define PCI_EXP_SLTCTL_MRLS					0x0004		/* MRL Sensor Changed Enable */
#define PCI_EXP_SLTCTL_PRSD					0x0008		/* Presence Detect Changed Enable */
#define PCI_EXP_SLTCTL_CMDC					0x0010		/* Command Completed Interrupt Enable */
#define PCI_EXP_SLTCTL_HPIE					0x0020		/* Hot-Plug Interrupt Enable */
#define PCI_EXP_SLTCTL_ATNI					0x00c0		/* Attention Indicator Control */
#define PCI_EXP_SLTCTL_PWRI					0x0300		/* Power Indicator Control */
#define PCI_EXP_SLTCTL_PWRC					0x0400		/* Power Controller Control */
#define PCI_EXP_SLTCTL_INTERLOCK			0x0800		/* Electromechanical Interlock Control */
#define PCI_EXP_SLTCTL_LLCHG				0x1000		/* Data Link Layer State Changed Enable */
#define PCI_EXP_SLTSTA						0x1a		/* Slot Status */
#define PCI_EXP_SLTSTA_ATNB					0x0001		/* Attention Button Pressed */
#define PCI_EXP_SLTSTA_PWRF					0x0002		/* Power Fault Detected */
#define PCI_EXP_SLTSTA_MRLS					0x0004		/* MRL Sensor Changed */
#define PCI_EXP_SLTSTA_PRSD					0x0008		/* Presence Detect Changed */
#define PCI_EXP_SLTSTA_CMDC					0x0010		/* Command Completed */
#define PCI_EXP_SLTSTA_MRL_ST				0x0020		/* MRL Sensor State */
#define PCI_EXP_SLTSTA_PRES					0x0040		/* Presence Detect State */
#define PCI_EXP_SLTSTA_INTERLOCK			0x0080		/* Electromechanical Interlock Status */
#define PCI_EXP_SLTSTA_LLCHG				0x0100		/* Data Link Layer State Changed */
#define PCI_EXP_RTCTL						0x1c		/* Root Control */
#define PCI_EXP_RTCTL_SECEE					0x0001		/* System Error on Correctable Error */
#define PCI_EXP_RTCTL_SENFEE				0x0002		/* System Error on Non-Fatal Error */
#define PCI_EXP_RTCTL_SEFEE					0x0004		/* System Error on Fatal Error */
#define PCI_EXP_RTCTL_PMEIE					0x0008		/* PME Interrupt Enable */
#define PCI_EXP_RTCTL_CRSVIS				0x0010		/* Configuration Request Retry Status Visible to SW */
#define PCI_EXP_RTCAP						0x1e		/* Root Capabilities */
#define PCI_EXP_RTCAP_CRSVIS				0x0010		/* Configuration Request Retry Status Visible to SW */
#define PCI_EXP_RTSTA						0x20		/* Root Status */
#define PCI_EXP_RTSTA_PME_REQID				0x0000ffff	/* PME Requester ID */
#define PCI_EXP_RTSTA_PME_STATUS			0x00010000	/* PME Status */
#define PCI_EXP_RTSTA_PME_PENDING			0x00020000	/* PME is Pending */
#define PCI_EXP_DEVCAP2						0x24		/* Device capabilities 2 */
#define PCI_EXP_DEVCTL2						0x28		/* Device Control */
#define PCI_EXP_DEV2_TIMEOUT_RANGE(x)		((x) & 0xf) /* Completion Timeout Ranges Supported */
#define PCI_EXP_DEV2_TIMEOUT_VALUE(x)		((x) & 0xf) /* Completion Timeout Value */
#define PCI_EXP_DEV2_TIMEOUT_DIS			0x0010		/* Completion Timeout Disable Supported */
#define PCI_EXP_DEV2_ARI					0x0020		/* ARI Forwarding */
#define PCI_EXP_DEVSTA2						0x2a		/* Device Status */
#define PCI_EXP_LNKCAP2						0x2c		/* Link Capabilities */
#define PCI_EXP_LNKCTL2						0x30		/* Link Control */
#define PCI_EXP_LNKCTL2_SPEED(x)			((x) & 0xf) /* Target Link Speed */
#define PCI_EXP_LNKCTL2_CMPLNC				0x0010		/* Enter Compliance */
#define PCI_EXP_LNKCTL2_SPEED_DIS			0x0020		/* Hardware Autonomous Speed Disable */
#define PCI_EXP_LNKCTL2_DEEMPHASIS(x)		(((x) >> 6) & 1) /* Selectable De-emphasis */
#define PCI_EXP_LNKCTL2_MARGIN(x)			(((x) >> 7) & 7) /* Transmit Margin */
#define PCI_EXP_LNKCTL2_MOD_CMPLNC			0x0400		/* Enter Modified Compliance */
#define PCI_EXP_LNKCTL2_CMPLNC_SOS			0x0800		/* Compliance SOS */
#define PCI_EXP_LNKCTL2_COM_DEEMPHASIS(x)	(((x) >> 12) & 1) /* Compliance De-emphasis */
#define PCI_EXP_LNKSTA2						0x32		/* Link Status */
#define PCI_EXP_LINKSTA2_DEEMPHASIS(x)		((x) & 1)	/* Current De-emphasis Level */
#define PCI_EXP_SLTCAP2						0x34		/* Slot Capabilities */
#define PCI_EXP_SLTCTL2						0x38		/* Slot Control */
#define PCI_EXP_SLTSTA2						0x3a		/* Slot Status */

/* MSI-X */
#define PCI_MSIX_ENABLE						0x8000
#define PCI_MSIX_MASK						0x4000
#define PCI_MSIX_TABSIZE					0x03ff
#define PCI_MSIX_TABLE						4
#define PCI_MSIX_PBA						8
#define PCI_MSIX_BIR						0x7

/* Subsystem vendor/device ID for PCI bridges */
#define PCI_SSVID_VENDOR					4
#define PCI_SSVID_DEVICE					6

/* Advanced Error Reporting */
#define PCI_ERR_UNCOR_STATUS				4			/* Uncorrectable Error Status */
#define PCI_ERR_UNC_TRAIN					0x00000001	/* Undefined in PCIe rev1.1 & 2.0 spec */
#define PCI_ERR_UNC_DLP						0x00000010	/* Data Link Protocol */
#define PCI_ERR_UNC_SDES					0x00000020	/* Surprise Down Error */
#define PCI_ERR_UNC_POISON_TLP				0x00001000	/* Poisoned TLP */
#define PCI_ERR_UNC_FCP						0x00002000	/* Flow Control Protocol */
#define PCI_ERR_UNC_COMP_TIME				0x00004000	/* Completion Timeout */
#define PCI_ERR_UNC_COMP_ABORT				0x00008000	/* Completer Abort */
#define PCI_ERR_UNC_UNX_COMP				0x00010000	/* Unexpected Completion */
#define PCI_ERR_UNC_RX_OVER					0x00020000	/* Receiver Overflow */
#define PCI_ERR_UNC_MALF_TLP				0x00040000	/* Malformed TLP */
#define PCI_ERR_UNC_ECRC					0x00080000	/* ECRC Error Status */
#define PCI_ERR_UNC_UNSUP					0x00100000	/* Unsupported Request */
#define PCI_ERR_UNC_ACS_VIOL				0x00200000	/* ACS Violation */
#define PCI_ERR_UNCOR_MASK					8			/* Uncorrectable Error Mask */
/* Same bits as above */
#define PCI_ERR_UNCOR_SEVER					12			/* Uncorrectable Error Severity */
/* Same bits as above */
#define PCI_ERR_COR_STATUS					16			/* Correctable Error Status */
#define PCI_ERR_COR_RCVR					0x00000001	/* Receiver Error Status */
#define PCI_ERR_COR_BAD_TLP					0x00000040	/* Bad TLP Status */
#define PCI_ERR_COR_BAD_DLLP				0x00000080	/* Bad DLLP Status */
#define PCI_ERR_COR_REP_ROLL				0x00000100	/* REPLAY_NUM Rollover */
#define PCI_ERR_COR_REP_TIMER				0x00001000	/* Replay Timer Timeout */
#define PCI_ERR_COR_REP_ANFE				0x00002000	/* Advisory Non-Fatal Error */
#define PCI_ERR_COR_MASK					20			/* Correctable Error Mask */
/* Same bits as above */
#define PCI_ERR_CAP							24			/* Advanced Error Capabilities */
#define PCI_ERR_CAP_FEP(x)					((x) & 31)	/* First Error Pointer */
#define PCI_ERR_CAP_ECRC_GENC				0x00000020	/* ECRC Generation Capable */
#define PCI_ERR_CAP_ECRC_GENE				0x00000040	/* ECRC Generation Enable */
#define PCI_ERR_CAP_ECRC_CHKC				0x00000080	/* ECRC Check Capable */
#define PCI_ERR_CAP_ECRC_CHKE				0x00000100	/* ECRC Check Enable */
#define PCI_ERR_HEADER_LOG					28			/* Header Log Register (16 bytes) */
#define PCI_ERR_ROOT_COMMAND				44			/* Root Error Command */
#define PCI_ERR_ROOT_STATUS					48
#define PCI_ERR_ROOT_COR_SRC				52
#define PCI_ERR_ROOT_SRC					54

/* Virtual Channel */
#define PCI_VC_PORT_REG1					4
#define PCI_VC_PORT_REG2					8
#define PCI_VC_PORT_CTRL					12
#define PCI_VC_PORT_STATUS					14
#define PCI_VC_RES_CAP						16
#define PCI_VC_RES_CTRL						20
#define PCI_VC_RES_STATUS					26

/* Power Budgeting */
#define PCI_PWR_DSR							4					/* Data Select Register */
#define PCI_PWR_DATA						8					/* Data Register */
#define PCI_PWR_DATA_BASE(x)				((x) & 0xff)		/* Base Power */
#define PCI_PWR_DATA_SCALE(x)				(((x) >> 8) & 3)	/* Data Scale */
#define PCI_PWR_DATA_PM_SUB(x)				(((x) >> 10) & 7)	/* PM Sub State */
#define PCI_PWR_DATA_PM_STATE(x)			(((x) >> 13) & 3)	/* PM State */
#define PCI_PWR_DATA_TYPE(x)				(((x) >> 15) & 7)	/* Type */
#define PCI_PWR_DATA_RAIL(x)				(((x) >> 18) & 7)	/* Power Rail */
#define PCI_PWR_CAP							12					/* Capability */
#define PCI_PWR_CAP_BUDGET(x)				((x) & 1)			/* Included in system budget */

/* Access Control Services */
#define PCI_ACS_CAP							0x04		/* ACS Capability Register */
#define PCI_ACS_CAP_VALID					0x0001		/* ACS Source Validation */
#define PCI_ACS_CAP_BLOCK					0x0002		/* ACS Translation Blocking */
#define PCI_ACS_CAP_REQ_RED					0x0004		/* ACS P2P Request Redirect */
#define PCI_ACS_CAP_CMPLT_RED				0x0008		/* ACS P2P Completion Redirect */
#define PCI_ACS_CAP_FORWARD					0x0010		/* ACS Upstream Forwarding */
#define PCI_ACS_CAP_EGRESS					0x0020		/* ACS P2P Egress Control */
#define PCI_ACS_CAP_TRANS					0x0040		/* ACS Direct Translated P2P */
#define PCI_ACS_CAP_VECTOR(x)				(((x) >> 8) & 0xff) /* Egress Control Vector Size */
#define PCI_ACS_CTRL						0x06		/* ACS Control Register */
#define PCI_ACS_CTRL_VALID					0x0001		/* ACS Source Validation Enable */
#define PCI_ACS_CTRL_BLOCK					0x0002		/* ACS Translation Blocking Enable */
#define PCI_ACS_CTRL_REQ_RED				0x0004		/* ACS P2P Request Redirect Enable */
#define PCI_ACS_CTRL_CMPLT_RED				0x0008		/* ACS P2P Completion Redirect Enable */
#define PCI_ACS_CTRL_FORWARD				0x0010		/* ACS Upstream Forwarding Enable */
#define PCI_ACS_CTRL_EGRESS					0x0020		/* ACS P2P Egress Control Enable */
#define PCI_ACS_CTRL_TRANS					0x0040		/* ACS Direct Translated P2P Enable */
#define PCI_ACS_EGRESS_CTRL					0x08		/* Egress Control Vector */

/* Alternative Routing-ID Interpretation */
#define PCI_ARI_CAP							0x04		/* ARI Capability Register */
#define PCI_ARI_CAP_MFVC					0x0001		/* MFVC Function Groups Capability */
#define PCI_ARI_CAP_ACS						0x0002		/* ACS Function Groups Capability */
#define PCI_ARI_CAP_NFN(x)					(((x) >> 8) & 0xff) /* Next Function Number */
#define PCI_ARI_CTRL						0x06		/* ARI Control Register */
#define PCI_ARI_CTRL_MFVC					0x0001		/* MFVC Function Groups Enable */
#define PCI_ARI_CTRL_ACS					0x0002		/* ACS Function Groups Enable */
#define PCI_ARI_CTRL_FG(x)					(((x) >> 4) & 7) /* Function Group */

/* Address Translation Service */
#define PCI_ATS_CAP							0x04		 /* ATS Capability Register */
#define PCI_ATS_CAP_IQD(x)					((x) & 0x1f) /* Invalidate Queue Depth */
#define PCI_ATS_CTRL						0x06		 /* ATS Control Register */
#define PCI_ATS_CTRL_STU(x)					((x) & 0x1f) /* Smallest Translation Unit */
#define PCI_ATS_CTRL_ENABLE					0x8000		 /* ATS Enable */

/* Single Root I/O Virtualization */
#define PCI_IOV_CAP							0x04		/* SR-IOV Capability Register */
#define PCI_IOV_CAP_VFM						0x00000001	/* VF Migration Capable */
#define PCI_IOV_CAP_IMN(x)					((x) >> 21) /* VF Migration Interrupt Message Number */
#define PCI_IOV_CTRL						0x08		/* SR-IOV Control Register */
#define PCI_IOV_CTRL_VFE					0x0001		/* VF Enable */
#define PCI_IOV_CTRL_VFME					0x0002		/* VF Migration Enable */
#define PCI_IOV_CTRL_VFMIE					0x0004		/* VF Migration Interrupt Enable */
#define PCI_IOV_CTRL_MSE					0x0008		/* VF MSE */
#define PCI_IOV_CTRL_ARI					0x0010		/* ARI Capable Hierarchy */
#define PCI_IOV_STATUS						0x0a		/* SR-IOV Status Register */
#define PCI_IOV_STATUS_MS					0x0001		/* VF Migration Status */
#define PCI_IOV_INITIALVF					0x0c		/* Number of VFs that are initially associated */
#define PCI_IOV_TOTALVF						0x0e		/* Maximum number of VFs that could be associated */
#define PCI_IOV_NUMVF						0x10		/* Number of VFs that are available */
#define PCI_IOV_FDL							0x12		/* Function Dependency Link */
#define PCI_IOV_OFFSET						0x14		/* First VF Offset */
#define PCI_IOV_STRIDE						0x16		/* Routing ID offset from one VF to the next one */
#define PCI_IOV_DID							0x1a		/* VF Device ID */
#define PCI_IOV_SUPPS						0x1c		/* Supported Page Sizes */
#define PCI_IOV_SYSPS						0x20		/* System Page Size */
#define PCI_IOV_BAR_BASE					0x24		/* VF BAR0, VF BAR1, ... VF BAR5 */
#define PCI_IOV_NUM_BAR						6			/* Number of VF BARs */
#define PCI_IOV_MSAO						0x3c		/* VF Migration State Array Offset */
#define PCI_IOV_MSA_BIR(x)					((x) & 7)	/* VF Migration State BIR */
#define PCI_IOV_MSA_OFFSET(x)				((x) & 0xfffffff8) /* VF Migration State Offset */

/*
 * The PCI interface treats multi-function devices as independent
 * devices. The slot/function address of each device is encoded
 * in a single byte as follows:
 *
 *	7:3 = slot
 *	2:0 = function
 */
#define PCI_DEVFN(slot,func)					((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_SLOT(devfn)						(((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)						((devfn) & 0x07)

/* Device classes and subclasses */
#define PCI_CLASS_NOT_DEFINED					0x0000
#define PCI_CLASS_NOT_DEFINED_VGA				0x0001

// values for the class_sub field for class_base = 0x00 (Device was built prior definition of the class code field)

// values for the class_sub field for class_base = 0x01 (Mass Storage Controller)
#define PCI_BASE_CLASS_STORAGE					0x01
#define PCI_CLASS_STORAGE_SCSI					0x0100
#define PCI_CLASS_STORAGE_IDE					0x0101
#define PCI_CLASS_STORAGE_FLOPPY				0x0102
#define PCI_CLASS_STORAGE_IPI					0x0103
#define PCI_CLASS_STORAGE_RAID					0x0104
#define PCI_CLASS_STORAGE_ATA					0x0105
#define PCI_CLASS_STORAGE_SATA					0x0106
#define PCI_CLASS_STORAGE_SATA_AHCI				0x010601
#define PCI_CLASS_STORAGE_SAS					0x0107
#define PCI_CLASS_STORAGE_OTHER					0x0180

// values for the class_sub field for class_base = 0x02 (Network Controller)
#define PCI_BASE_CLASS_NETWORK					0x02
#define PCI_CLASS_NETWORK_ETHERNET				0x0200
#define PCI_CLASS_NETWORK_TOKEN_RING			0x0201
#define PCI_CLASS_NETWORK_FDDI					0x0202
#define PCI_CLASS_NETWORK_ATM					0x0203
#define PCI_CLASS_NETWORK_ISDN					0x0204
#define PCI_CLASS_NETWORK_OTHER					0x0280

// values for the class_sub field for class_base = 0x03 (Display Controller)
#define PCI_BASE_CLASS_DISPLAY					0x03
#define PCI_CLASS_DISPLAY_VGA					0x0300
#define PCI_CLASS_DISPLAY_XGA					0x0301
#define PCI_CLASS_DISPLAY_3D					0x0302
#define PCI_CLASS_DISPLAY_OTHER					0x0380

// values for the class_sub field for class_base = 0x04 (Multimedia Controller)
#define PCI_BASE_CLASS_MULTIMEDIA				0x04
#define PCI_CLASS_MULTIMEDIA_VIDEO				0x0400 /* video */
#define PCI_CLASS_MULTIMEDIA_AUDIO				0x0401 /* audio */
#define PCI_CLASS_MULTIMEDIA_PHONE				0x0402
#define PCI_CLASS_MULTIMEDIA_AUDIO_DEV				0x0403 /* HD audio */
#define PCI_CLASS_MULTIMEDIA_OTHER				0x0480

// values for the class_sub field for class_base = 0x05 (Memory Controller)
#define PCI_BASE_CLASS_MEMORY					0x05
#define PCI_CLASS_MEMORY_RAM					0x0500
#define PCI_CLASS_MEMORY_FLASH					0x0501
#define PCI_CLASS_MEMORY_OTHER					0x0580

// values for the class_sub field for class_base = 0x06 (Bridge Device)
#define PCI_BASE_CLASS_BRIDGE					0x06
#define PCI_CLASS_BRIDGE_HOST					0x0600
#define PCI_CLASS_BRIDGE_ISA					0x0601
#define PCI_CLASS_BRIDGE_EISA					0x0602
#define PCI_CLASS_BRIDGE_MC					0x0603
#define PCI_CLASS_BRIDGE_PCI					0x0604
#define PCI_CLASS_BRIDGE_PCMCIA					0x0605
#define PCI_CLASS_BRIDGE_NUBUS					0x0606
#define PCI_CLASS_BRIDGE_CARDBUS				0x0607
#define PCI_CLASS_BRIDGE_RACEWAY				0x0608
#define PCI_CLASS_BRIDGE_PCI_SEMI				0x0609
#define PCI_CLASS_BRIDGE_IB_TO_PCI				0x060a
#define PCI_CLASS_BRIDGE_OTHER					0x0680

// values for the class_sub field for class_base = 0x07 (Simple Communications Controllers)
#define PCI_BASE_CLASS_COMMUNICATION				0x07
#define PCI_CLASS_COMMUNICATION_SERIAL				0x0700
#define PCI_CLASS_COMMUNICATION_PARALLEL			0x0701
#define PCI_CLASS_COMMUNICATION_MSERIAL				0x0702
#define PCI_CLASS_COMMUNICATION_MODEM				0x0703
#define PCI_CLASS_COMMUNICATION_OTHER				0x0780

// values for the class_sub field for class_base = 0x08 (Base System Peripherals)
#define PCI_BASE_CLASS_SYSTEM					0x08
#define PCI_CLASS_SYSTEM_PIC					0x0800
#define PCI_CLASS_SYSTEM_PIC_IOAPIC				0x080010
#define PCI_CLASS_SYSTEM_PIC_IOXAPIC				0x080020 // I/O APIC interrupt controller , 32 bye none-prefectable memory.
#define PCI_CLASS_SYSTEM_DMA					0x0801
#define PCI_CLASS_SYSTEM_TIMER					0x0802
#define PCI_CLASS_SYSTEM_RTC					0x0803
#define PCI_CLASS_SYSTEM_PCI_HOTPLUG				0x0804 // HotPlug Controller
#define PCI_CLASS_SYSTEM_SDHCI					0x0805
#define PCI_CLASS_SYSTEM_OTHER					0x0880

// values for the class_sub field for class_base = 0x09 (Input Devices)
#define PCI_BASE_CLASS_INPUT					0x09
#define PCI_CLASS_INPUT_KEYBOARD				0x0900
#define PCI_CLASS_INPUT_PEN					0x0901
#define PCI_CLASS_INPUT_MOUSE					0x0902
#define PCI_CLASS_INPUT_SCANNER					0x0903
#define PCI_CLASS_INPUT_GAMEPORT				0x0904
#define PCI_CLASS_INPUT_OTHER					0x0980

// values for the class_sub field for class_base = 0x0a (Docking Stations)
#define PCI_BASE_CLASS_DOCKING					0x0a
#define PCI_CLASS_DOCKING_GENERIC				0x0a00
#define PCI_CLASS_DOCKING_OTHER					0x0a80

// values for the class_sub field for class_base = 0x0b (processor)
#define PCI_BASE_CLASS_PROCESSOR				0x0b
#define PCI_CLASS_PROCESSOR_386					0x0b00
#define PCI_CLASS_PROCESSOR_486					0x0b01
#define PCI_CLASS_PROCESSOR_PENTIUM				0x0b02
#define PCI_CLASS_PROCESSOR_ALPHA				0x0b10
#define PCI_CLASS_PROCESSOR_POWERPC				0x0b20
#define PCI_CLASS_PROCESSOR_MIPS				0x0b30
#define PCI_CLASS_PROCESSOR_CO					0x0b40 // Co-Processor

// values for the class_sub field for class_base = 0x0c (serial bus controller)
#define PCI_BASE_CLASS_SERIAL					0x0c
#define PCI_CLASS_SERIAL_FIREWIRE				0x0c00   /* FireWire (IEEE 1394) */
#define PCI_CLASS_SERIAL_FIREWIRE_OHCI				0x0c10
#define PCI_CLASS_SERIAL_ACCESS					0x0c01
#define PCI_CLASS_SERIAL_SSA					0x0c02
#define PCI_CLASS_SERIAL_USB					0x0c03  /* Universal Serial Bus */
#define PCI_IF_UHCI					0x00    /* Universal Host Controller Interface */
#define PCI_IF_OHCI					0x10    /* Open Host Controller Interface */
#define PCI_IF_EHCI					0x20    /* Enhanced Host Controller Interface */
#define PCI_IF_XHCI					0x30    /* Extensible Host Controller Interface */
#define PCI_CLASS_SERIAL_FIBER					0x0c04
#define PCI_CLASS_SERIAL_SMBUS					0x0c05
#define PCI_CLASS_SERIAL_INFINIBAND				0x0c06

// values for the class_sub field for class_base = 0x0d (Wireless Controller)
#define PCI_BASE_CLASS_WIRELESS					0x0d
#define PCI_CLASS_WIRELESS_IRDA					0x0d00
#define PCI_CLASS_WIRELESS_IR					0x0d01
#define PCI_CLASS_WIRELESS_RF					0x0d10
#define PCI_CLASS_WIRELESS_BLUETOOTH			0x0d11
#define PCI_CLASS_WIRELESS_BROADBAND			0x0d12
#define PCI_CLASS_WIRELESS_80211A				0x0d20
#define PCI_CLASS_WIRELESS_80211B				0x0d21
#define PCI_CLASS_WIRELESS_WHCI					0x0d1010
#define PCI_CLASS_WIRELESS_OTHER				0x80

// values for the class_sub field for class_base = 0x0e (Intelligent I/O Controller)
#define PCI_BASE_CLASS_INTELLIGENT				0x0e
#define PCI_CLASS_INTELLIGENT_I2O				0x0e00

// values for the class_sub field for class_base = 0x0f (Satellite Communication Controller)
#define PCI_BASE_CLASS_SATELLITE				0x0f
#define PCI_CLASS_SATELLITE_TV					0x0f00
#define PCI_CLASS_SATELLITE_AUDIO				0x0f01
#define PCI_CLASS_SATELLITE_VOICE				0x0f03
#define PCI_CLASS_SATELLITE_DATA				0x0f04

// values for the class_sub field for class_base = 0x10 (Encryption and decryption controller)
#define PCI_BASE_CLASS_CRYPT					0x10
#define PCI_CLASS_CRYPT_NETWORK					0x1000
#define PCI_CLASS_CRYPT_ENTERTAINMENT				0x1010
#define PCI_CLASS_CRYPT_OTHER					0x1080
// values for the class_sub field for class_base = 0x12 (Data Acquisition and Signal Processing Controllers)
#define PCI_BASE_CLASS_SIGNAL					0x11
#define PCI_CLASS_SIGNAL_DPIO					0x1100
#define PCI_CLASS_SIGNAL_PERF_CTR				0x1101
#define PCI_CLASS_SIGNAL_SYNCHRONIZER				0x1110
#define PCI_CLASS_SIGNAL_OTHER					0x1180

// values for the class_sub field for class_base = 0xff (Device does not fit any defined class)
#define PCI_CLASS_OTHERS                        0xff

/* Several ID's we need in the library */
#define PCI_VENDOR_ID_APPLE					0x106b
#define PCI_VENDOR_ID_AMD					0x1022
#define PCI_VENDOR_ID_ATI					0x1002
#define PCI_VENDOR_ID_INTEL					0x8086
#define PCI_VENDOR_ID_NVIDIA				0x10de
#define PCI_VENDOR_ID_REALTEK				0x10ec
#define PCI_VENDOR_ID_TEXAS_INSTRUMENTS 	0x104c
#define PCI_VENDOR_ID_VIA					0x1106

#endif /* !__LIBSAIO_PCI_H */
