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
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 *
 */
/*
 * HISTORY
 * Revision 2.3  88/08/08  13:47:07  rvb
 * Allocate buffers dynamically vs statically.
 * Now b[i] and i_fs and i_buf, are allocated dynamically.
 * boot_calloc(size) allocates and zeros a  buffer rounded to a NPG
 * boundary.
 * Generalize boot spec to allow, xx()/mach, xx(n,[a..h])/mach,
 * xx([a..h])/mach, ...
 * Also default "xx" if unspecified and alloc just "/mach",
 * where everything is defaulted
 * Add routine, ptol(), to parse partition letters.
 *
 */
 
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)sys.c	7.1 (Berkeley) 6/5/86
 */

/*  Copyright 2007 VMware Inc.
    "Preboot" ramdisk support added by David Elliott
 */


#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "disk.h"
#include "ramdisk.h"
#include "xml.h"

#include <libkern/crypto/md5.h>
//#include <uuid/uuid.h>

#if 0 /* No OS X release has ever included this. */
#include <Kernel/uuid/namespace.h>
#else
// from our uuid/namespace.h (UFS and HFS uuids can live in the same space?)
static unsigned char kFSUUIDNamespaceSHA1[] = {0xB3,0xE2,0x0F,0x39,0xF2,0x92,0x11,0xD6,0x97,0xA4,0x00,0x30,0x65,0x43,0xEC,0xAC};
#endif

extern int multiboot_partition;
extern int multiboot_partition_set;
extern int multiboot_skip_partition;
extern int multiboot_skip_partition_set;

struct devsw {
	const char *  name;
	// size increased from char to short to handle non-BIOS internal devices
	unsigned short biosdev;
	int type;
};

// Device entries must be ordered by bios device numbers. 
static struct devsw devsw[] =
{
	{ "hd", 0x80,	kBIOSDevTypeHardDrive }, /* DEV_HD */
	{ "en", 0xE0,	kBIOSDevTypeNetwork   }, /* DEV_EN */
	{ "rd", 0x100,	kBIOSDevTypeHardDrive },
	{ "bt", 0x101,	kBIOSDevTypeHardDrive }, // turbo - type for booter partition
	{ 0, 0 }
};

// Pseudo BIOS devices
enum {
	kPseudoBIOSDevRAMDisk = 0x100,
	kPseudoBIOSDevBooter = 0x101
};

/*
 * Max number of file descriptors.
 */
#define NFILES  6

static struct iob iob[NFILES];

void * gFSLoadAddress = 0;

// Turbo - save what we think is our original BIOS boot volume if we have one 0xab
BVRef gBIOSBootVolume = NULL;
BVRef gBootVolume;

//static BVRef getBootVolumeRef( const char * path, const char ** outPath );
static BVRef newBootVolumeRef( int biosdev, int partno );

//==========================================================================
// LoadVolumeFile - LOW-LEVEL FILESYSTEM FUNCTION.
//            Load the specified file from the specified volume
//            to the load buffer at LOAD_ADDR.
//            If the file is fat, load only the i386 portion.

long LoadVolumeFile(BVRef bvr, const char *filePath)
{
    long         fileSize;

    // Read file into load buffer. The data in the load buffer will be
    // overwritten by the next LoadFile() call.

    gFSLoadAddress = (void *) LOAD_ADDR;

    fileSize = bvr->fs_loadfile(bvr, (char *)filePath);

    // Return the size of the file, or -1 if load failed.

    return fileSize;
}

//==========================================================================
// LoadFile - LOW-LEVEL FILESYSTEM FUNCTION.
//            Load the specified file to the load buffer at LOAD_ADDR.
//            If the file is fat, load only the i386 portion.

long LoadFile(const char * fileSpec)
{
	const char * filePath;
	BVRef        bvr;

	// Resolve the boot volume from the file spec.

	if ((bvr = getBootVolumeRef(fileSpec, &filePath)) == NULL) {
		return -1;
	}

	return LoadVolumeFile(bvr, filePath);
}

//==========================================================================

long ReadFileAtOffset(const char * fileSpec, void *buffer, uint64_t offset, uint64_t length)
{
	const char *filePath;
	BVRef bvr;

	if ((bvr = getBootVolumeRef(fileSpec, &filePath)) == NULL) {
		return -1;
	}

	if (bvr->fs_readfile == NULL) {
		return -1;
	}

	return bvr->fs_readfile(bvr, (char *)filePath, buffer, offset, length);
}

//==========================================================================

long LoadThinFatFile(const char *fileSpec, void **binary)
{
	const char	*filePath;
	FSReadFile	readFile;
	BVRef		bvr;
	unsigned long length, length2;

	// Resolve the boot volume from the file spec.

	if ((bvr = getBootVolumeRef(fileSpec, &filePath)) == NULL) {
		return -1;
	}

	*binary = (void *)kLoadAddr;
  
	// Read file into load buffer. The data in the load buffer will be
	// overwritten by the next LoadFile() call.

	gFSLoadAddress = (void *) LOAD_ADDR;

	readFile = bvr->fs_readfile;
  
	if (readFile != NULL) {
		// Read the first 4096 bytes (fat header)
		length = readFile(bvr, (char *)filePath, *binary, 0, 0x1000);

		if (length > 0) {
			if (ThinFatFile(binary, &length) == 0) {
				if (length == 0) {
					return 0;
				}

				// We found a fat binary; read only the thin part
				length = readFile(bvr, (char *)filePath, (void *)kLoadAddr, (unsigned long)(*binary) - kLoadAddr, length);
				*binary = (void *)kLoadAddr;
			} else 	{
				// Not a fat binary; read the rest of the file
				length2 = readFile(bvr, (char *)filePath, (void *)(kLoadAddr + length), length, 0);

				if (length2 == -1) {
					return -1;
				}

				length += length2;
			}
		}
	} else {
		length = bvr->fs_loadfile(bvr, (char *)filePath);

		if (length > 0) {
			ThinFatFile(binary, &length);
		}
	}
  
	return length;
}

//==========================================================================

#if UNUSED
long GetFSUUID(char *spec, char *uuidStr)
{
	BVRef      bvr;
	long       rval = -1;
	const char *devSpec;

	if ((bvr = getBootVolumeRef(spec, &devSpec)) == NULL) {
		return -1;
	}

	if(bvr->fs_getuuid) {
		rval = bvr->fs_getuuid(bvr, uuidStr);
	}

	return rval;
}
#endif

// filesystem-specific getUUID functions call this shared string generator

long CreateUUIDString(uint8_t uubytes[], int nbytes, char *uuidStr)
{
    unsigned  fmtbase, fmtidx, i;
    uint8_t   uuidfmt[] = { 4, 2, 2, 2, 6 };
    char     *p = uuidStr;
    MD5_CTX   md5c;
    uint8_t   mdresult[16];

    bzero(mdresult, sizeof(mdresult));

    // just like AppleFileSystemDriver
    MD5Init(&md5c);
    MD5Update(&md5c, kFSUUIDNamespaceSHA1, sizeof(kFSUUIDNamespaceSHA1));
    MD5Update(&md5c, uubytes, nbytes);
    MD5Final(mdresult, &md5c);

    // this UUID has been made version 3 style (i.e. via namespace)
    // see "-uuid-urn-" IETF draft (which otherwise copies byte for byte)
    mdresult[6] = 0x30 | ( mdresult[6] & 0x0F );
    mdresult[8] = 0x80 | ( mdresult[8] & 0x3F );


    // generate the text: e.g. 5EB1869F-C4FA-3502-BDEB-3B8ED5D87292
	i = 0;
	fmtbase = 0;

	for(fmtidx = 0; fmtidx < sizeof(uuidfmt); fmtidx++) {
		for (i = 0; i < uuidfmt[fmtidx]; i++) {
			uint8_t byte = mdresult[fmtbase + i];
			char nib = byte >> 4;
			*p = nib + '0';  // 0x4 -> '4'

			if (*p > '9') {
				*p = (nib - 9 + ('A'-1));  // 0xB -> 'B'
			}

			p++;

			nib = byte & 0xf;
			*p = nib + '0';  // 0x4 -> '4'

			if (*p > '9') {
				*p = (nib - 9 + ('A'-1));  // 0xB -> 'B'
			}

			p++;
		}

		fmtbase += i;

		if (fmtidx < sizeof(uuidfmt) - 1) {
			*(p++) = '-';
		}
		else
		{
			*p = '\0';
		}
	}

	return 0;
}


//==========================================================================
// GetDirEntry - LOW-LEVEL FILESYSTEM FUNCTION.
//               Fetch the next directory entry for the given directory.

long GetDirEntry(const char * dirSpec, long long * dirIndex, const char ** name, 
                 long * flags, long * time)
{
	const char * dirPath;
	BVRef        bvr;

	// Resolve the boot volume from the dir spec.

	if ((bvr = getBootVolumeRef(dirSpec, &dirPath)) == NULL) {
		return -1;
	}

	// Returns 0 on success or -1 when there are no additional entries.

    return bvr->fs_getdirentry( bvr,
                /* dirPath */   (char *)dirPath,
                /* dirIndex */  dirIndex,
                /* dirEntry */  (char **)name, flags, time, 0, 0 );
}

//==========================================================================
// GetFileInfo - LOW-LEVEL FILESYSTEM FUNCTION.
//               Get attributes for the specified file.

static char* gMakeDirSpec;

long GetFileInfo(const char * dirSpec, const char * name,
                 long * flags, long * time)
{
	long long index = 0;
	const char * entryName;

	if (gMakeDirSpec == 0) {
		gMakeDirSpec = (char *)malloc(1024);
	}

	if (!dirSpec) {
		long       idx, len;

		len = strlen(name);

		for (idx = len; idx && (name[idx] != '/' && name[idx] != '\\'); idx--) {}
		if (idx == 0) {
			if(name[idx] == '/' || name[idx] == '\\') ++name;   // todo: ensure other functions handel \ properly
			gMakeDirSpec[0] = '/';
			gMakeDirSpec[1] = '\0';
			gMakeDirSpec[idx] = '\0';
		} else {
			idx++;
			strncpy(gMakeDirSpec, name, idx);
			gMakeDirSpec[idx] = '\0'; // ISSUE: http://forge.voodooprojects.org/p/chameleon/issues/270/
			name += idx;
		}

		dirSpec = gMakeDirSpec;
	}

	while (GetDirEntry(dirSpec, &index, &entryName, flags, time) == 0)
	{
		if (strcmp(entryName, name) == 0) {
			return 0;  // success
		}
	}

	return -1;  // file not found
}

//==============================================================================

long GetFileBlock(const char *fileSpec, unsigned long long *firstBlock)
{
	const char * filePath;
	BVRef        bvr;

	// Resolve the boot volume from the file spec.

	if ((bvr = getBootVolumeRef(fileSpec, &filePath)) == NULL) {
		// printf("Boot volume for '%s' is bogus\n", fileSpec);
		return -1;
	}

	return bvr->fs_getfileblock(bvr, (char *)filePath, firstBlock);
}

//==========================================================================
// GetFreeFD()

static int GetFreeFd(void)
{
	int	fd;

	// Locate a free descriptor slot.
	for (fd = 0; fd < NFILES; fd++) {
		if (iob[fd].i_flgs == 0) {
			return fd;
	    	}
	}
	stop("Out of file descriptors");
	// not reached
	return -1;
}

//==========================================================================
// iob_from_fdesc()
//
// Return a pointer to an allocated 'iob' based on the file descriptor
// provided. Returns NULL if the file descriptor given is invalid.

static struct iob * iob_from_fdesc(int fdesc)
{
	register struct iob * io;

	if (fdesc < 0 || fdesc >= NFILES || ((io = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		return NULL;
	} else {
		return io;
	}
}

//==========================================================================
// openmem()

int openmem(char * buf, int len)
{
    int          fdesc;
    struct iob * io;

    fdesc = GetFreeFd();
    io = &iob[fdesc];
    bzero(io, sizeof(*io));

    // Mark the descriptor as taken. Set the F_MEM flag to indicate
    // that the file buffer is provided by the caller.

    io->i_flgs     = F_ALLOC | F_MEM;
    io->i_buf      = buf;
    io->i_filesize = len;

    return fdesc;
}

//==========================================================================
// open() - Open the file specified by 'path' for reading.

static int open_bvr(BVRef bvr, const char *filePath, int flags)
{
	struct iob	*io;
	int		fdesc;
	int		i;

	if (bvr == NULL) {
		return -1;
	}

	fdesc = GetFreeFd();
	io = &iob[fdesc];
	bzero(io, sizeof(*io));

	// Mark the descriptor as taken.
	io->i_flgs = F_ALLOC;

	// Find the next available memory block in the download buffer.
	io->i_buf = (char *) LOAD_ADDR;
	for (i = 0; i < NFILES; i++) {
		if ((iob[i].i_flgs != F_ALLOC) || (i == fdesc)) {
			continue;
		}
		io->i_buf = MAX(iob[i].i_filesize + iob[i].i_buf, io->i_buf);
	}

	// Load entire file into memory. Unnecessary open() calls must be avoided.
	gFSLoadAddress = io->i_buf;
	io->i_filesize = bvr->fs_loadfile(bvr, (char *)filePath);
	if (io->i_filesize < 0) {
		close(fdesc);
		return -1;
	}
	return fdesc;
}

int open(const char *path, int flags)
{
	const char	*filepath;
	BVRef		bvr;

	// Resolve the boot volume from the file spec.
	if ((bvr = getBootVolumeRef(path, &filepath)) != NULL) {
		return open_bvr(bvr, filepath, flags);
	}
	return -1;
}

int open_bvdev(const char *bvd, const char *path, int flags)
{
    const struct devsw	*dp;
	const char			*cp;
	BVRef				bvr;
	int					i;
	int					len;
	int					unit;
	int					partition;

	if ((i = open(path, flags)) >= 0) {
		return i;
	}

	if (bvd == NULL || (len = strlen(bvd)) < 2) {
		return -1;
	}

	for (dp=devsw; dp->name; dp++) {
		if (bvd[0] == dp->name[0] && bvd[1] == dp->name[1]) {
			unit = 0;
			partition = 0;
			/* get optional unit and partition */
			if (len >= 5 && bvd[2] == '(') { /* min must be present xx(0) */
				cp = &bvd[3];
				i = 0;
				while ((cp - path) < len && isdigit(*cp)) {
					i = i * 10 + *cp++ - '0';
					unit = i;
				}
				if (*cp++ == ',') {
					i = 0;
					while ((cp - path) < len && isdigit(*cp)) {
						i = i * 10 + *cp++ - '0';
						partition = i;
					}
				}
			}
			bvr = newBootVolumeRef(dp->biosdev + unit, partition);
			return open_bvr(bvr, path, flags);
		}
    }
	return -1;
}

//==========================================================================
// close() - Close a file descriptor.

int close(int fdesc)
{
	struct iob * io;

	if ((io = iob_from_fdesc(fdesc)) == NULL) {
		return (-1);
	}

	io->i_flgs = 0;

	return 0;
}

//==========================================================================
// lseek() - Reposition the byte offset of the file descriptor from the
//           beginning of the file. Returns the relocated offset.

int b_lseek(int fdesc, int offset, int ptr)
{
	struct iob * io;

	if ((io = iob_from_fdesc(fdesc)) == NULL) {
		return (-1);
	}

	io->i_offset = offset;

	return offset;
}

//==========================================================================
// tell() - Returns the byte offset of the file descriptor.

int tell(int fdesc)
{
	struct iob * io;

	if ((io = iob_from_fdesc(fdesc)) == NULL) {
		return 0;
	}

	return io->i_offset;
}

//==========================================================================
// read() - Read up to 'count' bytes of data from the file descriptor
//          into the buffer pointed to by buf.

int read(int fdesc, char * buf, int count)
{
	struct iob * io;

	if ((io = iob_from_fdesc(fdesc)) == NULL) {
		return (-1);
	}

	if ((io->i_offset + count) > (unsigned int)io->i_filesize) {
		count = io->i_filesize - io->i_offset;
	}

	if (count <= 0) {
		return 0;  // end of file
	}

	bcopy(io->i_buf + io->i_offset, buf, count);

	io->i_offset += count;

	return count;
}

//==========================================================================
// write() - Write up to 'count' bytes of data to the file descriptor
//          from the buffer pointed to by buf.

int write(int fdesc, const char * buf, int count)
{
    struct iob * io;
    
    if ((io = iob_from_fdesc(fdesc)) == NULL)
        return (-1);
	
    if ((io->i_offset + count) > (unsigned int)io->i_filesize)
        count = io->i_filesize - io->i_offset;
	
    if (count <= 0)
        return 0;  // end of file
	
    bcopy(buf, io->i_buf + io->i_offset, count);
	
    io->i_offset += count;
	
    return count;
}

int writebyte(int fdesc, char value)
{
    struct iob * io;
    
    if ((io = iob_from_fdesc(fdesc)) == NULL)
        return (-1);
	
    if ((io->i_offset + 1) > (unsigned int)io->i_filesize)
        return 0;  // end of file
	
    io->i_buf[io->i_offset++] = value;
	
    return 1;
}

int writeint(int fdesc, int value)
{
    struct iob * io;
    
    if ((io = iob_from_fdesc(fdesc)) == NULL)
        return (-1);
	
    if ((io->i_offset + 4) > (unsigned int)io->i_filesize)
        return 0;  // end of file
	
    bcopy(&value, io->i_buf + io->i_offset, 4);
	
    io->i_offset += 4;
	
    return 4;
}

//==========================================================================
// file_size() - Returns the size of the file described by the file
//               descriptor.

int file_size(int fdesc)
{
	struct iob * io;

	if ((io = iob_from_fdesc(fdesc)) == 0) {
		return 0;
	}

	return io->i_filesize;
}

//==========================================================================

struct dirstuff * vol_opendir(BVRef bvr, const char * path)
{
	struct dirstuff * dirp = 0;

	dirp = (struct dirstuff *) malloc(sizeof(struct dirstuff));

	if (dirp == NULL)
		goto error;

	dirp->dir_path = newString(path);
	if (dirp->dir_path == NULL)
		goto error;

	dirp->dir_bvr = bvr;

	return dirp;

error:
	closedir(dirp);

	return NULL;
}

//==========================================================================

struct dirstuff * opendir(const char * path)
{
	struct dirstuff * dirp = 0;
	const char *      dirPath;
	BVRef             bvr;

    if ((bvr = getBootVolumeRef(path, &dirPath)) == NULL)
        goto error;

    dirp = (struct dirstuff *) malloc(sizeof(struct dirstuff));
    if (dirp == NULL)
        goto error;

    dirp->dir_path = newString(dirPath);
    if (dirp->dir_path == NULL)
        goto error;

    dirp->dir_bvr = bvr;

    return dirp;

error:
    closedir(dirp);
    return NULL;
}

//==========================================================================

int closedir(struct dirstuff * dirp)
{
	if (dirp) {
		if (dirp->dir_path) {
			free(dirp->dir_path);
		}

		free(dirp);
	}

    return 0;
}

//==========================================================================

int readdir(struct dirstuff * dirp, const char ** name, long * flags,
	long * time)
{
	return dirp->dir_bvr->fs_getdirentry(dirp->dir_bvr,
		/* dirPath */   dirp->dir_path,
		/* dirIndex */  &dirp->dir_index,
		/* dirEntry */  (char **)name, flags, time, 0, 0);
}

//==========================================================================

int readdir_ext(struct dirstuff * dirp, const char ** name, long * flags,
	long * time, FinderInfo *finderInfo, long *infoValid)
{
	return dirp->dir_bvr->fs_getdirentry( dirp->dir_bvr,
			/* dirPath */   dirp->dir_path,
			/* dirIndex */  &dirp->dir_index,
			/* dirEntry */  (char **)name,
					flags, time,
					finderInfo, infoValid);
}

//==========================================================================

const char * systemConfigDir()
{
	if (gBootFileType == kNetworkDeviceType) {
		return "";
	}
	return "/Library/Preferences/SystemConfiguration";
}

//==========================================================================

int gBootFileType;

void scanBootVolumes(int biosdev, int * count)
{
	BVRef bvr = 0;

	bvr = diskScanBootVolumes(biosdev, count);
	if (bvr == NULL) {
		bvr = nbpScanBootVolumes(biosdev, count);
		if (bvr != NULL) {
			gBootFileType = kNetworkDeviceType;
		}
	}  else {
		gBootFileType = kBlockDeviceType;
	}
}

//==========================================================================

void scanDisks(int biosdev, int *count)
{
	#define MAX_HDD_COUNT 32
	int bvCount;
	int hd = 0;

	// Testing up to MAX_HDD_COUNT hard drives.
	while(!testBiosread(0x80 + hd, 0) && hd < MAX_HDD_COUNT) {
		bvCount = 0;
		scanBootVolumes(0x80 + hd, &bvCount);
		hd++;
	}

	// Also scanning CD/DVD drive.
	if (biosDevIsCDROM(gBIOSDev)) {
		bvCount = 0;
		scanBootVolumes(gBIOSDev, &bvCount);
	}
}

//==========================================================================

BVRef selectBootVolume( BVRef chain )
{
	bool filteredChain = false;
	bool foundPrimary = false;
	BVRef bvr, bvr1 = 0, bvr2 = 0;
	
	if (chain->filtered) {
		filteredChain = true;
	}
	
	if (multiboot_partition_set) {
		for ( bvr = chain; bvr; bvr = bvr->next ) {
			if ( bvr->part_no == multiboot_partition && bvr->biosdev == gBIOSDev ) {
				return bvr;
			}
		}
	}

	/*
	 * Checking "Default Partition" key in system configuration - use format: hd(x,y), the volume UUID or label -
	 * to override the default selection.
	 * We accept only kBVFlagSystemVolume or kBVFlagForeignBoot volumes.
	 */
	char *val = XMLDecode(getStringForKey(kDefaultPartition, &bootInfo->chameleonConfig));
	if (val) {
		for ( bvr = chain; bvr; bvr = bvr->next ) {
			if (matchVolumeToString(bvr, val, false)) {
				free(val);
				return bvr;
			}
		}
		free(val);
	}

	/*
	 * Scannig the volume chain backwards and trying to find 
	 * a HFS+ volume with valid boot record signature.
	 * If not found any active partition then we will
	 * select this volume as the boot volume.
	 */
	for ( bvr = chain; bvr; bvr = bvr->next ) {
		if (multiboot_skip_partition_set) {
			if (bvr->part_no == multiboot_skip_partition) continue;
		}
		if ( bvr->flags & kBVFlagPrimary && bvr->biosdev == gBIOSDev ) {
			foundPrimary = true;
		}

		// zhell -- Undo a regression that was introduced from r491 to 492.
		// if gBIOSBootVolume is set already, no change is required
		if ( bvr->flags & (kBVFlagBootable|kBVFlagSystemVolume)
			&& gBIOSBootVolume
			&& (!filteredChain || (filteredChain && bvr->visible))
			&& bvr->biosdev == gBIOSDev ) {
			bvr2 = bvr;
		}

		// zhell -- if gBIOSBootVolume is NOT set, we use the "if" statement
		// from r491,
		if ( bvr->flags & kBVFlagBootable
			&& ! gBIOSBootVolume
			&& bvr->biosdev == gBIOSDev ) {
			bvr2 = bvr;
		}
	}

	/*
	 * Use the standrad method for selecting the boot volume.
	 */
	if (foundPrimary)
	{
		for ( bvr = chain; bvr; bvr = bvr->next )
		{
			if ( bvr->flags & kBVFlagNativeBoot && bvr->biosdev == gBIOSDev ) {
				bvr1 = bvr;
			}
			if ( bvr->flags & kBVFlagPrimary && bvr->biosdev == gBIOSDev ) {
				bvr2 = bvr;
			}
		}
	}
	
	bvr = bvr2 ? bvr2 :
	bvr1 ? bvr1 : chain;
	
	return bvr;
}

//==========================================================================

#define LP '('
#define RP ')'
int gBIOSDev;

/*!
    This is like boot2's gBootVolume except it is for the internal use of
    libsaio to track which volume an unqualified path should be relative to.
    This replaces bootInfo->kernDev as the carrier of this information.
 */
static BVRef gRootVolume;

void setRootVolume(BVRef volume)
{
	gRootVolume = volume;
	// Veto non-native FS.  Basically that means don't allow the root volume to
	// be set to a volume we can't read files from.
	if(gRootVolume != NULL && ((gRootVolume->flags & kBVFlagNativeBoot) == 0)) {
		gRootVolume = NULL;
	}
}

void setBootGlobals(BVRef chain)
{
	// Record default boot device.
	gBootVolume = selectBootVolume(chain);
  
	// turbo - Save the ORIGINAL boot volume too for loading our mkext
	if (!gBIOSBootVolume) {
		gBIOSBootVolume = gBootVolume;
	}

	setRootVolume(gBootVolume);
}

/*!
    Extracts the volume selector from the pathname, returns the selected
    BVRef, and sets *outPath to the remainder of the path.
    If the path did not include a volume selector then the current volume
    is used.  When called with a volume selector the current volume
    is changed to the selected volume unless the volume selector is
    that of a ramdisk.
 */
BVRef getBootVolumeRef( const char * path, const char ** outPath )
{
    const char * cp;
    BVRef bvr = gRootVolume;
    int          biosdev = gBIOSDev;

    // Search for left parenthesis in the path specification.

	for (cp = path; *cp; cp++) {
		if (*cp == LP || *cp == '/') {
			break;
		}
	}

	if (*cp != LP) { // no left paren found
		cp = path;
		// Path is using the implicit current device so if there is
		// no current device, then we must fail.
		if (gRootVolume == NULL) {
			return NULL;
		}
	} else if ((cp - path) == 2) { // found "xx("
		const struct devsw * dp;
		const char * xp = path;

		int i;
		int unit = -1;
		int part = -1;

		cp++;

		// Check the 2 character device name pointed by 'xp'.

		for (dp = devsw; dp->name; dp++) {
			if ((xp[0] == dp->name[0]) && (xp[1] == dp->name[1])) {
				break;	// Found matching entry.
			}
		}

		if (dp->name == NULL) {
			error("Unknown device '%c%c'\n", xp[0], xp[1]);
			return NULL;
		}
        
		// Extract the optional unit number from the specification.
		// hd(unit) or hd(unit, part).

		i = 0;

		while (*cp >= '0' && *cp <= '9') {
			i = i * 10 + *cp++ - '0';
			unit = i;
		}

        // Unit is no longer optional and never really was.
        // If the user failed to specify it then the unit number from the previous kernDev
        // would have been used which makes little sense anyway.
        // For example, if the user did fd()/foobar and the current root device was the
        // second hard disk (i.e. unit 1) then fd() would select the second floppy drive!
        if (unit == -1) {
			return NULL;
		}

		// Extract the optional partition number from the specification.

		if (*cp == ',') {
			part = atoi(++cp);
		}

		// If part is not specified part will be -1 whereas before it would have been
		// whatever the last partition was which makes about zero sense if the device
		// has been switched.

		// Skip past the right paren.

		for ( ; *cp && *cp != RP; cp++) /* LOOP */;

		if (*cp == RP) {
			cp++;
		}
        
		biosdev = dp->biosdev + unit;

		bvr = newBootVolumeRef(biosdev, part);

		if (bvr == NULL) {
			return NULL;
		}
	} else {
		// Bad device specifier, skip past the right paren.

		for (cp++; *cp && *cp != RP; cp++) /* LOOP */;
		if (*cp == RP) {
			cp++;
		}

		// If gRootVolume was NULL, then bvr will be NULL as well which
		// should be caught by the caller.
	}

	// Returns the file path following the device spec.
	// e.g. 'hd(1,b)mach_kernel' is reduced to 'mach_kernel'.

	*outPath = cp;

	return bvr;
}

//==========================================================================
// Function name is a misnomer as scanBootVolumes usually calls diskScanBootVolumes
// which caches the information.  So it's only allocated on the first run.
static BVRef newBootVolumeRef( int biosdev, int partno )
{
	BVRef bvr, bvr1, bvrChain;

	bvr = bvr1 = NULL;

	// Try resolving "rd" and "bt" devices first.
	if (biosdev == kPseudoBIOSDevRAMDisk) {
		if (gRAMDiskVolume) {
			bvr1 = gRAMDiskVolume;
		}
	} else if (biosdev == kPseudoBIOSDevBooter) {
		if (gRAMDiskVolume != NULL && gRAMDiskBTAliased) {
			bvr1 = gRAMDiskVolume;
		} else {
			bvr1 = gBIOSBootVolume;
		}
	} else {
		// Fetch the volume list from the device.

		scanBootVolumes( biosdev, NULL );
		bvrChain = getBVChainForBIOSDev(biosdev);

		// Look for a perfect match based on device and partition number.

		for ( bvr1 = NULL, bvr = bvrChain; bvr; bvr = bvr->next ) {
			if ( ( bvr->flags & kBVFlagNativeBoot ) == 0 ) {
				continue;
			}

			bvr1 = bvr;
			if ( bvr->part_no == partno ) {
				break;
			}
		}
	}

	return bvr ? bvr : bvr1;
}

//==========================================================================
// getDeviceDescription() - Extracts unit number and partition number
// from bvr structure into "dw(u,p)" format.
// Returns length of the out string
int getDeviceDescription(BVRef bvr, char *str)
{
	if(!str) {
		return 0;
	}
    
	*str = '\0';

	if (bvr) {
		const struct devsw *dp = devsw;
		while(dp->name && bvr->biosdev >= dp->biosdev) {
			dp++;
        	}
		dp--;
		if (dp->name) {
			return sprintf(str, "%s(%d,%d)", dp->name, bvr->biosdev - dp->biosdev, bvr->part_no);
		}
	}
	
	return 0;
}
