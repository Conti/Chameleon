#!/bin/bash

echo "==============================================="
echo "Write Chameleon Stage 2 Loader:"
echo "*******************************"

# Receives stage2Loader: Name of file - boot
# Receives selectedDestination: for example, /Volumes/ChameleonBootUSB (or /Volumes/EFI if ESP install).
# Receives targetDevice: for example, /dev/disk3s1
# Receives targetVolume: for example, /Volumes/ChameleonBootUSB
# Receives scriptDir: The location of the main script dir.


if [ "$#" -eq 5 ]; then
	stage2Loader="$1"
	selectedDestination="$2"
	targetDevice="$3"
	targetVolume="$4"
	scriptDir="$5"
	echo "DEBUG: passed argument for stage2Loader = $stage2Loader"
	echo "DEBUG: passed argument for selectedDestination = $selectedDestination"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

# check to see if install to EFI system partition was selected
if [ "${selectedDestination}" = "/Volumes/EFI" ]; then
	#echo "DEBUG: Executing command: cp "${targetVolume}"/usr/standalone/i386/${stage2Loader} ${selectedDestination}"
	cp "${targetVolume}"/usr/standalone/i386/"${stage2Loader}" "${selectedDestination}"
	"$scriptDir"InstallLog.sh "${targetVolume}" "Written boot to ${selectedDestination}."
else
	#echo "DEBUG: Executing command: cp "${targetVolume}"/usr/standalone/i386/${stage2Loader} ${targetVolume}"
	cp "${targetVolume}"/usr/standalone/i386/"${stage2Loader}" "${targetVolume}"
	"$scriptDir"InstallLog.sh "${targetVolume}" "Written boot to ${targetVolume} on ${targetDevice}."
fi

#ÊCheck to see if the user wants to hide the boot file
#if [ -f "${selectedDestination}"/.Chameleon/nullhideboot ]; then
#	echo "Executing command: SetFile -a V ${targetVolume}/${stage2Loader}"
#	"${selectedDestination}"/.Chameleon/Resources/SetFile -a V "${targetVolume}"/"${stage2Loader}"
#fi

exit 0