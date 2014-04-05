#!/bin/bash

echo "==============================================="
echo "CheckDiskMicrocode: Any existing bootloaders?"
echo "*********************************************"

# Reads the GPTdiskProtectiveMBR and searches for an existing
# Windows bootloader and also for an existing Chameleon stage 0 loader
# which might be better changed depending on whether or not a Windows
# signature is found or not.
# The script then exits with the value 0 to indicate that Chameleon stage0
# loader can be written, or 1 to indicate not to write the stage0 loader.

# Receives targetDisk: for example, /dev/disk2.
# Receives diskSigCheck: 0 = Windows not installed / 1 = Windows installed.
# Receives targetVolume: Volume to install to.
# Receives scriptDir: The location of the main script dir.


if [ "$#" -eq 4 ]; then
	targetDisk="$1"
	diskSigCheck="$2"
	targetVolume="$3"
	scriptDir="$4"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for diskSigCheck = $diskSigCheck"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed - Exiting"
	exit 9
fi


# read the first 437 bytes of the MBR

mbr437=$( dd 2>/dev/null if="$targetDisk" count=1 | dd 2>/dev/null count=1 bs=437 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )
#mbr437md5=$( dd 2>/dev/null if="$targetDisk" count=1 | dd 2>/dev/null count=1 bs=437 | md5 )

if [ $( echo "${mbr437}" | awk -F0 '{print NF-1}' ) != 874 ]; then
	# There is already something on the MBR 

	# See if a Windows bootloader already exists
	# Check bytes 440-443 of the GPTdiskProtectiveMBR for a Windows Disk Signature
	windowsloader=$( dd 2>/dev/null if="$targetDisk" count=4 bs=1 | xxd | awk '{print $2$3}' )
	if [ "${windowsloader}" == "33c08ed0" ]  ; then
		#echo "DEBUG: Found existing Windows Boot Loader so will replace with Chameleon boot0md"
		"$scriptDir"InstallLog.sh "${targetVolume}" "Target disk has existing Windows boot loader - Will replace with boot0md (boot0workV2)."
	fi

	# See if a Chameleon stage0 boot file already exists

	# Note: The checks for Boot0, Boot0hfs, Boot0md and Boot0md (dmazar's Boot0workV2) assume the code stays the same.
	# if the code changes then the hex values 0b807c, 0a803c, ee7505 and 742b80 used for matching
	# need to be checked to see if they are the same or not.

	stage0type=$( dd 2>/dev/null if="$targetDisk" count=3 bs=1 skip=105 | xxd | awk '{print $2$3}' )
	if [ "${stage0type}" == "0b807c" ]; then
		#echo "DEBUG: Target has existing Chameleon stage 0 loader - Boot0hfs"
		"$scriptDir"InstallLog.sh "${targetVolume}" "Target disk already has existing Chameleon stage 0 loader - boot0hfs"

		# Script CheckDiskSignature.sh returned 0 if a Windows installation was NOT found
		if [ "$diskSigCheck" == "0" ]; then
			#echo "DEBUG: Found no existing Windows installation so will replace stage 0 loader with Boot0"
			"$scriptDir"InstallLog.sh "${targetVolume}" "Will replace boot0hfs with boot0 as Windows is not on target disk."
			exit 0
		fi
		exit 1
	fi

	if [ "${stage0type}" == "0a803c" ]; then
		#echo "DEBUG: Found existing Chameleon stage 0 loader - Boot0"
		"$scriptDir"InstallLog.sh "${targetVolume}" "Target disk already has existing Chameleon stage 0 loader - boot0"

		# Script CheckDiskSignature.sh returned 1 if a Windows installation was found
		if [ "$diskSigCheck" = "1" ]; then
			#echo "DEBUG: Found existing Windows installation so will replace stage 0 loader with boot0md"
			"$scriptDir"InstallLog.sh "${targetVolume}" "Will replace boot0 with boot0md (boot0workV2) as Windows is on target disk."
			exit 0
		fi
		exit 1
	fi

	if [ "${stage0type}" == "ee7505" ]; then
		#echo "DEBUG: Found existing Chameleon stage 0 loader - Boot0md"
		#echo "DEBUG: And will leave boot0md installed."
		"$scriptDir"InstallLog.sh "${targetVolume}" "Target disk already has existing Chameleon stage 0 loader - boot0md (v1)."
		exit 1
	fi
	
	if [ "${stage0type}" == "742b80" ]; then
		#echo "DEBUG: Found existing Chameleon stage 0 loader - Boot0workV2"
		#echo "DEBUG: And will leave Boot0workV2 installed."
		"$scriptDir"InstallLog.sh "${targetVolume}" "Target disk already has existing Chameleon stage 0 loader - boot0md (boot0workV2)."
		exit 1
	fi

	if [ "${stage0type}" != "0b807c" ] && [ "${stage0type}" != "0a803c" ] && [ "${stage0type}" != "ee7505" ] && [ "${stage0type}" != "742b80" ] && [ "${windowsloader}" != "33c08ed0" ]  ; then
		#echo "DEBUG: Something other than Chameleon or a Windows bootloader was found"
		test=$(echo "${mbr437}" | awk -F0 '{print NF-1}' )
		#echo "DEBUG: Disk microcode found: ${test} - Preserving."
		#echo "DEBUG: diskupdate is set to false"
		#echo "DEBUG: -----------------------------------------------"
		"$scriptDir"InstallLog.sh "${targetVolume}" "NOTE: Target has existing unrecognised bootcode in the MBR. Leaving as is."
		exit 1
	fi
#else
	#echo "DEBUG: The first 437 bytes of the MBR Disk Sector is blank - Updating"
	#"$scriptDir"InstallLog.sh "${targetVolume}" "Target has no bootcode in the MBR disk sector."
fi

echo "diskupdate is now set to true."
echo "-----------------------------------------------"
echo ""

exit 0
