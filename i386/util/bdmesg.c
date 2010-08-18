/*
 * BooterLog Dump Tool, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "IOKit/IOKitLib.h"


int main(int argc, char *argv[])
{
	io_registry_entry_t root = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
	if (!root)
	{
		printf("IORegistryEntry \"IOIOService:/\" not found.\n");
		return 0;
	}

	CFTypeRef bootLog = IORegistryEntryCreateCFProperty(root, CFSTR("boot-log"), kCFAllocatorDefault, 0);
	if (!bootLog)
	{
		printf("\"boot-log\" property not found.\n");
		return 0;
	}
	//CFShow(bootLog);
	const UInt8 *msglog = CFDataGetBytePtr((CFDataRef)bootLog);
	if (msglog)
		printf("%s\n", msglog);

	return 0;
}

