#!/bin/bash

# $1 Path to store built package

packagesidentity="org.chameleon"

packagename="Chameleon"

pkgroot="${0%/*}"

COL_BLACK="\x1b[30;01m"
COL_RED="\x1b[31;01m"
COL_GREEN="\x1b[32;01m"
COL_YELLOW="\x1b[33;01m"
COL_MAGENTA="\x1b[35;01m"
COL_CYAN="\x1b[36;01m"
COL_WHITE="\x1b[37;01m"
COL_BLUE="\x1b[34;01m"
COL_RESET="\x1b[39;49;00m"

#version=$( grep I386BOOT_CHAMELEONVERSION sym/i386/vers.h | awk '{ print $3 }' | tr -d '\"' )
version=$( cat version )
stage=${version##*-}
revision=$( grep I386BOOT_CHAMELEONREVISION sym/i386/vers.h | awk '{ print $3 }' | tr -d '\"' )
builddate=$( grep I386BOOT_BUILDDATE sym/i386/vers.h | awk '{ print $3,$4 }' | tr -d '\"' )
timestamp=$( date -j -f "%Y-%m-%d %H:%M:%S" "${builddate}" "+%s" )

# =================

develop=" Crazor, Dense, fassl, fxtentacle, iNDi, JrCs, Kabyl, kaitek, mackerintel, mercurysquad, munky, Slice, meklort, mozodojo, rekursor, Turbo, cparm, valv & zef "

credits=" andyvand, asereBLN, Azimut, bumby, cosmo1t, dfe, Galaxy, kalyway, Krazubu, MasterChief, netkas, sckevyn, smith@@, THeKiNG, DutchHockeyPro & Andy"

pkgdev=" blackosx, ErmaC , scrax"

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
echo -e $COL_BLACK"	---------------------"$COL_RESET
echo -e $COL_BLACK"	Building Slim Package"$COL_RESET
echo -e $COL_BLACK"	---------------------"$COL_RESET
echo ""

outline[$((outlinecount++))]="${indent[$xmlindent]}<choices-outline>"

# build core package
	echo "================= Core ================="
	((xmlindent++))
	packagesidentity="org.chameleon.core"
	mkdir -p ${1}/Core/Root/usr/sbin
	mkdir -p ${1}/Core/Root/usr/local/bin
	mkdir -p ${1}/Core/Root/usr/standalone/i386
#    if [ "$(ls -A "${1%/*}/i386/modules")" ]; then
#        echo "Modules found."
#        mkdir -p ${1}/Core/Root/usr/standalone/i386/modules
#        cp -R ${1%/*}/i386/modules ${1}/Core/Root/usr/standalone/i386
#    else
#        echo "No found modules into dir module"
#    fi
	ditto --noextattr --noqtn ${1%/*}/i386/boot ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot0 ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot0md ${1}/Core/Root/usr/standalone/i386
#	ditto --noextattr --noqtn ${1%/*}/i386/boot0hf ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot1f32 ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot1h ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot1he ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/boot1hp ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/cdboot ${1}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${1%/*}/i386/chain0 ${1}/Core/Root/usr/standalone/i386
# fixperms "${1}/Core/Root/"
	ditto --noextattr --noqtn ${1%/*}/i386/fdisk440 ${1}/Core/Root/usr/sbin
	ditto --noextattr --noqtn ${1%/*}/i386/bdmesg ${1}/Core/Root/usr/sbin
	local coresize=$( du -hkc "${1}/Core/Root" | tail -n1 | awk {'print $1'} )
	echo "	[BUILD] i386 "
	buildpackage "${1}/Core" "/" "0" "start_visible=\"false\" start_selected=\"true\"" >/dev/null 2>&1

# build Chameleon package
	echo "================= Chameleon ================="
	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Chameleon\">"
	choices[$((choicescount++))]="<choice\n\tid=\"Chameleon\"\n\ttitle=\"Chameleon_title\"\n\tdescription=\"Chameleon_description\"\n>\n</choice>\n"

	# build standard package 
		mkdir -p ${1}/Standard/Root
		mkdir -p ${1}/Standard/Scripts/Tools
		cp -f ${pkgroot}/Scripts/Standard/* ${1}/Standard/Scripts
		# ditto --arch i386 `which SetFile` ${1}/Standard/Scripts/Tools/SetFile
		echo "	[BUILD] Standard "
		buildpackage "${1}/Standard" "/" "${coresize}" "start_enabled=\"true\" start_selected=\"upgrade_allowed()\" selected=\"exclusive(choices['EFI']) &amp;&amp; exclusive(choices['noboot'])\"" >/dev/null 2>&1
	# End build standard package 

	# build efi package 
		mkdir -p ${1}/EFI/Root
		mkdir -p ${1}/EFI/Scripts/Tools
		cp -f ${pkgroot}/Scripts/EFI/* ${1}/EFI/Scripts
		# ditto --arch i386 `which SetFile` ${1}/EFI/Scripts/Tools/SetFile
		echo "	[BUILD] EFI "
		buildpackage "${1}/EFI" "/" "${coresize}" "start_visible=\"systemHasGPT()\" start_selected=\"false\" selected=\"exclusive(choices['Standard']) &amp;&amp; exclusive(choices['noboot'])\"" >/dev/null 2>&1
	# End build efi package

	# build reset choice package 
		mkdir -p ${1}/noboot/Root
		echo "	[BUILD] Reset choice "
		buildpackage "${1}/noboot" "/tmpcham" "" "start_visible=\"true\" start_selected=\"false\" selected=\"exclusive(choices['Standard']) &amp;&amp; exclusive(choices['EFI'])\"" >/dev/null 2>&1
	# End build reset choice package 

	# build Modules package
        echo "================= Modules ================="
                ###############################
                # AMDGraphicsEnabler.dylib    #
                # ATiGraphicsEnabler.dylib    #
                # IntelGraphicsEnabler.dylib  #
                # klibc.dylib                 #
                # NVIDIAGraphicsEnabler.dylib #
                # Resolution.dylib            #
                # uClibcxx.dylib              #
                ###############################
        if [ "$(ls -A "${1%/*}/i386/modules")" ]; then
        {
            outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Module\">"
            choices[$((choicescount++))]="<choice\n\tid=\"Module\"\n\ttitle=\"Module_title\"\n\tdescription=\"Module_description\"\n>\n</choice>\n"
            ((xmlindent++))
            packagesidentity="org.chameleon.module"
# -
            if [ -e ${1%/*}/i386/modules/AMDGraphicsEnabler.dylib ]; then
            {
                mkdir -p ${1}/AMDGraphicsEnabler/Root
                ditto --noextattr --noqtn ${1%/*}/i386/modules/AMDGraphicsEnabler.dylib ${1}/AMDGraphicsEnabler/Root
                echo "	[BUILD] AMDGraphicsEnabler "
                buildpackage "${1}/AMDGraphicsEnabler" "/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
            }
            fi
# -
            if [ -e ${1%/*}/i386/modules/ATiGraphicsEnabler.dylib ]; then
            {
                mkdir -p ${1}/ATiGraphicsEnabler/Root
                ditto --noextattr --noqtn ${1%/*}/i386/modules/ATiGraphicsEnabler.dylib ${1}/ATiGraphicsEnabler/Root
                echo "	[BUILD] ATiGraphicsEnabler "
                buildpackage "${1}/ATiGraphicsEnabler" "/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
            }
            fi
# -
            if [ -e ${1%/*}/i386/modules/IntelGraphicsEnabler.dylib ]; then
            {
                mkdir -p ${1}/IntelGraphicsEnabler/Root
                ditto --noextattr --noqtn ${1%/*}/i386/modules/IntelGraphicsEnabler.dylib ${1}/IntelGraphicsEnabler/Root
                echo "	[BUILD] IntelGraphicsEnabler "
                buildpackage "${1}/IntelGraphicsEnabler" "/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
            }
            fi
# -
            if [ -e ${1%/*}/i386/modules/klibc.dylib ]; then
            {
                mkdir -p ${1}/klibc/Root
                ditto --noextattr --noqtn ${1%/*}/i386/modules/klibc.dylib ${1}/klibc/Root
                echo "	[BUILD] klibc "
                buildpackage "${1}/klibc" "/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
            }
            fi
# -
            if [ -e ${1%/*}/i386/modules/NVIDIAGraphicsEnabler.dylib ]; then
            {
                mkdir -p ${1}/NVIDIAGraphicsEnabler/Root
                ditto --noextattr --noqtn ${1%/*}/i386/modules/NVIDIAGraphicsEnabler.dylib ${1}/NVIDIAGraphicsEnabler/Root
                echo "	[BUILD] NVIDIAGraphicsEnabler "
                buildpackage "${1}/NVIDIAGraphicsEnabler" "/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
            }
            fi
# -
            if [ -e ${1%/*}/i386/modules/Resolution.dylib ]; then
            {
                mkdir -p ${1}/AutoReso/Root
                ditto --noextattr --noqtn ${1%/*}/i386/modules/Resolution.dylib ${1}/AutoReso/Root
                echo "	[BUILD] Resolution "
                buildpackage "${1}/AutoReso" "/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
            }
            fi
# -
            if [ -e ${1%/*}/i386/modules/uClibcxx.dylib ]; then
            {
                mkdir -p ${1}/uClibc/Root
                ditto --noextattr --noqtn ${1%/*}/i386/modules/uClibcxx.dylib ${1}/uClibc/Root
                ditto --noextattr --noqtn ${1%/*}/i386/modules/klibc.dylib ${1}/uClibc/Root
                echo "	[BUILD] uClibc++ "
                buildpackage "${1}/uClibc" "/Extra/modules" "" "start_selected=\"false\"" >/dev/null 2>&1
            }
            fi
            ((xmlindent--))
            outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"
        }
        else
        {
            echo "      -= no modules to include =-"
        }
        fi
	# End build Modules packages
    ((xmlindent--))
    outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"
# End build Chameleon package

# build post install package
	echo "================= Post ================="
	mkdir -p ${1}/Post/Root
	mkdir -p ${1}/Post/Scripts
	cp -f ${pkgroot}/Scripts/Post/* ${1}/Post/Scripts
	echo "	[BUILD] Post "
	buildpackage "${1}/Post" "/" "" "start_visible=\"false\" start_selected=\"true\"" >/dev/null 2>&1
	outline[$((outlinecount++))]="${indent[$xmlindent]}</choices-outline>"

# build meta package

	makedistribution "${1}" "${2}" "${3}" "${4}" "${5}"

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

	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"${packagename// /}\"/>"

	if [ "${4}" ]; then
		local choiceoptions="${indent[$xmlindent]}${4}\n"	
	fi
	choices[$((choicescount++))]="<choice\n\tid=\"${packagename// /}\"\n\ttitle=\"${packagename}_title\"\n\tdescription=\"${packagename}_description\"\n${choiceoptions}>\n\t<pkg-ref id=\"${identifier}\" installKBytes='${installedsize}' version='${version}.0.0.${timestamp}' auth='root'>#${packagename// /}.pkg</pkg-ref>\n</choice>\n"

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
	xar -c -f "${1%/*}/$packagename.pkg" --compression none .
	popd >/dev/null

#   Here is the place for assign a Icon to the pkg
ditto -xk "${pkgroot}/Icons/pkg.zip" "${pkgroot}/Icons/"
DeRez -only icns "${pkgroot}/Icons/Icons/pkg.icns" > tempicns.rsrc
Rez -append tempicns.rsrc -o "${1%/*}/$packagename.pkg"
SetFile -a C "${1%/*}/$packagename.pkg"
rm -f tempicns.rsrc
rm -rf "${pkgroot}/Icons/Icons"
# End

}

main "${1}" "${2}" "${3}" "${4}" "${5}"

