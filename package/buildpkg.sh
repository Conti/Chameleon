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
echo -e $COL_BLACK"	----------------------------------"$COL_RESET
echo -e $COL_BLACK"	Building $packagename Install Package"$COL_RESET
echo -e $COL_BLACK"	----------------------------------"$COL_RESET
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
                # Supported Modules           #
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
            packagesidentity="org.chameleon.modules"
            
# -
            if [ -e ${1%/*}/i386/modules/klibc.dylib ]; then
            {
                mkdir -p ${1}/klibc/Root
                ditto --noextattr --noqtn ${1%/*}/i386/modules/klibc.dylib ${1}/klibc/Root
                echo "	[BUILD] klibc "
                buildpackage "${1}/klibc" "/Extra/modules" "" "start_selected=\"true\"" >/dev/null 2>&1
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
                buildpackage "${1}/uClibc" "/Extra/modules" "" "start_selected=\"true\"" >/dev/null 2>&1
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

# build Extras package
	#echo "================= Extras ================="
	#outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Extras\">"
	#choices[$((choicescount++))]="<choice\n\tid=\"Extras\"\n\ttitle=\"Extras_title\"\n\tdescription=\"Extras_description\"\n>\n</choice>\n"
	#((xmlindent++))
	#packagesidentity="org.chameleon.extras"

	# build utility package
	#	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Utility\">"
	#	choices[$((choicescount++))]="<choice\n\tid=\"Utility\"\n\ttitle=\"Utility_title\"\n\tdescription=\"Utility_description\"\n>\n</choice>\n"
	#	((xmlindent++))
	#	packagesidentity="org.chameleon.utilities"

	#	# build package for Chameleon PrefPanel
	#		mkdir -p "${1}/PrefPanel/Root"
	#		ditto --noextattr --noqtn "${pkgroot}/Configuration/PrefPanel/Chameleon.prefPane" "${1}/PrefPanel/Root"
	#		echo "	[BUILD] Chameleon Preference Panel "
	#		buildpackage "${1}/PrefPanel" "/Library/PreferencePanes/Chameleon.prefPane" "" "start_selected=\"false\"" >/dev/null 2>&1
	#	# End build package for Chameleon PrefPanel
		
	#	# build package for SMBIOSDefault
	#		mkdir -p "${1}/SMBIOSDefault/Root"
	#		ditto --noextattr --noqtn "${pkgroot}/Configuration/SMBIOSDefault/smbios.plist" "${1}/SMBIOSDefault/Root"
	#		echo "	[BUILD] SMBIOSDefault "
	#		buildpackage "${1}/SMBIOSDefault" "/Extra/Example" "" "start_selected=\"false\"" >/dev/null 2>&1
	#	# End build package for SMBIOSDefault
		
	#	# build package for Documentation
	#		mkdir -p "${1}/Documentation/Root"
	#		cp -f ${pkgroot}/../doc/BootHelp.txt ${1}/Documentation/Root
	#		cp -f ${pkgroot}/../doc/README ${1}/Documentation/Root
	#		cp -f ${pkgroot}/../doc/Users_Guide0.5.pdf ${1}/Documentation/Root
	#		echo "	[BUILD] Documentation "
	#		buildpackage "${1}/Documentation" "/Library/Documentation/Chameleon2" "" "start_selected=\"false\"" >/dev/null 2>&1
	#	# End build package for Documentation

	#	((xmlindent--))
	#	outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"
	# End utility package
		
	# build options packages
	echo "================= Options ================="
		outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Options\">"
		choices[$((choicescount++))]="<choice\n\tid=\"Options\"\n\ttitle=\"Options_title\"\n\tdescription=\"Options_description\"\n>\n</choice>\n"
		((xmlindent++))

		# build base options packages
		packagesidentity="org.chameleon.options"
		
		options=($( find "${pkgroot}/Scripts/BaseOptions" -type d -depth 1 -not -name '.svn' ))
		for (( i = 0 ; i < ${#options[@]} ; i++ )) 
		do
			mkdir -p "${1}/${options[$i]##*/}/Root"
			mkdir -p "${1}/${options[$i]##*/}/Scripts"
			ditto --noextattr --noqtn "${options[$i]}/postinstall" "${1}/${options[$i]##*/}/Scripts/postinstall"
			echo "	[BUILD] ${options[$i]##*/} "
			buildpackage "${1}/${options[$i]##*/}" "/" "" "start_selected=\"false\"" >/dev/null 2>&1
		done
		# End build base options packages

		# build resolution packages
			echo "================= Res. Options ================="
			outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Resolution\">"
			choices[$((choicescount++))]="<choice\n\tid=\"Resolution\"\n\ttitle=\"Resolution_title\"\n\tdescription=\"Resolution_description\"\n>\n</choice>\n"
			((xmlindent++))
			packagesidentity="org.chameleon.options.resolution"
			resolutions=($( find "${pkgroot}/Scripts/Resolutions" -type d -depth 1 -not -name '.svn' ))
			for (( i = 0 ; i < ${#resolutions[@]} ; i++ )) 
			do
				mkdir -p "${1}/${resolutions[$i]##*/}/Root/"
				mkdir -p "${1}/${resolutions[$i]##*/}/Scripts/"
				ditto --noextattr --noqtn "${resolutions[$i]}/postinstall" "${1}/${resolutions[$i]##*/}/Scripts/postinstall"
				echo "	[BUILD] ${resolutions[$i]##*/} "
				buildpackage "${1}/${resolutions[$i]##*/}" "/tmpcham" "" "start_selected=\"false\"" >/dev/null 2>&1
			done

			((xmlindent--))
			outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"
		# End build resolution packages
	
		# build Advanced packages
			echo "================= Adv. Options ================="
			outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Advanced\">"
			choices[$((choicescount++))]="<choice\n\tid=\"Advanced\"\n\ttitle=\"Advanced_title\"\n\tdescription=\"Advanced_description\"\n>\n</choice>\n"
			((xmlindent++))

			packagesidentity="org.chameleon.options.advanced"
			optionsadv=($( find "${pkgroot}/Scripts/Advanced" -type d -depth 1 -not -name '.svn' ))
			for (( i = 0 ; i < ${#optionsadv[@]} ; i++ )) 
			do
				mkdir -p "${1}/${optionsadv[$i]##*/}/Root"
				mkdir -p "${1}/${optionsadv[$i]##*/}/Scripts"
				ditto --noextattr --noqtn "${optionsadv[$i]}/postinstall" "${1}/${optionsadv[$i]##*/}/Scripts/postinstall"
				echo "	[BUILD] ${optionsadv[$i]##*/} "
				buildpackage "${1}/${optionsadv[$i]##*/}" "/" "" "start_selected=\"false\"" >/dev/null 2>&1
			done
		
			((xmlindent--))
			outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"
		# End build Advanced packages

		((xmlindent--))
		outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"

	# End build options packages
	
	# build theme packages
		echo "================= Themes ================="
		outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Themes\">"
		choices[$((choicescount++))]="<choice\n\tid=\"Themes\"\n\ttitle=\"Themes_title\"\n\tdescription=\"Themes_description\"\n>\n</choice>\n"
		((xmlindent++))
		packagesidentity="org.chameleon.themes"
		artwork="${1%/*}"
		themes=($( find "${artwork%/*}/artwork/themes" -type d -depth 1 -not -name '.svn' ))
		for (( i = 0 ; i < ${#themes[@]} ; i++ )) 
		do
			theme=$( echo ${themes[$i]##*/} | awk 'BEGIN{OFS=FS=""}{$1=toupper($1);print}' )
			mkdir -p "${1}/${theme}/Root/"
            rsync -r --exclude=.svn "${themes[$i]}" "${1}/${themes[$i]##*/}/Root/${theme}"
            # #### Comment out thx meklort
            # ditto --noextattr --noqtn "${themes[$i]}" "${1}/${themes[$i]##*/}/Root/${theme}" 
            # ####
            find "${1}/${themes[$i]##*/}" -name '.DS_Store' -or -name '.svn' -exec rm -R {} \+
			find "${1}/${themes[$i]##*/}" -type f -exec chmod 644 {} \+
			echo "	[BUILD] ${themes[$i]##*/} "
			buildpackage "${1}/${theme}" "/Extra/Themes" "" "start_selected=\"false\"" >/dev/null 2>&1
			rm -R -f "${1}/${i##*/}"
		done

		((xmlindent--))
		outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"
	# End build theme packages

	#((xmlindent--))
	#outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"
# End build Extras package

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
	xar -c -f "${1%/*}/${packagename// /}-${version}-r${revision}.pkg" --compression none .
	popd >/dev/null

#   Here is the place for assign a Icon to the pkg
#   command use to generate the file:
#   ditto -c -k --sequesterRsrc --keepParent Icon.icns Icon.zip
# ----
#    ditto -xk "${pkgroot}/Icons/pkg.zip" "${pkgroot}/Icons/"
#    DeRez -only icns "${pkgroot}/Icons/Icons/pkg.icns" > tempicns.rsrc
#    Rez -append tempicns.rsrc -o "${1%/*}/${packagename// /}-${version}-r${revision}.pkg"
#    SetFile -a C "${1%/*}/${packagename// /}-${version}-r${revision}.pkg"
#    rm -f tempicns.rsrc
#    rm -rf "${pkgroot}/Icons/Icons"
# End

	md5=$( md5 "${1%/*}/${packagename// /}-${version}-r${revision}.pkg" | awk {'print $4'} )
	echo "MD5 (${packagename// /}-${version}-r${revision}.pkg) = ${md5}" > "${1%/*}/${packagename// /}-${version}-r${revision}.pkg.md5"
	echo ""	

	echo -e $COL_BLACK"	--------------------------"$COL_RESET
	echo -e $COL_BLACK"	Building process complete!"$COL_RESET
	echo -e $COL_BLACK"	--------------------------"$COL_RESET
	echo ""	
	echo -e $COL_BLACK"	Build info."
	echo -e $COL_BLACK"	==========="
	echo -e $COL_BLUE"	Package name:	"$COL_BLACK"$packagename-${version}-r$revision.pkg"$COL_RESET
	echo -e $COL_BLUE"	MD5:		"$COL_BLACK"$md5"$COL_RESET
	echo -e $COL_BLUE"	Version:	"$COL_BLACK"$version"$COL_RESET
	echo -e $COL_BLUE"	Stage:		"$COL_BLACK"$stage"$COL_RESET
	echo -e $COL_BLUE"	Date/Time:	"$COL_BLACK"$builddate"$COL_RESET
	echo ""

}

main "${1}" "${2}" "${3}" "${4}" "${5}"

