#!/bin/bash

echo "==============================================="
echo "CheckFatType: Do we have FAT16 or FAT32?"
echo "****************************************"

# Looks for the following in the partition boot sector
# Byte number 19 to see if it's either 00 or 02
# Byte number 22 to see if it's either F8 or F0
# Byte number 25 to see if it's either 3F or 20
#
# Exit with value 1 for FAT16, 2 for FAT32 
# Exit with value 0 if nothing is found - this shouldn't happen.?

# Receives targetDeviceRaw: for example, /dev/rdisk0s2.
# Receives targetVolume: Volume to install to.
# Receives scriptDir: The location of the main script dir.


if [ "$#" -eq 3 ]; then
	targetDeviceRaw="$1"
	targetVolume="$2"
	scriptDir="$3"
	echo "DEBUG: passed argument = $targetDeviceRaw"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi


partitionBootSector=$( dd 2>/dev/null if="$targetDeviceRaw" count=1 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )
if [ "${partitionBootSector:36:2}" == "00" ] && [ "${partitionBootSector:42:2}" == "f8" ] && [ "${partitionBootSector:48:2}" == "3f" ]; then
	#echo "DEBUG: Found a FAT32 device formatted by Windows Explorer"
	"$scriptDir"InstallLog.sh "${targetVolume}" "${targetDeviceRaw} is on a FAT32 volume formatted by Windows Explorer"
	exit 2
fi
if [ "${partitionBootSector:36:2}" == "02" ] && [ "${partitionBootSector:42:2}" == "f8" ] && [ "${partitionBootSector:48:2}" == "3f" ]; then
	#echo "DEBUG: Found a FAT16 device formatted by Windows Explorer"
	"$scriptDir"InstallLog.sh "${targetVolume}" "${targetDeviceRaw} is on a FAT16 volume formatted by Windows Explorer"
	exit 1
fi
if [ "${partitionBootSector:36:2}" == "00" ] && [ "${partitionBootSector:42:2}" == "f0" ] && [ "${partitionBootSector:48:2}" == "20" ]; then
	#echo "DEBUG: Found a FAT32 device formatted by OS X Snow Leopard Disk Utility"
	"$scriptDir"InstallLog.sh "${targetVolume}" "${targetDeviceRaw} is on a FAT32 volume formatted by OS X Snow Leopard Disk Utility"
	exit 2
fi
if [ "${partitionBootSector:36:2}" == "02" ] && [ "${partitionBootSector:42:2}" == "f0" ] && [ "${partitionBootSector:48:2}" == "20" ]; then
	#echo "DEBUG: Found a FAT16 device formatted by OS X Snow Leopard Disk Utility"
	"$scriptDir"InstallLog.sh "${targetVolume}" "${targetDeviceRaw} is on a FAT16 volume formatted by OS X Snow Leopard Disk Utility"
	exit 1
fi
if [ "${partitionBootSector:36:2}" == "00" ] && [ "${partitionBootSector:42:2}" == "f8" ] && [ "${partitionBootSector:48:2}" == "20" ]; then
	#echo "DEBUG: Found a FAT32 device formatted by OS X Lion Disk Utility"
	"$scriptDir"InstallLog.sh "${targetVolume}" "${targetDeviceRaw} is on a FAT32 volume formatted by OS X Lion Disk Utility"
	exit 2
fi
if [ "${partitionBootSector:36:2}" == "02" ] && [ "${partitionBootSector:42:2}" == "f8" ] && [ "${partitionBootSector:48:2}" == "20" ]; then
	#echo "DEBUG: Found a FAT16 device formatted by OS X Lion Disk Utility"
	"$scriptDir"InstallLog.sh "${targetVolume}" "${targetDeviceRaw} is on a FAT16 volume formatted by OS X Lion Disk Utility"
	exit 1
fi

exit 0