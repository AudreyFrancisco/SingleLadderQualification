#!/bin/bash

INPUT_FOLDER=$(readlink -f $1)
SUFFIX=$2

#################################################################
### ENVIRONMENT
#################################################################
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$(readlink -f ${SCRIPT_DIR}/../../ )

command -v root >/dev/null 2>&1 || { echo "CERN ROOT not found.  Aborting." >&2; exit 1; }

cd ${INPUT_FOLDER}

OUTFILE=DAC_summary_${SUFFIX}.dat
echo -e "# DAC\tChip\tV\tDAC value\tADC value" > ${OUTFILE}

for f in $(ls *DAC_*_${SUFFIX}.dat | grep -v summary )
do
    DAC=$(echo $f   | cut -d'_' -f2 )
    CHIP=$(echo $f  | cut -d'_' -f3 )
    CHIP=${CHIP:4}
    VOLT=$(echo $f  | cut -d'_' -f5 )
    VOLT=${VOLT::-1}

    while IFS= read LINE
    do
	echo "${DAC} ${CHIP} ${VOLT} ${LINE}" >> ${OUTFILE}
    done < "$f"
done

DACS=$(cat ${OUTFILE}       | grep -v "\#" | cut -f1 -d' ' | uniq       | tr '\n' ' ' )
CHIPS=$(cat ${OUTFILE}      | grep -v "\#" | cut -f2 -d' ' | sort -u -n | tr '\n' ' ' )
VOLTAGES=$(cat ${OUTFILE}   | grep -v "\#" | cut -f3 -d' ' | sort -u -n | tr '\n' ' ' )
DAC_VALUES=$(cat ${OUTFILE} | grep -v "\#" | cut -f4 -d' ' | sort -u -n | tr '\n' ' ' )

echo ${DACS}
echo ${CHIPS}
echo ${VOLTAGES}
echo ${DAC_VALUES}

