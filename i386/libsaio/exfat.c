/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2004 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * dmazar, 14/7/2011
 * support for EXFAT volume label reading
 * EXFAT info from: http://www.ntfs.com/exfat-overview.htm
 *
 * EXFAT shares partition type with NTFS (0x7) and easiest way of
 * adding it was through ntfs.c module. All functions here are called
 * from similar ntfs.c funcs as fallback (if not NTFS, maybe it's EXFAT).
 */

#include "libsaio.h"
#include "sl.h"

#ifndef DEBUG_EXFAT
#define DEBUG_EXFAT 0
#endif

#if DEBUG_EXFAT
#define DBG(x...)		printf(x)
#define PAUSE()			getchar()
#else
#define DBG(x...)
#define PAUSE()
#endif

#define	EXFAT_BBID	"EXFAT   "
#define	EXFAT_BBIDLEN	8

#define MAX_BLOCK_SIZE		4096
#define MAX_CLUSTER_SIZE	32 * 1024 * 1024

#define ERROR -1


/*
 * boot sector of the partition
 * http://www.ntfs.com/exfat-boot-sector.htm
 */
struct exfatbootfile {
	u_int8_t        reserved1[3];	/* JumpBoot: 0xEB7690 */
	u_int8_t        bf_sysid[8];	/* FileSystemName: 'EXFAT   ' */
	u_int8_t        reserved2[53];	/* MustBeZero */
	u_int64_t       bf_prtoff;		/* PartitionOffset: In sectors; if 0, shall be ignored */
	u_int64_t       bf_vollen;		/* VolumeLength: Size of exFAT volume in sectors */
	u_int32_t       bf_fatoff;		/* FatOffset: In sectors */
	u_int32_t       bf_fatlen;		/* FatLength: In sectors. May exceed the required space in order to align the second FAT */
	u_int32_t       bf_cloff;		/* ClusterHeapOffset: In sectors. */
	u_int32_t       bf_clcnt;		/* ClusterCount: 2^32-11 is the maximum number of clusters could be described. */
	u_int32_t       bf_rdircl;		/* RootDirectoryCluster. */
	u_int32_t       bf_volsn;		/* VolumeSerialNumber. */
	u_int16_t       bf_fsrev;		/* FileSystemRevision: as MAJOR.minor, major revision is high byte, minor is low byte; currently 01.00. */
	u_int16_t       bf_volflags;	/* VolumeFlags. */
	u_int8_t        bf_bpss;		/* BytesPerSectorShift: Power of 2. Minimum 9 (512 bytes per sector), maximum 12 (4096 bytes per sector) */
	u_int8_t        bf_spcs;		/* SectorsPerClusterShift: Power of 2. Minimum 0 (1 sector per cluster), maximum 25 Ð BytesPerSectorShift, so max cluster size is 32 MB */
	u_int8_t        bf_nfats;		/* NumberOfFats: 2 is for TexFAT only */
	u_int8_t        bf_drvs;		/* DriveSelect: Extended INT 13h drive number; typically 0x80 */
	u_int8_t        bf_pused;		/* PercentInUse: 0..100 Ð percentage of allocated clusters rounded down to the integer 0xFF Ð percentage is not available */
	u_int8_t        reserved3[7];	/* Reserved */
	u_int8_t        bootcode[390];	/* BootCode */
	u_int16_t       bf_bsig;		/* BootSignature: 0xAA55 */
};

struct direntry_label {
	u_int8_t        type;			/* EntryType: 0x83 (or 0x03 if label is empty) */
	u_int8_t        llen;			/* CharacterCount: Length in Unicode characters (max 11) */
	u_int16_t       label[11];		/* VolumeLabel: Unicode characters (max 11) */
	u_int8_t        reserved1[8];	/* Reserved */
};


/**
 * Reads volume label into str.
 * Reads boot sector, performs dome checking, loads root dir
 * and parses volume label.
 */
void
EXFATGetDescription(CICell ih, char *str, long strMaxLen)
{
    struct exfatbootfile *boot;
    u_int32_t bytesPerSector = 0;
    u_int32_t sectorsPerCluster = 0;
    long long  rdirOffset = 0;
    char *buf = NULL;
    struct direntry_label *dire = NULL;
    int loopControl = 0;

    DBG("EXFAT: start %x:%x\n", ih->biosdev, ih->part_no);
	
    buf = (char *)malloc(MAX_BLOCK_SIZE);
    if (buf == 0)
    {
        goto error;
    }

    /*
     * Read the boot sector, check signatures, and do some minimal
     * sanity checking.  NOTE: the size of the read below is intended
     * to be a multiple of all supported block sizes, so we don't
     * have to determine or change the device's block size.
     */
    Seek(ih, 0);
    Read(ih, (long)buf, MAX_BLOCK_SIZE);

    // take our boot structure
    boot = (struct exfatbootfile *) buf;
    
    /*
     * The first three bytes are an Intel x86 jump instruction.  I assume it
     * can be the same forms as DOS FAT:
     *    0xE9 0x?? 0x??
     *    0xEC 0x?? 0x90
     * where 0x?? means any byte value is OK.
     */
    if (boot->reserved1[0] != 0xE9
        && (boot->reserved1[0] != 0xEB || boot->reserved1[2] != 0x90))
    {
        goto error;
    }

     // Check the "EXFAT   " signature.
    if (memcmp((const char *)boot->bf_sysid, EXFAT_BBID, EXFAT_BBIDLEN) != 0)
    {
        goto error;
    }

    /*
     * Make sure the bytes per sector and sectors per cluster are
     * powers of two, and within reasonable ranges.
     */
    bytesPerSector = 1 << boot->bf_bpss;	/* Just one byte; no swapping needed */
    DBG("EXFAT: bpss=%d, bytesPerSector=%d\n", boot->bf_bpss, bytesPerSector);
    if (boot->bf_bpss < 9 || boot->bf_bpss > 12)
    {
        DBG("EXFAT: invalid bytes per sector shift(%d)\n", boot->bf_bpss);
        goto error;
    }

    sectorsPerCluster = 1 << boot->bf_spcs;	/* Just one byte; no swapping needed */
    DBG("EXFAT: spcs=%d, sectorsPerCluster=%d\n", boot->bf_spcs, sectorsPerCluster);
    if (boot->bf_spcs > (25 - boot->bf_bpss))
    {
        DBG("EXFAT: invalid sectors per cluster shift (%d)\n", boot->bf_spcs);
        goto error;
    }
    
     // calculate root dir cluster offset
    rdirOffset = boot->bf_cloff + (boot->bf_rdircl - 2) * sectorsPerCluster;
    DBG("EXFAT: rdirOffset=%d\n", rdirOffset);

    // load MAX_BLOCK_SIZE bytes of root dir
    Seek(ih, rdirOffset * bytesPerSector);
    Read(ih, (long)buf, MAX_BLOCK_SIZE);
    DBG("buf 0 1 2 = %x %x %x\n", 0x00ff & buf[0], 0x00ff & buf[1], 0x00ff & buf[2]);

    str[0] = '\0';

    /*
     * Search for volume label dir entry (type 0x83), convert from unicode and put to str.
     * Set loopControl var to avoid searching outside of buf.
     */
    loopControl = MAX_BLOCK_SIZE / sizeof(struct direntry_label);
    dire = (struct direntry_label *)buf;
    while (loopControl && dire->type && dire->type != 0x83)
    {
        dire++;
        loopControl--;
    }
    if (dire->type == 0x83 && dire->llen > 0 && dire->llen <= 11)
    {
        utf_encodestr( dire->label, (int)dire->llen, (u_int8_t *)str, strMaxLen, OSLittleEndian );
    }
    DBG("EXFAT: label=%s\n", str);

    free(buf);
    PAUSE();
    return;

 error:
    if (buf) free(buf);
    DBG("EXFAT: error\n");
    PAUSE();
    return;
}

/**
 * Sets UUID to uuidStr.
 * Reads the boot sector, does some checking, generates UUID
 * (like the one you get on Windows???)
 */
long EXFATGetUUID(CICell ih, char *uuidStr)
{
    struct exfatbootfile *boot;
    void *buf = malloc(MAX_BLOCK_SIZE);
    if ( !buf )
        return -1;

    /*
     * Read the boot sector, check signatures, and do some minimal
     * sanity checking.	 NOTE: the size of the read below is intended
     * to be a multiple of all supported block sizes, so we don't
     * have to determine or change the device's block size.
     */
    Seek(ih, 0);
    Read(ih, (long)buf, MAX_BLOCK_SIZE);

    boot = (struct exfatbootfile *) buf;

    /*
     * Check the "EXFAT   " signature.
     */
    if (memcmp((const char *)boot->bf_sysid, EXFAT_BBID, EXFAT_BBIDLEN) != 0)
        return -1;

    // Check for non-null volume serial number
    if( !boot->bf_volsn )
        return -1;

    // Use UUID like the one you get on Windows
    sprintf(uuidStr, "%04X-%04X",   (unsigned short)(boot->bf_volsn >> 16) & 0xFFFF,
                                    (unsigned short)boot->bf_volsn & 0xFFFF);

    DBG("EXFATGetUUID: %x:%x = %s\n", ih->biosdev, ih->part_no, uuidStr);
    return 0;
}    

/**
 * Returns true if given buffer is the boot rec of the EXFAT volume.
 */
bool EXFATProbe(const void * buffer)
{
    bool result = false;

    // boot sector structure
    const struct exfatbootfile	* boot = buffer;

    // Looking for EXFAT signature.
    if (memcmp((const char *)boot->bf_sysid, EXFAT_BBID, EXFAT_BBIDLEN) == 0)
        result = true;
	
    DBG("EXFATProbe: %d\n", result ? 1 : 0);
    return result;
}
