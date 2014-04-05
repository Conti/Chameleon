/*
 * Supplemental ramdisk functions for the multiboot ramdisk driver.
 * Copyright 2009 Tamas Kosarszky. All rights reserved.
 *
 */

#include "boot.h"
#include "bootstruct.h"
#include "multiboot.h"
#include "ramdisk.h"

struct multiboot_info * gRAMDiskMI = NULL;

// gRAMDiskVolume holds the bvr for the mounted ramdisk image.
BVRef gRAMDiskVolume = NULL;
bool gRAMDiskBTAliased = false;
char gRAMDiskFile[512];

// Notify OS X that a ramdisk has been setup. XNU with attach this to /dev/md0
void md0Ramdisk()
{
	RAMDiskParam ramdiskPtr;
	char filename[512];
	const char* override_filename = 0;
	int fh = -1;
	int len;
	
	if(getValueForKey(kMD0Image, &override_filename, &len,  
				   &bootInfo->chameleonConfig))
	{
		// Use user specified md0 file
		snprintf(filename, sizeof(filename), "%s", override_filename);
		fh = open(filename, 0);
		
		if(fh < 0)
		{
			snprintf(filename, sizeof(filename), "rd(0,0)/Extra/%s", override_filename);
			fh = open(filename, 0);

			if(fh < 0)
			{
				snprintf(filename, sizeof(filename), "/Extra/%s", override_filename);
				fh = open(filename, 0);
			}
		}		
	}

	if(fh < 0)
	{
		sprintf(filename, "rd(0,0)/Extra/Postboot.img");
		fh = open(filename, 0);

		if(fh < 0)
		{
			sprintf(filename, "/Extra/Postboot.img");	// Check /Extra if not in rd(0,0)
			fh = open(filename, 0);
		}
	}		

	if (fh >= 0)
	{
		verbose("Enabling ramdisk %s\n", filename);

		ramdiskPtr.size  = file_size(fh);
		ramdiskPtr.base = AllocateKernelMemory(ramdiskPtr.size);

		if(ramdiskPtr.size && ramdiskPtr.base)
		{
			// Read new ramdisk image contents in kernel memory.
			if (read(fh, (char*) ramdiskPtr.base, ramdiskPtr.size) == ramdiskPtr.size)
			{
				AllocateMemoryRange("RAMDisk", ramdiskPtr.base, ramdiskPtr.size, kBootDriverTypeInvalid);
				Node* node = DT__FindNode("/chosen/memory-map", false);
				if(node != NULL)
				{
					DT__AddProperty(node, "RAMDisk", sizeof(RAMDiskParam),  (void*)&ramdiskPtr);		
				}
				else
				{
					verbose("Unable to notify Mac OS X of the ramdisk %s.\n", filename);
				}
			}
			else
			{
				verbose("Unable to read md0 image %s.\n", filename);
			}			
		}
		else
		{
			verbose("md0 image %s is empty.\n", filename);
		}

		close(fh);

	}
}

void umountRAMDisk()
{
	if (gRAMDiskMI != NULL)
	{
		// Release ramdisk BVRef and DiskBVMap.
		struct DiskBVMap *oldMap = diskResetBootVolumes(0x100);
		CacheReset();
		diskFreeMap(oldMap);

		// Free multiboot info and module structures.
		if ((void *)gRAMDiskMI->mi_mods_addr != NULL) free((void *)gRAMDiskMI->mi_mods_addr);
		if (gRAMDiskMI != NULL) free(gRAMDiskMI);

		// Reset multiboot structures.
		gMI = gRAMDiskMI = NULL;
		*gRAMDiskFile = '\0';

		// Release ramdisk driver hooks.
		p_get_ramdisk_info = NULL;
		p_ramdiskReadBytes = NULL;

		// Reset ramdisk bvr
		gRAMDiskVolume = NULL;
		printf("\nunmounting: done");
	}
}

int mountRAMDisk(const char * param)
{
	int fh = 0, ramDiskSize;
	int error = 0;

	// Get file handle for ramdisk file.
	fh = open(param, 0);
	if (fh != -1)
	{
		printf("\nreading ramdisk image: %s", param);

		ramDiskSize = file_size(fh);
		if (ramDiskSize > 0)
		{
			// Unmount previously mounted image if exists.
			umountRAMDisk();

			// Read new ramdisk image contents into PREBOOT_DATA area.
			if (read(fh, (char *)PREBOOT_DATA, ramDiskSize) != ramDiskSize) error = -1;
		}
		else error = -1;

		close(fh);
	}
	else error = -1;

	if (error == 0)
	{
		// Save filename in gRAMDiskFile to display information.
		strlcpy(gRAMDiskFile, param, sizeof(gRAMDiskFile));

		// Set gMI as well for the multiboot ramdisk driver hook.
		gMI = gRAMDiskMI = malloc(sizeof(multiboot_info));
		struct multiboot_module * ramdisk_module = malloc(sizeof(multiboot_module));

		// Fill in multiboot info and module structures.
		if (gRAMDiskMI != NULL && ramdisk_module != NULL)
		{
			gRAMDiskMI->mi_mods_count = 1;
			gRAMDiskMI->mi_mods_addr = (uint32_t)ramdisk_module;
			ramdisk_module->mm_mod_start = PREBOOT_DATA;
			ramdisk_module->mm_mod_end = PREBOOT_DATA + ramDiskSize;

			// Set ramdisk driver hooks.
			p_get_ramdisk_info = &multiboot_get_ramdisk_info;
			p_ramdiskReadBytes = &multibootRamdiskReadBytes;

			int partCount; // unused
			// Save bvr of the mounted image.
			gRAMDiskVolume = diskScanBootVolumes(0x100, &partCount);
			if(gRAMDiskVolume == NULL)
			{
				umountRAMDisk();
				printf("\nRamdisk contains no partitions.");
			}
			else
			{
				char dirSpec[128];

				// Reading ramdisk configuration.
				strlcpy(dirSpec, RAMDISKCONFIG_FILENAME, sizeof(dirSpec));

				if (loadConfigFile(dirSpec, &bootInfo->ramdiskConfig) == 0)
				{
					getBoolForKey("BTAlias", &gRAMDiskBTAliased, &bootInfo->ramdiskConfig);
				}
				else
				{
					printf("\nno ramdisk config...\n");
				}

				printf("\nmounting: done");
			}
		}
	}

	return error;
}

void setRAMDiskBTHook(bool mode)
{
	gRAMDiskBTAliased = mode;
	if (mode)
	{
		printf("\nEnabled bt(0,0) alias.");
	}
	else
	{
		printf("\nDisabled bt(0,0) alias.");
	}
}

void showInfoRAMDisk(void)
{
	int len;
	const char *val;

	if (gRAMDiskMI != NULL)
	{
		struct multiboot_module * ramdisk_module = (void *)gRAMDiskMI->mi_mods_addr;

		printf("\nfile: %s %d", gRAMDiskFile,
		ramdisk_module->mm_mod_end - ramdisk_module->mm_mod_start);
		printf("\nalias: %s", gRAMDiskBTAliased ? "enabled" : "disabled");

		// Display ramdisk information if available.
		if (getValueForKey("Info", &val, &len, &bootInfo->ramdiskConfig))
		{
			printf("\ninfo: %s", val);
		}
		else
		{
			printf("\nramdisk info not available.");
		}
	}
	else
	{
		printf("\nNo ramdisk mounted.");
	}
}

int loadPrebootRAMDisk()
{
	mountRAMDisk("bt(0,0)/Extra/Preboot.dmg");
	if (gRAMDiskMI != NULL)
	{
		printf("\n");
		return 0;
	}
	else
	{
		return -1;
	}
}

void processRAMDiskCommand(char ** argPtr, const char * cmd)
{
	char * ptr = *argPtr;
	char param[1024];
	getNextArg(&ptr, param);

	if (strcmp(cmd, "m") == 0)
	{
		mountRAMDisk(param);
		sleep(2);
	}
	else if (strcmp(cmd, "u") == 0)
	{
		umountRAMDisk();
		sleep(2);
	}
	else if (strcmp(cmd, "e") == 0)
	{
		setRAMDiskBTHook(true);
		sleep(2);
	}
	else if (strcmp(cmd, "d") == 0)
	{
		setRAMDiskBTHook(false);
		sleep(2);
	}
	else if (strcmp(cmd, "i") == 0)
	{
		setActiveDisplayPage(1);
		clearScreenRows(0, 24);
		setCursorPosition(0, 0, 1);
		showInfoRAMDisk();
		printf("\n\nPress any key to continue.\n");
		getchar();
		setActiveDisplayPage(0);
	}
	else
	{
		setActiveDisplayPage(1);
		clearScreenRows(0, 24);
		setCursorPosition(0, 0, 1);
		printf("\nusage:\n");
		printf("\n?rd i - display ramdisk information");
		printf("\n?rd m <filename> - mount ramdisk image\n?rd u - unmount ramdisk image");
		printf("\n?rd e - enable bt(0,0) alias\n?rd d - disable bt(0,0) alias");
		printf("\n\nPress any key to continue.\n");
		getchar();
		setActiveDisplayPage(0);
	}
}
