#!/bin/bash

# $0 SRCROOT directory
# $1 SYMROOT directory
# $2 directory where pkgs will be created

# Directory paths
declare -r PKGROOT="${0%/*}"
declare -r SRCROOT="$1"
declare -r SYMROOT="$2"
declare -r PKG_BUILD_DIR="$3"
declare -r SCPT_TPL_DIR="${PKGROOT}/Scripts.templates"

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
declare -r CHAMELEON_VERSION=$( cat version )

# stage
CHAMELEON_STAGE=${CHAMELEON_VERSION##*-}
CHAMELEON_STAGE=${CHAMELEON_STAGE/RC/Release Candidate }
CHAMELEON_STAGE=${CHAMELEON_STAGE/FINAL/2.1 Final}
declare -r CHAMELEON_STAGE

declare -r CHAMELEON_REVISION=$( grep I386BOOT_CHAMELEONREVISION vers.h | awk '{ print $3 }' | tr -d '\"' )
declare -r CHAMELEON_BUILDDATE=$( grep I386BOOT_BUILDDATE vers.h | awk '{ print $3,$4 }' | tr -d '\"' )
declare -r CHAMELEON_TIMESTAMP=$( date -j -f "%Y-%m-%d %H:%M:%S" "${CHAMELEON_BUILDDATE}" "+%s" )

# ====== CREDITS ======
declare -r CHAMELEON_DEVELOP=$(awk "NR==6{print;exit}"  ${PKGROOT}/../CREDITS)
declare -r CHAMELEON_CREDITS=$(awk "NR==10{print;exit}" ${PKGROOT}/../CREDITS)
declare -r CHAMELEON_PKGDEV=$(awk "NR==14{print;exit}"  ${PKGROOT}/../CREDITS)
declare -r CHAMELEON_CPRYEAR=$(awk "NR==18{print;exit}"  ${PKGROOT}/../CREDITS)
if [[ $(whoami | awk '{print $1}' | cut -d ":" -f3) == "admin" ]];then
    declare -r CHAMELEON_WHOBUILD="VoodooLabs BuildBot"
else
    declare -r CHAMELEON_WHOBUILD=$(whoami | awk '{print $1}' | cut -d ":" -f3)
fi

# ====== GLOBAL VARIABLES ======
declare -r LOG_FILENAME="Chameleon_Installer_Log.txt"

declare -a chameleonOptionType
declare -a chameleonOptionKey
declare -a chameleonOptionValues

declare -a pkgrefs
declare -a choice_key
declare -a choice_options
declare -a choice_pkgrefs
declare -a choice_parent_group_index
declare -a choice_group_items
declare -a choice_group_exclusive

# Init Main Group
choice_key[0]=""
choice_options[0]=""
choices_pkgrefs[0]=""
choice_group_items[0]=""
choice_group_exclusive[0]=""

# Package name
declare -r packagename="Chameleon"

# Package identifiers
declare -r chameleon_package_identity="org.chameleon"
declare -r modules_packages_identity="${chameleon_package_identity}.modules"

# ====== FUNCTIONS ======
trim () {
    local result="${1#"${1%%[![:space:]]*}"}"   # remove leading whitespace characters
    echo "${result%"${result##*[![:space:]]}"}" # remove trailing whitespace characters
}

function makeSubstitutions () {
    # Substition is like: Key=Value
    #
    # Optional arguments:
    #    --subst=<substition> : add a new substitution
    #
    # Last argument(s) is/are file(s) where substitutions must be made

    local ownSubst=""

    function addSubst () {
        local mySubst="$1"
        case "$mySubst" in
			*=*) keySubst=${mySubst%%=*}
				 valSubst=${mySubst#*=}
				 ownSubst=$(printf "%s\n%s" "$ownSubst" "s&@$keySubst@&$valSubst&g;t t")
				 ;;
			*) echo "Invalid substitution $mySubst" >&2
               exit 1
               ;;
		esac
    }

    # Check the arguments.
    while [[ $# -gt 0 ]];do
        local option="$1"
        case "$option" in
            --subst=*) shift; addSubst "${option#*=}" ;;
            -*)
                echo "Unrecognized makeSubstitutions option '$option'" >&2
                exit 1
                ;;
            *)  break ;;
        esac
    done

    if [[ $# -lt 1 ]];then
        echo "makeSubstitutions invalid number of arguments: at least one file needed" >&2
        exit 1
    fi

    local chameleonSubsts="
s&%CHAMELEONVERSION%&${CHAMELEON_VERSION%%-*}&g
s&%CHAMELEONREVISION%&${CHAMELEON_REVISION}&g
s&%CHAMELEONSTAGE%&${CHAMELEON_STAGE}&g
s&%DEVELOP%&${CHAMELEON_DEVELOP}&g
s&%CREDITS%&${CHAMELEON_CREDITS}&g
s&%PKGDEV%&${CHAMELEON_PKGDEV}&g
s&%CPRYEAR%&${CHAMELEON_CPRYEAR}&g
s&%WHOBUILD%&${CHAMELEON_WHOBUILD}&g
:t
/@[a-zA-Z_][a-zA-Z_0-9]*@/!b
s&@LOG_FILENAME@&${LOG_FILENAME}&g;t t"

    local allSubst="
$chameleonSubsts
$ownSubst"

    for file in "$@";do
        cp -pf "$file" "${file}.in"
        sed "$allSubst" "${file}.in" > "${file}"
        rm -f "${file}.in"
    done
}

addTemplateScripts () {
    # Arguments:
    #    --pkg-rootdir=<pkg_rootdir> : path of the pkg root dir
    #
    # Optional arguments:
    #    --subst=<substition> : add a new substitution
    #
    # Substition is like: Key=Value
    #
    # $n : Name of template(s) (templates are in package/Scripts.templates

    local pkgRootDir=""
    declare -a allSubst

    # Check the arguments.
    while [[ $# -gt 0 ]];do
        local option="$1"
        case "$option" in
            --pkg-rootdir=*)   shift; pkgRootDir="${option#*=}" ;;
            --subst=*) shift; allSubst[${#allSubst[*]}]="${option}" ;;
            -*)
                echo "Unrecognized addTemplateScripts option '$option'" >&2
                exit 1
                ;;
            *)  break ;;
        esac
    done
    if [[ $# -lt 1 ]];then
        echo "addTemplateScripts invalid number of arguments: you must specify a template name" >&2
        exit 1
    fi
    [[ -z "$pkgRootDir" ]] && { echo "Error addTemplateScripts: --pkg-rootdir option is needed" >&2 ; exit 1; }
    [[ ! -d "$pkgRootDir" ]] && { echo "Error addTemplateScripts: directory '$pkgRootDir' doesn't exists" >&2 ; exit 1; }

    for templateName in "$@";do
        local templateRootDir="${SCPT_TPL_DIR}/${templateName}"
        [[ ! -d "$templateRootDir" ]] && {
            echo "Error addTemplateScripts: template '$templateName' doesn't exists" >&2; exit 1; }

        # Copy files to destination
        rsync -pr --exclude=.svn --exclude="*~" "$templateRootDir/" "$pkgRootDir/Scripts/"
    done

    files=$( find "$pkgRootDir/Scripts/" -type f )
    if [[ ${#allSubst[*]} -gt 0 ]];then
        makeSubstitutions "${allSubst[@]}" $files
    else
        makeSubstitutions $files
    fi
}

getPackageRefId () {
    echo ${1//_/.}.${2//_/.} | tr [:upper:] [:lower:]
}

# Return index of a choice
getChoiceIndex () {
    # $1 Choice Id
    local found=0
    for (( idx=0 ; idx < ${#choice_key[*]}; idx++ ));do
        if [[ "${1}" == "${choice_key[$idx]}" ]];then
            found=1
            break
        fi
    done
    echo "$idx"
    return $found
}

# Add a new choice
addChoice () {
    # Optional arguments:
    #    --group=<group> : Group Choice Id
    #    --start-selected=<javascript code> : Specifies whether this choice is initially selected or unselected
    #    --start-enabled=<javascript code>  : Specifies the initial enabled state of this choice
    #    --start-visible=<javascript code>  : Specifies whether this choice is initially visible
    #    --pkg-refs=<pkgrefs> : List of package reference(s) id (separate by spaces)
    #
    # $1 Choice Id

    local option
    local groupChoice=""
    local choiceOptions=""
    local pkgrefs=""

    # Check the arguments.
    for option in "${@}";do
        case "$option" in
            --group=*)
                       shift; groupChoice=${option#*=} ;;
            --start-selected=*)
                         shift; choiceOptions="$choiceOptions start_selected=\"${option#*=}\"" ;;
            --start-enabled=*)
                         shift; choiceOptions="$choiceOptions start_enabled=\"${option#*=}\"" ;;
            --start-visible=*)
                         shift; choiceOptions="$choiceOptions start_visible=\"${option#*=}\"" ;;
            --pkg-refs=*)
                          shift; pkgrefs=${option#*=} ;;
            -*)
                echo "Unrecognized addChoice option '$option'" >&2
                exit 1
                ;;
            *)  break ;;
        esac
    done

    if [[ $# -ne 1 ]];then
        echo "addChoice invalid number of arguments: ${@}" >&2
        exit 1
    fi

    local choiceId="${1}"

    # Add choice in the group
    idx_group=$(getChoiceIndex "$groupChoice")
    found_group=$?
    if [[ $found_group -ne 1 ]];then
        # No group exist
        echo "Error can't add choice '$choiceId' to group '$groupChoice': group choice '$groupChoice' doesn't exists." >&2
        exit 1
    else
        set +u; oldItems=${choice_group_items[$idx_group]}; set -u
        choice_group_items[$idx_group]="$oldItems $choiceId"
    fi

    # Check that the choice doesn't already exists
    idx=$(getChoiceIndex "$choiceId")
    found=$?
    if [[ $found -ne 0 ]];then
        # Choice already exists
        echo "Error can't add choice '$choiceId': a choice with same name already exists." >&2
        exit 1
    fi

    # Record new node
    choice_key[$idx]="$choiceId"
    choice_options[$idx]=$(trim "${choiceOptions}") # Removing leading and trailing whitespace(s)
    choice_parent_group_index[$idx]=$idx_group
    choice_pkgrefs[$idx]="$pkgrefs"

    return $idx
}

# Add a group choice
addGroupChoices() {
    # Optional arguments:
    #    --parent=<parent> : parent group choice id
    #    --exclusive_zero_or_one_choice : only zero or one choice can be selected in the group
    #    --exclusive_one_choice : only one choice can be selected in the group
    #
    # $1 Choice Id

    local option
    local groupChoice=""
    local exclusive_function=""

    for option in "${@}";do
        case "$option" in
            --exclusive_zero_or_one_choice)
                       shift; exclusive_function="exclusive_zero_or_one_choice" ;;
            --exclusive_one_choice)
                       shift; exclusive_function="exclusive_one_choice" ;;
            --parent=*)
                       shift; groupChoice=${option#*=} ;;
            -*)
                echo "Unrecognized addGroupChoices option '$option'" >&2
                exit 1
                ;;
            *)  break ;;
        esac
    done

    if [[ $# -ne 1 ]];then
        echo "addGroupChoices invalid number of arguments: ${@}" >&2
        exit 1
    fi

    addChoice --group="$groupChoice" "${1}"
    local idx=$? # index of the new created choice

    choice_group_exclusive[$idx]="$exclusive_function"
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

recordChameleonOption () {
    local type="$1"
    local key="$2"
    local value="$3"

    # Search for an existing key
    local found=0
    for (( idx=0 ; idx < ${#chameleonOptionKey[@]}; idx++ ));do
        if [[ "$key" == "${chameleonOptionKey[$idx]}" ]];then
            found=1
            break
        fi
    done

    # idx contain the index of a new key or an existing one
    if [[ $found -eq 0 ]]; then
        # Create a new one
        chameleonOptionKey[$idx]="$key"
        chameleonOptionType[$idx]="$type"
        chameleonOptionValues[$idx]=""
    fi

    case "$type" in
        bool) ;;
        text|list) chameleonOptionValues[$idx]="${chameleonOptionValues[$idx]} $value" ;;
        *) echo "Error unknown type '$type' for '$key'" >&2
           exit 1
           ;;
    esac
}

generate_options_yaml_file () {
    local yamlFile="$1"
    echo "---" > $yamlFile
    for (( idx=0; idx < ${#chameleonOptionKey[@]}; idx++ ));do
        printf "%s:\n  type: %s\n" "${chameleonOptionKey[$idx]}" "${chameleonOptionType[$idx]}" >> $yamlFile
        case ${chameleonOptionType[$idx]} in
            text|list)
                printf "  values:\n" >> $yamlFile
                for value in ${chameleonOptionValues[$idx]};do
                    printf  "    - %s\n" "$value" >> $yamlFile
                done
                ;;
        esac
    done
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

# Add pre install choice
    echo "================= Preinstall ================="
    packagesidentity="${chameleon_package_identity}"
    choiceId="Pre"

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    addChoice --start-visible="false" --start-selected="true"  --pkg-refs="$packageRefId" "${choiceId}"

    # Package will be built at the end
# End pre install choice

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

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="false" --start-selected="true" --pkg-refs="$packageRefId" "${choiceId}"
# End build core package

# build Chameleon package
    echo "================= Chameleon ================="
    addGroupChoices --exclusive_one_choice "Chameleon"

    # build standard package
    choiceId="Standard"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" InstallerLog
    cp -f ${PKGROOT}/Scripts/Main/${choiceId}postinstall ${PKG_BUILD_DIR}/${choiceId}/Scripts/postinstall
    cp -f ${PKGROOT}/Scripts/Sub/* ${PKG_BUILD_DIR}/${choiceId}/Scripts
    ditto --arch i386 `which SetFile` ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources/SetFile

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --group="Chameleon"  --start-selected="true"  --pkg-refs="$packageRefId" "${choiceId}"
    # End build standard package

    # build efi package
if [[ 1 -eq 0 ]];then
    # Only standard installation is currently supported
    # We need to update the script to be able to install
    # Chameleon on EFI partition
    choiceId="EFI"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" InstallerLog
    cp -f ${PKGROOT}/Scripts/Main/ESPpostinstall ${PKG_BUILD_DIR}/${choiceId}/Scripts/postinstall
    cp -f ${PKGROOT}/Scripts/Sub/* ${PKG_BUILD_DIR}/${choiceId}/Scripts
    ditto --arch i386 `which SetFile` ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources/SetFile

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --group="Chameleon"  --start-visible="systemHasGPT()" --start-selected="false"  --pkg-refs="$packageRefId" "${choiceId}"
    # End build efi package
fi
    # build no bootloader choice package
    choiceId="noboot"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --group="Chameleon"  --start-selected="false"  --pkg-refs="$packageRefId" "${choiceId}"
    # End build no bootloader choice package

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
        addGroupChoices "Module"

# -
        if [[ "${CONFIG_RESOLUTION_MODULE}" == 'm' && -f "${SYMROOT}/i386/modules/Resolution.dylib" ]]; then
        {
            # Start build Resolution package module
            choiceId="AutoReso"
            moduleFile="Resolution.dylib"
            mkdir -p "${PKG_BUILD_DIR}/${choiceId}/Root"
            ditto --noextattr --noqtn "${SYMROOT}/i386/modules/$moduleFile" "${PKG_BUILD_DIR}/${choiceId}/Root"
            addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                               --subst="moduleName=$choiceId"               \
                               --subst="moduleFile=$moduleFile"             \
                               InstallModule

            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/Extra/modules"
            addChoice --group="Module"  --start-selected="false"  --pkg-refs="$packageRefId" "${choiceId}"
            # End build Resolution package module
        }
        fi

# -
        if [[ "${CONFIG_KLIBC_MODULE}" == 'm' && -f "${SYMROOT}/i386/modules/klibc.dylib" ]]; then
        {
            # Start build klibc package module
            choiceId="klibc"
            moduleFile="${choiceId}.dylib"
            mkdir -p "${PKG_BUILD_DIR}/${choiceId}/Root"
            ditto --noextattr --noqtn "${SYMROOT}/i386/modules/$moduleFile" ${PKG_BUILD_DIR}/${choiceId}/Root
            addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                               --subst="moduleName=$choiceId"               \
                               --subst="moduleFile=$moduleFile"             \
                               InstallModule

            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/Extra/modules"
            addChoice --group="Module"  --start-selected="false"  --pkg-refs="$packageRefId" "${choiceId}"
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
            moduleFile="uClibcxx.dylib"
            mkdir -p "${PKG_BUILD_DIR}/${choiceId}/Root"
            ditto --noextattr --noqtn "${SYMROOT}/i386/modules/$moduleFile" "${PKG_BUILD_DIR}/${choiceId}/Root"
            addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                               --subst="moduleName=$choiceId"               \
                               --subst="moduleFile=$moduleFile"             \
                               InstallModule

            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/Extra/modules"
            # Add the klibc package because the uClibc module is dependent of klibc module
            addChoice --group="Module"  --start-selected="false"  --pkg-refs="$packageRefId $klibcPackageRefId" "${choiceId}"
            # End build uClibc package module
        }
        fi

# -
        # Warning Keylayout module need additional files
        if [[ "${CONFIG_KEYLAYOUT_MODULE}" = 'm' && -f "${SYMROOT}/i386/modules/Keylayout.dylib" ]]; then
        {
            # Start build Keylayout package module
            choiceId="Keylayout"
            moduleFile="${choiceId}.dylib"
            mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/Extra/{modules,Keymaps}
            mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin
            layout_src_dir="${SRCROOT}/i386/modules/Keylayout/layouts/layouts-src"
            if [ -d "$layout_src_dir" ];then
                # Create a tar.gz from layout sources
                (cd "$layout_src_dir"; \
                    tar czf "${PKG_BUILD_DIR}/${choiceId}/Root/Extra/Keymaps/layouts-src.tar.gz" README *.slt)
            fi
            # Adding module
            ditto --noextattr --noqtn ${SYMROOT}/i386/modules/$moduleFile ${PKG_BUILD_DIR}/${choiceId}/Root/Extra/modules
            # Adding Keymaps
            ditto --noextattr --noqtn ${SRCROOT}/Keymaps ${PKG_BUILD_DIR}/${choiceId}/Root/Extra/Keymaps
            # Adding tools
            ditto --noextattr --noqtn ${SYMROOT}/i386/cham-mklayout ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin
            # Adding scripts
            addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                               --subst="moduleName=$choiceId"               \
                               --subst="moduleFile=$moduleFile"             \
                               InstallModule

            packageRefId=$(getPackageRefId "${modules_packages_identity}" "${choiceId}")
            buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"

            # Don't add a choice for Keylayout module
            # addChoice "${choiceId}" "Module" --start-selected="false"  "$packageRefId"
            # End build Keylayout package module
        }
        fi

    }
    else
    {
        echo "      -= no modules to include =-"
    }
    fi
# End build Modules packages
fi

# build Options packages

    addGroupChoices "Options"

    # ------------------------------------------------------
    # parse OptionalSettings folder to find files of boot options.
    # ------------------------------------------------------
    OptionalSettingsFolder="${PKGROOT}/OptionalSettings"

    while IFS= read -r -d '' OptionsFile; do

        # Take filename and strip .txt from end and path from front
        builtOptionsList=${OptionsFile%.txt}
        builtOptionsList=${builtOptionsList##*/}
        packagesidentity="${chameleon_package_identity}.options.$builtOptionsList"

        echo "================= $builtOptionsList ================="

        # ------------------------------------------------------
        # Read boot option file into an array.
        # ------------------------------------------------------
        availableOptions=() # array to hold the list of boot options, per 'section'.
        exclusiveFlag=""    # used to indicate list has exclusive options
        while read textLine; do
            # ignore lines in the file beginning with a #
            [[ $textLine = \#* ]] && continue
            local optionName="" key="" value=""
            case "$textLine" in
                Exclusive=[Tt][Rr][Uu][Ee]) exclusiveFlag="--exclusive_zero_or_one_choice" ;;
                Exclusive=*) continue ;;
                *@*:*=*)
                   availableOptions[${#availableOptions[*]}]="$textLine" ;;
                *) echo "Error: invalid line '$textLine' in file '$OptionsFile'" >&2
                   exit 1
                   ;;
            esac
        done < "$OptionsFile"

        addGroupChoices  --parent="Options" $exclusiveFlag "${builtOptionsList}"

        # ------------------------------------------------------
        # Loop through options in array and process each in turn
        # ------------------------------------------------------
        for textLine in "${availableOptions[@]}"; do
            # split line - taking all before ':' as option name
            # and all after ':' as key/value
            type=$( echo "${textLine%%@*}" | tr '[:upper:]' '[:lower:]' )
            tmp=${textLine#*@}
            optionName=${tmp%%:*}
            keyValue=${tmp##*:}
            key=${keyValue%%=*}
            value=${keyValue#*=}

            # create folders required for each boot option
            mkdir -p "${PKG_BUILD_DIR}/$optionName/Root/"

            case "$type" in
                bool) startSelected="check_chameleon_bool_option('$key','$value')" ;;
                text) startSelected="check_chameleon_text_option('$key','$value')" ;;
                list) startSelected="check_chameleon_list_option('$key','$value')" ;;
                *) echo "Error: invalid type '$type' in line '$textLine' in '$OptionsFile'" >&2
                   exit 1
                   ;;
            esac
            recordChameleonOption "$type" "$key" "$value"
            addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/$optionName" \
                               --subst="optionType=$type"                   \
                               --subst="optionKey=$key"                     \
                               --subst="optionValue=$value"                 \
                               AddOption
            packageRefId=$(getPackageRefId "${packagesidentity}" "${optionName}")
            buildpackage "$packageRefId" "${optionName}" "${PKG_BUILD_DIR}/${optionName}" "/"
            addChoice --group="${builtOptionsList}"  \
                --start-selected="$startSelected"    \
                --pkg-refs="$packageRefId" "${optionName}"
        done

    done < <( find "${OptionalSettingsFolder}" -depth 1 -type f -name '*.txt' -print0 )

# End build options packages

if [[ -n "${CONFIG_KEYLAYOUT_MODULE}" ]];then
# build Keymaps options packages
    echo "================= Keymaps Options ================="
    addGroupChoices --exclusive_zero_or_one_choice "Keymaps"
    packagesidentity="${chameleon_package_identity}.options.keymaps"
    keylayoutPackageRefId=""
    if [[ "${CONFIG_MODULES}" == 'y' && "${CONFIG_KEYLAYOUT_MODULE}" = 'm' ]];then
        keylayoutPackageRefId=$(getPackageRefId "${modules_packages_identity}" "Keylayout")
    fi

    chameleon_keylayout_key="KeyLayout"
    # ------------------------------------------------------
    # Available Keylayout boot options are discovered by
    # reading contents of /Keymaps folder after compilation
    # ------------------------------------------------------
    availableOptions=($( find "${SRCROOT}/Keymaps" -type f -depth 1 -name '*.lyt' | sed 's|.*/||;s|\.lyt||' ))
    # Adjust array contents to match expected format
    # for boot options which is: name:key=value
    for (( i = 0 ; i < ${#availableOptions[@]} ; i++ )); do
        # Start build of a keymap package module
        choiceId="${availableOptions[i]}"
        mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root

        recordChameleonOption "text" "$chameleon_keylayout_key" "$choiceId"
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                           --subst="optionType=text"                    \
                           --subst="optionKey=$chameleon_keylayout_key" \
                           --subst="optionValue=$choiceId"              \
                           AddOption
        packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
        buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
        # Add the Keylayout package because the Keylayout module is needed
        addChoice --group="Keymaps"  \
            --start-selected="check_chameleon_text_option('${chameleon_keylayout_key}','${choiceId}')" \
            --pkg-refs="$packageRefId $keylayoutPackageRefId" "${choiceId}"
    done

# End build Keymaps options packages
fi

# build theme packages
    echo "================= Themes ================="
    addGroupChoices "Themes"

    # Using themes section from Azi's/package branch.
    packagesidentity="${chameleon_package_identity}.themes"
    artwork="${SRCROOT}/artwork/themes"
    themes=($( find "${artwork}" -type d -depth 1 -not -name '.svn' ))
    for (( i = 0 ; i < ${#themes[@]} ; i++ )); do
        themeName=$( echo ${themes[$i]##*/} | awk 'BEGIN{OFS=FS=""}{$1=toupper($1);print}' )
        themeDir="$themeName"
        mkdir -p "${PKG_BUILD_DIR}/${themeName}/Root/"
        rsync -r --exclude=.svn --exclude="*~" "${themes[$i]}/" "${PKG_BUILD_DIR}/${themeName}/Root/${themeName}"
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${themeName}" \
                           --subst="themeName=$themeName"                \
                           --subst="themeDir=$themeDir"                  \
                           InstallTheme

        packageRefId=$(getPackageRefId "${packagesidentity}" "${themeName}")
        buildpackage "$packageRefId" "${themeName}" "${PKG_BUILD_DIR}/${themeName}" "/Extra/Themes"
        addChoice --group="Themes"  --start-selected="false"  --pkg-refs="$packageRefId"  "${themeName}"
    done
# End build theme packages# End build Extras package

# build pre install package
    echo "================= Pre ================="
    packagesidentity="${chameleon_package_identity}"
    choiceId="Pre"

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts/Resources
    local yamlFile="Resources/chameleon_options.yaml"
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                       --subst="YAML_FILE=${yamlFile}" ${choiceId}
    generate_options_yaml_file "${PKG_BUILD_DIR}/${choiceId}/Scripts/$yamlFile"
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
# End build pre install package

# build post install package
    echo "================= Post ================="
    packagesidentity="${chameleon_package_identity}"
    choiceId="Post"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}
    cp -f ${PKGROOT}/Scripts/Sub/UnMountEFIvolumes.sh ${PKG_BUILD_DIR}/${choiceId}/Scripts

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice  --start-visible="false" --start-selected="true"  --pkg-refs="$packageRefId" "${choiceId}"
# End build post install package

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

        echo -e "\t[BUILD] ${packageName}"

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
        header+="version=\"${CHAMELEON_VERSION}\" "

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
        pkgrefs[${#pkgrefs[*]}]="\t<pkg-ref id=\"${packageRefId}\" installKBytes='${installedsize}' version='${CHAMELEON_VERSION}.0.0.${CHAMELEON_TIMESTAMP}'>#${packageName}.pkg</pkg-ref>"

        rm -rf "${packagePath}"
    fi
}

generateOutlineChoices() {
    # $1 Main Choice
    # $2 indent level
    local idx=$(getChoiceIndex "$1")
    local indentLevel="$2"
    local indentString=""
    for ((level=1; level <= $indentLevel ; level++)); do
        indentString="\t$indentString"
    done
    set +u; subChoices="${choice_group_items[$idx]}"; set -u
    if [[ -n "${subChoices}" ]]; then
        # Sub choices exists
        echo -e "$indentString<line choice=\"$1\">"
        for subChoice in $subChoices;do
            generateOutlineChoices $subChoice $(($indentLevel+1))
        done
        echo -e "$indentString</line>"
    else
        echo -e "$indentString<line choice=\"$1\"/>"
    fi
}

generateChoices() {
    for (( idx=1; idx < ${#choice_key[*]} ; idx++)); do
        local choiceId=${choice_key[$idx]}
        local choiceOptions=${choice_options[$idx]}
        local choiceParentGroupIndex=${choice_parent_group_index[$idx]}
        set +u; local group_exclusive=${choice_group_exclusive[$choiceParentGroupIndex]}; set -u

        # Create the node and standard attributes
        local choiceNode="\t<choice\n\t\tid=\"${choiceId}\"\n\t\ttitle=\"${choiceId}_title\"\n\t\tdescription=\"${choiceId}_description\""

        # Add options like start_selected, etc...
        [[ -n "${choiceOptions}" ]] && choiceNode="${choiceNode}\n\t\t${choiceOptions}"

        # Add the selected attribute if options are mutually exclusive
        if [[ -n "$group_exclusive" ]];then
            local group_items="${choice_group_items[$choiceParentGroupIndex]}"
            case $group_exclusive in
                exclusive_one_choice)
                    local selected_option=$(exclusive_one_choice "$choiceId" "$group_items") ;;
                exclusive_zero_or_one_choice)
                    local selected_option=$(exclusive_zero_or_one_choice "$choiceId" "$group_items") ;;
                *) echo "Error: unknown function to generate exclusive mode '$group_exclusive' for group '${choice_key[$choiceParentGroupIndex]}'" >&2
                   exit 1
                   ;;
            esac
            choiceNode="${choiceNode}\n\t\tselected=\"$selected_option\""
        fi

        choiceNode="${choiceNode}>"

        # Add the package references
        for pkgRefId in ${choice_pkgrefs[$idx]};do
            choiceNode="${choiceNode}\n\t\t<pkg-ref id=\"${pkgRefId}\"/>"
        done

        # Close the node
        choiceNode="${choiceNode}\n\t</choice>\n"

        echo -e "$choiceNode"
    done
}

makedistribution ()
{
    declare -r distributionDestDir="${SYMROOT}"
    declare -r distributionFilename="${packagename// /}-${CHAMELEON_VERSION}-r${CHAMELEON_REVISION}.pkg"
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

    local start_indent_level=2
    echo -e "\n\t<choices-outline>" >> "${PKG_BUILD_DIR}/${packagename}/Distribution"
    for main_choice in ${choice_group_items[0]};do
        generateOutlineChoices $main_choice $start_indent_level >> "${PKG_BUILD_DIR}/${packagename}/Distribution"
    done
    echo -e "\t</choices-outline>\n" >> "${PKG_BUILD_DIR}/${packagename}/Distribution"

    generateChoices >> "${PKG_BUILD_DIR}/${packagename}/Distribution"

    for (( i=0; i < ${#pkgrefs[*]} ; i++)); do
        echo -e "${pkgrefs[$i]}" >> "${PKG_BUILD_DIR}/${packagename}/Distribution"
    done

    echo -e "\n</installer-gui-script>"  >> "${PKG_BUILD_DIR}/${packagename}/Distribution"

#   Create the Resources directory
    ditto --noextattr --noqtn "${PKGROOT}/Resources" "${PKG_BUILD_DIR}/${packagename}/Resources"

#   CleanUp the directory
    find "${PKG_BUILD_DIR}/${packagename}" \( -type d -name '.svn' \) -o -name '.DS_Store' -exec rm -rf {} \;

    # Make substitutions for version, revision, stage, developers, credits, etc..
    makeSubstitutions $( find "${PKG_BUILD_DIR}/${packagename}/Resources" -type f )

#   Create the final package
    pkgutil --flatten "${PKG_BUILD_DIR}/${packagename}" "${distributionFilePath}"

#   Here is the place to assign an icon to the pkg
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
    echo -e $COL_BLUE"  Version:      "$COL_RESET"$CHAMELEON_VERSION"
    echo -e $COL_BLUE"  Stage:        "$COL_RESET"$CHAMELEON_STAGE"
    echo -e $COL_BLUE"  Date/Time:    "$COL_RESET"$CHAMELEON_BUILDDATE"
    echo -e $COL_BLUE"  Built by:     "$COL_RESET"$CHAMELEON_WHOBUILD"
    echo -e $COL_BLUE"  Copyright $CHAMELEON_CPRYEAR ""$COL_RESET"
    echo ""

}

# build packages
main

# build meta package
makedistribution
