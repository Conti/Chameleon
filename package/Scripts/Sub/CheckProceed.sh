#!/bin/bash

echo "==============================================="
echo "Check Proceed: Can the installation continue?"
echo "***********************************************"

# Checks the selected volume is present and the disk is partitioned
# Now also check for another existing Chameleon installation on the same disk.
# Exit with 0 to indicate okay to proceed, no problems.
# Exit with 1 to indicate okay to proceed, but target disk doesn't have EFI system partition.
# Exit with 2 to indicate not to proceed.

# Receives targetVolume: Volume to install to (will be '/Volumes/EFI' if EFI install)
# Receives targetDevice: Stores device number, for example /dev/disk2s1.
# Receives installerVolume: Volume to write the installer log to.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 4 ]; then
	targetVolume="$1"
	targetDevice="$2"
	installerVolume="$3"
	scriptDir="$4"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
	echo "DEBUG: passed argument for installerVolume = $installerVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

# Does target volume exist?
if [ -z "$targetVolume" ]; then
	echo "*** Cannot find the volume. Exiting."
	"$scriptDir"InstallLog.sh "${installerVolume}" "FAIL: Cannot file the volume: $targetVolume."
	exit 2
fi

# Does target volume use slices?
if [ "$targetDevice" = "$targetDevice#*disk*s" ]; then
	echo "*** ERROR Volume does not use slices. Exiting."
	"$scriptDir"InstallLog.sh "${installerVolume}" "FAIL: $targetVolume doesn't use slices."
	exit 2		
fi

# Check to find if an EFI system partition exists on the disk.
# This is used in two cases:
# A) When checking for existing Chameleon installations.
# B) When the user chooses the EFI system partition install option,
#    and installing to a 'small' HFS device like a 1GB USB flash 
#    drive which won't have an EFI System Partition.

# Take target device and check if slice 1 is not named "EFI"
stripped=$( echo ${targetDevice#/dev/} )
if [ ! $(echo ${stripped#*disk*s}) = 1 ]; then
	stripped=$( echo ${stripped%s*})"s1"
fi
if [ ! $( diskutil list | grep ${stripped} | awk {'print $2'} ) = "EFI" ]; then
	if [ "$targetVolume" = "/Volumes/EFI" ]; then
		"$scriptDir"InstallLog.sh "${installerVolume}" "FAIL: Selected disk does not have an EFI System Partition."
	fi
	exit 1
fi

exit 0