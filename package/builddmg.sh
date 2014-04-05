#!/bin/bash

# $1 Path to store built dmg

pkgroot="${0%/*}"
SYM_ROOT="${1%/*}"
DMG_ROOT=$SYM_ROOT

OPENUP_TOOL=${1%/*}/i386/openUp

SRC_FOLDER=${1%/*}/source
VOLUME_NAME="Chameleon"
TEMP_NAME="rw.dmg"
DMG_TEMP_NAME=${DMG_ROOT}/${TEMP_NAME}
TEMPLATE_DMG=${pkgroot}/dmg/ro.dmg
EULA_RSRC=${pkgroot}/dmg/SLAResources


# =============================
# Setting color for text output 
# =============================

COL_BLACK="\x1b[30;01m"
COL_RED="\x1b[31;01m"
COL_GREEN="\x1b[32;01m"
COL_YELLOW="\x1b[33;01m"
COL_MAGENTA="\x1b[35;01m"
COL_CYAN="\x1b[36;01m"
COL_WHITE="\x1b[37;01m"
COL_BLUE="\x1b[34;01m"
COL_RESET="\x1b[39;49;00m"

# ======================
# Setting Chameleon info
# ======================

version=$( cat version )
stage=${version##*-}
revision=$( grep I386BOOT_CHAMELEONREVISION vers.h | awk '{ print $3 }' | tr -d '\"' )
builddate=$( grep I386BOOT_BUILDDATE vers.h | awk '{ print $3,$4 }' | tr -d '\"' )
timestamp=$( date -j -f "%Y-%m-%d %H:%M:%S" "${builddate}" "+%s" )
CHAMELEON_PACKAGE_NAME=${VOLUME_NAME}-${version}-r${revision}

# =========================
# Start of building process
# =========================

echo ""	
echo -e $COL_BLACK"	----------------------"$COL_RESET
echo -e $COL_BLACK"	Building $VOLUME_NAME DMG"$COL_RESET
echo -e $COL_BLACK"	----------------------"$COL_RESET
echo ""

# =================================
# 1) Clean previous builded contents
# =================================

	if [ -x ${SRC_FOLDER} ]; then
		echo "	Deleting previous existing source folder/content "
		rm -R ${SRC_FOLDER} 
		rm -f ${DMG_TEMP_NAME}
	fi

# ===========================
# 2) Create the source folder
# ===========================

	echo "	[mkdir] Creating source folder "
	mkdir "${SRC_FOLDER}"

# ==================================
# 3) Copy content into source folder
# ==================================

	ditto -xk "${pkgroot}/Icons/i386.zip" "${SRC_FOLDER}/"
	ditto -xk "${pkgroot}/Icons/doc.zip" "${SRC_FOLDER}/"
	ditto -xk "${pkgroot}/Icons/pan.zip" "${SRC_FOLDER}/"
	ditto -xk "${pkgroot}/Icons/tm.zip" "${SRC_FOLDER}/"
	
	#mv ${SYM_ROOT}/${VOLUME_NAME}.pkg ${SRC_FOLDER}/${VOLUME_NAME}.pkg
	cp -r ${SYM_ROOT}/${CHAMELEON_PACKAGE_NAME}.pkg ${SRC_FOLDER}/${CHAMELEON_PACKAGE_NAME}.pkg
	#cp -r ${pkgroot}/doc/* ${SRC_FOLDER}/Documentation/
	cp -r ${SYM_ROOT%/*}/doc/BootHelp.txt ${SRC_FOLDER}/Documentation/
	cp -r ${SYM_ROOT%/*}/doc/Users_Guide0.5.pdf ${SRC_FOLDER}/Documentation/
	cp -r ${pkgroot}/Configuration/PrefPanel/* ${SRC_FOLDER}/PrefPanel/
	cp -r ${SYM_ROOT}/i386/* ${SRC_FOLDER}/i386/
	cp -r ${SYM_ROOT%/*}/artwork/themes/* ${SRC_FOLDER}/Themes/
	#rm -rf ${SRC_FOLDER}`find . -type d -name .svn`
	
	# The above line caused problems with svn reporting changes to all
	# directories in the Chameleon source folder that exist before compiling
	# svn status would show the following:
	# ~       Chameleon.xcodeproj
	# ~       artwork
	# ~       i386
	# ~       package
	# ~       doc
	# I've changed the code to this for now to get round the problem.
	# Hopefully someone else can find out why it was happenening.
	svnFilesToRemove=($( find "${SRC_FOLDER}" -type d -name '.svn'))
	for (( i = 0 ; i < ${#svnFilesToRemove[@]} ; i++ ))
	do
		rm -rf ${svnFilesToRemove[$i]}
	done

# =======================================
# 4) Find the size of the folder contents
# =======================================

	FOLDER_SIZE=`/usr/bin/du -s "${SRC_FOLDER}" | sed s/[^0-9].*//`

# =====================================================
# 4) Allow for partition table and other overhead (10%)
# =====================================================

	IMAGE_SIZE=$(($FOLDER_SIZE * 110/100))

# ============================================
# 5) Minimum size for an HFS+ partition is 4Mb
# ============================================

	[ $IMAGE_SIZE -lt 19960 ] && IMAGE_SIZE=19960 # [ $IMAGE_SIZE -lt 8300 ] && IMAGE_SIZE=8300

# =================================================================
# 6) Make sure NEXT_ROOT is not set (if we're building with an SDK)
# =================================================================

	unset NEXT_ROOT
	echo "	Source folder size = $FOLDER_SIZE"
	echo "	DMG image size (+10%) = $IMAGE_SIZE"
	echo " "

# =======================================
# 7) Convert the DMG template into RW-DMG
# =======================================

	echo "	[hdutil] Creating disk image "
	test -f "${DMG_TEMP_NAME}" && rm -f "${DMG_TEMP_NAME}"

	hdiutil convert $TEMPLATE_DMG -format UDRW -o "${DMG_TEMP_NAME}" >/dev/null 2>&1
	hdiutil resize -limits "${DMG_TEMP_NAME}" >/dev/null 2>&1
	hdiutil resize -size 15m "${DMG_TEMP_NAME}" >/dev/null 2>&1
	echo " "

# ===========
# 8) Mount it
# ===========

	echo "	[hdutil] Mounting disk image "
	MOUNT_DIR=/Volumes/$VOLUME_NAME
	DEV_NAME=$(hdiutil attach -readwrite -noverify -noautoopen "${DMG_TEMP_NAME}" | egrep '^/dev/' | sed 1q | awk '{print $1}')  >/dev/null 2>&1

	echo "	Device name:	$DEV_NAME"
	echo "	Mount directory: $MOUNT_DIR"
	echo " "

# =====================================
# 9) Make sure it's not world writeable
# =====================================

	mv ${SRC_FOLDER}/${CHAMELEON_PACKAGE_NAME}.pkg ${MOUNT_DIR}/${VOLUME_NAME}.pkg
	cp -R ${SRC_FOLDER}/Documentation ${MOUNT_DIR}/
	cp -R ${SRC_FOLDER}/PrefPanel ${MOUNT_DIR}/
	cp -R ${SRC_FOLDER}/i386 ${MOUNT_DIR}/
	cp -R ${SRC_FOLDER}/Themes ${MOUNT_DIR}/

	echo "	[chmod] Fixing permission for \"${MOUNT_DIR}\""
	chmod -Rf go-w "${MOUNT_DIR}" || true
	chmod -f a-w "${MOUNT_DIR}"/.DS_Store || true
	echo "	Done fixing permissions."
	echo " "

# =============================================
# 10) Make the top window open itself on mount:
# =============================================

	echo "	[openUp] Setting auto open flag"
	if [ -x ${OPENUP_TOOL} ]; then
		echo "	Applying openUp..."
		${OPENUP_TOOL} "${MOUNT_DIR}" >/dev/null 2>&1
	fi
	echo " "

# ===========
# 11) Unmount
# ===========

	echo "	[hdutil] Unmounting disk image"
	hdiutil detach "${DEV_NAME}" >/dev/null 2>&1
	echo " "

# ==================
# 12) Compress image
# ==================

	test -f "${DMG_ROOT}/${VOLUME_NAME}.dmg" && rm -f "${DMG_ROOT}/${VOLUME_NAME}.dmg" 
	echo "	[hdutil] Compressing disk image"
	hdiutil convert ${DMG_TEMP_NAME} -format UDZO -imagekey zlib-level=9 -o ${DMG_ROOT}/${VOLUME_NAME} >/dev/null 2>&1
	rm -f "${DMG_TEMP_NAME}"
	echo " "

# =========================
# 13) Adding EULA resources
# =========================

if [ ! -z "${EULA_RSRC}" -a "${EULA_RSRC}" != "-null-" ]; then
	echo "	[ResMerger] Adding EULA resources"
    hdiutil unflatten ${DMG_ROOT}/${VOLUME_NAME}.dmg  >/dev/null 2>&1
    ResMerger -a ${EULA_RSRC} -o ${DMG_ROOT}/${VOLUME_NAME}.dmg
    hdiutil flatten ${DMG_ROOT}/${VOLUME_NAME}.dmg >/dev/null 2>&1
fi

# =======================
# 14) Adding Icon to .dmg
# =======================

	ditto -xk ${pkgroot}/Icons/dmg.zip "${pkgroot}/Icons"
	DeRez -only icns ${pkgroot}/Icons/Icons/dmg.icns > tempicns.rsrc
	Rez -append tempicns.rsrc -o ${DMG_ROOT}/${VOLUME_NAME}.dmg
	SetFile -a C ${DMG_ROOT}/${VOLUME_NAME}.dmg
	rm -f tempicns.rsrc
	rm -rf "${pkgroot}/Icons/Icons"
	rm -R ${SRC_FOLDER} 

# ===
# END
# ===

	echo "	===================="
	echo "	Finish $VOLUME_NAME.dmg"
	echo "	===================="
	echo ""
#-----

exit 0
