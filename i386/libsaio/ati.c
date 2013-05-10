/*
 * ATI Graphics Card Enabler, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "ati.h"


static const char *chip_family_name[] = {
	"UNKNOW",
	"R420",
	"RV410",
	"RV515",
	"R520",
	"RV530",
	"RV560",
	"RV570",
	"R580",
	/* IGP */
	"RS600",
	"RS690",
	"RS740",
	"RS780",
	"RS880",
	/* R600 */
	"R600",
	"RV610",
	"RV620",
	"RV630",
	"RV635",
	"RV670",
	/* R700 */
	"RV710",
	"RV730",
	"RV740",
	"RV772",
	"RV770",
	"RV790",
	/* Evergreen */
	"Cedar",
	"Cypress",
	"Hemlock",
	"Juniper",
	"Redwood",
	"Broadway",
	//	"Madison",
	//	"Park",
	/* Northern Islands */
	//	"Antilles",
	"Barts",
	"Caicos",
	"Cayman",
	"Turks",
	/* Southern Islands */
	"Tahiti",
	"Pitcairn",
	//	"CapeVerde",
	//	"Thames",
	//	"Lombok",
	//	"NewZealand",
	""
};


static card_config_t card_configs[] = {
	{NULL,		0},
	/* OLDController */
	{"Wormy",	2},
	{"Alopias",	2},
	{"Caretta",	1},
	{"Kakapo",	3},
	{"Kipunji",	4},
	{"Peregrine",	2},
	{"Raven",	3},
	{"Sphyrna",	1},
	/* AMD2400Controller */
	{"Iago",	2},
	/* AMD2600Controller */
	{"Hypoprion",	2},
	{"Lamna",	2},
	/* AMD3800Controller */
	{"Megalodon",	3},
	{"Triakis",	2},
	/* AMD4600Controller */
	{"Flicker",	3},
	{"Gliff",	3},
	{"Shrike",	3},
	/* AMD4800Controller */
	{"Cardinal",	2},
	{"Motmot",	2},
	{"Quail",	3},
	/* AMD5000Controller */
	{"Douc",	2},
	{"Langur",	3},
	{"Uakari",	4},
	{"Zonalis",	6},
	{"Alouatta",	4},
	{"Hoolock",	3},
	{"Vervet",	4},
	{"Baboon",	3},
	{"Eulemur",	3},
	{"Galago",	2},
	{"Colobus",	2},
	{"Mangabey",	2},
	{"Nomascus",	4},
	{"Orangutan",	2},
	/* AMD6000Controller */
	{"Pithecia",	3},
	{"Bulrushes",	6},
	{"Cattail",	4},
	{"Hydrilla",	5},
	{"Duckweed",	4},
	{"Fanwort",	4},
	{"Elodea",	5},
	{"Kudzu",	2},
	{"Gibba",	5},
	{"Lotus",	3},
	{"Ipomoea",	3},
	{"Muskgrass",	4},
	{"Juncus",	4},
	{"Osmunda",     4},
	{"Pondweed",    3},
	{"Spikerush",   4},
	{"Typha",       5},
	/* AMD7000Controller */
	{"Aji",         4},
	{"Buri",        4},
	{"Chutoro",     5},
	{"Dashimaki",   4},
	{"Ebi",         5},
	{"Gari",        5},
	{"Futomaki",    4},
	{"Hamachi",     4},
	{"OPM",         6},
	{"Ikura",       6},
	{"IkuraS",      1}
};





static radeon_card_info_t radeon_cards[] = {
	
	// Earlier cards are not supported
	//
	// Layout is device_id, subsys_id (subsystem id plus vendor id), chip_family_name, display name, frame buffer
	// Cards are grouped by device id and vendor id then sorted by subsystem id to make it easier to add new cards
	//

	{ 0x9400,	0x01611043, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",                kNull		},
	{ 0x9400,	0x021E1043, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",                kNull		},
	{ 0x9400,	0x25521002, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",                kNull		},
	{ 0x9400,	0x30001002, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 PRO",               kNull		},
	{ 0x9400,	0x31421002, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",                kNull		},

	{ 0x9440,	0x05021002, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",                   kMotmot		},
	{ 0x9440,	0x0851174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",                   kMotmot		},
	{ 0x9440,	0x114A174B, CHIP_FAMILY_RV770,		"Sapphire Radeon HD4870 Vapor-X",       kCardinal	},
	{ 0x9440,	0x24401682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",                   kMotmot		},
	{ 0x9440,	0x24411682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",                   kMotmot		},
	{ 0x9440,	0x24441682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",                   kMotmot		},
	{ 0x9440,	0x24451682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",                   kMotmot		},

	{ 0x9441,	0x02601043, CHIP_FAMILY_RV770,		"ASUS EAH4870x2",                kMotmot		},
	{ 0x9441,	0x02841043, CHIP_FAMILY_RV770,		"ASUS EAH4870x2",                kMotmot		},
	{ 0x9441,	0x24401682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870 X2",                kMotmot		},
	{ 0x9441,	0x25421002, CHIP_FAMILY_RV770,		"PowerColor HD 4870 X2",                kMotmot		},

	{ 0x9442,	0x05021002, CHIP_FAMILY_RV770,		"MSI R4850-T2D512",                     kMotmot		},
	{ 0x9442,	0x080110B0, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x9442,	0x24701682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x9442,	0x24711682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x9442,	0xE104174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x9442,	0xE810174B, CHIP_FAMILY_RV770,		"Sapphire HD 4850",                     kMotmot		},

	{ 0x944A,	0x02A21028, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x944A,	0x30001043, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x944A,	0x30001458, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x944A,	0x30001462, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x944A,	0x30001545, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x944A,	0x30001682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x944A,	0x3000174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x944A,	0x30001787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},
	{ 0x944A,	0x300017AF, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",                   kMotmot		},

	{ 0x944C,	0x05021002, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830",                   kMotmot		},
	{ 0x944C,	0x20031787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830",                   kMotmot		},
	{ 0x944C,	0x24801682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830",                   kMotmot		},
	{ 0x944C,	0x24811682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830",                   kMotmot		},

	{ 0x944E,	0x30001787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4730",                   kMotmot		},
	{ 0x944E,	0x30101787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810",                   kMotmot		},
	{ 0x944E,	0x31001787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4820",                   kMotmot		},
	{ 0x944E,	0x3260174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810",                   kMotmot		},
	{ 0x944E,	0x3261174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810",                   kMotmot		},

	{ 0x9460,	0x27021682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4890",                   kMotmot		},
	{ 0x9460,	0xE115174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4890",                   kMotmot		},
	{ 0x9460,	0xE118174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4890",                   kMotmot		},

	{ 0x9480,	0x01211025, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",                  kGliff		},
 	{ 0x9480,	0x03111025, CHIP_FAMILY_RV730,		"ATI Radeon HD 5165M",                  kPeregrine		},
 	{ 0x9480,	0x03121025, CHIP_FAMILY_RV730,		"ATI Radeon HD 5165M",                  kPeregrine		},
 	{ 0x9480,	0x031C1025, CHIP_FAMILY_RV730,		"ATI Radeon HD 5165M",                  kPeregrine		},
 	{ 0x9480,	0x031D1025, CHIP_FAMILY_RV730,		"ATI Radeon HD 5165M",                  kPeregrine		},
 	{ 0x9480,	0x036C1025, CHIP_FAMILY_RV730,		"ATI Radeon HD 5165M",                  kPeregrine		},
 	{ 0x9480,	0x036D1025, CHIP_FAMILY_RV730,		"ATI Radeon HD 5165M",                  kPeregrine		},
	{ 0x9480,	0x3628103C, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",                  kGliff		},
	{ 0x9480,	0x9035104D, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",                  kGliff		},
	{ 0x9480,	0xFD001179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",                  kPeregrine		},
	{ 0x9480,	0xFDD01179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",                  kPeregrine		},
	{ 0x9480,	0xFD121179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",                  kPeregrine		},
	{ 0x9480,	0xFD501179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",                  kPeregrine		},
	{ 0x9480,	0xFF001179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600M",                  kPeregrine		},
	{ 0x9480,	0xFF151179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600M",                  kPeregrine		},
	{ 0x9480,	0xFF221179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600M",                  kPeregrine		},
	{ 0x9480,	0xFF501179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600M",                  kPeregrine		},
	{ 0x9480,	0xFF801179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600M",                  kPeregrine		},
	{ 0x9480,	0xFF821179, CHIP_FAMILY_RV730,		"ATI Radeon HD 5165M",                  kPeregrine		},
	{ 0x9480,	0xFFA01179, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600M",                  kPeregrine		},
	{ 0x9480,	0xFFA21179, CHIP_FAMILY_RV730,		"ATI Radeon HD 5165M",                  kPeregrine		},

	{ 0x9490,	0x20031787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4670",                   kFlicker	},
	{ 0x9490,	0x25421028, CHIP_FAMILY_RV730,		"ATI Radeon HD 4670",                   kFlicker	},
	{ 0x9490,	0x30501787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710",                   kPeregrine		},
	{ 0x9490,	0x300017AF, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710",                   kPeregrine		},
	{ 0x9490,	0x4710174B, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710",                   kPeregrine		},

	{ 0x9498,	0x10001043, CHIP_FAMILY_RV730,		"ASUS EAHG4670",                   kPeregrine		},
	{ 0x9498,	0x20091787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650",                   kPeregrine		},
	{ 0x9498,	0x21CF1458, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600",                   kPeregrine		},
	{ 0x9498,	0x24511682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650",                   kPeregrine		},
	{ 0x9498,	0x24521682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650",                   kPeregrine		},
	{ 0x9498,	0x24541682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650",                   kPeregrine		},
	{ 0x9498,	0x29331682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4670",                   kPeregrine		},
	{ 0x9498,	0x29341682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4670",                   kPeregrine		},
	{ 0x9498,	0x30501787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4700",                   kPeregrine		},
	{ 0x9498,	0x31001787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4720",                   kPeregrine		},

	{ 0x94B3,	0x0D001002, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",                   kFlicker	},
	{ 0x94B3,	0x1170174B, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",                   kFlicker	},
	{ 0x94B3,	0x29001682, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",                   kFlicker	},

	{ 0x94C1,	0x0D021002, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",                kNull		},
	{ 0x94C1,	0x10021002, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Pro",               kNull		},
	{ 0x94C1,	0x0D021028, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",                kNull		},
	{ 0x94C1,	0x21741458, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",                kNull		},
	{ 0x94C1,	0x10331462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",                kNull		},
	{ 0x94C1,	0x10401462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",                kNull		},
	{ 0x94C1,	0x11101462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",                kNull		},

	{ 0x94C3,	0x03421002, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0x01011A93, CHIP_FAMILY_RV610,		"Qimonda Radeon HD 2400 PRO",			kNull		},
	{ 0x94C3,	0x03021028, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0x03421002, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0x04021028, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0x10321462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0x10411462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400",                   kNull		},
	{ 0x94C3,	0x11041462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400",                   kNull		},
	{ 0x94C3,	0x11051462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400",                   kNull		},
	{ 0x94C3,	0x203817AF, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400",                   kNull		},
	{ 0x94C3,	0x216A1458, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0x21721458, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0x2247148C, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 LE",                kNull		},
	{ 0x94C3,	0x22471787, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 LE",                kNull		},
	{ 0x94C3,	0x30001025, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350",                   kNull		},
	{ 0x94C3,	0x30001458, CHIP_FAMILY_RV610,		"ATI Radeon HD 3410",                   kNull		},
	{ 0x94C3,	0x30001462, CHIP_FAMILY_RV610,		"ATI Radeon HD 3410",                   kNull		},
	{ 0x94C3,	0x3000148C, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350",                   kNull		},
	{ 0x94C3,	0x30001642, CHIP_FAMILY_RV610,		"ATI Radeon HD 3410",                   kNull		},
	{ 0x94C3,	0x3000174B, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350",                   kNull		},
	{ 0x94C3,	0x30001787, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350",                   kNull		},
	{ 0x94C3,	0x37161642, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0x94C31002, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0xE370174B, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},
	{ 0x94C3,	0xE400174B, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",               kNull		},

	{ 0x9501,	0x25421002, CHIP_FAMILY_RV670,		"ATI Radeon HD 3870",                   kNull		},
	{ 0x9501,	0x30001002, CHIP_FAMILY_RV670,		"ATI Radeon HD 3690",                   kNull		},
	{ 0x9501,	0x3000174B, CHIP_FAMILY_RV670,		"Sapphire Radeon HD 3690",              kNull		},
	{ 0x9501,	0x30001787, CHIP_FAMILY_RV670,		"ATI Radeon HD 3690",                   kNull		},
	{ 0x9501,	0x4750174B, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",                   kNull		},

	{ 0x9505,	0x024A1043, CHIP_FAMILY_RV670,		"ASUS MA 3850",                    kNull		},
	{ 0x9505,	0x25421002, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850",                   kNull		},
	{ 0x9505,	0x30001002, CHIP_FAMILY_RV630,		"ATI Radeon HD 3690",                   kNull		},
	{ 0x9505,	0x3000148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850",                   kNull		},
	{ 0x9505,	0x3000174B, CHIP_FAMILY_RV670,		"Sapphire Radeon HD 3690",              kNull		},
	{ 0x9505,	0x30001787, CHIP_FAMILY_RV630,		"ATI Radeon HD 3690",                   kNull		},
	{ 0x9505,	0x30011043, CHIP_FAMILY_RV670,		"ATI Radeon HD 4730",                   kNull		},
	{ 0x9505,	0x3001148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 4730",                   kNull		},
	{ 0x9505,	0x3001174B, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",                   kNull		},
	{ 0x9505,	0x3002148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 4730",                   kNull		},
	{ 0x9505,	0x3003148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",                   kNull		},
	{ 0x9505,	0x3004148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",                   kNull		},
	{ 0x9505,	0x3010174B, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",                   kNull		},
	{ 0x9505,	0x301017AF, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",                   kNull		},
	{ 0x9505,	0x4730174B, CHIP_FAMILY_RV670,		"ATI Radeon HD 4730",                   kNull		},
	{ 0x9505,	0xE630174B, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",            kNull		},

	{ 0x9540,	0x30501787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4590",                   kPeregrine		},
	{ 0x9540,	0x4590174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4590",                   kPeregrine		},

	{ 0x954F,	0x16131462, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",                   kPeregrine		},
	{ 0x954F,	0x20081787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4350",                   kPeregrine		},
	{ 0x954F,	0x29201682, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",                   kPeregrine		},
	{ 0x954F,	0x29211682, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",                   kPeregrine		},
	{ 0x954F,	0x3000174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4520",                   kPeregrine		},
	{ 0x954F,	0x301017AF, CHIP_FAMILY_RV710,		"ATI Radeon HD 4450",                   kPeregrine		},
	{ 0x954F,	0x30501787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4450",                   kPeregrine		},
	{ 0x954F,	0x30901682, CHIP_FAMILY_RV710,		"XFX Radeon HD 4570",                   kPeregrine		},
	{ 0x954F,	0x31001787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4520",                   kPeregrine		},
	{ 0x954F,	0x4450174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4450",                   kPeregrine		},
	{ 0x954F,	0x4570174B, CHIP_FAMILY_RV710,		"Sapphire Radeon HD 4570",              kPeregrine		},
	{ 0x954F,	0x66661043, CHIP_FAMILY_RV710,		"ASUS EAH4550",                   kPeregrine		},
	{ 0x954F,	0xE990174B, CHIP_FAMILY_RV710,		"Sapphire Radeon HD 4350",              kPeregrine		},

	{ 0x9552,	0x04341028, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4330",          kShrike		},
	{ 0x9552,	0x21AC1458, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500",              kPeregrine		},
	{ 0x9552,	0x21ED1458, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500",              kPeregrine		},
	{ 0x9552,	0x308B103C, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4330",          kShrike		},
	{ 0x9552,	0x3000148C, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500",              kPeregrine		},
	{ 0x9552,	0x3000174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500",              kPeregrine		},
	{ 0x9552,	0x30001787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500",              kPeregrine		},
	{ 0x9552,	0x300017AF, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500",              kPeregrine		},
	{ 0x9552,	0x44721545, CHIP_FAMILY_RV710,		"VisionTek Radeon 4350",                kPeregrine		},

	{ 0x9553,	0x18751043, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4570",			kShrike		},
	{ 0x9553,	0x1B321043, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4570",			kShrike		},
	{ 0x9553,	0x215B17AA, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4530/4570",			kShrike		},
	{ 0x9553,	0x3092174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",		kPeregrine		},
	{ 0x9553,	0x39291642, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4570",			kPeregrine		},
	{ 0x9553,	0xFD001179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 5145",			kPeregrine		},
	{ 0x9553,	0xFD121179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 5145",			kPeregrine		},
	{ 0x9553,	0xFD501179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 5145",			kPeregrine		},
	{ 0x9553,	0xFD921179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 5145",			kPeregrine		},
	{ 0x9553,	0xFDD01179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 5145",			kPeregrine		},
	{ 0x9553,	0xFF001179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500",			kPeregrine		},
	{ 0x9553,	0xFF151179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500",			kPeregrine		},
	{ 0x9553,	0xFF161179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 5145",			kPeregrine		},
	{ 0x9553,	0xFF221179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500",			kPeregrine		},
	{ 0x9553,	0xFF401179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500",			kPeregrine		},
	{ 0x9553,	0xFF501179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500",			kPeregrine		},
	{ 0x9553,	0xFF801179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500",			kPeregrine		},
	{ 0x9553,	0xFF821179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 5145",			kPeregrine		},
	{ 0x9553,	0xFFA01179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 5145",			kPeregrine		},
	{ 0x9553,	0xFFA21179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500",			kPeregrine		},
	{ 0x9553,	0xFFC01179, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500",			kPeregrine		},

 	{ 0x9555,	0x29241682, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",                   kNull		},
	{ 0x9555,	0x24651682, CHIP_FAMILY_RV710,		"ATI Radeon HD4300/HD4500",             kNull		}, 
	{ 0x9555,	0x3711174B, CHIP_FAMILY_RV710,		"ATI Radeon HD4300/HD4500",             kNull		},

	{ 0x9581,	0x011F1025, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600",                   kNull		},
	{ 0x9581,	0x0562107B, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600",                   kNull		},
	{ 0x9581,	0x15621043, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600",                   kNull		},
	{ 0x9581,	0x3000148C, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600",                   kNull		},
	{ 0x9581,	0x30C5103C, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600",                   kNull		},
	{ 0x9581,	0x3C2D17AA, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600",                   kNull		},
	{ 0x9581,	0x63F61462, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600",                   kNull		},
	{ 0x9581,	0x95811002, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600",                   kNull		},
	{ 0x9581,	0xFF001179, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600",                   kNull		},
	{ 0x9581,	0xFF011179, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600",                   kNull		},

	{ 0x9583,	0x0083106B, CHIP_FAMILY_RV630,		"ATI Mobility Radeon HD 2600 XT",		kNull		},
	{ 0x9583,	0x11071734, CHIP_FAMILY_RV630,		"ATI Mobility Radeon HD 2600 XT",		kNull		},
	{ 0x9583,	0x3000148C, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600",                   kNull		},
	{ 0x9583,	0x30D4103C, CHIP_FAMILY_RV630,		"ATI Mobility Radeon HD 2600 XT",		kNull		},

	{ 0x9588,	0x01021A93, CHIP_FAMILY_RV630,		"Qimonda Radeon HD 2600 XT",			kNull		},

	{ 0x9589,	0x01001A93, CHIP_FAMILY_RV630,		"Qimonda Radeon HD 2600 PRO",			kNull		},
	{ 0x9589,	0x0E41174B, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600",                   kNull		},
	{ 0x9589,	0x30001462, CHIP_FAMILY_RV630,		"ATI Radeon HD 3610",                   kNull		},
	{ 0x9589,	0x30001642, CHIP_FAMILY_RV630,		"ATI Radeon HD 3610",                   kNull		},
	{ 0x9589,	0x30001787, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600",                   kNull		},

	{ 0x9591,	0x15453992, CHIP_FAMILY_RV635,		"ATI Radeon HD 3600",                   kNull		},
	{ 0x9591,	0x2303148C, CHIP_FAMILY_RV635,		"VisionTek Radeon HD 2600/3600 Series", kNull		},

	{ 0x9598,	0xB3831002, CHIP_FAMILY_RV635,		"ATI All-in-Wonder HD",                 kNull		},
	{ 0x9598,	0x30001043, CHIP_FAMILY_RV635,		"ATI Radeon HD 3730",                   kNull		},
	{ 0x9598,	0x3000148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 3730",                   kNull		},
	{ 0x9598,	0x30001545, CHIP_FAMILY_RV635,		"VisionTek Radeon HD 2600 XT",			kNull		},
	{ 0x9598,	0x3000174B, CHIP_FAMILY_RV635,		"Sapphire Radeon HD 3730",              kNull		},
	{ 0x9598,	0x30011043, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",                   kNull		},
	{ 0x9598,	0x3001148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 4580",                   kNull		},
	{ 0x9598,	0x3031148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",                   kNull		},
	{ 0x9598,	0x30011545, CHIP_FAMILY_RV635,		"VisionTek Radeon HD 2600 Pro",			kNull		},
	{ 0x9598,	0x3001174B, CHIP_FAMILY_RV635,		"Sapphire Radeon HD 3750",              kNull		},
	{ 0x9598,	0x300117AF, CHIP_FAMILY_RV635,		"ATI Radeon HD 3750",                   kNull		},
	{ 0x9598,	0x301017AF, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",                   kNull		},
	{ 0x9598,	0x301117AF, CHIP_FAMILY_RV635,		"ATI Radeon HD 4580",                   kNull		},
	{ 0x9598,	0x30501787, CHIP_FAMILY_RV635,		"ATI Radeon HD 4610",                   kNull		},
	{ 0x9598,	0x4570174B, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",                   kNull		},
	{ 0x9598,	0x4580174B, CHIP_FAMILY_RV635,		"ATI Radeon HD 4580",                   kNull		},
	{ 0x9598,	0x4610174B, CHIP_FAMILY_RV635,		"ATI Radeon HD 4610",                   kNull		},

	{ 0x95C0,	0x3000148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 3550",                   kNull		},
	{ 0x95C0,	0x3000174B, CHIP_FAMILY_RV620,		"Sapphire Radeon HD 3550",              kNull		},
	{ 0x95C0,	0x3002174B, CHIP_FAMILY_RV620,		"ATI Radeon HD 3570",                   kNull		},
	{ 0x95C0,	0x3020174B, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",                   kNull		},
	{ 0x95C0,	0xE3901745, CHIP_FAMILY_RV620,		"ATI Radeon HD 3550",                   kNull		},

	{ 0x95C5,	0x01041A93, CHIP_FAMILY_RV620,		"Qimonda Radeon HD 3450",               kNull		},
	{ 0x95C5,	0x01051A93, CHIP_FAMILY_RV620,		"Qimonda Radeon HD 3450",               kNull		},
	{ 0x95C5,	0x3000148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 3450",                   kNull		},
	{ 0x95C5,	0x3001148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 3550",                   kNull		},
	{ 0x95C5,	0x3002148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 4230",                   kNull		},
	{ 0x95C5,	0x3003148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",                   kNull		},
	{ 0x95C5,	0x3010174B, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",                   kNull		},
	{ 0x95C5,	0x301017AF, CHIP_FAMILY_RV620,		"ATI Radeon HD 4230",                   kNull		},
	{ 0x95C5,	0x3032148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",                   kNull		},
	{ 0x95C5,	0x3033148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 4230",                   kNull		},
	{ 0x95C5,	0x30501787, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",                   kNull		},
	{ 0x95C5,	0x4250174B, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",                   kNull		},

	/* Evergreen */
	{ 0x6898,	0x00D0106B, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",                   kLangur		},
	{ 0x6898,	0x032E1043, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",                   kUakari		},
	{ 0x6898,	0x038C1043, CHIP_FAMILY_CYPRESS,	"ASUS 5870 Eyefinity 6",                kNull		},
	{ 0x6898,	0x0B001002, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",                   kZonalis	},
	{ 0x6898,	0x21E51458, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",                   kUakari		},
	{ 0x6898,	0x29611682, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",                   kUakari		},
	{ 0x6898,	0xE140174B, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",                   kUakari		},

	{ 0x6899,	0x200A1787, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",                   kUakari		},
	{ 0x6899,	0x21E41458, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",                   kUakari		},
	{ 0x6899,	0x22901787, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",                   kUakari		},
	{ 0x6899,	0xE140174B, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",                   kUakari		},
	{ 0x6899,	0xE174174B, CHIP_FAMILY_CYPRESS,	"ATI Sapphire Radeon HD 5850",          kUakari		},

	{ 0x689C,	0x034A1043, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5970",                   kUakari		},
	{ 0x689C,	0x03521043, CHIP_FAMILY_HEMLOCK,	"ASUS ARES",                            kUakari		},
	{ 0x689C,	0x039E1043, CHIP_FAMILY_HEMLOCK,	"ASUS EAH5870",                         kUakari		},
	{ 0x689C,	0x25421002, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5970",                   kUakari		},
	{ 0x689C,	0x30201682, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5970",                   kUakari		},

	{ 0x68A0,	0x03081025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5830",			kNomascus	},
	{ 0x68A0,	0x030A1025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5830",			kNomascus	},
	{ 0x68A0,	0x043A1028, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5870",			kNomascus	},

	{ 0x68A1,	0x03081025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x030A1025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x03671025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x03681025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x038B1025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x038C1025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x042E1025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x042F1025, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x144D103C, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kNomascus	},
	{ 0x68A1,	0x1522103C, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x22411462, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},
	{ 0x68A1,	0x39961462, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5850",			kHoolock	},

	{ 0x68A8,	0x04421025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x04511025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x048F1028, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6870M",                  kHoolock	},
	{ 0x68A8,	0x04901028, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6870M",                  kHoolock	},
	{ 0x68A8,	0x04B91028, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6870M",                  kHoolock	},
	{ 0x68A8,	0x04BA1028, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6870M",                  kHoolock	},
	{ 0x68A8,	0x050A1025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x050B1025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x050C1025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x050E1025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x050F1025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x05131025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x05141025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x05151025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x05161025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x05251025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x05261025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x056D1025, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0x159B103C, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},
	{ 0x68A8,	0xC0AD144D, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6850M",                  kUakari		},

	{ 0x68B8,	0x00CF106B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kHoolock	},
	{ 0x68B8,	0x0044144D, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6770",                   kVervet		},
	{ 0x68B8,	0x1482174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x200A1787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x200B1787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x21D71458, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x21F61458, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x22881787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x25431002, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x25431458, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x29901682, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x29911682, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x30001002, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6700",                   kVervet		},
	{ 0x68B8,	0x6880103C, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0x6881103C, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6770",                   kVervet		},
	{ 0x68B8,	0xE144174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kHoolock	},
	{ 0x68B8,	0xE147174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0xE160174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",                   kVervet		},
	{ 0x68B8,	0xEA60174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6770",                   kVervet		},

	{ 0x68BA,	0x03FE1043, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},
	{ 0x68BA,	0x1482174B, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},
	{ 0x68BA,	0x174B1482, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},
	{ 0x68BA,	0x200A1787, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},
	{ 0x68BA,	0x21421462, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},
	{ 0x68BA,	0x25431458, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},
	{ 0x68BA,	0x31501682, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},
	{ 0x68BA,	0x31521682, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},
	{ 0x68BA,	0x31531682, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},
	{ 0x68BA,	0xE144174B, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6770",                   kVervet		},

	{ 0x68BE,	0x200A1787, CHIP_FAMILY_JUNIPER,	"HIS ATI 5750",                   kVervet		},
	{ 0x68BE,	0x22881787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5750",                   kVervet		},
	{ 0x68BE,	0x3000148C, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",                   kNull		},
	{ 0x68BE,	0x3000174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",                   kNull		},
	{ 0x68BE,	0x300017AF, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",                   kNull		},
	{ 0x68BE,	0x39821642, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",                   kNull		},

	{ 0x68BF,	0x220E1458, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",                   kVervet		},
	{ 0x68BF,	0x3000148C, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",                   kVervet		},
	{ 0x68BF,	0x31401682, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",                   kVervet		},
	{ 0x68BF,	0x6750174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",                   kVervet		},
	{ 0x68BF,	0xE144174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",                   kHoolock	},

	{ 0x68C0,	0x1594103C, CHIP_FAMILY_REDWOOD,	"AMD Radeon HD 6570M",                  kNull		},
	{ 0x68C0,	0x392717AA, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5730",			kNull		},
	{ 0x68C0,	0x395217AA, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5730",			kNull		},
	{ 0x68C0,	0x84721043, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5000",			kNull		},

	{ 0x68C1,	0x02051025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x02961025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x030A1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x033D1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x033E1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kNull		},
	{ 0x68C1,	0x03471025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x03561025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x03581025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x035A1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x035C1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x03641025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x036D1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kNull		},
	{ 0x68C1,	0x03791025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x037E1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x03821025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		},
	{ 0x68C1,	0x04121025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kNull		},
	{ 0x68C1,	0x042E1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kNull		},
	{ 0x68C1,	0x042F1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kNull		},
	{ 0x68C1,	0x9071104D, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kEulemur	},
	{ 0x68C1,	0x1449103C, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kEulemur	},
	{ 0x68C1,	0xFD001179, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kEulemur	},
	{ 0x68C1,	0xFD121179, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kEulemur	},
	{ 0x68C1,	0xFD501179, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kEulemur	},
	{ 0x68C1,	0xFDD01179, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kEulemur	},

	{ 0x68C8,	0x2306103C, CHIP_FAMILY_REDWOOD,	"ATI FirePro V4800 (FireGL)",			kNull		},
	{ 0x68C8,	0x240A1002, CHIP_FAMILY_REDWOOD,	"ATI FirePro V4800 (FireGL)",			kNull		},
	{ 0x68C8,	0x240A1028, CHIP_FAMILY_REDWOOD,	"ATI FirePro V4800 (FireGL)",			kNull		},

	{ 0x68D8,	0x03561043, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",                   kBaboon		},
	{ 0x68D8,	0x03C01043, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",                   kNull		},
	{ 0x68D8,	0x20091787, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",                   kNull		},
	{ 0x68D8,	0x21D91458, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",                   kBaboon		},
	{ 0x68D8,	0x21F41458, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",                   kNull		},
	{ 0x68D8,	0x22051462, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5690",                   kNull		},
	{ 0x68D8,	0x22941787, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5690",                   kNull		},
	{ 0x68D8,	0x30001787, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730",                   kNull		},
	{ 0x68D8,	0x301017AF, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730",                   kNull		},
	{ 0x68D8,	0x301117AF, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5690",                   kNull		},
	{ 0x68D8,	0x30601682, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5690",                   kNull		},
	{ 0x68D8,	0x30651682, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5690",                   kNull		},
	{ 0x68D8,	0x56701545, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5690",                   kNull		},
	{ 0x68D8,	0x5690174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5690",                   kNull		},
	{ 0x68D8,	0x5730174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730",                   kNull		},
	{ 0x68D8,	0x68E01028, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",                   kBaboon		},
	{ 0x68D8,	0xE151174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",                   kEulemur	},
	{ 0x68D8,	0xE155174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",                   kNull		},
	{ 0x68D8,	0xE166174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",                   kUakari		},

	{ 0x68D9,	0x03CE1043, CHIP_FAMILY_REDWOOD,	"ASUS EAH5550 series",                  kNull		},
	{ 0x68D9,	0x22401462, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5570",                   kNull		},
	{ 0x68D9,	0x3000148C, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 6510",                   kNull		},
	{ 0x68D9,	0x3000174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 6510",                   kNull		},
	{ 0x68D9,	0x301017AF, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5630",                   kNull		},
	{ 0x68D9,	0x39691642, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5570",                   kNull		},

	{ 0x68DA,	0x3000148C, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 6390",                   kNull		},
	{ 0x68DA,	0x3000174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 6390",                   kNull		},
	{ 0x68DA,	0x30001787, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5630",                   kNull		},
	{ 0x68DA,	0x300017AF, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 6390",                   kNull		},
	{ 0x68DA,	0x301017AF, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5630",                   kNull		},
	{ 0x68DA,	0x5630174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5630",                   kNull		},

	{ 0x68E0,	0x02931025, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x03581025, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x03591025, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x035C1025, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x035D1025, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x036D1025, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kLangur     },
	{ 0x68E0,	0x04471028, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x04561028, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x04831025, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x1433103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x1441103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x144A103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0x1BF21043, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kNull       },
	{ 0x68E0,	0x848F1043, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kNull       },
	{ 0x68E0,	0xFD001179, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0xFD121179, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0xFD501179, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0xFD921179, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E0,	0xFDD01179, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},

	{ 0x68E1,	0x04661028, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x10021B61, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450M",                  kEulemur	},
	{ 0x68E1,	0x10501462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x1426103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x142A103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 545vM",                  kEulemur	},
	{ 0x68E1,	0x14E110CF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5400M",                  kEulemur	},
	{ 0x68E1,	0x21D81458, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x21E21458, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x21F11458, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x22021458, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x23421462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x23431462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x25481458, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5400M Series",           kEulemur	},
	{ 0x68E1,	0x254A1458, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5000M Series",           kEulemur	},
	{ 0x68E1,	0x30001043, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x3000148C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x30001682, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x3000174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	}, 
	{ 0x68E1,	0x30001787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	}, // 5430M ???
	{ 0x68E1,	0x300017AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x3001148C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x301417AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kEulemur	},
	{ 0x68E1,	0x3002148C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
   	{ 0x68E1,	0x3003148C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450M",                  kEulemur	},
	{ 0x68E1,	0x54501545, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x5470174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",                  kEulemur	},
	{ 0x68E1,	0x60001092, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x60001682, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x6000174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},
	{ 0x68E1,	0x6230174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kEulemur	},
	{ 0x68E1,	0x6350174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kEulemur	},
	{ 0x68E1,	0xFDD01179, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",                  kEulemur	},

	{ 0x68E4,	0x04821025, CHIP_FAMILY_CEDAR,		"AMD Radeon HD 6370M",                  kNull		},
	{ 0x68E4,	0x1426103C, CHIP_FAMILY_CEDAR,		"AMD Radeon HD 6370M",                  kNull		},
	{ 0x68E4,	0x1C921043, CHIP_FAMILY_CEDAR,		"AMD Radeon HD 6370M",                  kNull		},
	{ 0x68E4,	0x397917AA, CHIP_FAMILY_CEDAR,		"AMD Radeon HD 6370M",                  kNull		},
	{ 0x68E4,	0x397F17AA, CHIP_FAMILY_CEDAR,		"AMD Radeon HD 7370M",                  kNull		},
	{ 0x68E4,	0x84A01043, CHIP_FAMILY_CEDAR,		"AMD Radeon HD 6370M",                  kNull		},

	{ 0x68F9,	0x00011019, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x00021019, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x00191019, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x010E1002, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x010E1028, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x03741043, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x03CA1043, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x05181025, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x05191025, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x174B3000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6230",                   kNull		},
	{ 0x68F9,	0x174B6250, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6250",                   kNull		},
	{ 0x68F9,	0x174B6290, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6290",                   kNull		},
	{ 0x68F9,	0x174BE164, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x20091787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x21261028, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x2126103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x21301462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x21311462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x21331462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kEulemur	},
	{ 0x68F9,	0x21801462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x21811462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x21821462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x21831462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x22911787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x22301462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x22311462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x23401462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x24951462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x2AAC103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x2AEC103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x30001787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",                   kNull		},
	{ 0x68F9,	0x300017AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6250",                   kNull		},
	{ 0x68F9,	0x3001148C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6250",                   kNull		},
	{ 0x68F9,	0x30011787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5530",                   kNull		},
	{ 0x68F9,	0x3002148C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6290",                   kNull		},
	{ 0x68F9,	0x30021787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5490",                   kNull		},
	{ 0x68F9,	0x300217AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6290",                   kNull		},
	{ 0x68F9,	0x3003148C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6230",                   kNull		},
	{ 0x68F9,	0x301117AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",                   kNull		},
	{ 0x68F9,	0x301217AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5490",                   kNull		},
	{ 0x68F9,	0x301317AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",                   kNull		},
	{ 0x68F9,	0x301417AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x30321682, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",			kEulemur	},
	{ 0x68F9,	0x303A1682, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",			kEulemur	},
	{ 0x68F9,	0x3580103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x360217AA, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x360317AA, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x360F17AA, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x361917AA, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x39831642, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x39841642, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x39871642, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x3987174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x39971462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0x3A051642, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kNull		},
	{ 0x68F9,	0x3B311642, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350A",                  kNull		},
	{ 0x68F9,	0x5470174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",                   kNull		},
	{ 0x68F9,	0x5490174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5490",                   kNull		},
	{ 0x68F9,	0x5530174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5530",                   kNull		},
	{ 0x68F9,	0x6230174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6230",                   kNull		},
	{ 0x68F9,	0x6350174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6350",                   kNull		},
	{ 0x68F9,	0x7350148C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 7350",                   kNull		},
	{ 0x68F9,	0xE127174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0xE145174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0xE153174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},
	{ 0x68F9,	0xE164174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",                   kEulemur	},

	/* Northen Islands */
	{ 0x6718,	0x03B81043, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0x03BC1043, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0x0B001002, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0x20101458, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0x20101787, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0x22001458, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0x22011458, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0x23061787, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		}, // HIS
	{ 0x6718,	0x23701462, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		}, // MSI
	{ 0x6718,	0x24701462, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		}, // MSI
	{ 0x6718,	0x31301682, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0x67181002, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0xE182174B, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},
	{ 0x6718,	0xE203174B, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",                   kGibba		},

	{ 0x6719,	0x03BE1043, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0x03D41043, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0x0B001002, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0x186B174B, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0x20101787, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0x21FD1458, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0x23071787, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0x24611462, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0x31211682, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0x31221682, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},
	{ 0x6719,	0xE189174B, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",                   kGibba		},

//	{ 0x671D,	0x0B2A1002, CHIP_FAMILY_ANTILLES,	"AMD Radeon HD 6990",                   kNull		},
//	{ 0x671D,	0x1B2A1002, CHIP_FAMILY_ANTILLES,	"AMD Radeon HD 6990",                   kNull		},
//	{ 0x671D,	0x31601682, CHIP_FAMILY_ANTILLES,	"AMD Radeon HD 6990",                   kNull		},

	{ 0x6720,	0x048F1028, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6970M",                  kElodea		},
	{ 0x6720,	0x04901028, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6970M",                  kElodea		},
	{ 0x6720,	0x04A41028, CHIP_FAMILY_BARTS,		"AMD FirePro M8900M",                   kElodea		},
	{ 0x6720,	0x04B91028, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6970M",                  kElodea		},
	{ 0x6720,	0x04BA1028, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6970M",                  kElodea		},
	{ 0x6720,	0x51021558, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6970M",                  kElodea		},
	{ 0x6720,	0x51041558, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6990M",                  kElodea		},
	{ 0x6720,	0x71001558, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6970M",                  kElodea		},
	{ 0x6720,	0x72001558, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6970M",                  kElodea		},
	{ 0x6720,	0x72011558, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6990M",                  kElodea		},
	{ 0x6720,	0xC0AD144D, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6970M",                  kElodea		},

	{ 0x6738,	0x00D01002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x03AE1043, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x03C61043, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x174B174B, CHIP_FAMILY_BARTS,		"Sapphire Radeon HD6870",               kBulrushes	}, // ?? kDuckweed ??
	{ 0x6738,	0x20101787, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x21FA1002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x21FA1458, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x23051787, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x25101462, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x31031682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x31041682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x31071682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0x31081682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},  // ?? kJuncus ??
	{ 0x6738,	0x67381002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},
	{ 0x6738,	0xE178174B, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",                   kDuckweed	},

	{ 0x6739,	0x03B41043, CHIP_FAMILY_BARTS,		"ASUS EAH6850 DirectCU",                   kDuckweed	},
	{ 0x6739,	0x174B174B, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x200F1787, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x21F81458, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kBulrushes	},
	{ 0x6739,	0x23041787, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x24001462, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x24411462, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x25201462, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x30001787, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x31101682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x31131682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x31141682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x31161682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x31171682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0x67391002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0xAA881002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0xE174174B, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0xE177174B, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},
	{ 0x6739,	0xE183174B, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",                   kDuckweed	},

	{ 0x673E,	0x174B174B, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790",                   kNull		},
	{ 0x673E,	0x23101787, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790",                   kNull		},
	{ 0x673E,	0x31701682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790",                   kNull		},
	{ 0x673E,	0x31721682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790",                   kNull		},

	{ 0x6740,	0x04A31028, CHIP_FAMILY_TURKS,		"Dell HD 6770M",                        kNull		}, // ??
	{ 0x6740,	0x1D121043, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6730M",                  kNull		},
	{ 0x6740,	0x1631103C, CHIP_FAMILY_TURKS,		"AMD FirePro M5950",                    kNull		},
	{ 0x6740,	0x1657103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M",                  kNull		},
	{ 0x6740,	0x165A103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M",                  kNull		},
	{ 0x6740,	0x3388103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M",                  kNull		},
	{ 0x6740,	0x3389103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M",                  kNull		}, // ?? kHydrilla ??
	{ 0x6740,	0x6740106B, CHIP_FAMILY_TURKS,		"Apple HD 6770M",                       kNull		}, // ??

	{ 0x6741,	0x050E1025, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M",                  kNull		},
	{ 0x6741,	0x050F1025, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M",                  kNull		},
	{ 0x6741,	0x05131025, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M",                  kNull		},
	{ 0x6741,	0x1646103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6750M",                  kNull		},
	{ 0x6741,	0x1688103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 7690M",                  kNull		},
	{ 0x6741,	0x358D103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6630M/6650M/6750M",      kNull		},
 	{ 0x6741,	0x9080104D, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6630M",                  kNull		},

	{ 0x6758,	0x00121028, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0x0B0E1002, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kPithecia	},
	{ 0x6758,	0x0B0E1028, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0x20121787, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kPithecia	},
	{ 0x6758,	0x20141787, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0x22051458, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0x31811682, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0x31831682, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0x67581002, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0x6882103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0xE181174B, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0xE1941746, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kBulrushes	},
	{ 0x6758,	0xE194174B, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",                   kMangabey	}, 

	{ 0x6759,	0x20121787, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6570",                   kPithecia	},
	{ 0x6759,	0xE193174B, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6570",                   kPithecia	},

	{ 0x675F,	0x23141787, CHIP_FAMILY_TURKS,		"AMD Radeon HD 5570",                   kNull		},
	{ 0x675F,	0x254B1458, CHIP_FAMILY_TURKS,		"AMD Radeon HD 5570",                   kNull		},
	{ 0x675F,	0x6510148C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 5570",                   kNull		},
	{ 0x675F,	0x6510174B, CHIP_FAMILY_TURKS,		"AMD Radeon HD 5570",                   kNull		},

	{ 0x6760,	0x00031179, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450M",                  kIpomoea	},
	{ 0x6760,	0x04C11028, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450M",                  kIpomoea	},
	{ 0x6760,	0x04CA1028, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450M",                  kIpomoea	},
	{ 0x6760,	0x04CC1028, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450M",                  kIpomoea	},
	{ 0x6760,	0x101C1043, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450M",                  kIpomoea	},
	{ 0x6760,	0x165A103C, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6470M",                  kIpomoea	},
	{ 0x6760,	0x167D103C, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6470M",                  kIpomoea	},
	{ 0x6760,	0x1CB21043, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6470M",                  kIpomoea	},
	{ 0x6760,	0x20011043, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6470M",                  kIpomoea	},
	{ 0x6760,	0x85171043, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 7470M",                  kNull		},

	{ 0x6779,	0x00001002, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x03DA1043, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x03DC1043, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x20121787, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x21201028, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x21251462, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x22031458, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x22041458, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x23111787, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x32001682, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x64501092, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0x909D1B0A, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0xE164174B, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kIpomoea	},
	{ 0x6779,	0xE180174B, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kPithecia	},
	{ 0x6779,	0xE190174B, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},
	{ 0x6779,	0xE199174B, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",                   kBulrushes	},

	/* Southen Islands */

	{ 0x6798,	0x04181043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji  },
	{ 0x6798,	0x041C1043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji  },
	{ 0x6798,	0x04201043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji	},
	{ 0x6798,	0x04421043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji	},
	{ 0x6798,	0x04441043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji	},
	{ 0x6798,	0x04461043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji	},
	{ 0x6798,	0x04481043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji	},
	{ 0x6798,	0x044A1043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji	},
	{ 0x6798,	0x044C1043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji	},
	{ 0x6798,	0x044E1043, CHIP_FAMILY_TAHITI,		"Asus HD7970 7970",                   kAji	},
	{ 0x6798,	0x0B001002, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},
	{ 0x6798,	0x201C1787, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	}, // 7990 ???
	{ 0x6798,	0x23171787, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},
	{ 0x6798,	0x254D1458, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},
	{ 0x6798,	0x27701462, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},
	{ 0x6798,	0x30001002, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},
	{ 0x6798,	0x30001787, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},
	{ 0x6798,	0x32101682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},
	{ 0x6798,	0x32111682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},
	{ 0x6798,	0x32121682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji    },
	{ 0x6798,	0x32131682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},
	{ 0x6798,	0x99991043, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7990",                   kAji	}, // Asus
	{ 0x6798,	0xE208174B, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970",                   kAji	},

	{ 0x679A,	0x04241043, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x04261043, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x0B001002, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x0B011002, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 8900",                   kAji	},
	{ 0x679A,	0x201C1787, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x23161787, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x254C1458, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x27601462, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x27711462, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x30001002, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x30001462, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 8950",                   kAji	},
	{ 0x679A,	0x3000174B, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x30001787, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x32121682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x32201682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x32211682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x32221682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x6616103C, CHIP_FAMILY_TAHITI,		"HP Radeon HD 7950",                    kAji	},
	{ 0x679A,	0x30001002, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0x7950174B, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0xE207174B, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},
	{ 0x679A,	0xE208174B, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950",                   kAji	},

	{ 0x6800,	0x03711558, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970M",           kDashimaki	},
	{ 0x6800,	0x05501028, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970M",           kDashimaki	},
	{ 0x6800,	0x05511028, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970M",           kDashimaki	},
	{ 0x6800,	0x05544028, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970M",           kDashimaki	},
	{ 0x6800,	0x057B1028, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970M",           kDashimaki	},
	{ 0x6800,	0x05861028, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970M",           kDashimaki	},
	{ 0x6800,	0x05871028, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970M",           kDashimaki	},
	{ 0x6800,	0x05881028, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970M",           kDashimaki	},

	{ 0x6818,	0x042F1043, CHIP_FAMILY_TAHITI,		"Asus HD 7870",                         kAji		},
	{ 0x6818,	0x04311043, CHIP_FAMILY_TAHITI,		"Asus HD 7870",                         kAji		},
	{ 0x6818,	0x0B041002, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 7870",                   kAji		},
	{ 0x6818,	0x0B051002, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 8800",                   kAji		},
	{ 0x6818,	0x201C1787, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 7870",                   kAji		},
	{ 0x6818,	0x23211787, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 7870",                   kAji		},
	{ 0x6818,	0x25541458, CHIP_FAMILY_TAHITI,		"Gigabyte HD 7870",                     kAji		},
	{ 0x6818,	0x27401462, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 7870",                   kAji		},
	{ 0x6818,	0x32501682, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 7870",                   kChutoro		},
	{ 0x6818,	0x32511682, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 7870",                   kAji		},
	{ 0x6818,	0x7870174B, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 7870",                   kAji		},
	{ 0x6818,	0x8B04174B, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 8860",                   kAji		},
	{ 0x6818,	0xE217174B, CHIP_FAMILY_TAHITI,		"ATI Radeon HD 7870",                   kAji		},

	{ 0x6819,	0x042C1043, CHIP_FAMILY_TAHITI,		"Asus HD 7850",                         kAji		}, // Asus
	{ 0x6819,	0x04311043, CHIP_FAMILY_TAHITI,		"Asus HD 7850",                         kAji		}, // Asus
	{ 0x6819,	0x04331043, CHIP_FAMILY_TAHITI,		"Asus HD 7850",                         kAji		}, // Asus
	{ 0x6819,	0x043A1043, CHIP_FAMILY_TAHITI,		"Asus HD 7850",                         kAji		}, // Asus
	{ 0x6819,	0x045B1043, CHIP_FAMILY_TAHITI,		"Asus HD 7850",                         kAji		}, // Asus
	{ 0x6819,	0x0B041002, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // ATI
	{ 0x6819,	0x201C1787, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // HIS
	{ 0x6819,	0x23201787, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // HIS
	{ 0x6819,	0x25531458, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // Gigabyte
	{ 0x6819,	0x27301462, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // MSI
	{ 0x6819,	0x27311462, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // MSI
	{ 0x6819,	0x27321462, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // MSI
	{ 0x6819,	0x32601682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // XFX
	{ 0x6819,	0x32621682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // XFX
	{ 0x6819,	0x32641682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // XFX
	{ 0x6819,	0xE218174B, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // Sapphire
	{ 0x6819,	0xE221174B, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850",                   kAji		}, // Sapphire

	{ 0x682F,	0x15271043, CHIP_FAMILY_TAHITI,		"Asus Radeon HD 7700M",                kAji		},
	{ 0x682F,	0x1831103C, CHIP_FAMILY_TAHITI,		"HP Radeon HD 7730M",                  kAji		},
	{ 0x682F,	0x1832103C, CHIP_FAMILY_TAHITI,		"HP Radeon HD 7730M",                  kAji		},
	{ 0x682F,	0x1834103C, CHIP_FAMILY_TAHITI,		"HP Radeon HD 7730M",                  kAji		},
	{ 0x682F,	0x18A7103C, CHIP_FAMILY_TAHITI,		"HP Radeon HD 7730M",                  kAji		},
	{ 0x682F,	0xC0DA144D, CHIP_FAMILY_TAHITI,		"Samsung Radeon HD 7730M",             kAji		},

	{ 0x683D,	0x00301002, CHIP_FAMILY_TAHITI,		"Radeon HD 8760 OEM",                  kAji		}, // VERDE?
	{ 0x683D,	0x00301019, CHIP_FAMILY_TAHITI,		"Radeon HD 8760 OEM",                  kAji		}, // VERDE?
	{ 0x683D,	0x04211043, CHIP_FAMILY_TAHITI,		"Asus Radeon HD 7770",                  kAji		},
	{ 0x683D,	0x23041002, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7700",                   kAji		},
	{ 0x683D,	0x25561458, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7770",                   kAji		},
	{ 0x683D,	0x27101462, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7770",                   kAji		},
	{ 0x683D,	0x2B301002, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7770",                   kAji		},
	{ 0x683D,	0x32331682, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7770",                   kAji		},
	{ 0x683D,	0x6886103C, CHIP_FAMILY_TAHITI,		"HP Radeon HD 7700",                    kAji		},
	{ 0x683D,	0x6890103C, CHIP_FAMILY_TAHITI,		"Radeon HD 8760 OEM",                  kAji		}, // VERDE?
	{ 0x683D,	0xE214174B, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7770",                   kAji		},

	{ 0x683F,	0x04231043, CHIP_FAMILY_TAHITI,		"Asus HD 7750",                         kChutoro		},
	{ 0x683F,	0x04271043, CHIP_FAMILY_TAHITI,		"Asus HD 7750",                         kChutoro		},
	{ 0x683F,	0x04591043, CHIP_FAMILY_TAHITI,		"Asus HD 7750",                         kChutoro		},
	{ 0x683F,	0x200B1787, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7750",                   kChutoro		},
	{ 0x683F,	0x23181787, CHIP_FAMILY_TAHITI,		"Vertex3D HD 7750",                     kChutoro		},
	{ 0x683F,	0x25511458, CHIP_FAMILY_TAHITI,		"Gigabyte HD 7750",                     kChutoro		},
	{ 0x683F,	0x27921462, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7750",                   kChutoro		},
	{ 0x683F,	0x2B301002, CHIP_FAMILY_TAHITI,		"Ati HD 7750",                          kChutoro		},
	{ 0x683F,	0x32411682, CHIP_FAMILY_TAHITI,		"XFX HD 7750",                          kChutoro		},
	{ 0x683F,	0x32421682, CHIP_FAMILY_TAHITI,		"XFX HD 7750",                          kChutoro		},
	{ 0x683F,	0x32451682, CHIP_FAMILY_TAHITI,		"XFX HD 7750",                          kChutoro		},
	{ 0x683F,	0xE2131019, CHIP_FAMILY_TAHITI,		"Diamond HD 7750",                      kChutoro		},
	{ 0x683F,	0xE213174B, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7750",                   kChutoro		},
	{ 0x683F,	0xE215174B, CHIP_FAMILY_TAHITI,		"Sapphire HD 7750",                     kChutoro		},

	{ 0x6840,	0x01241002, CHIP_FAMILY_LOMBOK,		"AMD Radeon HD 7600M Series",           kPondweed   },
	{ 0x6840,	0x01341002, CHIP_FAMILY_LOMBOK,		"AMD Radeon HD 7600M Series",           kPondweed   },
	{ 0x6840,	0x050E1025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x050F1025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x05131025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x05141025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x056D1025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x059A1025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x059B1025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x059E1025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x06001025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x06061025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x06961025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x06971025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x06981025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x06991025, CHIP_FAMILY_LOMBOK,		"Acer HD 7670M",           kPondweed   },
	{ 0x6840,	0x100A1043, CHIP_FAMILY_LOMBOK,		"Asus HD 7670M",           kPondweed   },
	{ 0x6840,	0x104B1043, CHIP_FAMILY_LOMBOK,		"Asus HD 7670M",           kPondweed   },
	{ 0x6840,	0x10DC1043, CHIP_FAMILY_LOMBOK,		"Asus HD 7670M",           kPondweed   },
	{ 0x6840,	0x1813103C, CHIP_FAMILY_LOMBOK,		"HP HD 7590M",           kPondweed   },
	{ 0x6840,	0x182F103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x1830103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x1835103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x183A103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x183C103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x183E103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x1840103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x1842103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x1844103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x1848103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x184A103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x184C103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x1895103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x1897103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x18A5103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x18A7103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x18F4103C, CHIP_FAMILY_LOMBOK,		"HP HD 7670M",           kPondweed   },
	{ 0x6840,	0x21211043, CHIP_FAMILY_LOMBOK,		"Asus HD 7670M",           kPondweed   },
	{ 0x6840,	0x21221043, CHIP_FAMILY_LOMBOK,		"Asus HD 7670M",           kPondweed   },
	{ 0x6840,	0x21231043, CHIP_FAMILY_LOMBOK,		"Asus HD 7670M",           kPondweed   },
	{ 0x6840,	0x21251043, CHIP_FAMILY_LOMBOK,		"Asus HD 7670M",           kPondweed   },
	{ 0x6840,	0x21271043, CHIP_FAMILY_LOMBOK,		"Asus HD 7670M",           kPondweed   },
	{ 0x6840,	0x397017AA, CHIP_FAMILY_LOMBOK,		"Lenovo HD 7670M",           kPondweed   },
	{ 0x6840,	0x397B17AA, CHIP_FAMILY_LOMBOK,		"Lenovo HD 7670M",           kPondweed   },
	{ 0x6840,	0xC0C5144D, CHIP_FAMILY_LOMBOK,		"Samsung HD 6000M series",    kPondweed   },
	{ 0x6840,	0xC0CE144D, CHIP_FAMILY_LOMBOK,		"Samsung HD 7670M",           kPondweed   },
	{ 0x6840,	0xC0DA144D, CHIP_FAMILY_LOMBOK,		"Samsung HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB111179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB221179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB231179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB2C1179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB311179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB321179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB381179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB391179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB3A1179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB401179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB411179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB471179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB481179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB511179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB521179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB531179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB811179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB821179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFB831179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFC561179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFCD41179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },
	{ 0x6840,	0xFCEE1179, CHIP_FAMILY_LOMBOK,		"Toshiba HD 7670M",           kPondweed   },

	/*old series*/

	{ 0x5D48,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Mobile ",                kNull		 },
	{ 0x5D49,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Mobile ",                kNull		 },
	{ 0x5D4A,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Mobile ",                kNull		 },

	{ 0x5D4C,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x5D4D,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x5D4E,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x5D4F,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x5D50,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Desktop ",               kNull		 },

	{ 0x5D52,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Desktop ",               kNull		 },

	{ 0x5D57,	0x00000000, CHIP_FAMILY_R420,		"ATI Radeon HD Desktop ",               kNull		 },

	{ 0x5E48,	0x00000000, CHIP_FAMILY_RV410,		"ATI Radeon HD Desktop ",               kNull		 },

	{ 0x5E4A,	0x00000000, CHIP_FAMILY_RV410,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x5E4B,	0x00000000, CHIP_FAMILY_RV410,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x5E4C,	0x00000000, CHIP_FAMILY_RV410,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x5E4D,	0x00000000, CHIP_FAMILY_RV410,		"ATI Radeon HD Desktop ",               kNull		 },

	{ 0x5E4F,	0x00000000, CHIP_FAMILY_RV410,		"ATI Radeon HD Desktop ",               kNull		 },

	{ 0x7100,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x7101,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Mobile ",                kNull		 },
	{ 0x7102,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Mobile ",                kNull		 },
	{ 0x7103,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Mobile ",                kNull		 },
	{ 0x7104,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x7105,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x7106,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Mobile ",                kNull		 },
	{ 0x7108,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x7109,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x710A,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x710B,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x710C,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },

	{ 0x710E,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },
	{ 0x710F,	0x00000000, CHIP_FAMILY_R520,		"ATI Radeon HD Desktop ",               kNull		 },

	{ 0x7140,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7141,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7142,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7143,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7144,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x7145,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x7146,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7147,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },

	{ 0x7149,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x714A,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x714B,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x714C,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x714D,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x714E,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x714F,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },

	{ 0x7151,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7152,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7153,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },

	{ 0x715E,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x715F,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },

	{ 0x7180,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7181,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7183,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },

	{ 0x7186,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x7187,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },
	{ 0x7188,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },

	{ 0x718A,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x718B,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x718C,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },
	{ 0x718D,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },

	{ 0x718F,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },

	{ 0x7193,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },

	{ 0x7196,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kCaretta		 },

	{ 0x719B,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },

	{ 0x719F,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kCaretta		 },

	{ 0x71C0,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },
	{ 0x71C1,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },
	{ 0x71C2,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },
	{ 0x71C3,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },
	{ 0x71C4,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Mobile ",                kWormy		 },

	{ 0x71C5,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Mobile ",                kWormy		 },
	{ 0x71C6,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },
	{ 0x71C7,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },

	{ 0x71CD,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },
	{ 0x71CE,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },

	{ 0x71D2,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },

	{ 0x71D4,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Mobile ",                kWormy		 },
	{ 0x71D5,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Mobile ",                kWormy		 },
	{ 0x71D6,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Mobile ",                kWormy		 },

	{ 0x71DA,	0x00000000, CHIP_FAMILY_RV530,		"ATI Radeon HD Desktop ",               kWormy		 },

	{ 0x71DE,	0x00000000, CHIP_FAMILY_RV530,		"ASUS M66 ATI Radeon Mobile ",          kWormy		 },

	{ 0x7200,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Desktop ",               kWormy		 },

	{ 0x7210,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kWormy		 },
	{ 0x7211,	0x00000000, CHIP_FAMILY_RV515,		"ATI Radeon HD Mobile ",                kWormy		 },

	{ 0x7240,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },

	{ 0x7243,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7244,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7245,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7246,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7247,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7248,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7249,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x724A,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x724B,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x724C,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x724D,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x724E,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x724F,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Desktop ",               kAlopias		 },

	{ 0x7280,	0x00000000, CHIP_FAMILY_RV570,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7281,	0x00000000, CHIP_FAMILY_RV560,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7283,	0x00000000, CHIP_FAMILY_RV560,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7284,	0x00000000, CHIP_FAMILY_R580,		"ATI Radeon HD Mobile ",                kAlopias		 },

	{ 0x7287,	0x00000000, CHIP_FAMILY_RV560,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7288,	0x00000000, CHIP_FAMILY_RV570,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7289,	0x00000000, CHIP_FAMILY_RV570,		"ATI Radeon HD Desktop ",               kAlopias		 },

	{ 0x728B,	0x00000000, CHIP_FAMILY_RV570,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x728C,	0x00000000, CHIP_FAMILY_RV570,		"ATI Radeon HD Desktop ",               kAlopias		 },

	{ 0x7290,	0x00000000, CHIP_FAMILY_RV560,		"ATI Radeon HD Desktop ",               kAlopias		 },
	{ 0x7291,	0x00000000, CHIP_FAMILY_RV560,		"ATI Radeon HD Desktop ",               kAlopias		 },

	{ 0x7293,	0x00000000, CHIP_FAMILY_RV560,		"ATI Radeon HD Desktop ",               kAlopias		 },

	{ 0x7297,	0x00000000, CHIP_FAMILY_RV560,		"ATI Radeon HD Desktop ",               kAlopias		 },

	/* IGP */

	{ 0x791E,	0x00000000, CHIP_FAMILY_RS690,		"ATI Radeon IGP ",                      kNull			},
	{ 0x791F,	0x00000000, CHIP_FAMILY_RS690,		"ATI Radeon IGP ",                      kNull			},
	{ 0x796C,	0x00000000, CHIP_FAMILY_RS740,		"ATI Radeon IGP ",                      kNull			},
	{ 0x796D,	0x00000000, CHIP_FAMILY_RS740,		"ATI Radeon IGP ",                      kNull			},
	{ 0x796E,	0x00000000, CHIP_FAMILY_RS740,		"ATI Radeon IGP ",                      kNull			},
	{ 0x796F,	0x00000000, CHIP_FAMILY_RS740,		"ATI Radeon IGP ",                      kNull			},

	/* standard/default models */

	{ 0x9400,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",                kNull		},
	{ 0x9401,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",                kNull		},
	{ 0x9402,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",                kNull		},
	{ 0x9403,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",                kNull		},
	{ 0x9405,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",                kNull		},
	{ 0x940A,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",                kNull		},
	{ 0x940B,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",                kNull		},
	{ 0x940F,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",                kNull		},

	{ 0x9440,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9441,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870 X2",                kMotmot		},
	{ 0x9442,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9443,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850 X2",                kMotmot		},
	{ 0x9444,	0x00000000, CHIP_FAMILY_RV770,		"ATI FirePro V8750 (FireGL)",           kMotmot		},
	{ 0x9446,	0x00000000, CHIP_FAMILY_RV770,		"ATI FirePro V7770 (FireGL)",		kMotmot		},
	{ 0x9447,	0x00000000, CHIP_FAMILY_RV770,		"ATI FirePro V8700 Duo (FireGL)",	kMotmot		},
	{ 0x944A,	0x00000000, CHIP_FAMILY_RV770,		"ATI Mobility Radeon HD 4850",		kMotmot		},
	{ 0x944B,	0x00000000, CHIP_FAMILY_RV770,		"ATI Mobility Radeon HD 4850 X2",	kMotmot		},
	{ 0x944C,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830 Series",		kMotmot		},
	{ 0x944E,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810 Series",		kMotmot		},

	{ 0x9450,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9270",                  kMotmot		},
	{ 0x9452,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9250",                  kMotmot		},

	{ 0x9456,	0x00000000, CHIP_FAMILY_RV770,		"ATI FirePro V8700 (FireGL)",		kMotmot		},
	{ 0x945A,	0x00000000, CHIP_FAMILY_RV770,		"ATI Mobility Radeon HD 4870",		kMotmot		},

	{ 0x9460,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9462,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},

	{ 0x9480,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650 Series",		kGliff		},

	{ 0x9487,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD Series",		kGliff		},
	{ 0x9488,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650 Series",		kGliff		},
	{ 0x9489,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD Series",		kGliff		},
	{ 0x948A,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD Series",		kGliff		},
	{ 0x948F,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD Series",		kGliff		},
	{ 0x9490,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710 Series",		kGliff		},
	{ 0x9491,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",		kGliff		},
	{ 0x9495,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650 Series",		kGliff		},

	{ 0x9498,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710 Series",		kGliff		},
	{ 0x949C,	0x00000000, CHIP_FAMILY_RV730,		"ATI FirePro V7750 (FireGL)",		kGliff		},
	{ 0x949E,	0x00000000, CHIP_FAMILY_RV730,		"ATI FirePro V5700 (FireGL)",		kGliff		},
	{ 0x949F,	0x00000000, CHIP_FAMILY_RV730,		"ATI FirePro V3750 (FireGL)",		kGliff		},

	{ 0x94A0,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4830M",                   kFlicker	},
	{ 0x94A1,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4860M",                   kFlicker	},
	{ 0x94A3,	0x00000000, CHIP_FAMILY_RV740,		"ATI FirePro M7740",                   kFlicker	},
	{ 0x94B1,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD",                   kFlicker	},
	{ 0x94B3,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",                   kFlicker	},
	{ 0x94B4,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4700 Series",		kFlicker	},
	{ 0x94B5,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",                   kFlicker	},
	{ 0x94B9,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD",                   kFlicker	},

	{ 0x94C0,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD Series",            kIago		},
	{ 0x94C1,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",            kIago		},

	{ 0x94C3,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",            kIago		},
	{ 0x94C4,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",            kIago		},
	{ 0x94C5,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",            kIago		},
	{ 0x94C6,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",            kIago		},
	{ 0x94C7,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350",                   kIago		},
	{ 0x94C8,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",            kIago		},
	{ 0x94C9,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",            kIago		},

	{ 0x94CB,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",            kIago		},
	{ 0x94CC,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",            kIago		},
	{ 0x94CD,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO Series",            kIago		},

	{ 0x9500,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",            kMegalodon	},
	{ 0x9501,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3690 Series",            kMegalodon	},

	{ 0x9504,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850M Series",            kMegalodon	},
	{ 0x9505,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",            kMegalodon	},
	{ 0x9506,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850 X2 M Series",            kMegalodon	},
	{ 0x9507,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3830",                   kMegalodon	},
	{ 0x9508,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3870M Series",            kMegalodon	},
	{ 0x9509,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3870 X2 MSeries",            kMegalodon	},

	{ 0x950F,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3870 X2",                kMegalodon	},

	{ 0x9511,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850 X2",                kMegalodon	},

	{ 0x9513,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 3850 X2",                kMegalodon	},
	{ 0x9515,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850 Series",            kMegalodon	},
	{ 0x9517,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD Series",            kMegalodon	},

	{ 0x9519,	0x00000000, CHIP_FAMILY_RV670,		"AMD FireStream 9170",                  kMegalodon	},

	{ 0x9540,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",                   kFlicker	},
	{ 0x9541,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD",                   kFlicker	},
	{ 0x9542,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD",                   kFlicker	},
	{ 0x954E,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD",                   kFlicker	},
	{ 0x954F,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4350",                   kFlicker	},

	{ 0x9552,	0x00000000, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4300/4500 Series",	kShrike		},
	{ 0x9553,	0x00000000, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500M/5100M Series",	kShrike		},

	{ 0x9555,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD4300/HD4500 series",		kShrike		},

	{ 0x9557,	0x00000000, CHIP_FAMILY_RV710,		"ATI FirePro RG220",                   kFlicker	},

	{ 0x955F,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4330M series",                   kFlicker	},

	{ 0x9580,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD Series",			kHypoprion	},
	{ 0x9581,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",			kHypoprion	},

	{ 0x9583,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",			kHypoprion	},

	{ 0x9586,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT Series",			kHypoprion	},
	{ 0x9587,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 Pro Series",			kHypoprion	},
	{ 0x9588,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT",			kHypoprion	},
	{ 0x9589,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 3610 Series",			kHypoprion	},
	{ 0x958A,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",			kLamna		},
	{ 0x958B,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",			kLamna		},
	{ 0x958C,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",			kLamna		},
	{ 0x958D,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",			kLamna		},
	{ 0x958E,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",			kLamna		},
	{ 0x958F,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD Series",			kHypoprion	},

	{ 0x9591,	0x00000000, CHIP_FAMILY_RV635,		"ATI Radeon HD 3600 Series",			kMegalodon		},

	{ 0x9598,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",			kMegalodon	},

	{ 0x95C0,	0x00000000, CHIP_FAMILY_RV620,		"ATI Radeon HD 3400 Series",			kIago		},

	{ 0x95C5,	0x00000000, CHIP_FAMILY_RV620,		"ATI Radeon HD 3400 Series",			kIago		},

	/* IGP */
	{ 0x9610,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon HD 3200 Graphics",			kNull		},
	{ 0x9611,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon 3100 Graphics",             	kNull		},

	{ 0x9614,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon HD 3300 Graphics",			kNull		},

	{ 0x9616,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon 3000 Graphics",             	kNull		},

	{ 0x9710,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4200 Series",			kNull		},

	{ 0x9714,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4290 Series",			kNull		},
	{ 0x9715,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4250 Series",			kNull		},

	{ 0x9723,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 5450 Series",			kNull		},

	{ 0x9802,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 6310 Series",			kNull		},
	{ 0x9803,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 6310 Series",			kNull		},
	{ 0x9804,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 6310 Series",			kNull		},
	{ 0x9805,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 6250 Series",			kNull		},
	{ 0x9806,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 6320 Series",			kNull		},

	/* Evergreen */
	{ 0x688D,	0x00000000, CHIP_FAMILY_CYPRESS,	"AMD FireStream 9350 Series",			kUakari		},

	{ 0x6898,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",			kUakari		},
	{ 0x6899,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",			kUakari		},

//	{ 0x689B,	0x00000000, CHIP_FAMILY_???,		"AMD Radeon HD 6800 Series",			kNull		},
	{ 0x689C,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5900 Series",			kUakari		},

	{ 0x689E,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5800 Series",			kUakari		},

	{ 0x68A0,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5800 Series",   kNomascus	}, // CHIP_FAMILY_BROADWAY ??
	{ 0x68A1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Mobility Radeon HD 5800 Series",   kNomascus	}, // CHIP_FAMILY_BROADWAY ??

	{ 0x68A8,	0x00000000, CHIP_FAMILY_JUNIPER,	"AMD Mobility Radeon HD 6800 Series",   kNomascus		},
	{ 0x68A9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI FirePro V5800 (FireGL)",			kNull		},


	{ 0x68B0,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Mobility Radeon HD 5800 Series",   kHoolock		}, // CHIP_FAMILY_BROADWAY ??
	{ 0x68B1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",			kHoolock		},

	{ 0x68B8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",			kHoolock		},
	{ 0x68B9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5600 Series",			kHoolock		},
	{ 0x68BA,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6700 Series",			kHoolock		},

	{ 0x68BE,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",			kHoolock		},
	{ 0x68BF,	0x00000000, CHIP_FAMILY_JUNIPER,	"AMD Radeon HD 6700 Series",			kHoolock		},

	{ 0x68C0,	0x00000000, CHIP_FAMILY_REDWOOD,	"AMD Radeon HD 6570M/5700 Series",		kBaboon		},
	{ 0x68C1,	0x00000000, CHIP_FAMILY_REDWOOD,	"AMD Radeon HD 6500M/5600/5700 Series",		kBaboon		},
	{ 0x68C8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5650 Series",			kVervet		},	
	{ 0x68C9,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI FirePro V3800 (FireGL)",			kBaboon		},

	{ 0x68D8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670 Series",			kBaboon		},
	{ 0x68D9,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500/5600 Series",		kBaboon		},
	{ 0x68DA,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",			kBaboon		},

//	{ 0x68DE,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD ??? Series",			kNull		},


	{ 0x68E0,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5400 Series",   kEulemur	},
	{ 0x68E1,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Mobility Radeon HD 5400 Series",   kEulemur	},

	{ 0x68E4,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6370M Series",			kEulemur		},
	{ 0x68E5,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 6300M Series",			kEulemur		},

//	{ 0x68E8,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD ??? Series",			kNull		},
//	{ 0x68E9,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD ??? Series",			kNull		},

//	{ 0x68F8,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD ??? Series",			kNull		},
	{ 0x68F9,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470 Series",			kEulemur		},
	{ 0x68FA,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 7300 Series",			kNull		},

//	{ 0x68FE,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD ??? Series",			kNull		},


	/* Northen Islands */
	{ 0x6718,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970 Series",			kLotus		},
	{ 0x6719,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950 Series",			kGibba		},

	{ 0x671C,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970 Series",			kLotus		},
	{ 0x671D,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950 Series",			kLotus		},

	{ 0x671F,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6930 Series",			kLotus		},

	{ 0x6720,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6900M Series",			kFanwort		},

	{ 0x6722,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6900M Series",			kFanwort		},
	{ 0x6729,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6900M Series",			kFanwort		},
	{ 0x6738,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870 Series",			kDuckweed	},
	{ 0x6739,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850 Series",			kDuckweed	},

	{ 0x673E,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790 Series",			kDuckweed		},

	{ 0x6740,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M Series",			kCattail		},
	{ 0x6741,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6750M Series",			kCattail		},

	{ 0x6745,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6600M Series",			kCattail		},
	{ 0x6749,	0x00000000, CHIP_FAMILY_TURKS,		"ATI Radeon FirePro V4900",			kPithecia		},
	{ 0x674A,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6600M Series",			kCattail		},
	{ 0x6750,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6600A Series",			kPithecia		},

	{ 0x6758,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670 Series",			kPithecia	},
	{ 0x6759,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6570/7570 Series",		kPithecia		},

	{ 0x675D,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 7570M Series",			kCattail		},

	{ 0x675F,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6570 Series",			kBulrushes		},
	{ 0x6760,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400M Series",			kHydrilla		},
	{ 0x6761,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6430M Series",			kHydrilla		},
	{ 0x6768,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400M Series",			kHydrilla	},

	{ 0x6770,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400 Series",			kBulrushes		},

	{ 0x6772,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 7400A Series",			kBulrushes		},

	{ 0x6778,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 7000 Series",			kBulrushes		},
	{ 0x6779,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450 Series",			kBulrushes	},

	{ 0x677B,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 7400 Series",			kBulrushes		},

	/* Southen Islands */

	{ 0x6780,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7900 Series",             kFutomaki		}, // ATI7000Controller.kext

//	{ 0x6784,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD ??? Series",             kFutomaki		},

//	{ 0x6788,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD ??? Series",             kFutomaki		},

	{ 0x678A,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7900 Series",             kFutomaki		},

	{ 0x6790,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7900 Series",             kFutomaki		}, // ATI7000Controller.kext
	{ 0x6791,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7900 Series",             kFutomaki		},
	{ 0x6792,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7900 Series",             kFutomaki		},

	{ 0x6798,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970 X-Edition",            kFutomaki		}, // ATI7000Controller.kext
	{ 0x6799,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7990 Series",            kAji		},
	{ 0x679A,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950 Series",            kFutomaki		}, // ATI7000Controller.kext
	{ 0x679B,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7900 Series",             kFutomaki		},

	{ 0x679E,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7870 XT",             kFutomaki		}, // ATI7000Controller.kext
	{ 0x679F,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7950 Series",             kFutomaki		},

	{ 0x6800,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7970M",           kFutomaki	}, // ATI7000Controller.kext
//	{ 0x6801,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD ???M Series",            kFutomaki		},
//	{ 0x6802,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD ???M Series",            kFutomaki		},

	{ 0x6806,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7600 Series",             kFutomaki	}, // ATI7000Controller.kext

	{ 0x6808,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7600 Series",             kFutomaki	}, // ATI7000Controller.kext
//	{ 0x6809,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD ??? Series",             kNull		},
//	{ 0x6810,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD ??? Series",             kNull		},

	{ 0x6818,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7800 Series",            kFutomaki	}, // CHIP_FAMILY_PITCAIRN ??// ATI7000Controller.kext
	{ 0x6819,	0x00000000, CHIP_FAMILY_TAHITI,		"AMD Radeon HD 7850 Series",            kFutomaki	},// CHIP_FAMILY_PITCAIRN ??
	{ 0x6820,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7700 Series",            kBuri	}, // ATI7000Controller.kext
	{ 0x6821,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7700 Series",            kBuri	}, // ATI7000Controller.kext

//	{ 0x6823,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 8800M Series",            kBuri	},
//	{ 0x6824,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7700M Series",            kBuri	},
	{ 0x6825,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7870 Series",            kBuri	}, // ATI7000Controller.kext
	{ 0x6826,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7700 Series",            kBuri	},
	{ 0x6827,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7850M/8850M Series",            kBuri	}, // ATI7000Controller.kext
//	{ 0x6828,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD ??? Series",             kBuri	},
//	{ 0x6829,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD ??? Series",             kBuri	},

	{ 0x682B,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 8800M Series",            kBuri	},

	{ 0x682D,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7700 Series",            kBuri	}, // ATI7000Controller.kext

	{ 0x682F,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7730 Series",            kBuri	}, // ATI7000Controller.kext

	{ 0x6830,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7800M Series",             kBuri	},
	{ 0x6831,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7700 Series",             kBuri	},

	{ 0x6837,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7700 Series",             kBuri	},
//	{ 0x6838,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD ??? Series",             kBuri	},
	{ 0x6839,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7700 Series",            kBuri	}, // ATI7000Controller.kext

	{ 0x683B,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7700 Series",            kBuri	}, // ATI7000Controller.kext

	{ 0x683D,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7770 Series",            kBuri	}, // ATI7000Controller.kext

	{ 0x683F,	0x00000000, CHIP_FAMILY_VERDE,		"AMD Radeon HD 7750 Series",            kBuri	}, // ATI7000Controller.kext

	{ 0x6840,	0x00000000, CHIP_FAMILY_LOMBOK,		"AMD Radeon HD 7670M Series",		kPondweed   }, // THAMES??
	{ 0x6841,	0x00000000, CHIP_FAMILY_THAMES,		"AMD Radeon HD 7500M/7600M Series",	kPondweed	},
	{ 0x6842,	0x00000000, CHIP_FAMILY_THAMES,		"AMD Radeon HD 7000M Series",		kPondweed	},
	{ 0x6843,	0x00000000, CHIP_FAMILY_THAMES,		"AMD Radeon HD 7670M Series",		kPondweed	},
	{ 0x6849,	0x00000000, CHIP_FAMILY_LOMBOK,		"AMD Radeon HD 7600M Series",		kPondweed   },

//	{ 0x684C,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD ??? Series",             kNull	},
	{ 0x6850,	0x00000000, CHIP_FAMILY_LOMBOK,		"AMD Radeon HD 7600M Series",           kPondweed   },
	{ 0x6858,	0x00000000, CHIP_FAMILY_LOMBOK,		"AMD Radeon HD 7400 Series",           kPondweed   },
	{ 0x6859,	0x00000000, CHIP_FAMILY_LOMBOK,		"AMD Radeon HD 7600M Series",           kPondweed   },

	{ 0x0000,	0x00000000, CHIP_FAMILY_UNKNOW,		NULL,						kNull		}
};


dev_prop_t ati_devprop_list[] = {
	{FLAGTRUE,	false,	"@0,AAPL,boot-display",		get_bootdisplay_val,	NULVAL							},
//	{FLAGTRUE,	false,	"@0,ATY,EFIDisplay",		NULL,					STRVAL("TMDSA")					},
	
//	{FLAGTRUE,	true,	"@0,AAPL,vram-memory",		get_vrammemory_val,		NULVAL							},
//	{FLAGTRUE,	true,	"@0,compatible",			get_name_val,			NULVAL							},
//	{FLAGTRUE,	true,	"@0,connector-type",		get_conntype_val,		NULVAL							},
//	{FLAGTRUE,	true,	"@0,device_type",			NULL,					STRVAL("display")				},
//	{FLAGTRUE,	false,	"@0,display-connect-flags", NULL,					DWRVAL((uint32_t)0)				},
//	{FLAGTRUE,	true,	"@0,display-type",			NULL,					STRVAL("NONE")					},
	{FLAGTRUE,	true,	"@0,name",					get_name_val,			NULVAL							},
//	{FLAGTRUE,	true,	"@0,VRAM,memsize",			get_vrammemsize_val,	NULVAL							},
	
//	{FLAGTRUE,	false,	"AAPL,aux-power-connected", NULL,					DWRVAL((uint32_t)1)				},
//	{FLAGTRUE,	false,	"AAPL,backlight-control",	NULL,					DWRVAL((uint32_t)0)				},
	{FLAGTRUE,	false,	"ATY,bin_image",			get_binimage_val,		NULVAL							},
	{FLAGTRUE,	false,	"ATY,Copyright",			NULL,	STRVAL("Copyright AMD Inc. All Rights Reserved. 2005-2010") },
	{FLAGTRUE,	false,	"ATY,Card#",				get_romrevision_val,	NULVAL							},
	{FLAGTRUE,	false,	"ATY,VendorID",				NULL,					WRDVAL((uint16_t)0x1002)		},
	{FLAGTRUE,	false,	"ATY,DeviceID",				get_deviceid_val,		NULVAL							},
	
//	{FLAGTRUE,	false,	"ATY,MCLK",					get_mclk_val,			NULVAL							},
//	{FLAGTRUE,	false,	"ATY,SCLK",					get_sclk_val,			NULVAL							},
//	{FLAGTRUE,	false,	"ATY,RefCLK",				get_refclk_val,			DWRVAL((uint32_t)0x0a8c)		},
	
//	{FLAGTRUE,	false,	"ATY,PlatformInfo",			get_platforminfo_val,	NULVAL							},
	
	{FLAGTRUE,	false,	"name",						get_nameparent_val,		NULVAL							},
	{FLAGTRUE,	false,	"device_type",				get_nameparent_val,		NULVAL							},
	{FLAGTRUE,	false,	"model",					get_model_val,			STRVAL("ATI Radeon")			},
//	{FLAGTRUE,	false,	"VRAM,totalsize",			get_vramtotalsize_val,	NULVAL							},
	{FLAGTRUE,	false, "hda-gfx",					get_hdmiaudio,	NULVAL},
	
	{FLAGTRUE,	false,	NULL,						NULL,					NULVAL							}
};

bool get_hdmiaudio(value_t * val)
{
	bool doit = false;
	if(getBoolForKey(kEnableHDMIAudio, &doit, &bootInfo->chameleonConfig) && doit){
		val->type = kStr;
		val->size = strlen("onboard-1") + 1;
		val->data = (uint8_t *)"onboard-1";

		return true;
	}
	return false;
}

bool get_bootdisplay_val(value_t *val)
{
	static uint32_t v = 0;
	
	if (v)
		return false;
	
	if (!card->posted)
		return false;
	
	v = 1;
	val->type = kCst;
	val->size = 4;
	val->data = (uint8_t *)&v;
	
	return true;
}

bool get_vrammemory_val(value_t *val)
{
	return false;
}

bool get_name_val(value_t *val)
{
	val->type = aty_name.type;
	val->size = aty_name.size;
	val->data = aty_name.data;
	
	return true;
}

bool get_nameparent_val(value_t *val)
{
	val->type = aty_nameparent.type;
	val->size = aty_nameparent.size;
	val->data = aty_nameparent.data;
	
	return true;
}

bool get_model_val(value_t *val)
{
	if (!card->info->model_name)
		return false;
	
	val->type = kStr;
	val->size = strlen(card->info->model_name) + 1;
	val->data = (uint8_t *)card->info->model_name;
	
	return true;
}

bool get_conntype_val(value_t *val)
{
//Connector types:
//0x4 : DisplayPort
//0x400: DL DVI-I
//0x800: HDMI

	return false;
}

bool get_vrammemsize_val(value_t *val)
{
	static int idx = -1;
	static uint64_t memsize;
	
	idx++;
	memsize = ((uint64_t)card->vram_size << 32);
	if (idx == 0)
		memsize = memsize | (uint64_t)card->vram_size;
	
	val->type = kCst;
	val->size = 8;
	val->data = (uint8_t *)&memsize;
	
	return true;
}

bool get_binimage_val(value_t *val)
{
	if (!card->rom)
		return false;
	
	val->type = kPtr;
	val->size = card->rom_size;
	val->data = card->rom;
	
	return true;
}

bool get_romrevision_val(value_t *val)
{
	uint8_t *rev;
	if (!card->rom)
		return false;

	rev = card->rom + *(uint8_t *)(card->rom + OFFSET_TO_GET_ATOMBIOS_STRINGS_START);

	val->type = kPtr;
	val->size = strlen((char *)rev);
	val->data = malloc(val->size);
	
	if (!val->data)
		return false;

	memcpy(val->data, rev, val->size);
	
	return true;
}

bool get_deviceid_val(value_t *val)
{
	val->type = kCst;
	val->size = 2;
	val->data = (uint8_t *)&card->pci_dev->device_id;
	
	return true;
}

bool get_mclk_val(value_t *val)
{
	return false;
}

bool get_sclk_val(value_t *val)
{
	return false;
}

bool get_refclk_val(value_t *val)
{
	return false;
}

bool get_platforminfo_val(value_t *val)
{
	val->data = malloc(0x80);
	if (!val->data)
		return false;
	
	bzero(val->data, 0x80);
	
	val->type		= kPtr;
	val->size		= 0x80;
	val->data[0]	= 1;
	
	return true;
}

bool get_vramtotalsize_val(value_t *val)
{
	val->type = kCst;
	val->size = 4;
	val->data = (uint8_t *)&card->vram_size;
	
	return true;
}

void free_val(value_t *val)
{
	if (val->type == kPtr)
		free(val->data);
	
	bzero(val, sizeof(value_t));
}

void devprop_add_list(dev_prop_t devprop_list[])
{
	value_t *val = malloc(sizeof(value_t));
	int i, pnum;
	
	for (i = 0; devprop_list[i].name != NULL; i++)
	{
		if ((devprop_list[i].flags == FLAGTRUE) || (devprop_list[i].flags | card->flags))
		{
			if (devprop_list[i].get_value && devprop_list[i].get_value(val))
			{
				devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
				free_val(val);
				
				if (devprop_list[i].all_ports)
				{
					for (pnum = 1; pnum < card->ports; pnum++)
					{
						if (devprop_list[i].get_value(val))
						{
							devprop_list[i].name[1] = 0x30 + pnum; // convert to ascii
							devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
							free_val(val);
						}
					}
					devprop_list[i].name[1] = 0x30; // write back our "@0," for a next possible card
				}
			}
			else
			{
				if (devprop_list[i].default_val.type != kNul)
				{
					devprop_add_value(card->device, devprop_list[i].name,
						devprop_list[i].default_val.type == kCst ?
						(uint8_t *)&(devprop_list[i].default_val.data) : devprop_list[i].default_val.data,
						devprop_list[i].default_val.size);
				}
				
				if (devprop_list[i].all_ports)
				{
					for (pnum = 1; pnum < card->ports; pnum++)
					{
						if (devprop_list[i].default_val.type != kNul)
						{
							devprop_list[i].name[1] = 0x30 + pnum; // convert to ascii
							devprop_add_value(card->device, devprop_list[i].name,
								devprop_list[i].default_val.type == kCst ?
								(uint8_t *)&(devprop_list[i].default_val.data) : devprop_list[i].default_val.data,
								devprop_list[i].default_val.size);
						}
					}
					devprop_list[i].name[1] = 0x30; // write back our "@0," for a next possible card
				}
			}
		}
	}

	free(val);
}

bool validate_rom(option_rom_header_t *rom_header, pci_dt_t *pci_dev)
{
	option_rom_pci_header_t *rom_pci_header;
	
	if (rom_header->signature != 0xaa55)
		return false;

	rom_pci_header = (option_rom_pci_header_t *)((uint8_t *)rom_header + rom_header->pci_header_offset);
	
	if (rom_pci_header->signature != 0x52494350)
		return false;
	
	if (rom_pci_header->vendor_id != pci_dev->vendor_id || rom_pci_header->device_id != pci_dev->device_id)
		return false;
	
	return true;
}

bool load_vbios_file(const char *key, uint16_t vendor_id, uint16_t device_id, uint32_t subsys_id)
{
	int fd;
	char file_name[24];
	bool do_load = false;

	getBoolForKey(key, &do_load, &bootInfo->chameleonConfig);
	if (!do_load)
		return false;

	sprintf(file_name, "/Extra/%04x_%04x_%08x.rom", vendor_id, device_id, subsys_id);
	if ((fd = open_bvdev("bt(0,0)", file_name, 0)) < 0)
		return false;

	card->rom_size = file_size(fd);
	card->rom = malloc(card->rom_size);
	if (!card->rom)
		return false;

	read(fd, (char *)card->rom, card->rom_size);

	if (!validate_rom((option_rom_header_t *)card->rom, card->pci_dev))
	{
		card->rom_size = 0;
		card->rom = 0;
		return false;
	}
	
	card->rom_size = ((option_rom_header_t *)card->rom)->rom_size * 512;

	close(fd);

	return true;
}

void get_vram_size(void)
{
	ati_chip_family_t chip_family = card->info->chip_family;
	
	card->vram_size = 0;

	if (chip_family >= CHIP_FAMILY_CEDAR)
		// size in MB on evergreen
		// XXX watch for overflow!!!
		card->vram_size = RegRead32(R600_CONFIG_MEMSIZE) * 1024 * 1024;
	else
		if (chip_family >= CHIP_FAMILY_R600)
			card->vram_size = RegRead32(R600_CONFIG_MEMSIZE);
}

bool read_vbios(bool from_pci)
{
	option_rom_header_t *rom_addr;
	
	if (from_pci)
	{
		rom_addr = (option_rom_header_t *)(pci_config_read32(card->pci_dev->dev.addr, PCI_ROM_ADDRESS) & ~0x7ff);
		verbose(" @0x%x", rom_addr);
	}
	else
		rom_addr = (option_rom_header_t *)0xc0000;
	
	if (!validate_rom(rom_addr, card->pci_dev))
		return false;
	
	card->rom_size = rom_addr->rom_size * 512;
	if (!card->rom_size)
		return false;
	
	card->rom = malloc(card->rom_size);
	if (!card->rom)
		return false;
	
	memcpy(card->rom, (void *)rom_addr, card->rom_size);
	
	return true;
}

bool read_disabled_vbios(void)
{
	bool ret = false;
	ati_chip_family_t chip_family = card->info->chip_family;
	
	if (chip_family >= CHIP_FAMILY_RV770)
	{
		uint32_t viph_control		= RegRead32(RADEON_VIPH_CONTROL);
		uint32_t bus_cntl			= RegRead32(RADEON_BUS_CNTL);
		uint32_t d1vga_control		= RegRead32(AVIVO_D1VGA_CONTROL);
		uint32_t d2vga_control		= RegRead32(AVIVO_D2VGA_CONTROL);
		uint32_t vga_render_control = RegRead32(AVIVO_VGA_RENDER_CONTROL);
		uint32_t rom_cntl			= RegRead32(R600_ROM_CNTL);
		uint32_t cg_spll_func_cntl	= 0;
		uint32_t cg_spll_status;
		
		// disable VIP
		RegWrite32(RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
		
		// enable the rom
		RegWrite32(RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
		
		// Disable VGA mode
		RegWrite32(AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		RegWrite32(AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		RegWrite32(AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
		
		if (chip_family == CHIP_FAMILY_RV730)
		{
			cg_spll_func_cntl = RegRead32(R600_CG_SPLL_FUNC_CNTL);
			
			// enable bypass mode
			RegWrite32(R600_CG_SPLL_FUNC_CNTL, (cg_spll_func_cntl | R600_SPLL_BYPASS_EN));
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
				cg_spll_status = RegRead32(R600_CG_SPLL_STATUS);
			
			RegWrite32(R600_ROM_CNTL, (rom_cntl & ~R600_SCK_OVERWRITE));
		}
		else
			RegWrite32(R600_ROM_CNTL, (rom_cntl | R600_SCK_OVERWRITE));

		ret = read_vbios(true);
		
		// restore regs
		if (chip_family == CHIP_FAMILY_RV730)
		{
			RegWrite32(R600_CG_SPLL_FUNC_CNTL, cg_spll_func_cntl);
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
			cg_spll_status = RegRead32(R600_CG_SPLL_STATUS);
		}
		RegWrite32(RADEON_VIPH_CONTROL, viph_control);
		RegWrite32(RADEON_BUS_CNTL, bus_cntl);
		RegWrite32(AVIVO_D1VGA_CONTROL, d1vga_control);
		RegWrite32(AVIVO_D2VGA_CONTROL, d2vga_control);
		RegWrite32(AVIVO_VGA_RENDER_CONTROL, vga_render_control);
		RegWrite32(R600_ROM_CNTL, rom_cntl);
	}
	else
		if (chip_family >= CHIP_FAMILY_R600)
		{
			uint32_t viph_control				= RegRead32(RADEON_VIPH_CONTROL);
			uint32_t bus_cntl					= RegRead32(RADEON_BUS_CNTL);
			uint32_t d1vga_control				= RegRead32(AVIVO_D1VGA_CONTROL);
			uint32_t d2vga_control				= RegRead32(AVIVO_D2VGA_CONTROL);
			uint32_t vga_render_control			= RegRead32(AVIVO_VGA_RENDER_CONTROL);
			uint32_t rom_cntl					= RegRead32(R600_ROM_CNTL);
			uint32_t general_pwrmgt				= RegRead32(R600_GENERAL_PWRMGT);
			uint32_t low_vid_lower_gpio_cntl	= RegRead32(R600_LOW_VID_LOWER_GPIO_CNTL);
			uint32_t medium_vid_lower_gpio_cntl = RegRead32(R600_MEDIUM_VID_LOWER_GPIO_CNTL);
			uint32_t high_vid_lower_gpio_cntl	= RegRead32(R600_HIGH_VID_LOWER_GPIO_CNTL);
			uint32_t ctxsw_vid_lower_gpio_cntl	= RegRead32(R600_CTXSW_VID_LOWER_GPIO_CNTL);
			uint32_t lower_gpio_enable			= RegRead32(R600_LOWER_GPIO_ENABLE);
			
			// disable VIP
			RegWrite32(RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
			
			// enable the rom
			RegWrite32(RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
			
			// Disable VGA mode
			RegWrite32(AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			RegWrite32(AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			RegWrite32(AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
			RegWrite32(R600_ROM_CNTL, ((rom_cntl & ~R600_SCK_PRESCALE_CRYSTAL_CLK_MASK) | (1 << R600_SCK_PRESCALE_CRYSTAL_CLK_SHIFT) | R600_SCK_OVERWRITE));
			RegWrite32(R600_GENERAL_PWRMGT, (general_pwrmgt & ~R600_OPEN_DRAIN_PADS));
			RegWrite32(R600_LOW_VID_LOWER_GPIO_CNTL, (low_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_MEDIUM_VID_LOWER_GPIO_CNTL, (medium_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_HIGH_VID_LOWER_GPIO_CNTL, (high_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_CTXSW_VID_LOWER_GPIO_CNTL, (ctxsw_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_LOWER_GPIO_ENABLE, (lower_gpio_enable | 0x400));
			
			ret = read_vbios(true);
			
			// restore regs
			RegWrite32(RADEON_VIPH_CONTROL, viph_control);
			RegWrite32(RADEON_BUS_CNTL, bus_cntl);
			RegWrite32(AVIVO_D1VGA_CONTROL, d1vga_control);
			RegWrite32(AVIVO_D2VGA_CONTROL, d2vga_control);
			RegWrite32(AVIVO_VGA_RENDER_CONTROL, vga_render_control);
			RegWrite32(R600_ROM_CNTL, rom_cntl);
			RegWrite32(R600_GENERAL_PWRMGT, general_pwrmgt);
			RegWrite32(R600_LOW_VID_LOWER_GPIO_CNTL, low_vid_lower_gpio_cntl);
			RegWrite32(R600_MEDIUM_VID_LOWER_GPIO_CNTL, medium_vid_lower_gpio_cntl);
			RegWrite32(R600_HIGH_VID_LOWER_GPIO_CNTL, high_vid_lower_gpio_cntl);
			RegWrite32(R600_CTXSW_VID_LOWER_GPIO_CNTL, ctxsw_vid_lower_gpio_cntl);
			RegWrite32(R600_LOWER_GPIO_ENABLE, lower_gpio_enable);
		}

	return ret;
}

bool radeon_card_posted(void)
{
	uint32_t reg;
	
	// first check CRTCs
	reg = RegRead32(RADEON_CRTC_GEN_CNTL) | RegRead32(RADEON_CRTC2_GEN_CNTL);
	if (reg & RADEON_CRTC_EN)
		return true;
	
	// then check MEM_SIZE, in case something turned the crtcs off
	reg = RegRead32(R600_CONFIG_MEMSIZE);
	if (reg)
		return true;
	
	return false;
}

#if 0
bool devprop_add_pci_config_space(void)
{
	int offset;
	
	uint8_t *config_space = malloc(0x100);
	if (!config_space)
		return false;
	
	for (offset = 0; offset < 0x100; offset += 4)
		config_space[offset / 4] = pci_config_read32(card->pci_dev->dev.addr, offset);
	
	devprop_add_value(card->device, "ATY,PCIConfigSpace", config_space, 0x100);
	free(config_space);
	
	return true;
}
#endif

static bool init_card(pci_dt_t *pci_dev)
{
	bool	add_vbios = true;
	char	name[24];
	char	name_parent[24];
	int		i;
	int		n_ports = 0;
	
	card = malloc(sizeof(card_t));
	if (!card)
		return false;
	bzero(card, sizeof(card_t));

	card->pci_dev = pci_dev;
	
	for (i = 0; radeon_cards[i].device_id ; i++)
	{
		if (radeon_cards[i].device_id == pci_dev->device_id)
		{
			//card->info = &radeon_cards[i]; // Jief
			if ((radeon_cards[i].subsys_id == 0x00000000) || (radeon_cards[i].subsys_id == pci_dev->subsys_id.subsys_id))
				card->info = &radeon_cards[i];
				break;
		}
	}

	//why can't this check go down to 1411?
	//If we move it down we would still allow the cfg_name check

	if (card->info == NULL) // Jief
	{
		verbose("Unsupported ATI card! Device ID: [%04x:%04x] Subsystem ID: [%08x] \n", 
				pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id);
		return false;
	}
	
	card->fb		= (uint8_t *)(pci_config_read32(pci_dev->dev.addr, PCI_BASE_ADDRESS_0) & ~0x0f);
	card->mmio		= (uint8_t *)(pci_config_read32(pci_dev->dev.addr, PCI_BASE_ADDRESS_2) & ~0x0f);
	card->io		= (uint8_t *)(pci_config_read32(pci_dev->dev.addr, PCI_BASE_ADDRESS_4) & ~0x03);

	verbose("Framebuffer @0x%08X  MMIO @0x%08X	I/O Port @0x%08X ROM Addr @0x%08X\n",
		card->fb, card->mmio, card->io, pci_config_read32(pci_dev->dev.addr, PCI_ROM_ADDRESS));
	
	card->posted = radeon_card_posted();
	verbose("ATI card %s, ", card->posted ? "POSTed" : "non-POSTed");
	
	get_vram_size();
	
	getBoolForKey(kATYbinimage, &add_vbios, &bootInfo->chameleonConfig);
	
	if (add_vbios)
	{
		if (!load_vbios_file(kUseAtiROM, pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id.subsys_id))
		{
			verbose("reading VBIOS from %s", card->posted ? "legacy space" : "PCI ROM");
			if (card->posted)
				read_vbios(false);
			else
				read_disabled_vbios();
			verbose("\n");
		}
	}


	if (card->info->chip_family >= CHIP_FAMILY_CEDAR)
	{
		card->flags |= EVERGREEN;
	}


	// Check AtiConfig key for a framebuffer name,
	card->cfg_name = getStringForKey(kAtiConfig, &bootInfo->chameleonConfig);

	// if none,
	if (!card->cfg_name)
	{
		// use cfg_name on radeon_cards, to retrive the default name from card_configs,
		card->cfg_name = card_configs[card->info->cfg_name].name;
		
		// which means one of the fb's or kNull
		verbose("Framebuffer set to device's default: %s\n", card->cfg_name);
	}
	else
	{
		// else, use the fb name returned by AtiConfig.
		verbose("(AtiConfig) Framebuffer set to: %s\n", card->cfg_name);
	}

	// Check AtiPorts key for nr of ports,
	card->ports = getIntForKey(kAtiPorts, &n_ports, &bootInfo->chameleonConfig);
	// if a value bigger than 0 ?? is found, (do we need >= 0 ?? that's null FB on card_configs)
	if (n_ports > 0)
	{
		card->ports = n_ports; // use it.
		verbose("(AtiPorts) # of ports set to: %d\n", card->ports);
	}
	else
	{
		// else, match cfg_name with card_configs list and retrive default nr of ports.
		for (i = 0; i < kCfgEnd; i++)
			if (strcmp(card->cfg_name, card_configs[i].name) == 0)
				card->ports = card_configs[i].ports; // default

		verbose("# of ports set to framebuffer's default: %d\n", card->ports);
	}


	sprintf(name, "ATY,%s", card->cfg_name);
	aty_name.type = kStr;
	aty_name.size = strlen(name) + 1;
	aty_name.data = (uint8_t *)name;
	
	sprintf(name_parent, "ATY,%sParent", card->cfg_name);
	aty_nameparent.type = kStr;
	aty_nameparent.size = strlen(name_parent) + 1;
	aty_nameparent.data = (uint8_t *)name_parent;
	
	return true;
}

bool setup_ati_devprop(pci_dt_t *ati_dev)
{
	char *devicepath;

	if (!init_card(ati_dev))
		return false;

	// -------------------------------------------------
	// Find a better way to do this (in device_inject.c)
	if (!string)
		string = devprop_create_string();
	
	devicepath = get_pci_dev_path(ati_dev);
	card->device = devprop_add_device(string, devicepath);
	if (!card->device)
		return false;
	// -------------------------------------------------
	
#if 0
	uint64_t fb	= (uint32_t)card->fb;
	uint64_t mmio	= (uint32_t)card->mmio;
	uint64_t io	= (uint32_t)card->io;
	devprop_add_value(card->device, "ATY,FrameBufferOffset", &fb, 8);
	devprop_add_value(card->device, "ATY,RegisterSpaceOffset", &mmio, 8);
	devprop_add_value(card->device, "ATY,IOSpaceOffset", &io, 8);
#endif
	
	devprop_add_list(ati_devprop_list);

	// -------------------------------------------------
	// Find a better way to do this (in device_inject.c)
	//Azi: XXX tried to fix a malloc error in vain; this is related to XCode 4 compilation!
	stringdata = malloc(sizeof(uint8_t) * string->length);
	memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
	stringlength = string->length;
	// -------------------------------------------------
	
	verbose("ATI %s %s %dMB (%s) [%04x:%04x] (subsys [%04x:%04x]):: %s\n",
			chip_family_name[card->info->chip_family], card->info->model_name,
			(uint32_t)(card->vram_size / (1024 * 1024)), card->cfg_name,
			ati_dev->vendor_id, ati_dev->device_id,
			ati_dev->subsys_id.subsys.vendor_id, ati_dev->subsys_id.subsys.device_id,
			devicepath);
	
	free(card);
	
	return true;
}
