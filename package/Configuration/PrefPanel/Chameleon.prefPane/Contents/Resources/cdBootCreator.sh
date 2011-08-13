#!/bin/sh

# Copyright 2009 org.darwinx86.app. All rights reserved.

# Directories
extra=/tmp/isodir
mydir=`dirname $0`
tempDir=/tmp
finaldir=~/Desktop
isodir=/tmp/newiso
extradir=/tmp/newiso/Extra
preDir=/tmp/newiso/Extra/Preboot/
prebootDir=/tmp/newiso/Extra/Preboot/Extra
backDir=~/Desktop/Lizard
bkpdir=~/Desktop/Lizard/Previous-dmg

echo "Starting script..."

#errors
if ([ ! -f $extra/Extensions.mkext ] && [ ! -d $extra/Extensions ]);then
	echo "- Error: no Extensions.mkext or Extensions folder. One of them is required"
	echo " ------------------------"
	exit
fi

if [ ! -f $extra/com.apple.Boot.plist ];then
   echo "- Error: no com.apple.boot.plist found. File required"
   echo " ------------------------"
   exit
fi
if [ ! -f $extra/cdboot ];then
   echo "- Error: no cdboot found. File required"
   echo " ------------------------"
   exit
fi

# Create a work directory
echo " - temps folders created"
echo " - Checking files"
#copy Extra files to temp directory
if [ -f $extra/cdboot ];then
   cp -R $extra/cdboot $isodir
fi

if [ -f $extra/dsdt.aml ];then
   cp -R $extra/dsdt.aml $prebootDir
fi

if [ -f $extra/DSDT.aml ];then
	cp -R $extra/DSDT.aml $prebootDir
fi

if [ -f $extra/NVIDIA.ROM ];then
	cp -R $extra/NVIDIA.ROM $prebootDir
fi

if [ -f $extra/smbios.plist ];then
   cp -R $extra/smbios.plist $prebootDir
fi

if [ -f $extra/Extensions.mkext ];then
   cp -R $extra/Extensions.mkext $prebootDir
fi

if [ -d $extra/Extensions ];then
   cp -Rp $extra/Extensions $prebootDir
fi

if [ -f $extra/com.apple.Boot.plist ];then
   	cp -R $extra/com.apple.Boot.plist $prebootDir
fi
if [ -f $tempDir/com.apple.Boot.plist ];then
    cp -R $tempDir/com.apple.Boot.plist $extradir
fi

echo "- Files copied in temp folder"

# ramdisk creator
hdiutil create -srcfolder $preDir/ -layout GPTSPUD -fs HFS+ -format UDRW -volname Preboot $extradir/Preboot.dmg
rm -R $preDir
echo " - ram disk created"

# boot cd creator
hdiutil makehybrid -o BootCD.iso $isodir/ -iso -hfs -joliet -eltorito-boot $isodir/cdboot -no-emul-boot -hfs-volume-name "Boot CD" -joliet-volume-name "Boot CD"
echo " - hybrid image created"

# Create output and backup directories
if [ -f $finaldir/BootCD.iso ];then
   if [ ! -d $bkpdir ];then
   mkdir $backDir
   mkdir $bkpdir
   echo " - backup folder created"
	fi
   mv -f $finaldir/BootCD.iso $bkpdir/BootCd-$(date +"%d-%y-%Hh%M").iso
    echo " - previous ISO moved into backup folder"
fi
mv BootCD.iso $finaldir
echo " - ISO moved on desktop"

# cleanup
echo " - Perform cleaning"
echo " - ISO created succefully"
echo " ------------------------"
exit 0