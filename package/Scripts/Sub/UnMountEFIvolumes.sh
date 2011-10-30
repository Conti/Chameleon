#!/bin/bash

echo "==============================================="
echo "Unmount all volumes named EFI"
echo "*****************************"

# loop through and un-mount ALL mounted 'EFI' system partitions - Thanks kizwan

# Receives targetVolumeChosenByUser: To write install log to.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 2 ]; then
	targetVolumeChosenByUser="$1"
	scriptDir="$2"
	echo "DEBUG: passed argument for targetVolumeChosenByUser = $targetVolumeChosenByUser"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

# Count of 5 exists incase for some reason /Volumes/EFI fails
# be unmounted in which case the loop would run forever.
attempts=1
while [ "$( df | grep EFI )" ] && [ $attempts -lt 5 ]; do
	"$scriptDir"InstallLog.sh "${targetVolumeChosenByUser}" "Volume named 'EFI' is mounted..."
	"$scriptDir"InstallLog.sh "${targetVolumeChosenByUser}" "Unmounting $( df | grep EFI | awk '{print $1}' )"
	umount -f $( df | grep EFI | awk '{print $1}' )
	(( attempts++ ))
done
if [ $attempts = 5 ]; then
	"$scriptDir"InstallLog.sh "${targetVolumeChosenByUser}" "Failed to unmount 'EFI' System Partition."
	exit 1
fi

exit 0



