#!/bin/bash

INPUT_FOLDER=$(readlink -f $1)

#################################################################
### ENVIRONMENT
#################################################################
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$(readlink -f ${SCRIPT_DIR}/../../ )

command -v root >/dev/null 2>&1 || { echo "CERN ROOT not found.  Aborting." >&2; exit 1; }

#cd $(readlink -f ${SCRIPT_DIR}/../)
#find ${INPUT_FOLDER} -name "ThresholdScan*Chip0_0.dat" -exec root -l -b -q All_FitThresholdsIB.C+g'("'{}'")' \;


for i in $(seq 0 8)
do
    rm -f ${INPUT_FOLDER}/vgo_summary_chip${i}.txt;
    rm -f ${INPUT_FOLDER}/vbb_summary_chip${i}.txt;
    find ${INPUT_FOLDER}/VGO* -name "ThresholdSummary*Chip0_0.dat" -exec bash -c "echo -e {} | cut -d '/' -f8 | cut -d'O' -f2 | tr -d '\n'; head -n1 {}"  \; >> ${INPUT_FOLDER}/vgo_summary_chip${i}.txt
    find ${INPUT_FOLDER}/VBB* -name "ThresholdSummary*Chip0_0.dat" -exec bash -c "echo -e {} | cut -d '/' -f8 | cut -d'B' -f3 | tr -d '\n'; head -n1 {}"  \; >> ${INPUT_FOLDER}/vbb_summary_chip${i}.txt
done
