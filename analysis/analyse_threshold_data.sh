#!/bin/bash

INPUT_PATH=$(readlink -f $1)

if [ ! -f "${INPUT_PATH}/run.log" ]
then
    echo "Input folder does not exist!"
    exit
fi

###################################################################################################
# setup environment / load functions
ROOT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd ) # determine where this script is located
source ${ROOT_DIR}/../scripts/common/functions.sh

###################################################################################################
### determine 'run conditions'
eval $(determine_run_conditions)


# is ROOT available?
check_root


# precompile helpers
root -l <<EOF
.L ${ROOT_DIR}/helpers.cpp++g
EOF


###################################################################################################
### process the raw data

for f in $(find ${INPUT_PATH} -name "ThresholdScan_*.dat")
do
    cd $(dirname $f)
    BASENAME=$(echo $(basename $f))
    fconfig="ScanConfig"${BASENAME:13:14}".cfg"
    VCASN=$(cat $fconfig | grep VCASN | grep -v VCASN2 | tr -s ' ' | cut -d ' ' -f2)
    ITHR=$(cat $fconfig | grep ITHR  | cut -f2        | tr -s ' ' | cut -d ' ' -f2)
    root -l -b <<EOF
.L ${ROOT_DIR}/helpers.cpp+g
.x ${ROOT_DIR}/FitThresholds.C+g("${f}", kTRUE, ${ITHR}, ${VCASN}, kTRUE)
EOF
done

###################################################################################################
### summarise the information

SUMMARY_FILE=${INPUT_PATH}/threshold_summary.txt
rm $SUMMARY_FILE

for f in $(find ${INPUT_PATH} -name "ThresholdSummary.dat")
do
    FOLDER=$(dirname $f)
    PARTS=( $(echo $FOLDER | tr '/' ' ') )
    V=""
    VBB=""
    for p in "${PARTS[@]}"
    do
        #echo $p
        if [[ $p == V1* ]]
        then
            V=${p:1}
        elif [[ $p == VBB* ]]
        then
            VBB=${p:3}
        fi
    done
    while read line 
    do
	printf "%f %f %s %s %s %s %s %s %s\n" $V $VBB $line >> ${SUMMARY_FILE}
    done < "$f"
done
cat $SUMMARY_FILE

### generate ROOT
cd ${INPUT_PATH}
root -l <<EOF
TFile* f = new TFile("${INPUT_PATH}/threshold_summary.root", "RECREATE")
TTree* t = new TTree("voltageDependence", "")
t->ReadFile("${SUMMARY_FILE}", "V/F:Vbb/F:Ithr/i:Vcasn/i:Good/L:THR/F:THRrms/F:Noise/F:NoiseRMS/F", ' ');
t->Write();
f->Close();
delete f;
f = 0x0;
EOF

echo "done"
