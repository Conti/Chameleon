/*
 * Copyright 2008 mackerintel
 */
/*
 * AsereBLN: cleanup
 */

#ifndef __LIBSAIO_SMBIOS_PATCHER_H
#define __LIBSAIO_SMBIOS_PATCHER_H

#include "libsaio.h"
#include "SMBIOS.h"

/* From Foundation/Efi/Guid/Smbios/SmBios.h */
/* Modified to wrap Data4 array init with {} */
#define EFI_SMBIOS_TABLE_GUID {0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}

#define SMBIOS_RANGE_START      0x000F0000
#define SMBIOS_RANGE_END        0x000FFFFF

#define SMBIOS_ORIGINAL		0
#define SMBIOS_PATCHED		1

struct smbios_table_header 
{
	uint8_t		type;
	uint8_t		length;
	uint16_t	handle;
} __attribute__ ((packed));

struct smbios_property
{
	const char		*name;
	uint8_t		table_type;
	enum {SMSTRING, SMWORD, SMBYTE, SMOWORD} value_type;
	int		offset;
	int		(*auto_int) (const char *name, int table_num);
	const char	*(*auto_str) (const char *name, int table_num);
	const char	*(*auto_oword) (const char *name, int table_num);
};

struct smbios_table_description
{
	uint8_t		type;
	int		len;
	int		(*numfunc)(int tablen);
};

/** call with flag SMBIOS_ORIGINAL to get orig. entrypoint
   or call with flag SMBIOS_PATCHED to get patched smbios entrypoint
*/
extern struct DMIHeader *getSmbiosTableStructure(struct SMBEntryPoint	*smbios, int type, int min_length);
extern struct SMBEntryPoint	*getSmbios(int);
extern struct DMIHeader* FindNextDmiTableOfType(int type, int minlen);
extern struct DMIHeader* FindFirstDmiTableOfType(int type, int minlen);

#endif /* !__LIBSAIO_SMBIOS_PATCHER_H */
