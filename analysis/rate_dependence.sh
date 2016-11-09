#!/bin/bash

INPUT_PATH=$(readlink -f $1)

if [ ! -d "${INPUT_PATH}/data" ]
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

for f in $(find ${INPUT_PATH} -name "NoiseOccupancyExt_*.dat")
do
    root -l -b <<EOF
.L ${ROOT_DIR}/helpers.cpp+g
.x ${ROOT_DIR}/NoiseOccupancyRawToHisto.C("${f}")
EOF
done

###################################################################################################
### summarise the information

chip_name=$(find ${INPUT_PATH} -maxdepth 1 -type f -empty)
chip_name=${chip_name##*/}
FILE=${INPUT_PATH}/fhr_summary.txt

echo -e "#Name\tVbb\tIthr\tVcasn\tMode\tFreq\tStrobe\t0\t20\t50\t200\t500" > ${FILE}
for f in $(find ${INPUT_PATH} -name "fhr.txt")
do
    IFS='/' read -r -a folders <<< "${f}"
    for element in ${folders[@]}
    do
        case ${element} in
            "VBB-"*)
                vbb=${element:4}
                vbb=$(printf "%0.1f" $vbb)
                ;;
            "ITHR"*)
                ithr=${element:4}
                ithr=$(printf "%d" $(echo $ithr | sed 's/^0*//'))
                ;;
            "VCASN"*)
                vcasn=${element:5}
                vcasn=$(printf "%d" $(echo ${vcasn} | sed 's/^0*//'))
                ;;
            "MODE"*)
                mode=${element:4}
                ;;
            "TRG_FREQ"*)
                trg_freq=${element:8}
                trg_freq=$(printf "%d" $(echo ${trg_freq} | sed 's/^0*//'))
                ;;
            "STROBEB"*)
                strobeb=${element:7}
                strobeb=$(printf "%d" $(echo ${strobeb} | sed 's/^0*//'))
                ;;
        esac
    done
    masking=( 0 20 50 200 500)
    n_masking=${#masking[@]}
    i_masking=0
    for m in "${masking[@]}"
    do
        fhr[${i_masking}]=$(cat ${f} | egrep "^${m}" | head -n1 |  cut -f2)
        let i_masking=i_masking+1
    done
    echo $chip_name $vbb $ithr $vcasn $mode $trg_freq $strobeb ${fhr[@]} >> ${FILE}
done

echo "done"
