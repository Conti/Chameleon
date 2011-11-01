#!/bin/bash

echo "==============================================="
echo "Check Previous Chameleon: Will there be problems?"
echo "***********************************************"

# Checks for another existing Chameleon installation on the same disk
# and tries to make sure the user doesn't end up with an un-bootable
# system due to having installed Chameleon previously elsewhere.

# Called from the Standard/postinstall and EFI/postinstall scripts
# /Volumes/EFI should already be mounted before this is called.

# Receives targetDisk: for example, /dev/disk3.
# Receives targetDeviceRaw: for example, /dev/rdisk3s1.
# Receives targetDevice: Stores device number, for example /dev/disk2s1.
# Receives installerVolume: Volume to write the installer log to.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 5 ]; then
	targetDisk="$1"
	targetDeviceRaw="$2"
	targetDevice="$3"
	installerVolume="$4"
	scriptDir="$5"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for targetDeviceRaw = $targetDeviceRaw"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
	echo "DEBUG: passed argument for installerVolume = $installerVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi


# ===============================================
# Prepare some vars
# ===============================================
sliceNumber=$( echo ${targetDeviceRaw#*disk*s} )

# strip slice from end
targetDiskRawNoSlice=$( echo ${targetDeviceRaw%$sliceNumber} )

# Are there any other partitions on the disk?
# How many actual partitions are there?
numSlices=$(( $( diskutil list | grep $( echo ${targetDisk#/dev/} ) | sed -n '$=' ) -2 ))


# ===============================================
# Checking the disk for existing Chameleon installations
# if there is more than one partition on the disk.
# ===============================================
if [ $numSlices -gt 1 ]; then 
	"$scriptDir"InstallLog.sh "${installerVolume}" "LineBreak"
	"$scriptDir"InstallLog.sh "${installerVolume}" "Checking for previous chameleon installations on ${targetDisk#/dev/}"

	# Check the disk's MBR for existing stage 0 boot code (code from CheckDiskMicrocode.sh script)
	stage0type=$( dd 2>/dev/null if="$targetDisk" count=3 bs=1 skip=105 | xxd | awk '{print $2$3}' )
	if [ "${stage0type}" == "0a803c" ] || [ "${stage0type}" == "ee7505" ] || [ "${stage0type}" == "742b80" ]; then
		stage0type=2
	elif [ "${stage0type}" == "0b807c" ]; then
		stage0type=1
	fi
	
	#Scan all partitions for Chameleon code
	for (( i=1; i <= $numSlices; i++ ));
	do
		if [ $stage0type == 1 ] || [ $stage0type == 2 ]; then
			stagesFound=1
		else
			stagesFound=0
		fi
		stage1Existence="NONE"
		stage2Existence=0
		targetDiskRaw=$targetDiskRawNoSlice$i
					
		# Check for existence of a bootable partition boot sector containing either boot1h or boot1f32
		boot1Search=$( dd 2>/dev/null if="$targetDiskRaw" count=1 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )
		if [ "${boot1Search:0:16}" == "fa31c08ed0bcf0ff" ] && [ "${boot1Search:1020:2}" == "55" ]; then
			(( stagesFound++ ))
			stage1Existence="boot1h"
		elif [ "${boot1Search:0:4}" == "e962" ] && [ "${boot1Search:180:12}" == "424f4f542020" ] && [ "${boot1Search:1020:2}" == "55" ]; then
			(( stagesFound++ ))
			stage1Existence="boot1f32"
		fi
		
		# Check for existing stage 2 boot file.
		# Include checking the EFI system partition if it exists and is mounted.
		if [ -e "$( df | grep ${targetDisk}s${i} | awk '{ print $6 }' )"/boot ]; then
			(( stagesFound++ ))
			stage2Existence=1
		fi
					
		if [ $stagesFound -ge 2 ] && [ "$stage1Existence" != "NONE" ] && [ $i -ne $sliceNumber ]; then
			# There is previous Chameleon stage 1 code on a partition boot sector,
			# and either a complete or incomplete installation (ie. boot0 or boot are missing).
			
			if [ $stagesFound == 3 ] && [ $i -lt $sliceNumber ]; then
				# Exisitng installation found which will still be default.
				message="************************** TAKE NOTE *****************************
**** There is an existing Chameleon installation on $targetDiskRaw
**** and that installation will still be the default loader as it's
**** on an earlier partition. If you want this new installation on
**** $installerVolume to be default then you will need to remove the
**** boot file from $targetDiskRaw and re-run this installer.
**************************************************************"
				"$scriptDir"InstallLog.sh "${installerVolume}" "${message}"
			fi
			if [ $stagesFound == 3 ] && [ $i -gt $sliceNumber ]; then
				# Exisitng installation found which will no longer be default.
				message="NOTE: There is an existing Chameleon installation on $targetDiskRaw
NOTE: but this installation on $targetDevice will be the default loader
NOTE: because you're installing to an earlier partition on the disk."
				"$scriptDir"InstallLog.sh "${installerVolume}" "${message}"
			fi
			
			
			# User could see a b1f:error or boot0:error if the following conditions are true:
			# A) Boot0hfs, Boot0md or Boot0md (dmazar's Boot0workV2) is being used.
			# B) The previous stage 1 code is on a lower partiton than the one being installed to now.
			# C) boot is missing from that partition.

			if [ $stagesFound == 2 ] && [ $stage2Existence == 0 ]; then
				# Exisitng boot0 and boot1 only found - missing boot
				"$scriptDir"InstallLog.sh "${installerVolume}" "INFO: Found $stage1Existence installed to ${targetDisk}s${i}"
							
				# stage0type=2 is used to know if 'A' is true.
				if [ $stage0type == 2 ]; then
					# i = current slice we're checking, slicenumber = slice trying to install to.
					if [ $i -lt $sliceNumber ]; then
						"$scriptDir"InstallLog.sh "${installerVolume}" "WARN: Conditions point to the possibility of a boot failure"

						# Fix by making previous parition bootsector un-bootable	
						message="---
FIX: Make ${targetDisk}s${i} boot sector un-bootable by changing byte 1FEh to 00.
NOTE: Any Extra folder you had there will still be there. If you want to use
NOTE: ${targetDisk}s${i} again as your boot partition then re-run this installer
NOTE: selecting it as the target, ONLY choosing the 'Chameleon Bootloader' option
NOTE: and NONE of the other options.
---"
						"$scriptDir"InstallLog.sh "${installerVolume}" "${message}"
						
						# /Volumes/EFI needs unmounting before changing partition boot sector
						if [ $i == 1 ]; then
							umount /Volumes/EFI
						else
							diskutil unmount "${targetDisk}"s${i}
						fi
												
						# Change Byte 01FExh to 00 (510 decimal)
						# Same code can be used for HFS or FAT32
						dd if=${targetDisk}s${i} count=1 bs=512 of=/tmp/originalBootSector
						cp /tmp/originalBootSector /tmp/newBootSector
						dd if="$scriptDir/patch" of=/tmp/newBootSector bs=1 count=1 seek=510 conv=notrunc
						dd if=/tmp/newBootSector of=${targetDisk}s${i} count=1 bs=512
						
						# /Volumes/EFI needs re-mounting so EFI/postinstall script can use it.
						# Don't check for a GPT as wouldn't have got here if it wasn't
						if [ $i == 1 ]; then
							"$scriptDir"MountESP.sh "${targetDisk}" "${installerVolume}" "${scriptDir}"
						else
							diskutil mount "${targetDisk}"s${i}
						fi
						
					else
						"$scriptDir"InstallLog.sh "${installerVolume}" "INFO: but won't interfere as you're installing to an earlier partition."
					fi
				elif [ $stage0type == 1 ]; then
					# boot0 was found which looks for boot1 on the first active partition.
					"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: so select to boot that partition (if used) with active flag."
				#else
					#echo "DEBUG: Boot0 not found"
				fi
			fi
		fi
		
	done
#else
	#echo "DEBUG: Just one slice"
fi

exit 0