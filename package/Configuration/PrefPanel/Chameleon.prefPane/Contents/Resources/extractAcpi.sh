#/bin/sh
set -e
set -u
ACPI_DIR="${HOME}/Desktop/ACPI"
ioreg=
if [[ $# -eq 1 && -f "$1" ]]; then
    ioreg="$(grep ' "ACPI Tables" =' "$1")"
else
    ioreg="$(ioreg -lw0 | grep ' "ACPI Tables" =')"
fi

ioreg=${ioreg#*\{}
ioreg=${ioreg%\}*}

declare -a tables
ioreg="${ioreg//,/ }"

tables=($ioreg)

echo "Number of ACPI tables: ${#tables[@]}"
re='"([^"]+)"=<([^>]+)>'
dumped=0
for t in "${tables[@]}"; do
    #echo Table: $t
    if [[ $t =~ $re ]]; then
        [[ $dumped = 0 ]] && mkdir -p ${ACPI_DIR}
        ((++dumped))
        echo
        echo "Dumping table: ${BASH_REMATCH[1]}"
        #echo "Content: ${BASH_REMATCH[2]}"
        echo "${BASH_REMATCH[2]}" | xxd -r -p > "${ACPI_DIR}/${BASH_REMATCH[1]}".aml
        echo "AML code dumped to \"${ACPI_DIR}/${BASH_REMATCH[1]}.aml\""
        type -p iasl &>/dev/null && iasl -d "${ACPI_DIR}/${BASH_REMATCH[1]}".aml \
        && echo "DSL code decompiled to \"${ACPI_DIR}/${BASH_REMATCH[1]}.dsl\""
        echo
    fi
done
#if [[ $dumped -gt 0 ]]; then
#    zip -r ACPI.zip ACPI && echo "Zipped your ACPI tables in file \"ACPI.zip\""
#fi
