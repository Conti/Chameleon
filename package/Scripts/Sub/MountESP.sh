#!/bin/bash

echo "==============================================="
echo "MountESP: Mount the EFI system partition"
echo "***********************************************"

# Creates a mountpoint and mounts /Volumes/EFI of the
# supplied disk which would have been pre-checked as using a GPT

# Receives targetDisk: for example /dev/disk2.
# Receives installerVolume: Volume to write the installer log to.
# Receives efiPartitionExist: either 0 or 1
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 4 ]; then
	targetDisk="$1"
	installerVolume="$2"
	efiPartitionExist="$3"
	scriptDir="$4"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for installerVolume = $installerVolume"
	echo "DEBUG: passed argument for efiPartitionExist = $efiPartitionExist"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

# Check the first partition is actually type 'EFI'
# as we could be checking a USB flash drive <4GB
if [ ${efiPartitionExist} = 1 ]; then

	# Does the mountpoint exist?
	if [ ! -e "/Volumes/EFI" ]; then
		mkdir -p "/Volumes/EFI"
	fi

	# Mount '/Volumes/EFI' using the correct format type
	if [ "$( fstyp "${targetDisk}"s1 | grep hfs )" ]; then
		"$scriptDir"InstallLog.sh "${installerVolume}" "Mounting ${targetDisk}s1 as /Volumes/EFI."
		mount_hfs "${targetDisk}"s1 "/Volumes/EFI"
	fi
	if [ "$( fstyp "${targetDisk}"s1 | grep msdos )" ]; then
		"$scriptDir"InstallLog.sh "${installerVolume}" "Mounting ${targetDisk}s1 as /Volumes/EFI."
		mount_msdos -u 0 -g 0 "${targetDisk}"s1 "/Volumes/EFI"
	fi
else
	"$scriptDir"InstallLog.sh "${installerVolume}" "Target volume doesn't have an EFI system partition."
fi

exit 0