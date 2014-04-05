/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 2.0 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  load.c - Functions for decoding a Mach-o Kernel.
 *
 *  Copyright (c) 1998-2003 Apple Computer, Inc.
 *
 */

#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach/machine/thread_status.h>

#include <sl.h>

static long DecodeSegment(long cmdBase, unsigned int*load_addr, unsigned int *load_size);
static long DecodeUnixThread(long cmdBase, unsigned int *entry);
static long DecodeSymbolTable(long cmdBase);


static unsigned long gBinaryAddress;
bool   gHaveKernelCache;			/* XXX aserebln: uninitialized? and only set to true, never to false */
cpu_type_t archCpuType=CPU_TYPE_I386;


//==============================================================================
// Public function.

long ThinFatFile(void **binary, unsigned long *length)
{
	unsigned long nfat, swapped, size = 0;
	struct fat_header *fhp = (struct fat_header *)*binary;
	struct fat_arch   *fap = (struct fat_arch *)((unsigned long)*binary + sizeof(struct fat_header));
	cpu_type_t fapcputype;
	uint32_t fapoffset;
	uint32_t fapsize;	
  
	if (fhp->magic == FAT_MAGIC) {
		nfat = fhp->nfat_arch;
		swapped = 0;
	} else if (fhp->magic == FAT_CIGAM) {
		nfat = OSSwapInt32(fhp->nfat_arch);
		swapped = 1;
	} else {
		return -1;
	}

	for (; nfat > 0; nfat--, fap++) {
		if (swapped) {
			fapcputype = OSSwapInt32(fap->cputype);
			fapoffset = OSSwapInt32(fap->offset);
			fapsize = OSSwapInt32(fap->size);
		} else {
			fapcputype = fap->cputype;
			fapoffset = fap->offset;
			fapsize = fap->size;
		}

		if (fapcputype == archCpuType) {
			*binary = (void *) ((unsigned long)*binary + fapoffset);
			size = fapsize;
			break;
		}
	}

	if (length != 0) {
		*length = size;
	}

	return 0;
}


//==============================================================================

long DecodeMachO(void *binary, entry_t *rentry, char **raddr, int *rsize)
{
	struct mach_header *mH;
	unsigned long  ncmds, cmdBase, cmd, cmdsize, cmdstart;
	//  long   headerBase, headerAddr, headerSize;
	unsigned int vmaddr = ~0;
	unsigned int vmend = 0;
	unsigned long  cnt;
	long  ret = -1;
	unsigned int entry = 0;

	gBinaryAddress = (unsigned long)binary;

	mH = (struct mach_header *)(gBinaryAddress);

#if DEBUG
	printf("magic:      %x\n", (unsigned)mH->magic);
	printf("cputype:    %x\n", (unsigned)mH->cputype);
	printf("cpusubtype: %x\n", (unsigned)mH->cpusubtype);
	printf("filetype:   %x\n", (unsigned)mH->filetype);
	printf("ncmds:      %x\n", (unsigned)mH->ncmds);
	printf("sizeofcmds: %x\n", (unsigned)mH->sizeofcmds);
	printf("flags:      %x\n", (unsigned)mH->flags);
	getchar();
#endif

	switch (archCpuType)
	{
		case CPU_TYPE_I386:

			if (mH->magic != MH_MAGIC) {
				error("Mach-O file has bad magic number\n");
				return -1;
			}

			cmdstart = (unsigned long)gBinaryAddress + sizeof(struct mach_header);
			break;

		case CPU_TYPE_X86_64:

			if (mH->magic != MH_MAGIC_64 && mH->magic == MH_MAGIC) {
				return -1;
			}

			if (mH->magic != MH_MAGIC_64) {
				error("Mach-O file has bad magic number\n");
				return -1;
			}

			cmdstart = (unsigned long)gBinaryAddress + sizeof(struct mach_header_64);
			break;

		default:

			error("Unknown CPU type\n");
			return -1;
	}

	cmdBase = cmdstart;
	ncmds = mH->ncmds;

	for (cnt = 0; cnt < ncmds; cnt++)
	{
		cmd = ((long *)cmdBase)[0];
		cmdsize = ((long *)cmdBase)[1];
		unsigned int load_addr;
		unsigned int load_size;

		switch (cmd) {
			case LC_SEGMENT_64:
			case LC_SEGMENT:
			ret = DecodeSegment(cmdBase, &load_addr, &load_size);

			if (ret == 0 && load_size != 0 && load_addr >= KERNEL_ADDR)
			{
				vmaddr = MIN(vmaddr, load_addr);
				vmend = MAX(vmend, load_addr + load_size);
			}
			break;

			case LC_UNIXTHREAD:
				ret = DecodeUnixThread(cmdBase, &entry);
			break;

			case LC_SYMTAB:
			break;

			default:
#if NOTDEF
			printf("Ignoring cmd type %d.\n", (unsigned)cmd);
#endif
			break;
		}


	if (ret != 0) {
		return -1;
	}

	cmdBase += cmdsize;
	}

	*rentry = (entry_t)( (unsigned long) entry & 0x3fffffff );
	*rsize = vmend - vmaddr;
	*raddr = (char *)vmaddr;

	cmdBase = cmdstart;

	for (cnt = 0; cnt < ncmds; cnt++) {
	cmd = ((long *)cmdBase)[0];
	cmdsize = ((long *)cmdBase)[1];
		
	if (cmd == LC_SYMTAB) {
		if (DecodeSymbolTable(cmdBase) != 0) {
			return -1;
		}
        }

		cmdBase += cmdsize;
    }
	
	return ret;
}


//==============================================================================
// Private function.


static long DecodeSegment(long cmdBase, unsigned int *load_addr, unsigned int *load_size)
{
	char *segname;
	long   vmsize, filesize;
	unsigned long vmaddr, fileaddr;

	if (((long *)cmdBase)[0] == LC_SEGMENT_64) {
		struct segment_command_64 *segCmd;
		segCmd = (struct segment_command_64 *)cmdBase;
		vmaddr = (segCmd->vmaddr & 0x3fffffff);
		vmsize = segCmd->vmsize;	  
		fileaddr = (gBinaryAddress + segCmd->fileoff);
		filesize = segCmd->filesize;
		segname = segCmd->segname;

#ifdef DEBUG
  printf("segname: %s, vmaddr: %x, vmsize: %x, fileoff: %x, filesize: %x, nsects: %d, flags: %x.\n",
	 segCmd->segname, (unsigned)vmaddr, (unsigned)vmsize, (unsigned)fileaddr, (unsigned)filesize,
         (unsigned) segCmd->nsects, (unsigned)segCmd->flags);
  getchar();
#endif
	} else {
		struct segment_command *segCmd;

		segCmd = (struct segment_command *)cmdBase;

		vmaddr = (segCmd->vmaddr & 0x3fffffff);
		vmsize = segCmd->vmsize;
		fileaddr = (gBinaryAddress + segCmd->fileoff);
		filesize = segCmd->filesize;
		segname = segCmd->segname;

#ifdef DEBUG
	printf("segname: %s, vmaddr: %x, vmsize: %x, fileoff: %x, filesize: %x, nsects: %d, flags: %x.\n",
	segCmd->segname, (unsigned)vmaddr, (unsigned)vmsize, (unsigned)fileaddr, (unsigned)filesize, (unsigned) segCmd->nsects, (unsigned)segCmd->flags);
	getchar();
#endif
	}

	if (vmsize == 0 || filesize == 0) {
		*load_addr = ~0;
		*load_size = 0;
		return 0;
	}

		if (! ((vmaddr >= KERNEL_ADDR && (vmaddr + vmsize) <= (KERNEL_ADDR + KERNEL_LEN)) ||
			 (vmaddr >= HIB_ADDR && (vmaddr + vmsize) <= (HIB_ADDR + HIB_LEN)))) {
			stop("Kernel overflows available space");
		}

		if (vmsize && ((strcmp(segname, "__PRELINK_INFO") == 0) || (strcmp(segname, "__PRELINK") == 0))) {
			gHaveKernelCache = true;
		}

	// Copy from file load area.
	if (vmsize>0 && filesize > 0) {
		bcopy((char *)fileaddr, (char *)vmaddr, vmsize > filesize ? filesize : vmsize);
	}

	// Zero space at the end of the segment.
	if (vmsize > filesize) {
		bzero((char *)(vmaddr + filesize), vmsize - filesize);
	}

	*load_addr = vmaddr;
	*load_size = vmsize;

	return 0;
}

//==============================================================================

static long DecodeUnixThread(long cmdBase, unsigned int *entry)
{
	switch (archCpuType) {
		case CPU_TYPE_I386:
		{
			i386_thread_state_t *i386ThreadState;
			i386ThreadState = (i386_thread_state_t *) (cmdBase + sizeof(struct thread_command) + 8);

			*entry = i386ThreadState->eip;
			return 0;
		}
			
		case CPU_TYPE_X86_64:
		{
			x86_thread_state64_t *x86_64ThreadState;
			x86_64ThreadState = (x86_thread_state64_t *) (cmdBase + sizeof(struct thread_command) + 8);
			*entry = x86_64ThreadState->rip;
			return 0;
		}
			
		default:
			error("Unknown CPU type\n");
			return -1;
	}
}


//==============================================================================

static long DecodeSymbolTable(long cmdBase)
{
	long tmpAddr, symsSize, totalSize;
	long gSymbolTableAddr;
	long gSymbolTableSize;

	struct symtab_command *symTab, *symTableSave;

	symTab = (struct symtab_command *)cmdBase;

#if DEBUG

	printf("symoff: %x, nsyms: %x, stroff: %x, strsize: %x\n", symTab->symoff, symTab->nsyms, symTab->stroff, symTab->strsize);
	getchar();
#endif

	symsSize = symTab->stroff - symTab->symoff;
	totalSize = symsSize + symTab->strsize;

	gSymbolTableSize = totalSize + sizeof(struct symtab_command);
	gSymbolTableAddr = AllocateKernelMemory(gSymbolTableSize);
	// Add the SymTab to the memory-map.
	AllocateMemoryRange("Kernel-__SYMTAB", gSymbolTableAddr, gSymbolTableSize, -1);

	symTableSave = (struct symtab_command *)gSymbolTableAddr;
	tmpAddr = gSymbolTableAddr + sizeof(struct symtab_command);

	symTableSave->symoff = tmpAddr;
	symTableSave->nsyms = symTab->nsyms;
	symTableSave->stroff = tmpAddr + symsSize;
	symTableSave->strsize = symTab->strsize;
	
	bcopy((char *)(gBinaryAddress + symTab->symoff), (char *)tmpAddr, totalSize);

	return 0;
}
