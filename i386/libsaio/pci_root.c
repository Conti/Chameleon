/*
 * Copyright 2009 netkas
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"

#ifndef DEBUG_PCIROOT
#define DEBUG_PCIROOT 1
#endif

#if DEBUG_PCIROOT
#define DBG(x...)  printf(x)
#else
#define DBG(x...)
#endif

static int rootuid = 10; //value means function wasnt ran yet

static unsigned int findrootuid(unsigned char * dsdt, int len)
{
	int i;
	for (i=0; i<64 && i<len-5; i++) //not far than 64 symbols from pci root 
	{
		if(dsdt[i] == '_' && dsdt[i+1] == 'U' && dsdt[i+2] == 'I' && dsdt[i+3] == 'D' && dsdt[i+5] == 0x08)
		{
			return dsdt[i+4];
		}
	}
	return 11;
}

static unsigned int findpciroot(unsigned char * dsdt,int len)
{
	int i;

	for (i=0; i<len-4; i++) {
		if(dsdt[i] == 'P' && dsdt[i+1] == 'C' && dsdt[i+2] == 'I' && (dsdt[i+3] == 0x08 || dsdt [i+4] == 0x08)) {
			return findrootuid(dsdt+i, len-i);
		}
	}
	return 10;
}

int getPciRootUID(void)
{
	void *new_dsdt;
	const char *val;
	int len,fsize;
	const char * dsdt_filename=NULL;
	extern int search_and_get_acpi_fd(const char *, const char **);

	if (rootuid < 10) return rootuid;
	rootuid = 0;	/* default uid = 0 */

	if (getValueForKey(kPCIRootUID, &val, &len, &bootInfo->bootConfig)) {
		if (isdigit(val[0])) rootuid = val[0] - '0';
		goto out;
	}
	/* Chameleon compatibility */
	else if (getValueForKey("PciRoot", &val, &len, &bootInfo->bootConfig)) {
		if (isdigit(val[0])) rootuid = val[0] - '0';
		goto out;
	}
	/* PCEFI compatibility */
	else if (getValueForKey("-pci0", &val, &len, &bootInfo->bootConfig)) {
		rootuid = 0;
		goto out;
	}
	else if (getValueForKey("-pci1", &val, &len, &bootInfo->bootConfig)) {
		rootuid = 1;
		goto out;
	}

	int fd = search_and_get_acpi_fd("DSDT.aml", &dsdt_filename);

	// Check booting partition
	if (fd<0)
	{	  
	  verbose("No DSDT found, using 0 as uid value.\n");
	  rootuid = 0;
	  return rootuid;
	}
	
	fsize = file_size(fd);

	if ((new_dsdt = malloc(fsize)) == NULL) {
		verbose("[ERROR] alloc DSDT memory failed\n");
		close (fd);
		goto out;
	}
	if (read (fd, new_dsdt, fsize) != fsize) {
		verbose("[ERROR] read %s failed\n", dsdt_filename);
		close (fd);
		goto out;
	}
	close (fd);

	rootuid = findpciroot(new_dsdt, fsize);
	free(new_dsdt);

	// make sure it really works: 
	if (rootuid == 11) rootuid=0; //usually when _UID isnt present, it means uid is zero
	else if (rootuid < 0 || rootuid > 9) 
	{
		printf("PciRoot uid value wasnt found, using 0, if you want it to be 1, use -PciRootUID flag");
		rootuid = 0;
	}
out:
	verbose("Using PCI-Root-UID value: %d\n", rootuid);
	return rootuid;
}
