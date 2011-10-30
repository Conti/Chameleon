#!/bin/bash

echo "==============================================="
echo "Set Active Partition ONLY if Windows is not installed"
echo "*****************************************************"

# Sets partition active if Windows is not installed.

# Receives diskSigCheck: code is 1 for a Windows install, 0 for no Windows install
# Receives targetDiskRaw: for example, /dev/rdisk1
# Receives targetSlice: for example, 1
# Receives targetVolume: Volume to install to.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 5 ]; then
	diskSigCheck="$1"
	targetDiskRaw="$2"
	targetSlice="$3"
	targetVolume="$4"
	scriptDir="$5"

	echo "DEBUG: passed argument for diskSigCheck = $diskSigCheck"
	echo "DEBUG: passed argument for targetDiskRaw = $targetDiskRaw"
	echo "DEBUG: passed argument for targetSlice = $targetSlice"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

# Append fdisk output to the installer log
"$scriptDir"InstallLog.sh "${targetVolume}" "fdisk ${targetDiskRaw}"

if [ ${diskSigCheck} == "0" ]; then
	#Windows is not installed so let's change the active partition"

	partitionactive=$( fdisk -d ${targetDiskRaw} | grep -n "*" | awk -F: '{print $1}')
	if [ "${partitionactive}" ] && [ "${partitionactive}" = "${targetSlice}" ]; then
		"$scriptDir"InstallLog.sh "${targetVolume}" "${targetDiskRaw#/dev/r}, slice "${targetSlice}" is already set active. No need to change it."
	else
		"$scriptDir"InstallLog.sh "${targetVolume}" "Setting ${targetVolume} partition active."
		# BadAxe requires EFI partition to be flagged active.
		# but it doesn't' hurt to do it for any non-windows partition.

		fdisk -e ${targetDiskRaw} <<-MAKEACTIVE
		print
		flag ${targetSlice}
		write
		y
		quit
		MAKEACTIVE
	fi
else
	# TO DO
	# Add check to make sure that the active partition is actually the Windows partition
	# before printing next statement.
	#echo "DEBUG: Windows is installed so we let that remain the active partition"
	"$scriptDir"InstallLog.sh "${targetVolume}" "Windows is installed so that can remain the active partition"
fi

exit 0