#!/bin/sh

# getDevProp.sh
# smbios-cham
#
# Created by ronan & thomas on 12/08/09.
# Copyright 2009 org.darwinx86.app. All rights reserved.
# adapted for Chameleon control panel use by Rekursor
#
# Directories
cdir=`dirname $0`
tmpDir=/tmp/Chameleon
dmpdir=${tmpDir}/devprop

# Create a dump directory
if [[ ! -d $dmpdir ]];then
   mkdir -p $dmpdir 
fi
if [[ ! -d $tmpDir ]];then
   mkdir -p $tmpDir 
fi
# Dump Device properties
ioreg -lw0 -p IODeviceTree -n efi -r -x |grep device-properties | sed 's/.*<//;s/>.*//;' | cat > $dmpdir/chameleon-devprop.hex

$cdir/gfxutil -s -n -i hex -o xml $dmpdir/chameleon-devprop.hex $dmpdir/chameleon-devprop.plist


# Splash the result up !!
open $dmpdir/chameleon-devprop.plist
if [[ ! -d $dmpdir ]];then
   rm -r $dmpdir 
fi
if [[ ! -d $tmpDir ]];then
   rm -r $tmpDir 
fi

#end
#echo $?