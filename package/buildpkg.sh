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

declare -a pkgrefs

# Package identifiers
modules_packages_identity="org.chameleon.modules"

getPackageRefId () {
    echo ${1//_/.}.${2//_/.} | tr [:upper:] [:lower:]
}

addChoice () {
    # $1 Choice Id
    # $2 Choice Options
    # $3..$n Package reference id (optional)
    local choiceId="${1}"
    local choiceOptions="${2}"
    local choiceNode="\t<choice\n\t\tid=\"${choiceId}\"\n\t\ttitle=\"${choiceId}_title\"\n\t\tdescription=\"${choiceId}_description\""
    [[ -n "${choiceOptions}" ]] && choiceNode="${choiceNode}\n\t\t${choiceOptions}"
    choiceNode="${choiceNode}>"
    if [[ $# -ge 3 ]];then
        for pkgRefId in ${@:3};do
            choiceNode="${choiceNode}\n\t\t<pkg-ref id=\"${pkgRefId}\"/>"
        done
    fi
    choiceNode="${choiceNode}\n\t</choice>\n"

    outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"${choiceId}\"/>"
    choices[$((choicescount++))]="$choiceNode"
}

exclusive_one_choice () {
    # $1 Current choice (ie: test1)
    # $2..$n Others choice(s) (ie: "test2" "test3"). Current can or can't be in the others choices
    local myChoice="${1}"
    local result;
    local separator=' || ';
    for choice in ${@:2};do
        if [[ "$choice" != "$myChoice" ]];then
            result="${result}choices['$choice'].selected${separator}";
        fi
    done
    echo "!(${result%$separator})"
}

exclusive_zero_or_one_choice () {
    # $1 Current choice (ie: test1)
    # $2..$n Others choice(s) (ie: "test2" "test3"). Current can or can't be in the others choices
    local myChoice="${1}"
    local result;
    echo "(my.choice.selected &amp;&amp; $(exclusive_one_choice ${@}))"
}

main ()
{

# clean up the destination path

    rm -R -f "${1}"
    echo ""
    echo -e $COL_CYAN"  ----------------------------------"$COL_RESET
    echo -e $COL_CYAN"  Building $packagename Install Package"$COL_RESET
    echo -e $COL_CYAN"  ----------------------------------"$COL_RESET
    echo ""

    outline[$((outlinecount++))]="${indent[$xmlindent]}<choices-outline>"

# build pre install package
    echo "================= Preinstall ================="
    ((xmlindent++))
    packagesidentity="org.chameleon"
    choiceId="Pre"
    mkdir -p ${1}/${choiceId}/Root
    mkdir -p ${1}/${choiceId}/Scripts
    ditto --noextattr --noqtn ${1%/*/*}/revision ${1}/${choiceId}/Scripts/Resources/revision
    ditto --noextattr --noqtn ${1%/*/*}/version ${1}/${choiceId}/Scripts/Resources/version
    cp -f ${pkgroot}/Scripts/Main/preinstall ${1}/${choiceId}/Scripts
    cp -f ${pkgroot}/Scripts/Sub/InstallLog.sh ${1}/${choiceId}/Scripts
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/"
    addChoice "${choiceId}"  "start_visible=\"false\" start_selected=\"true\""  "$packageRefId"
# End build pre install package

# build core package
    echo "================= Core ================="
    packagesidentity="org.chameleon"
    choiceId="Core"
    mkdir -p ${1}/${choiceId}/Root/usr/local/bin
    mkdir -p ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/boot     ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/boot0    ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/boot0md  ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/boot1f32 ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/boot1h   ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/boot1he  ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/boot1hp  ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/cdboot   ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/chain0   ${1}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${1%/*}/i386/fdisk440 ${1}/${choiceId}/Root/usr/local/bin
    ditto --noextattr --noqtn ${1%/*}/i386/bdmesg   ${1}/${choiceId}/Root/usr/local/bin
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/"
    addChoice "${choiceId}"  "start_visible=\"false\" start_selected=\"true\""  "$packageRefId"
# End build core package

# build install type
    echo "================= Chameleon ================="
    outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"InstallType\">"
    choices[$((choicescount++))]="\t<choice\n\t\tid=\"InstallType\"\n\t\ttitle=\"InstallType_title\"\n\t\tdescription=\"InstallType_description\">\n\t</choice>\n"
    ((xmlindent++))
    packagesidentity="org.chameleon.type"
    allChoices="New Upgrade"

    # build new install package
    choiceId="New"
    mkdir -p ${1}/${choiceId}/Root
    echo "" > "${1}/${choiceId}/Root/install_type_new"
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/$chamTemp"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_selected=\"!choices['Upgrade'].selected\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build new install package

    # build upgrade package
    choiceId="Upgrade"
    mkdir -p ${1}/${choiceId}/Root
    echo "" > "${1}/${choiceId}/Root/install_type_upgrade"
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/$chamTemp"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_selected=\"chameleon_boot_plist_exists()\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build upgrade package

   ((xmlindent--))
   outline[$((outlinecount++))]="${indent[$xmlindent]}</line>"
# End build install type

# build Chameleon package
    echo "================= Chameleon ================="
    outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"Chameleon\">"
    choices[$((choicescount++))]="\t<choice\n\t\tid=\"Chameleon\"\n\t\ttitle=\"Chameleon_title\"\n\t\tdescription=\"Chameleon_description\">\n\t</choice>\n"
    ((xmlindent++))

    allChoices="Standard EFI noboot"

    # build standard package
    choiceId="Standard"
    mkdir -p ${1}/${choiceId}/Root
    mkdir -p ${1}/${choiceId}/Scripts/Resources
    cp -f ${pkgroot}/Scripts/Main/${choiceId}postinstall ${1}/${choiceId}/Scripts/postinstall
    cp -f ${pkgroot}/Scripts/Sub/* ${1}/${choiceId}/Scripts
    ditto --arch i386 `which SetFile` ${1}/${choiceId}/Scripts/Resources/SetFile
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_selected=\"true\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build standard package

    # build efi package
    choiceId="EFI"
    mkdir -p ${1}/${choiceId}/Root
    mkdir -p ${1}/${choiceId}/Scripts/Resources
    cp -f ${pkgroot}/Scripts/Main/ESPpostinstall ${1}/${choiceId}/Scripts/postinstall
    cp -f ${pkgroot}/Scripts/Sub/* ${1}/${choiceId}/Scripts
    ditto --arch i386 `which SetFile` ${1}/${choiceId}/Scripts/Resources/SetFile
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_visible=\"systemHasGPT()\" start_selected=\"false\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build efi package

    # build no bootloader choice package
    choiceId="noboot"
    mkdir -p ${1}/${choiceId}/Root
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_selected=\"false\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build no bootloader choice package

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

# -
        if [ -e ${1%/*}/i386/modules/klibc.dylib ]; then
        {
            # Start build klibc package module
            choiceId="klibc"
            mkdir -p ${1}/${choiceId}/Root
            ditto --noextattr --noqtn ${1%/*}/i386/modules/${choiceId}.dylib ${1}/${choiceId}/Root
            echo -e "\t[BUILD] ${choiceId} "
            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/$chamTemp/Extra/modules"
            addChoice "${choiceId}"  "start_selected=\"false\""  "$packageRefId"
            # End build klibc package module
        }
        fi

# -
        if [ -e ${1%/*}/i386/modules/Resolution.dylib ]; then
        {
            # Start build Resolution package module
            choiceId="AutoReso"
            mkdir -p ${1}/${choiceId}/Root
            ditto --noextattr --noqtn ${1%/*}/i386/modules/Resolution.dylib ${1}/${choiceId}/Root
            echo -e "\t[BUILD] ${choiceId} "
            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/$chamTemp/Extra/modules"
            addChoice "${choiceId}"  "start_selected=\"false\""  "$packageRefId"
            # End build Resolution package module
        }
        fi

# -
        if [ -e ${1%/*}/i386/modules/uClibcxx.dylib ]; then
        {
            # Start build uClibc package module
            choiceId="uClibc"
            mkdir -p ${1}/${choiceId}/Root
            ditto --noextattr --noqtn ${1%/*}/i386/modules/uClibcxx.dylib ${1}/${choiceId}/Root
            echo -e "\t[BUILD] ${choiceId} "
            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/$chamTemp/Extra/modules"
            # Add the klibc package because the uClibc module is dependent of klibc module
            addChoice "${choiceId}"  "start_selected=\"false\""  \
             "$packageRefId" $(getPackageRefId "${modules_packages_identity}" "klibc")
            # End build uClibc package module
        }
        fi

# -
        # Warning Keylayout module need additional files
        if [ -e ${1%/*}/i386/modules/Keylayout.dylib ]; then
        {
            # Start build Keylayout package module
            choiceId="Keylayout"
            mkdir -p ${1}/${choiceId}/Root/Extra/{modules,Keymaps}
            mkdir -p ${1}/${choiceId}/Root/usr/local/bin
            layout_src_dir="${1%/sym/*}/i386/modules/Keylayout/layouts/layouts-src"
            if [ -d "$layout_src_dir" ];then
                # Create a tar.gz from layout sources
                (cd "$layout_src_dir"; \
                    tar czf "${1}/Keylayout/Root/Extra/Keymaps/layouts-src.tar.gz" README *.slt)
            fi
            # Adding module
            ditto --noextattr --noqtn ${1%/*}/i386/modules/${choiceId}.dylib ${1}/${choiceId}/Root/Extra/modules
            # Adding Keymaps
            ditto --noextattr --noqtn ${1%/sym/*}/Keymaps ${1}/${choiceId}/Root/Extra/Keymaps
            # Adding tools
            ditto --noextattr --noqtn ${1%/*}/i386/cham-mklayout ${1}/${choiceId}/Root/usr/local/bin
            echo -e "\t[BUILD] ${choiceId} "
            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/$chamTemp"

            # Don't add a choice for Keylayout module
            # addChoice "${choiceId}"  "start_selected=\"false\""  "$packageRefId"
            # End build Keylayout package module
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

# build Options packages

    outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"Options\">"
    choices[$((choicescount++))]="\t<choice\n\t\tid=\"Options\"\n\t\ttitle=\"Options_title\"\n\t\tdescription=\"Options_description\">\n\t</choice>\n"
    ((xmlindent++))


    # ------------------------------------------------------
    # parse OptionalSettings folder to find files of boot options.
    # ------------------------------------------------------
    OptionalSettingsFolder="${pkgroot}/OptionalSettings"
    OptionalSettingsFiles=($( find "${OptionalSettingsFolder}" -depth 1 ! -name '.svn' ! -name '.DS_Store' ))

    for (( i = 0 ; i < ${#OptionalSettingsFiles[@]} ; i++ ))
    do

		# Take filename and Strip .txt from end and path from front
		builtOptionsList=$( echo ${OptionalSettingsFiles[$i]%.txt} )
		builtOptionsList=$( echo ${builtOptionsList##*/} )
		echo "================= $builtOptionsList ================="
		outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"${builtOptionsList}\">"
		choices[$((choicescount++))]="\t<choice\n\t\tid=\"${builtOptionsList}\"\n\t\ttitle=\"${builtOptionsList}_title\"\n\t\tdescription=\"${builtOptionsList}_description\">\n\t</choice>\n"
		((xmlindent++))
		packagesidentity="org.chameleon.options.$builtOptionsList"

		# ------------------------------------------------------
		# Read boot option file into an array.
		# ------------------------------------------------------
		availableOptions=() # array to hold the list of boot options, per 'section'.
		exclusiveFlag=0 # used to indicate list has exclusive options
		count=0 # used as index for stepping through array.
		while read textLine; do
			# ignore lines in the file beginning with a # and Exclusive=False
			if [[ ${textLine} != \#* ]] && [[ ${textLine} != "Exclusive=False" ]];then
				# check for 'Exclusive=True' option in file
				if [[ ${textLine} == "Exclusive=True" ]];then
					exclusiveFlag=1
				else
					availableOptions[${#availableOptions[@]}]=$textLine
				fi
			fi
		done < ${OptionalSettingsFiles[$i]}

		# ------------------------------------------------------
		# Loop through options in array and process each in turn
		# ------------------------------------------------------
		allChoices="${availableOptions[@]//:*/}"
		for (( c = 0 ; c < ${#availableOptions[@]} ; c++ )); do
			textLine=${availableOptions[c]}
			# split line - taking all before ':' as option name
			# and all after ':' as key/value
			optionName=${textLine%:*}
			keyValue=${textLine##*:}

			# create folders required for each boot option
			mkdir -p "${1}/$optionName/Root/"

			# create dummy file with name of key/value
			echo "" > "${1}/$optionName/Root/${keyValue}"

			echo -e "\t[BUILD] ${optionName} "
			packageRefId=$(getPackageRefId "${packagesidentity}" "${optionName}")
			buildpackage "$packageRefId" "${optionName}" "${1}/${optionName}" "/$chamTemp/options"
			exclusiveSelect=""
			if [[ ${exclusiveFlag} -eq 1 ]];then
				exclusiveSelect="selected=\"$(exclusive_zero_or_one_choice "$optionName" "$allChoices")\""
			fi
			addChoice "${optionName}"  "start_selected=\"false\" ${exclusiveSelect}"  "$packageRefId"
		done

		((xmlindent--))
		outline[$((outlinecount++))]="${indent[$xmlindent]}</line>"
	done

	# build KeyLayout options packages
	echo "================= Keymaps Options ================="
	outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"KeyLayout\">"
	choices[$((choicescount++))]="\t<choice\n\t\tid=\"KeyLayout\"\n\t\ttitle=\"KeyLayout_title\"\n\t\tdescription=\"KeyLayout_description\">\n\t</choice>\n"
	((xmlindent++))
	packagesidentity="org.chameleon.options.keylayout"
	keylayoutPackageRefId=$(getPackageRefId "${modules_packages_identity}" "Keylayout")

	# ------------------------------------------------------
	# Available Keylayout boot options are discovered by
	# reading contents of /Keymaps folder after compilation
	# ------------------------------------------------------
	availableOptions=($( find "${1%/sym/*}/Keymaps" -type f -depth 1 -name '*.lyt' | sed 's|.*/||;s|\.lyt||' ))
	allChoices="${availableOptions[@]}"
	# Adjust array contents to match expected format
	# for boot options which is: name:key=value
	for (( i = 0 ; i < ${#availableOptions[@]} ; i++ )); do
		# availableOptions[i]=${availableOptions[i]}":KeyLayout="${availableOptions[i]}
		# Start build of a keymap package module
		choiceId="${availableOptions[i]}"
		mkdir -p ${1}/${choiceId}/Root

		# create dummy file with name of key/value
		echo "" > "${1}/${choiceId}/Root/KeyLayout=${availableOptions[i]}"

		echo -e "\t[BUILD] ${choiceId} "
		packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
		buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/$chamTemp/options"
		exclusiveChoice=$(exclusive_zero_or_one_choice "$choiceId" "$allChoices")
		# Add the Keylayout package because the Keylayout module is needed
		addChoice "${choiceId}"  "start_selected=\"false\" selected=\"${exclusiveChoice}\""  \
		 "$packageRefId" "$keylayoutPackageRefId"
		# End build uClibc package module
	done

	((xmlindent--))
	outline[$((outlinecount++))]="${indent[$xmlindent]}</line>"

	# End build KeyLayout options packages

	((xmlindent--))
	outline[$((outlinecount++))]="${indent[$xmlindent]}</line>"
# End build options packages

# build theme packages
	echo "================= Themes ================="
	outline[$((outlinecount++))]="${indent[$xmlindent]}<line choice=\"Themes\">"
	choices[$((choicescount++))]="\t<choice\n\t\tid=\"Themes\"\n\t\ttitle=\"Themes_title\"\n\t\tdescription=\"Themes_description\">\n\t</choice>\n"
	((xmlindent++))

	# Using themes section from Azi's/package branch.
	packagesidentity="org.chameleon.themes"
	artwork="${1%/sym/package}/artwork/themes"
	themes=($( find "${artwork}" -type d -depth 1 -not -name '.svn' ))
	for (( i = 0 ; i < ${#themes[@]} ; i++ )); do
		theme=$( echo ${themes[$i]##*/} | awk 'BEGIN{OFS=FS=""}{$1=toupper($1);print}' )
		mkdir -p "${1}/${theme}/Root/"
		rsync -r --exclude=.svn "${themes[$i]}/" "${1}/${theme}/Root/${theme}"
		echo -e "\t[BUILD] ${theme}"
		packageRefId=$(getPackageRefId "${packagesidentity}" "${theme}")
		buildpackage "$packageRefId" "${theme}" "${1}/${theme}" "/$chamTemp/Extra/Themes"
		addChoice "${theme}"  "start_selected=\"false\""  "$packageRefId"
	done

	((xmlindent--))
	outline[$((outlinecount++))]="${indent[$xmlindent]}</line>"
# End build theme packages# End build Extras package

# build post install package
    echo "================= Post ================="
    packagesidentity="org.chameleon"
    choiceId="Post"
    mkdir -p ${1}/${choiceId}/Root
    mkdir -p ${1}/${choiceId}/Scripts
    cp -f ${pkgroot}/Scripts/Main/postinstall ${1}/${choiceId}/Scripts
    cp -f ${pkgroot}/Scripts/Sub/InstallLog.sh ${1}/${choiceId}/Scripts
    cp -f ${pkgroot}/Scripts/Sub/UnMountEFIvolumes.sh ${1}/${choiceId}/Scripts
    ditto --noextattr --noqtn ${1%/*/*}/revision ${1}/${choiceId}/Scripts/Resources/revision
    ditto --noextattr --noqtn ${1%/*/*}/version ${1}/${choiceId}/Scripts/Resources/version
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${1}/${choiceId}" "/"
    addChoice "${choiceId}"  "start_visible=\"false\" start_selected=\"true\""  "$packageRefId"
# End build post install package

	((xmlindent--))
	outline[$((outlinecount++))]="${indent[$xmlindent]}</choices-outline>"

# build meta package

    makedistribution "${1}" "${2}" "${3}" "${4}" #"${5}"

# clean up
	#   rm -R -f "${1}"

}

buildpackage ()
{
	#  $1 Package Reference Id (ie: org.chameleon.themes.default)
	#  $2 Package Name (ie: Default)
	#  $3 Path to package to build containing Root and/or Scripts
	#  $4 Target install location
	#  $5 Size (optional)
	if [[ -d "${3}/Root" ]]; then
		local packageRefId="$1"
		local packageName="$2"
		local packagePath="$3"
		local targetPath="$4"
		local packageSize="$5"

		find "${packagePath}" -name '.DS_Store' -delete
		local filecount=$( find "${packagePath}/Root" | wc -l )
		if [ "${packageSize}" ]; then
			local installedsize="${packageSize}"
		else
			local installedsize=$( du -hkc "${packagePath}/Root" | tail -n1 | awk {'print $1'} )
		fi
		local header="<?xml version=\"1.0\"?>\n<pkg-info format-version=\"2\" "

		#[ "${3}" == "relocatable" ] && header+="relocatable=\"true\" "

		header+="identifier=\"${packageRefId}\" "
		header+="version=\"${version}\" "

		[ "${targetPath}" != "relocatable" ] && header+="install-location=\"${targetPath}\" "

		header+="auth=\"root\">\n"
		header+="\t<payload installKBytes=\"${installedsize##* }\" numberOfFiles=\"${filecount##* }\"/>\n"
		rm -R -f "${packagePath}/Temp"

		[ -d "${packagePath}/Temp" ] || mkdir -m 777 "${packagePath}/Temp"
		[ -d "${packagePath}/Root" ] && mkbom "${packagePath}/Root" "${packagePath}/Temp/Bom"

		if [ -d "${packagePath}/Scripts" ]; then
			header+="\t<scripts>\n"
			for script in $( find "${packagePath}/Scripts" -type f \( -name 'pre*' -or -name 'post*' \) ); do
				header+="\t\t<${script##*/} file=\"./${script##*/}\"/>\n"
			done
			header+="\t</scripts>\n"
			# Create the Script archive file (cpio format)
			(cd "${packagePath}/Scripts" && find . -print | cpio -o -z -R 0:0 --format cpio > "${packagePath}/Temp/Scripts") 2>&1 | \
			    grep -vE '^[0-9]+\s+blocks?$' # to remove cpio stderr messages
        fi

		header+="</pkg-info>"
		echo -e "${header}" > "${packagePath}/Temp/PackageInfo"

		# Create the Payload file (cpio format)
		(cd "${packagePath}/Root" && find . -print | cpio -o -z -R 0:0 --format cpio > "${packagePath}/Temp/Payload") 2>&1 | \
	        grep -vE '^[0-9]+\s+blocks?$' # to remove cpio stderr messages

		# Create the package
		(cd "${packagePath}/Temp" && xar -c -f "${packagePath}/../${packageName}.pkg" --compression none .)

		# Add the package to the list of build packages
		pkgrefs[${#pkgrefs[*]}]="\t<pkg-ref id=\"${packageRefId}\" installKBytes='${installedsize}' version='${version}.0.0.${timestamp}'>#${packageName}.pkg</pkg-ref>"

		rm -rf "${packagePath}"
	fi
}

makedistribution ()
{
    distributionDestDir="${1%/*}"
    distributionFilename="${packagename// /}-${version}-r${revision}.pkg"
    distributionFilePath="${distributionDestDir}/${distributionFilename}"

    rm -f "${distributionDestDir}/${packagename// /}"*.pkg

    mkdir -p "${1}/${packagename}"

    find "${1}" -type f -name '*.pkg' -depth 1 | while read component
	do
        pkg="${component##*/}" # ie: EFI.pkg
        pkgdir="${1}/${packagename}/${pkg}"
        # expand individual packages
        pkgutil --expand "${1%}/${pkg}" "$pkgdir"
        rm -f "${1%}/${pkg}"
    done

#   Create the Distribution file
    ditto --noextattr --noqtn "${pkgroot}/Distribution" "${1}/${packagename}/Distribution"

    for (( i=0; i < ${#outline[*]} ; i++)); do
		echo -e "${outline[$i]}" >> "${1}/${packagename}/Distribution"
	done

    for (( i=0; i < ${#choices[*]} ; i++)); do
		echo -e "${choices[$i]}" >> "${1}/${packagename}/Distribution"
	done

    for (( i=0; i < ${#pkgrefs[*]} ; i++)); do
        echo -e "${pkgrefs[$i]}" >> "${1}/${packagename}/Distribution"
    done

    echo -e "\n</installer-gui-script>"  >> "${1}/${packagename}/Distribution"

#   Create the Resources directory
    ditto --noextattr --noqtn "${pkgroot}/Resources" "${1}/${packagename}/Resources"

#   CleanUp the directory
    find "${1}/${packagename}" -type d -name '.svn' -exec rm -rf {} \; 2>/dev/null
    find "${1}/${packagename}" -name '.DS_Store' -delete

#   Add Chameleon Version and Revision
    perl -i -p -e "s/%CHAMELEONVERSION%/${version%%-*}/g" $( find "${1}/${packagename}/Resources" -type f )
    perl -i -p -e "s/%CHAMELEONREVISION%/${revision}/g"   $( find "${1}/${packagename}/Resources" -type f )

#   Add Chameleon Stage
    stage=${stage/RC/Release Candidate }
    stage=${stage/FINAL/2.0 Final}
    perl -i -p -e "s/%CHAMELEONSTAGE%/${stage}/g" $( find "${1}/${packagename}/Resources" -type f )

#   Adding Developer and credits
    perl -i -p -e "s/%DEVELOP%/${develop}/g" $( find "${1}/${packagename}/Resources" -type f )
    perl -i -p -e "s/%CREDITS%/${credits}/g" $( find "${1}/${packagename}/Resources" -type f )
    perl -i -p -e "s/%PKGDEV%/${pkgdev}/g"   $( find "${1}/${packagename}/Resources" -type f )

#   Create the final package
    pkgutil --flatten "${1}/${packagename}" "${distributionFilePath}"

#   Here is the place for assign a Icon to the pkg
    ditto -xk "${pkgroot}/Icons/pkg.zip" "${1}/Icons/"
    DeRez -only icns "${1}/Icons/Icons/pkg.icns" > "${1}/Icons/tempicns.rsrc"
    Rez -append "${1}/Icons/tempicns.rsrc" -o "${distributionFilePath}"
    SetFile -a C "${distributionFilePath}"
    rm -rf "${1}/Icons"

# End

    md5=$( md5 "${distributionFilePath}" | awk {'print $4'} )
    echo "MD5 (${distributionFilePath}) = ${md5}" > "${distributionFilePath}.md5"
    echo ""

    echo -e $COL_GREEN" --------------------------"$COL_RESET
    echo -e $COL_GREEN" Building process complete!"$COL_RESET
    echo -e $COL_GREEN" --------------------------"$COL_RESET
    echo ""
    echo -e $COL_GREEN" Build info."
    echo -e $COL_GREEN" ==========="
    echo -e $COL_BLUE"  Package name: "$COL_RESET"${distributionFilename}"
    echo -e $COL_BLUE"  MD5:          "$COL_RESET"$md5"
    echo -e $COL_BLUE"  Version:      "$COL_RESET"$version"
    echo -e $COL_BLUE"  Stage:        "$COL_RESET"$stage"
    echo -e $COL_BLUE"  Date/Time:    "$COL_RESET"$builddate"
    echo ""

}

main "${1}" "${2}" "${3}" "${4}" #"${5}"
