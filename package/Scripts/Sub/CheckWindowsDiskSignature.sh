#!/bin/bash

echo "==============================================="
echo "CheckWindowsDiskSignature: Is Windows installed?"
echo "************************************************"

# Checks the disk sector for a 4-byte Windows disk signature
# if one is found then it exits with 1, otherwise it exits with 0

# Receives targetdisk: for example, /dev/disk0
# Receives targetVolume: Volume to install to.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 3 ]; then
	targetDisk="$1"
	targetVolume="$2"
	scriptDir="$3"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed - Exiting"
	exit 9
fi

disksignature=$( dd 2>/dev/null if="$targetDisk" count=1 | dd 2>/dev/null count=4 bs=1 skip=440 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )

#echo "DEBUG: ${disksignature}"

if [ "${disksignature}" = "00000000" ]; then
	#echo "DEBUG: No Windows installation detected."
	exit 0
else
	#echo "DEBUG: Detected an existing Windows installation"
	"$scriptDir"InstallLog.sh "${targetVolume}" "Detected a Windows installation on this volume."
	exit 1
fi

exit 0