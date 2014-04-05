/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright 1994 NeXT Computer, Inc.
 * All rights reserved.
 */

#ifndef __LIBSAIO_BIOS_H
#define __LIBSAIO_BIOS_H

#include "bootargs.h"

typedef union {
	unsigned int      rx;
	unsigned short    rr;
	struct {
		unsigned char l;
		unsigned char h;
	} r;
} machineRegister_t;

typedef struct {
	unsigned short cf  :1;
	unsigned short     :1;
	unsigned short pf  :1;
	unsigned short     :1;
	unsigned short af  :1;
	unsigned short     :1;
	unsigned short zf  :1;
	unsigned short sf  :1;
	unsigned short tf  :1;
	unsigned short _if :1;
	unsigned short df  :1;
	unsigned short of  :1;
	unsigned short iopl:2;
	unsigned short nt  :1;
} machineFlags_t;

typedef struct {
	unsigned int      intno;
	machineRegister_t eax;
	machineRegister_t ebx;
	machineRegister_t ecx;
	machineRegister_t edx;
	machineRegister_t edi;
	machineRegister_t esi;
	machineRegister_t ebp;
	unsigned short    cs;
	unsigned short    ds;
	unsigned short    es;
	machineFlags_t    flags;
} biosBuf_t;

#define EBIOS_FIXED_DISK_ACCESS		0x01
#define EBIOS_LOCKING_ACCESS		0x02
#define EBIOS_ENHANCED_DRIVE_INFO	0x04

#define BASE_HD_DRIVE			0x80

#if 0
/* 
 * ACPI defined memory range types.
 */
enum {
	kMemoryRangeUsable   = 1,    // RAM usable by the OS.
	kMemoryRangeReserved = 2,    // Reserved. (Do not use)
	kMemoryRangeACPI     = 3,    // ACPI tables. Can be reclaimed.
	kMemoryRangeNVS      = 4,    // ACPI NVS memory. (Do not use)

	/* Undefined types should be treated as kMemoryRangeReserved */
}; 
#endif

/*
 * Memory range descriptor.
 */
typedef struct MemoryRange {
	unsigned long long base;     // 64-bit base address
	unsigned long long length;   // 64-bit length in bytes
	unsigned long      type;     // type of memory range
	unsigned long      reserved;
} MemoryRange;

#endif /* !__LIBSAIO_BIOS_H */
