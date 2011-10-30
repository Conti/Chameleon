#!/bin/bash

#echo "==============================================="
#echo "InstallLog: Create/Append installation log"
#echo "**********************************************"

# Writes to the Chameleon_Installer_Log.txt file created
# by the preinstall script at the start of installation.

# Receives two parameters
# $1 = selected volume for location of the install log
# $2 = text to write to the installer log

if [ "$#" -eq 2 ]; then
	logLocation="$1"
	verboseText="$2"
	#echo "DEBUG: passed argument = ${logLocation}"
	#echo "DEBUG: passed argument = ${verboseText}"
else
	echo "InstallLog: Error - wrong number of values passed"
	exit 9
fi



logName="Chameleon_Installer_Log.txt"
logFile="${logLocation}"/$logName


if [ -f "${logFile}" ]; then

	# Append messages to the log as passed by other scripts.
	if [ "${verboseText}" = "Diskutil" ]; then
		diskutil list >>"${logFile}"
	echo "======================================================" >>"${logFile}"
	fi

	if [ "${verboseText}" = "LineBreak" ]; then
		echo "======================================================" >>"${logFile}"
	fi

	if [[ "${verboseText}" == *fdisk* ]]; then
		targetDiskRaw="${verboseText#fdisk *}"
		fdisk $targetDiskRaw >>"${logFile}"
		echo " " >>"${logFile}"
	fi

	if [ "${verboseText}" != "LineBreak" ] && [[ "${verboseText}" != *fdisk* ]] && [[ "${verboseText}" != "Diskutil" ]]; then
		echo "${verboseText}" >> "${logFile}"
	fi
fi

exit 0