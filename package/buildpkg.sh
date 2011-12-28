#!/bin/bash

# $0 SRCROOT directory
# $1 SYMROOT directory
# $2 directory where pkgs will be created

# Directory paths
declare -r PKGROOT="${0%/*}"
declare -r SRCROOT="$1"
declare -r SYMROOT="$2"
declare -r PKG_BUILD_DIR="$3"

if [[ $# -lt 3 ]];then
    echo "Too few arguments. Aborting..." >&2 && exit 1
fi

if [[ ! -d "$SYMROOT" ]];then
	echo "Directory ${SYMROOT} doesn't exit. Aborting..." >&2 && exit 1
fi

# Prevent the script from doing bad things
set -u  # Abort with unset variables
#set -e # Abort with any error can be suppressed locally using EITHER cmd||true OR set -e;cmd;set +e

# ====== CONFIGURATION ======
CONFIG_MODULES=""
CONFIG_KLIBC_MODULE=""
CONFIG_UCLIBCXX_MODULE=""
CONFIG_RESOLUTION_MODULE=""
CONFIG_KEYLAYOUT_MODULE=""
source "${SRCROOT}/auto.conf"

# ====== COLORS ======

declare -r COL_BLACK="\x1b[30;01m"
declare -r COL_RED="\x1b[31;01m"
declare -r COL_GREEN="\x1b[32;01m"
declare -r COL_YELLOW="\x1b[33;01m"
declare -r COL_MAGENTA="\x1b[35;01m"
declare -r COL_CYAN="\x1b[36;01m"
declare -r COL_WHITE="\x1b[37;01m"
declare -r COL_BLUE="\x1b[34;01m"
declare -r COL_RESET="\x1b[39;49;00m"

# ====== REVISION/VERSION ======

declare -r version=$( cat version )

# stage
stage=${version##*-}
stage=${stage/RC/Release Candidate }
stage=${stage/FINAL/2.1 Final}
declare -r stage

declare -r revision=$( grep I386BOOT_CHAMELEONREVISION vers.h | awk '{ print $3 }' | tr -d '\"' )
declare -r builddate=$( grep I386BOOT_BUILDDATE vers.h | awk '{ print $3,$4 }' | tr -d '\"' )
declare -r timestamp=$( date -j -f "%Y-%m-%d %H:%M:%S" "${builddate}" "+%s" )

# ====== CREDITS ======

declare -r develop=$(awk "NR==6{print;exit}"  ${PKGROOT}/../CREDITS)
declare -r credits=$(awk "NR==10{print;exit}" ${PKGROOT}/../CREDITS)
declare -r pkgdev=$(awk "NR==14{print;exit}"  ${PKGROOT}/../CREDITS)

# =================

xmlindent=0

indent[0]="\t"
indent[1]="\t\t"
indent[2]="\t\t\t"
indent[3]="\t\t\t\t"

# ====== GLOBAL VARIABLES ======
declare -a pkgrefs
declare -a outline
declare -a choices

# Package name
declare -r packagename="Chameleon"

# Package identifiers
declare -r chameleon_package_identity="org.chameleon"
declare -r modules_packages_identity="${chameleon_package_identity}.modules"
declare -r chamTemp="usr/local/chamTemp"

# ====== FUNCTIONS ======

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

    outline[${#outline[*]}]="${indent[$xmlindent]}<line choice=\"${choiceId}\"/>"
    choices[${#choices[*]}]="$choiceNode"
}

exclusive_one_choice () {
    # $1 Current choice (ie: test1)
    # $2..$n Others choice(s) (ie: "test2" "test3"). Current can or can't be in the others choices
    local myChoice="${1}"
    local result="";
    local separator=' || ';
    for choice in ${@:2};do
        if [[ "$choice" != "$myChoice" ]];then
            result="${result}choices['$choice'].selected${separator}";
        fi
    done
    if [[ -n "$result" ]];then
        echo "!(${result%$separator})"
    fi
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

    rm -R -f "${PKG_BUILD_DIR}"
    echo ""
    echo -e $COL_CYAN"  ----------------------------------"$COL_RESET
    echo -e $COL_CYAN"  Building $packagename Install Package"$COL_RESET
    echo -e $COL_CYAN"  ----------------------------------"$COL_RESET
    echo ""

    outline[${#outline[*]}]="${indent[$xmlindent]}<choices-outline>"

# build pre install package
    echo "================= Preinstall ================="
    ((xmlindent++))
    packagesidentity="${chameleon_package_identity}"
    choiceId="Pre"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts
    ditto --noextattr --noqtn ${SRCROOT}/revision ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources/revision
    ditto --noextattr --noqtn ${SRCROOT}/version  ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources/version
    cp -f ${PKGROOT}/Scripts/Main/preinstall   ${PKG_BUILD_DIR}/${choiceId}/Scripts
    cp -f ${PKGROOT}/Scripts/Sub/InstallLog.sh ${PKG_BUILD_DIR}/${choiceId}/Scripts
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice "${choiceId}"  "start_visible=\"false\" start_selected=\"true\""  "$packageRefId"
# End build pre install package

# build core package
    echo "================= Core ================="
    packagesidentity="${chameleon_package_identity}"
    choiceId="Core"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot     ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot0    ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot0md  ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot1f32 ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot1h   ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot1he  ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot1hp  ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/cdboot   ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/chain0   ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/fdisk440 ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin
    ditto --noextattr --noqtn ${SYMROOT}/i386/bdmesg   ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice "${choiceId}"  "start_visible=\"false\" start_selected=\"true\""  "$packageRefId"
# End build core package

# build install type
    echo "================= Chameleon ================="
    outline[${#outline[*]}]="${indent[$xmlindent]}<line choice=\"InstallType\">"
    choices[${#choices[*]}]="\t<choice\n\t\tid=\"InstallType\"\n\t\ttitle=\"InstallType_title\"\n\t\tdescription=\"InstallType_description\">\n\t</choice>\n"
    ((xmlindent++))
    packagesidentity="${chameleon_package_identity}.type"
    allChoices="New Upgrade"

    # build new install package
    choiceId="New"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    echo "" > "${PKG_BUILD_DIR}/${choiceId}/Root/install_type_new"
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/$chamTemp"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_selected=\"!choices['Upgrade'].selected\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build new install package

    # build upgrade package
    choiceId="Upgrade"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    echo "" > "${PKG_BUILD_DIR}/${choiceId}/Root/install_type_upgrade"
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/$chamTemp"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_selected=\"chameleon_boot_plist_exists()\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build upgrade package

   ((xmlindent--))
   outline[${#outline[*]}]="${indent[$xmlindent]}</line>"
# End build install type

# build Chameleon package
    echo "================= Chameleon ================="
    outline[${#outline[*]}]="${indent[$xmlindent]}<line choice=\"Chameleon\">"
    choices[${#choices[*]}]="\t<choice\n\t\tid=\"Chameleon\"\n\t\ttitle=\"Chameleon_title\"\n\t\tdescription=\"Chameleon_description\">\n\t</choice>\n"
    ((xmlindent++))

    allChoices="Standard EFI noboot"

    # build standard package
    choiceId="Standard"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources
    cp -f ${PKGROOT}/Scripts/Main/${choiceId}postinstall ${PKG_BUILD_DIR}/${choiceId}/Scripts/postinstall
    cp -f ${PKGROOT}/Scripts/Sub/* ${PKG_BUILD_DIR}/${choiceId}/Scripts
    ditto --arch i386 `which SetFile` ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources/SetFile
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_selected=\"true\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build standard package

    # build efi package
    choiceId="EFI"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources
    cp -f ${PKGROOT}/Scripts/Main/ESPpostinstall ${PKG_BUILD_DIR}/${choiceId}/Scripts/postinstall
    cp -f ${PKGROOT}/Scripts/Sub/* ${PKG_BUILD_DIR}/${choiceId}/Scripts
    ditto --arch i386 `which SetFile` ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources/SetFile
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_visible=\"systemHasGPT()\" start_selected=\"false\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build efi package

    # build no bootloader choice package
    choiceId="noboot"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    exclusiveChoice=$(exclusive_one_choice "$choiceId" "$allChoices")
    addChoice "${choiceId}"  "start_selected=\"false\" selected=\"${exclusiveChoice}\""  "$packageRefId"
    # End build no bootloader choice package

    ((xmlindent--))
    outline[${#outline[*]}]="${indent[$xmlindent]}</line>"
# End build Chameleon package

if [[ "${CONFIG_MODULES}" == 'y' ]];then
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
    if [ "$(ls -A "${SYMROOT}/i386/modules")" ]; then
    {
        outline[${#outline[*]}]="${indent[$xmlindent]}<line choice=\"Module\">"
        choices[${#choices[*]}]="\t<choice\n\t\tid=\"Module\"\n\t\ttitle=\"Module_title\"\n\t\tdescription=\"Module_description\">\n\t</choice>\n"
        ((xmlindent++))

# -
        if [[ "${CONFIG_RESOLUTION_MODULE}" == 'm' && -f "${SYMROOT}/i386/modules/Resolution.dylib" ]]; then
        {
            # Start build Resolution package module
            choiceId="AutoReso"
            mkdir -p "${PKG_BUILD_DIR}/${choiceId}/Root"
            ditto --noextattr --noqtn "${SYMROOT}/i386/modules/Resolution.dylib" "${PKG_BUILD_DIR}/${choiceId}/Root"
            echo -e "\t[BUILD] ${choiceId} "
            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/$chamTemp/Extra/modules"
            addChoice "${choiceId}"  "start_selected=\"false\""  "$packageRefId"
            # End build Resolution package module
        }
        fi

# -
        if [[ "${CONFIG_KLIBC_MODULE}" == 'm' && -f "${SYMROOT}/i386/modules/klibc.dylib" ]]; then
        {
            # Start build klibc package module
            choiceId="klibc"
            mkdir -p "${PKG_BUILD_DIR}/${choiceId}/Root"
            ditto --noextattr --noqtn "${SYMROOT}/i386/modules/${choiceId}.dylib" ${PKG_BUILD_DIR}/${choiceId}/Root
            echo -e "\t[BUILD] ${choiceId} "
            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/$chamTemp/Extra/modules"
            addChoice "${choiceId}"  "start_selected=\"false\""  "$packageRefId"
            # End build klibc package module
        }
        fi

# -
        if [[ "${CONFIG_UCLIBCXX_MODULE}" = 'm' && -n "${CONFIG_KLIBC_MODULE}" && \
              -f "${SYMROOT}/i386/modules/uClibcxx.dylib" ]]; then
        {
            klibcPackageRefId=""
            if [[ "${CONFIG_KLIBC_MODULE}" == 'm' ]];then
                klibcPackageRefId=$(getPackageRefId "${modules_packages_identity}" "klibc")
            fi
            # Start build uClibc package module
            choiceId="uClibc"
            mkdir -p "${PKG_BUILD_DIR}/${choiceId}/Root"
            ditto --noextattr --noqtn "${SYMROOT}/i386/modules/uClibcxx.dylib" "${PKG_BUILD_DIR}/${choiceId}/Root"
            echo -e "\t[BUILD] ${choiceId} "
            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/$chamTemp/Extra/modules"
            # Add the klibc package because the uClibc module is dependent of klibc module
            addChoice "${choiceId}"  "start_selected=\"false\""  "$packageRefId" "$klibcPackageRefId"
            # End build uClibc package module
        }
        fi

# -
        # Warning Keylayout module need additional files
        if [[ "${CONFIG_KEYLAYOUT_MODULE}" = 'm' && -f "${SYMROOT}/i386/modules/Keylayout.dylib" ]]; then
        {
            # Start build Keylayout package module
            choiceId="Keylayout"
            mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/Extra/{modules,Keymaps}
            mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin
            layout_src_dir="${SRCROOT}/i386/modules/Keylayout/layouts/layouts-src"
            if [ -d "$layout_src_dir" ];then
                # Create a tar.gz from layout sources
                (cd "$layout_src_dir"; \
                    tar czf "${PKG_BUILD_DIR}/${choiceId}/Root/Extra/Keymaps/layouts-src.tar.gz" README *.slt)
            fi
            # Adding module
            ditto --noextattr --noqtn ${SYMROOT}/i386/modules/${choiceId}.dylib ${PKG_BUILD_DIR}/${choiceId}/Root/Extra/modules
            # Adding Keymaps
            ditto --noextattr --noqtn ${SRCROOT}/Keymaps ${PKG_BUILD_DIR}/${choiceId}/Root/Extra/Keymaps
            # Adding tools
            ditto --noextattr --noqtn ${SYMROOT}/i386/cham-mklayout ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin
            echo -e "\t[BUILD] ${choiceId} "
            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/$chamTemp"

            # Don't add a choice for Keylayout module
            # addChoice "${choiceId}"  "start_selected=\"false\""  "$packageRefId"
            # End build Keylayout package module
        }
        fi

        ((xmlindent--))
        outline[${#outline[*]}]="${indent[$xmlindent]}</line>"
    }
    else
    {
        echo "      -= no modules to include =-"
    }
    fi
# End build Modules packages
fi

# build Options packages

    outline[${#outline[*]}]="${indent[$xmlindent]}<line choice=\"Options\">"
    choices[${#choices[*]}]="\t<choice\n\t\tid=\"Options\"\n\t\ttitle=\"Options_title\"\n\t\tdescription=\"Options_description\">\n\t</choice>\n"
    ((xmlindent++))


    # ------------------------------------------------------
    # parse OptionalSettings folder to find files of boot options.
    # ------------------------------------------------------
    OptionalSettingsFolder="${PKGROOT}/OptionalSettings"
    OptionalSettingsFiles=($( find "${OptionalSettingsFolder}" -depth 1 ! -name '.svn' ! -name '.DS_Store' ))

    for (( i = 0 ; i < ${#OptionalSettingsFiles[@]} ; i++ ))
    do

		# Take filename and Strip .txt from end and path from front
		builtOptionsList=$( echo ${OptionalSettingsFiles[$i]%.txt} )
		builtOptionsList=$( echo ${builtOptionsList##*/} )
		echo "================= $builtOptionsList ================="
		outline[${#outline[*]}]="${indent[$xmlindent]}<line choice=\"${builtOptionsList}\">"
		choices[${#choices[*]}]="\t<choice\n\t\tid=\"${builtOptionsList}\"\n\t\ttitle=\"${builtOptionsList}_title\"\n\t\tdescription=\"${builtOptionsList}_description\">\n\t</choice>\n"
		((xmlindent++))
		packagesidentity="${chameleon_package_identity}.options.$builtOptionsList"

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
			mkdir -p "${PKG_BUILD_DIR}/$optionName/Root/"

			# create dummy file with name of key/value
			echo "" > "${PKG_BUILD_DIR}/$optionName/Root/${keyValue}"

			echo -e "\t[BUILD] ${optionName} "
			packageRefId=$(getPackageRefId "${packagesidentity}" "${optionName}")
			buildpackage "$packageRefId" "${optionName}" "${PKG_BUILD_DIR}/${optionName}" "/$chamTemp/options"
			exclusiveSelect=""
			if [[ ${exclusiveFlag} -eq 1 ]];then
				exclusiveSelect="selected=\"$(exclusive_zero_or_one_choice "$optionName" "$allChoices")\""
			fi
			addChoice "${optionName}"  "start_selected=\"false\" ${exclusiveSelect}"  "$packageRefId"
		done

		((xmlindent--))
		outline[${#outline[*]}]="${indent[$xmlindent]}</line>"
	done

if [[ -n "${CONFIG_KEYLAYOUT_MODULE}" ]];then
# build KeyLayout options packages
	echo "================= Keymaps Options ================="
	outline[${#outline[*]}]="${indent[$xmlindent]}<line choice=\"KeyLayout\">"
	choices[${#choices[*]}]="\t<choice\n\t\tid=\"KeyLayout\"\n\t\ttitle=\"KeyLayout_title\"\n\t\tdescription=\"KeyLayout_description\">\n\t</choice>\n"
	((xmlindent++))
	packagesidentity="${chameleon_package_identity}.options.keylayout"
    keylayoutPackageRefId=""
    if [[ "${CONFIG_MODULES}" == 'y' && "${CONFIG_KEYLAYOUT_MODULE}" = 'm' ]];then
        keylayoutPackageRefId=$(getPackageRefId "${modules_packages_identity}" "Keylayout")
    fi

	# ------------------------------------------------------
	# Available Keylayout boot options are discovered by
	# reading contents of /Keymaps folder after compilation
	# ------------------------------------------------------
	availableOptions=($( find "${SRCROOT}/Keymaps" -type f -depth 1 -name '*.lyt' | sed 's|.*/||;s|\.lyt||' ))
	allChoices="${availableOptions[@]}"
	# Adjust array contents to match expected format
	# for boot options which is: name:key=value
	for (( i = 0 ; i < ${#availableOptions[@]} ; i++ )); do
		# availableOptions[i]=${availableOptions[i]}":KeyLayout="${availableOptions[i]}
		# Start build of a keymap package module
		choiceId="${availableOptions[i]}"
		mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root

		# create dummy file with name of key/value
		echo "" > "${PKG_BUILD_DIR}/${choiceId}/Root/KeyLayout=${availableOptions[i]}"

		echo -e "\t[BUILD] ${choiceId} "
		packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
		buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/$chamTemp/options"
		exclusiveChoice=$(exclusive_zero_or_one_choice "$choiceId" "$allChoices")
		# Add the Keylayout package because the Keylayout module is needed
		addChoice "${choiceId}"  "start_selected=\"false\" selected=\"${exclusiveChoice}\""  \
		 "$packageRefId" "$keylayoutPackageRefId"
		# End build uClibc package module
	done

	((xmlindent--))
	outline[${#outline[*]}]="${indent[$xmlindent]}</line>"

# End build KeyLayout options packages
fi
    ((xmlindent--))
	outline[${#outline[*]}]="${indent[$xmlindent]}</line>"
# End build options packages

# build theme packages
	echo "================= Themes ================="
	outline[${#outline[*]}]="${indent[$xmlindent]}<line choice=\"Themes\">"
	choices[${#choices[*]}]="\t<choice\n\t\tid=\"Themes\"\n\t\ttitle=\"Themes_title\"\n\t\tdescription=\"Themes_description\">\n\t</choice>\n"
	((xmlindent++))

	# Using themes section from Azi's/package branch.
	packagesidentity="${chameleon_package_identity}.themes"
	artwork="${SRCROOT}/artwork/themes"
	themes=($( find "${artwork}" -type d -depth 1 -not -name '.svn' ))
	for (( i = 0 ; i < ${#themes[@]} ; i++ )); do
		theme=$( echo ${themes[$i]##*/} | awk 'BEGIN{OFS=FS=""}{$1=toupper($1);print}' )
		mkdir -p "${PKG_BUILD_DIR}/${theme}/Root/"
		rsync -r --exclude=.svn "${themes[$i]}/" "${PKG_BUILD_DIR}/${theme}/Root/${theme}"
		echo -e "\t[BUILD] ${theme}"
		packageRefId=$(getPackageRefId "${packagesidentity}" "${theme}")
		buildpackage "$packageRefId" "${theme}" "${PKG_BUILD_DIR}/${theme}" "/$chamTemp/Extra/Themes"
		addChoice "${theme}"  "start_selected=\"false\""  "$packageRefId"
	done

	((xmlindent--))
	outline[${#outline[*]}]="${indent[$xmlindent]}</line>"
# End build theme packages# End build Extras package

# build post install package
    echo "================= Post ================="
    packagesidentity="${chameleon_package_identity}"
    choiceId="Post"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts
    cp -f ${PKGROOT}/Scripts/Main/postinstall ${PKG_BUILD_DIR}/${choiceId}/Scripts
    cp -f ${PKGROOT}/Scripts/Sub/InstallLog.sh ${PKG_BUILD_DIR}/${choiceId}/Scripts
    cp -f ${PKGROOT}/Scripts/Sub/UnMountEFIvolumes.sh ${PKG_BUILD_DIR}/${choiceId}/Scripts
    ditto --noextattr --noqtn ${SRCROOT}/revision ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources/revision
    ditto --noextattr --noqtn ${SRCROOT}/version ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources/version
    echo -e "\t[BUILD] ${choiceId} "
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice "${choiceId}"  "start_visible=\"false\" start_selected=\"true\""  "$packageRefId"
# End build post install package

	((xmlindent--))
	outline[${#outline[*]}]="${indent[$xmlindent]}</choices-outline>"

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
		set +u # packageSize is optional
        local packageSize="$5"
        set -u

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
    declare -r distributionDestDir="${SYMROOT}"
    declare -r distributionFilename="${packagename// /}-${version}-r${revision}.pkg"
    declare -r distributionFilePath="${distributionDestDir}/${distributionFilename}"

    rm -f "${distributionDestDir}/${packagename// /}"*.pkg

    mkdir -p "${PKG_BUILD_DIR}/${packagename}"

    find "${PKG_BUILD_DIR}" -type f -name '*.pkg' -depth 1 | while read component
	do
        pkg="${component##*/}" # ie: EFI.pkg
        pkgdir="${PKG_BUILD_DIR}/${packagename}/${pkg}"
        # expand individual packages
        pkgutil --expand "${PKG_BUILD_DIR}/${pkg}" "$pkgdir"
        rm -f "${PKG_BUILD_DIR}/${pkg}"
    done

#   Create the Distribution file
    ditto --noextattr --noqtn "${PKGROOT}/Distribution" "${PKG_BUILD_DIR}/${packagename}/Distribution"

    for (( i=0; i < ${#outline[*]} ; i++)); do
		echo -e "${outline[$i]}" >> "${PKG_BUILD_DIR}/${packagename}/Distribution"
	done

    for (( i=0; i < ${#choices[*]} ; i++)); do
		echo -e "${choices[$i]}" >> "${PKG_BUILD_DIR}/${packagename}/Distribution"
	done

    for (( i=0; i < ${#pkgrefs[*]} ; i++)); do
        echo -e "${pkgrefs[$i]}" >> "${PKG_BUILD_DIR}/${packagename}/Distribution"
    done

    echo -e "\n</installer-gui-script>"  >> "${PKG_BUILD_DIR}/${packagename}/Distribution"

#   Create the Resources directory
    ditto --noextattr --noqtn "${PKGROOT}/Resources" "${PKG_BUILD_DIR}/${packagename}/Resources"

#   CleanUp the directory
    find "${PKG_BUILD_DIR}/${packagename}" \( -type d -name '.svn' \) -o -name '.DS_Store' -exec rm -rf {} \;

#   Add Chameleon Version and Revision
    perl -i -p -e "s/%CHAMELEONVERSION%/${version%%-*}/g" $( find "${PKG_BUILD_DIR}/${packagename}/Resources" -type f )
    perl -i -p -e "s/%CHAMELEONREVISION%/${revision}/g"   $( find "${PKG_BUILD_DIR}/${packagename}/Resources" -type f )

#   Add Chameleon Stage
    perl -i -p -e "s/%CHAMELEONSTAGE%/${stage}/g" $( find "${PKG_BUILD_DIR}/${packagename}/Resources" -type f )

#   Adding Developer and credits
    perl -i -p -e "s/%DEVELOP%/${develop}/g" $( find "${PKG_BUILD_DIR}/${packagename}/Resources" -type f )
    perl -i -p -e "s/%CREDITS%/${credits}/g" $( find "${PKG_BUILD_DIR}/${packagename}/Resources" -type f )
    perl -i -p -e "s/%PKGDEV%/${pkgdev}/g"   $( find "${PKG_BUILD_DIR}/${packagename}/Resources" -type f )

#   Create the final package
    pkgutil --flatten "${PKG_BUILD_DIR}/${packagename}" "${distributionFilePath}"

#   Here is the place for assign a Icon to the pkg
    ditto -xk "${PKGROOT}/Icons/pkg.zip" "${PKG_BUILD_DIR}/Icons/"
    DeRez -only icns "${PKG_BUILD_DIR}/Icons/Icons/pkg.icns" > "${PKG_BUILD_DIR}/Icons/tempicns.rsrc"
    Rez -append "${PKG_BUILD_DIR}/Icons/tempicns.rsrc" -o "${distributionFilePath}"
    SetFile -a C "${distributionFilePath}"
    rm -rf "${PKG_BUILD_DIR}/Icons"

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

# build packages
main

# build meta package
makedistribution
