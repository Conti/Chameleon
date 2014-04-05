/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#ifndef _PEXPERT_I386_BOOT_H
#define _PEXPERT_I386_BOOT_H

#include <stdint.h>

/*
 * What the booter leaves behind for the kernel.
 */

/*
 * Types of boot driver that may be loaded by the booter.
 */
enum {
    kBootDriverTypeInvalid = 0,
    kBootDriverTypeKEXT    = 1,
    kBootDriverTypeMKEXT   = 2
};

enum {
    kEfiReservedMemoryType	= 0,
    kEfiLoaderCode		= 1,
    kEfiLoaderData		= 2,
    kEfiBootServicesCode	= 3,
    kEfiBootServicesData	= 4,
    kEfiRuntimeServicesCode	= 5,
    kEfiRuntimeServicesData	= 6,
    kEfiConventionalMemory	= 7,
    kEfiUnusableMemory		= 8,
    kEfiACPIReclaimMemory	= 9,
    kEfiACPIMemoryNVS		= 10,
    kEfiMemoryMappedIO		= 11,
    kEfiMemoryMappedIOPortSpace = 12,
    kEfiPalCode			= 13,
    kEfiMaxMemoryType		= 14
};

/*
 * Memory range descriptor.
 */
typedef struct EfiMemoryRange {
    uint32_t Type;
    uint32_t Pad;
    uint64_t PhysicalStart;
    uint64_t VirtualStart;
    uint64_t NumberOfPages;
    uint64_t Attribute;
} EfiMemoryRange;

#define BOOT_LINE_LENGTH        1024
#define BOOT_STRING_LEN         BOOT_LINE_LENGTH

/*
 * Video information.. 
 */

struct Boot_Video {
	uint32_t	v_baseAddr;	/* Base address of video memory */
	uint32_t	v_display;	/* Display Code (if Applicable */
	uint32_t	v_rowBytes;	/* Number of bytes per pixel row */
	uint32_t	v_width;	/* Width */
	uint32_t	v_height;	/* Height */
	uint32_t	v_depth;	/* Pixel Depth */
};

typedef struct Boot_Video	Boot_Video;

/* Values for v_display */

#define GRAPHICS_MODE			1
#define FB_TEXT_MODE			2

/* Boot argument structure - passed into Mach kernel at boot time.
 * "Revision" can be incremented for compatible changes
 */
// Lion
#define kBootArgsRevision		0
#define kBootArgsVersion		2

// Snow Leopard and older
#define kBootArgsPreLionRevision	6
#define kBootArgsPreLionVersion		1

/* Snapshot constants of previous revisions that are supported */

#define kBootArgsEfiMode32		32
#define kBootArgsEfiMode64		64

typedef struct boot_args {
    uint16_t    Revision;	/* Revision of boot_args structure */
    uint16_t    Version;	/* Version of boot_args structure */

    uint8_t     efiMode;    /* 32 = 32-bit, 64 = 64-bit */
    uint8_t     debugMode;  /* Bit field with behavior changes */
    uint8_t     __reserved1[2];

    char        CommandLine[BOOT_LINE_LENGTH];	/* Passed in command line */

    uint32_t    MemoryMap;  /* Physical address of memory map */
    uint32_t    MemoryMapSize;
    uint32_t    MemoryMapDescriptorSize;
    uint32_t    MemoryMapDescriptorVersion;

    Boot_Video	Video;		/* Video Information */

    uint32_t    deviceTreeP;	  /* Physical address of flattened device tree */
    uint32_t    deviceTreeLength; /* Length of flattened tree */

    uint32_t    kaddr;            /* Physical address of beginning of kernel text */
    uint32_t    ksize;            /* Size of combined kernel text+data+efi */

    uint32_t    efiRuntimeServicesPageStart; /* physical address of defragmented runtime pages */
    uint32_t    efiRuntimeServicesPageCount;
    uint64_t    efiRuntimeServicesVirtualPageStart; /* virtual address of defragmented runtime pages */

    uint32_t    efiSystemTable;   /* physical address of system table in runtime area */
    uint32_t    __reserved2;

    uint32_t    performanceDataStart; /* physical address of log */
    uint32_t    performanceDataSize;

    uint32_t    keyStoreDataStart; /* physical address of key store data */
    uint32_t    keyStoreDataSize;
    uint64_t	bootMemStart;
    uint64_t	bootMemSize;
    uint64_t    PhysicalMemorySize;
    uint64_t    FSBFrequency;
    uint32_t    __reserved4[734];

} boot_args;

typedef struct boot_args_pre_lion {
    uint16_t    Revision;	/* Revision of boot_args structure */
    uint16_t    Version;	/* Version of boot_args structure */

    char        CommandLine[BOOT_LINE_LENGTH];	/* Passed in command line */

    uint32_t    MemoryMap;  /* Physical address of memory map */
    uint32_t    MemoryMapSize;
    uint32_t    MemoryMapDescriptorSize;
    uint32_t    MemoryMapDescriptorVersion;

    Boot_Video	Video;		/* Video Information */

    uint32_t    deviceTreeP;	  /* Physical address of flattened device tree */
    uint32_t    deviceTreeLength; /* Length of flattened tree */

    uint32_t    kaddr;            /* Physical address of beginning of kernel text */
    uint32_t    ksize;            /* Size of combined kernel text+data+efi */

    uint32_t    efiRuntimeServicesPageStart; /* physical address of defragmented runtime pages */
    uint32_t    efiRuntimeServicesPageCount;
    uint32_t    efiSystemTable;   /* physical address of system table in runtime area */

    uint8_t     efiMode;       /* 32 = 32-bit, 64 = 64-bit */
    uint8_t     __reserved1[3];
    uint32_t    __reserved2[1];
    uint32_t    performanceDataStart; /* physical address of log */
    uint32_t    performanceDataSize;
    uint64_t    efiRuntimeServicesVirtualPageStart; /* virtual address of defragmented runtime pages */
    uint32_t    __reserved3[2];

} boot_args_pre_lion;

extern char gMacOSVersion[8];

#endif /* _PEXPERT_I386_BOOT_H */
