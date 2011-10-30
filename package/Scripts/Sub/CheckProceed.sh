#!/bin/bash

echo "==============================================="
echo "Check Proceed: Can the installation continue?"
echo "***********************************************"

# Checks the selected volume is present and the disk is partitioned
# Now also check for another existing Chameleon installation on the same disk.

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
	exit 1
#else
	#echo "DEBUG: Confirming target volume exists"
fi


# Does target volume use slices?
if [ "$targetDevice" = "$targetDevice#*disk*s" ]; then
	echo "*** ERROR Volume does not use slices. Exiting."
	"$scriptDir"InstallLog.sh "${installerVolume}" "FAIL: $targetVolume doesn't use slices."
	exit 1		
#else
	#echo "DEBUG: Confirming target device uses slices"
fi


# Add check for installing to a 'small' HFS device like a
# 1GB USB flash drive which won't have an EFI System Partition.
if [ "$targetVolume" = "/Volumes/EFI" ]; then
	# Take target device and check slice 1 matches partition named "EFI"
	stripped=$( echo ${targetDevice#/dev/} )
	if [ ! $(echo ${stripped#*disk*s}) = 1 ]; then
		stripped=$( echo ${stripped%s*})"s1"
	fi
	if [ ! $( diskutil list | grep ${stripped} | awk {'print $2'} ) = "EFI" ]; then
		#echo "DEBUG: *** The selected volume doesn't have an EFI System Partition. Exiting."
		"$scriptDir"InstallLog.sh "${installerVolume}" "FAIL: Selected disk does not have an EFI System Partition."
		exit 1
	fi
fi

exit 0