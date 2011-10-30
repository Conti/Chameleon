#!/bin/bash

# $1 Path to store built package

packagesidentity="org.chameleon"
packagename="Chameleon"
pkgroot="${0%/*}"
chamTemp="usr/local/chamTemp"

COL_BLACK="\x1b[30;01m"
COL_RED="\x1b[31;01m"
COL_GREEN="\x1b[32;01m"
COL_YELLOW="\x1b[33;01m"
COL_MAGENTA="\x1b[35;01m"
COL_CYAN="\x1b[36;01m"
COL_WHITE="\x1b[37;01m"
COL_BLUE="\x1b[34;01m"
COL_RESET="\x1b[39;49;00m"

version=$( cat version )
stage=${version##*-}
revision=$( grep I386BOOT_CHAMELEONREVISION vers.h | awk '{ print $3 }' | tr -d '\"' )
builddate=$( grep I386BOOT_BUILDDATE vers.h | awk '{ print $3,$4 }' | tr -d '\"' )
timestamp=$( date -j -f "%Y-%m-%d %H:%M:%S" "${builddate}" "+%s" )

# =================

develop=$(awk "NR==6{print;exit}" ${pkgroot}/../CREDITS)
credits=$(awk "NR==10{print;exit}" ${pkgroot}/../CREDITS)
pkgdev=$(awk "NR==14{print;exit}" ${pkgroot}/../CREDITS)

# =================

distributioncount=0
xmlindent=0

indent[0]="\t"
indent[1]="\t\t"
indent[2]="\t\t\t"
indent[3]="\t\t\t\t"

main ()
{

# clean up the destination path

rm -R -f "${1}"
echo ""	
echo -e $COL_CYAN"	---------------------------------------"$COL_RESET
echo -e $COL_CYAN"	Building $packagename Slim Install Package"$COL_RESET
echo -e $COL_CYAN"	---------------------------------------"$COL_RESET
echo ""

outline[$((outlinecount++))]="${indent[$xmlindent]}<choices-outline>"

# build pre install package
	echo "================= Preinstall ================="
	((xmlindent++))
	packagesidentity="org.chameleon"
	mkdir -p ${1}/Pre/Root
	mkdir -p ${1}/Pre/Scripts
	ditto --noextattr --noqtn ${1%/*/*}/revision ${1}/Pre/Scripts/Resources/revision
	ditto --noextattr --noqtn ${1%/*/*}/version ${1}/Pre/Scripts/Resources/version
	cp -f ${pkgroot}/Scripts/Main/preinstall ${1}/Pre/Scripts
	cp -f ${pkgroot}/Scripts/Sub/InstallLog.sh ${1}/Pre/Scripts
	echo "	[BUILD] Pre "
	buildpackage "${1}/Pre" "/" "" "start_visible=\"false\" start_selected=\"true\"" >/dev/null 2>&1
# End build pre install package

# build core package
	echo "================= Core ================="
	packagesidentity="org.chameleon"
	mkdir -p ${1}/Core/Root/usr/local/bin
	mkdir -p ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot0 ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot0md ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot1f32 ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot1h ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot1he ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot1hp ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/cdboot ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/chain0 ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/fdisk440 ${1}/Core/Root/usr/local/bin
	ditto --noextattr --noqtn ${1%/*}/i386/bdmesg ${1}/Core/Root/usr/local/bin
	local coresize=$( du -hkc "${1}/Core/Root" | tail -n1 | awk {'print $1'} )
	echo "	[BUILD] i386 "
	buildpackage "${1}/Core" "/" "0" "start_visible=\"false\" start_selected=\"true\"" >/dev/null 2>&1
# End build core package

# build install type
	echo "================= Chameleon ================="
	outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"InstallType\">"
	choices[$((choicescount++))]="\t<choice\n\t\tid=\"InstallType\"\n\t\ttitle=\"InstallType_title\"\n\t\tdescription=\"InstallType_description\">\n\t</choice>\n"
	((xmlindent++))
	packagesidentity="org.chameleon.type"
	
	# build new install package 
		mkdir -p ${1}/New/Root
		echo "" > "${1}/New/Root/install_type_new"
		echo "	[BUILD] New "
        buildpackage "${1}/New" "/$chamTemp" "" "start_enabled=\"true\" selected=\"exclusive(choices['Upgrade'])\"" >/dev/null 2>&1
	# End build new install package 

	# build upgrade package 
		mkdir -p ${1}/Upgrade/Root
		echo "" > "${1}/Upgrade/Root/install_type_upgrade"
		echo "	[BUILD] Upgrade "
		buildpackage "${1}/Upgrade" "/$chamTemp" "" "start_selected=\"false\" selected=\"exclusive(choices['New'])\"" >/dev/null 2>&1
	# End build upgrade package

   ((xmlindent--))
   outline[$((outlinecount++))]="${indent[$xmlindent]}</line>"
# End build install type	

# build Chameleon package
	echo "================= Chameleon ================="
	outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"Chameleon\">"
	choices[$((choicescount++))]="\t<choice\n\t\tid=\"Chameleon\"\n\t\ttitle=\"Chameleon_title\"\n\t\tdescription=\"Chameleon_description\">\n\t</choice>\n"
	((xmlindent++))
	
	# build standard package 
		mkdir -p ${1}/Standard/Root
		mkdir -p ${1}/Standard/Scripts/Resources
		cp -f ${pkgroot}/Scripts/Main/Standardpostinstall ${1}/Standard/Scripts/postinstall
		cp -f ${pkgroot}/Scripts/Sub/* ${1}/Standard/Scripts
		ditto --arch i386 `which SetFile` ${1}/Standard/Scripts/Resources/SetFile
		ditto --noextattr --noqtn ${1%/*/*}/revision ${1}/Standard/Scripts/Resources/revision
		ditto --noextattr --noqtn ${1%/*/*}/version ${1}/Standard/Scripts/Resources/version
		echo "	[BUILD] Standard "
        buildpackage "${1}/Standard" "/" "${coresize}" "start_enabled=\"true\" selected=\"exclusive(choices['EFI']) &amp;&amp; exclusive(choices['noboot'])\"" >/dev/null 2>&1
	# End build standard package 

	# build efi package 
		mkdir -p ${1}/EFI/Root
		mkdir -p ${1}/EFI/Scripts/Resources
		cp -f ${pkgroot}/Scripts/Main/ESPpostinstall ${1}/EFI/Scripts/postinstall
		cp -f ${pkgroot}/Scripts/Sub/* ${1}/EFI/Scripts
		ditto --arch i386 `which SetFile` ${1}/EFI/Scripts/Resources/SetFile
		ditto --noextattr --noqtn ${1%/*/*}/revision ${1}/EFI/Scripts/Resources/revision
		ditto --noextattr --noqtn ${1%/*/*}/version ${1}/EFI/Scripts/Resources/version
		echo "	[BUILD] EFI "
		buildpackage "${1}/EFI" "/" "${coresize}" "start_visible=\"systemHasGPT()\" selected=\"exclusive(choices['Standard']) &amp;&amp; exclusive(choices['noboot'])\"" >/dev/null 2>&1
	# End build efi package

	# build reset choice package 
		mkdir -p ${1}/noboot/Root
		echo "	[BUILD] Reset choice "
		buildpackage "${1}/noboot" "/$chamTemp" "" "selected=\"exclusive(choices['Standard']) &amp;&amp; exclusive(choices['EFI'])\"" >/dev/null 2>&1
	# End build reset choice package 

    ((xmlindent--))
    outline[$((outlinecount++))]="${indent[$xmlindent]}</line>"
# End build Chameleon package

# build Modules package
	echo "================= Modules ================="
	###############################
	# Supported Modules           #
	###############################
	# klibc.dylib                 #
	# Resolution.dylib            #
	# uClibcxx.dylib              #
	# Keylayout.dylib             #
	###############################
	if [ "$(ls -A "${1%/*}/i386/modules")" ]; then
	{
		outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"Module\">"
		choices[$((choicescount++))]="\t<choice\n\t\tid=\"Module\"\n\t\ttitle=\"Module_title\"\n\t\tdescription=\"Module_description\">\n\t</choice>\n"
		((xmlindent++))
		packagesidentity="org.chameleon.modules"
# -
		if [ -e ${1%/*}/i386/modules/klibc.dylib ]; then
		{
			mkdir -p ${1}/klibc/Root
			ditto --noextattr --noqtn ${1%/*}/i386/modules/klibc.dylib ${1}/klibc/Root
			echo "	[BUILD] klibc "
			buildpackage "${1}/klibc" "/$chamTemp/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
		}
		fi
# -
		if [ -e ${1%/*}/i386/modules/Resolution.dylib ]; then
		{
			mkdir -p ${1}/AutoReso/Root
			ditto --noextattr --noqtn ${1%/*}/i386/modules/Resolution.dylib ${1}/AutoReso/Root
			echo "	[BUILD] Resolution "
			buildpackage "${1}/AutoReso" "/$chamTemp/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
		}
		fi
# -
		if [ -e ${1%/*}/i386/modules/uClibcxx.dylib ]; then
		{
			mkdir -p ${1}/uClibc/Root
			ditto --noextattr --noqtn ${1%/*}/i386/modules/uClibcxx.dylib ${1}/uClibc/Root
			ditto --noextattr --noqtn ${1%/*}/i386/modules/klibc.dylib ${1}/uClibc/Root
			echo "	[BUILD] uClibc++ "
			buildpackage "${1}/uClibc" "/$chamTemp/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
		}
		fi
# -
		if [ -e ${1%/*}/i386/modules/Keylayout.dylib ]; then
		{
			mkdir -p ${1}/Keylayout/Root
			ditto --noextattr --noqtn ${1%/*}/i386/modules/Keylayout.dylib ${1}/Keylayout/Root
			echo "	[BUILD] Keylayout "
			buildpackage "${1}/Keylayout" "/$chamTemp/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
		}
		fi

		((xmlindent--))
		outline[$((outlinecount++))]="${indent[$xmlindent]}</line>"
	}
	else
	{
		echo "      -= no modules to include =-"
	}
	fi
# End build Modules packages

# build post install package
	echo "================= Post ================="
	packagesidentity="org.chameleon"
	mkdir -p ${1}/Post/Root
	mkdir -p ${1}/Post/Scripts
	cp -f ${pkgroot}/Scripts/Main/postinstall ${1}/Post/Scripts
	cp -f ${pkgroot}/Scripts/Sub/InstallLog.sh ${1}/Post/Scripts
	cp -f ${pkgroot}/Scripts/Sub/UnMountEFIvolumes.sh ${1}/Post/Scripts
	ditto --noextattr --noqtn ${1%/*/*}/revision ${1}/Post/Scripts/Resources/revision
	ditto --noextattr --noqtn ${1%/*/*}/version ${1}/Post/Scripts/Resources/version
	echo "	[BUILD] Post "
	buildpackage "${1}/Post" "/" "" "start_visible=\"false\" start_selected=\"true\"" >/dev/null 2>&1
# End build post install package

#((xmlindent--))
outline[$((outlinecount++))]="${indent[$xmlindent]}</choices-outline>"

# build meta package

	makedistribution "${1}" "${2}" "${3}" "${4}" #"${5}"

# clean up 

	rm -R -f "${1}"

}

fixperms ()
{
	# $1 path
	find "${1}" -type f -exec chmod 644 {} \;
	find "${1}" -type d -exec chmod 755 {} \;
	chown -R 0:0 "${1}"
}

buildpackage ()
{
#  $1 Path to package to build containing Root and or Scripts
#  $2 Install Location
#  $3 Size
#  $4 Options

if [ -d "${1}/Root" ] && [ "${1}/Scripts" ]; then

	local packagename="${1##*/}"
	local identifier=$( echo ${packagesidentity}.${packagename//_/.} | tr [:upper:] [:lower:] )
	find "${1}" -name '.DS_Store' -delete
	local filecount=$( find "${1}/Root" | wc -l )
	if [ "${3}" ]; then
		local installedsize="${3}"
	else
		local installedsize=$( du -hkc "${1}/Root" | tail -n1 | awk {'print $1'} )
	fi
	local header="<?xml version=\"1.0\"?>\n<pkg-info format-version=\"2\" "

	#[ "${3}" == "relocatable" ] && header+="relocatable=\"true\" "		

	header+="identifier=\"${identifier}\" "
	header+="version=\"${version}\" "

	[ "${2}" != "relocatable" ] && header+="install-location=\"${2}\" "

	header+="auth=\"root\">\n"
	header+="\t<payload installKBytes=\"${installedsize##* }\" numberOfFiles=\"${filecount##* }\"/>\n"
	rm -R -f "${1}/Temp"

	[ -d "${1}/Temp" ] || mkdir -m 777 "${1}/Temp"
	[ -d "${1}/Root" ] && mkbom "${1}/Root" "${1}/Temp/Bom"

	if [ -d "${1}/Scripts" ]; then 
		header+="\t<scripts>\n"
		for script in $( find "${1}/Scripts" -type f \( -name 'pre*' -or -name 'post*' \) )
		do
			header+="\t\t<${script##*/} file=\"./${script##*/}\"/>\n"
		done
		header+="\t</scripts>\n"
		chown -R 0:0 "${1}/Scripts"
		pushd "${1}/Scripts" >/dev/null
		find . -print | cpio -o -z -H cpio > "../Temp/Scripts"
		popd >/dev/null
	fi

	header+="</pkg-info>"
	echo -e "${header}" > "${1}/Temp/PackageInfo"
	pushd "${1}/Root" >/dev/null
	find . -print | cpio -o -z -H cpio > "../Temp/Payload"
	popd >/dev/null
	pushd "${1}/Temp" >/dev/null

	xar -c -f "${1%/*}/${packagename// /}.pkg" --compression none .

	popd >/dev/null

	outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"${packagename// /}\"/>"

	if [ "${4}" ]; then
		local choiceoptions="\t\t${4}"
	fi
	choices[$((choicescount++))]="\t<choice\n\t\tid=\"${packagename// /}\"\n\t\ttitle=\"${packagename}_title\"\n\t\tdescription=\"${packagename}_description\"\n${choiceoptions}>\n\t\t<pkg-ref id=\"${identifier}\" installKBytes='${installedsize}' version='${version}.0.0.${timestamp}' >#${packagename// /}.pkg</pkg-ref>\n\t</choice>\n"	
	rm -R -f "${1}"
fi
}

makedistribution ()
{
	rm -f "${1%/*}/${packagename// /}"*.pkg

	find "${1}" -type f -name '*.pkg' -depth 1 | while read component
	do
		mkdir -p "${1}/${packagename}/${component##*/}"
		pushd "${1}/${packagename}/${component##*/}" >/dev/null
		xar -x -f "${1%}/${component##*/}"
		popd >/dev/null
	done

	ditto --noextattr --noqtn "${pkgroot}/Distribution" "${1}/${packagename}/Distribution"
	ditto --noextattr --noqtn "${pkgroot}/Resources" "${1}/${packagename}/Resources"

	find "${1}/${packagename}/Resources" -type d -name '.svn' -exec rm -R -f {} \; 2>/dev/null

	for (( i=0; i < ${#outline[*]} ; i++));
		do
			echo -e "${outline[$i]}" >> "${1}/${packagename}/Distribution"
		done

	for (( i=0; i < ${#choices[*]} ; i++));
		do
			echo -e "${choices[$i]}" >> "${1}/${packagename}/Distribution"
		done

	echo "</installer-gui-script>"  >> "${1}/${packagename}/Distribution"

	perl -i -p -e "s/%CHAMELEONVERSION%/${version%%-*}/g" `find "${1}/${packagename}/Resources" -type f`
	perl -i -p -e "s/%CHAMELEONREVISION%/${revision}/g" `find "${1}/${packagename}/Resources" -type f`

#  Adding Developer and credits
	perl -i -p -e "s/%DEVELOP%/${develop}/g" `find "${1}/${packagename}/Resources" -type f`
	perl -i -p -e "s/%CREDITS%/${credits}/g" `find "${1}/${packagename}/Resources" -type f`
	perl -i -p -e "s/%PKGDEV%/${pkgdev}/g" `find "${1}/${packagename}/Resources" -type f`

	stage=${stage/RC/Release Candidate }
	stage=${stage/FINAL/2.0 Final}
	perl -i -p -e "s/%CHAMELEONSTAGE%/${stage}/g" `find "${1}/${packagename}/Resources" -type f`

	find "${1}/${packagename}" -name '.DS_Store' -delete
	pushd "${1}/${packagename}" >/dev/null
	xar -c -f "${1%/*}/${packagename// /}-${version}-r${revision}.pkg" --compression none .
	popd >/dev/null

#   Here is the place for assign a Icon to the pkg
    ditto -xk "${pkgroot}/Icons/pkg.zip" "${pkgroot}/Icons/"
    DeRez -only icns "${pkgroot}/Icons/Icons/pkg.icns" > tempicns.rsrc
    Rez -append tempicns.rsrc -o "${1%/*}/$packagename-${version}-r$revision.pkg"
    SetFile -a C "${1%/*}/$packagename-${version}-r$revision.pkg"
    rm -f tempicns.rsrc
    rm -rf "${pkgroot}/Icons/Icons"
# End

	echo ""	

	echo -e $COL_GREEN"	--------------------------"$COL_RESET
	echo -e $COL_GREEN"	Building process complete!"$COL_RESET
	echo -e $COL_GREEN"	--------------------------"$COL_RESET
	echo ""	
	echo -e $COL_GREEN"	Build info."
	echo -e $COL_GREEN"	==========="
	echo -e $COL_BLUE"	Package name:	"$COL_RESET"$packagename-${version}-r$revision.pkg"
	echo -e $COL_BLUE"	MD5:		"$COL_RESET"$md5"
	echo -e $COL_BLUE"	Version:	"$COL_RESET"$version"
	echo -e $COL_BLUE"	Stage:		"$COL_RESET"$stage"
	echo -e $COL_BLUE"	Date/Time:	"$COL_RESET"$builddate"
	echo ""

}

main "${1}" "${2}" "${3}" "${4}" #"${5}"

