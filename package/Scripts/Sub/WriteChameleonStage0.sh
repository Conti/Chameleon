#!/bin/bash

echo "==============================================="
echo "Write Chameleon Stage 0 Loader:"
echo "*******************************"

# Writes Chameleon stage 0 loader.

# Receives disksignature: 0 = Windows not found, 1 = Windows Found
# Receives stage0Loader: for example, boot0
# Receives stage0Loaderdualboot: for example, boot0md
# Receives targetDisk: for example, /dev/disk3
# Receives targetResources: location of fdisk440
# Receives targetVolume: for example, /Volumes/USB
# Receives scriptDir: The location of the main script dir.


if [ "$#" -eq 7 ]; then
	disksignature="$1"
	stage0Loader="$2"
	stage0Loaderdualboot="$3"
	targetDisk="$4"
	targetResources="$5"
	targetVolume="$6"
	scriptDir="$7"
	echo "DEBUG: passed argument for disksignature = $disksignature"
	echo "DEBUG: passed argument for stage0Loader = $stage0Loader"
	echo "DEBUG: passed argument for stage0Loaderdualboot = $stage0Loaderdualboot"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for targetResources = $targetResources"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi


if [ ${disksignature} = "0" ]; then
	# There’s no Windows disk signature so we can write boot0
		
	#echo "DEBUG: Executing command: ${targetResources}fdisk440 -u -f /usr/standalone/i386/${stage0Loader} -y ${targetDisk}"
	"${targetResources}"fdisk440 -u -f "${targetVolume}"/usr/standalone/i386/${stage0Loader} -y ${targetDisk}
    "$scriptDir"InstallLog.sh "${targetVolume}" "Written ${stage0Loader} to ${targetDisk}."
else
	# Windows is also installed on the HDD so we need to write boot0md
		
	#echo "DEBUG: Executing command: ${targetResources}fdisk440 -u -f /usr/standalone/i386/${stage0Loaderdualboot} -y ${targetDisk}"
	"${targetResources}"fdisk440 -u -f "${targetVolume}"/usr/standalone/i386/${stage0Loaderdualboot} -y ${targetDisk}
    "$scriptDir"InstallLog.sh "${targetVolume}" "Written ${stage0Loaderdualboot} to ${targetDisk}."
fi

exit 0