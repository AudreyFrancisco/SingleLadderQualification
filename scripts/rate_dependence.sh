#!/bin/bash
###################################################################################################
### Configuration
###################################################################################################

VBB_LIST=( 0.0 3.0 6.0 ) # in V
I_THR_LIST=( 51 100 ) # in DAC
V_CASN_LIST=( 57 105 135 ) # in DAC
V_CLIP_LIST=( 0 60 100 ) # in DAC
V_RESETD_LIST=( 147 170 170 ) # in DAC

# how many V_CASN values to process per back-bias voltage
V_CASN_PER_VBB=$(echo "${#V_CASN_LIST[@]} / ${#VBB_LIST[@]}" | bc)

MODE_LIST=( A B ) # chip readout mode
STROBEB_LIST=( 4 0 ) # continuous integration: 0,
                     # different from 0: multiple of 25ns clock cycles

TRG_FREQ_LIST=( 20000 50000 100000 200000 500000 1000000 ) # in Hz
TRG_TRAIN_LENGTH=( 100 ) # number of consecutive triggers


HAMEG=/dev/ttyHAMEG0
CURR=( 0.8 0.0 0.02 ) # 5V, unconnected, Vbb


###################################################################################################


###################################################################################################
# setup environment / load functions
ROOT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd ) # determine where this script is located
source ${ROOT_DIR}/common/functions.sh

###################################################################################################
### determine 'run conditions'
eval $(determine_run_conditions)

###################################################################################################
### execution

if [ "$#" -lt 1 ]
then
    # ask for chip ID
    echo "Please enter the name of the chip followed by [ENTER]:"
    read CHIPNAME
else
    CHIPNAME=$1
fi


# is ROOT available?
check_root

# create output folder
FULLPATH=${ROOT_DIR}/../Data/rate_dependence_${DATE}_${GIT_INFO}${SUFFIX}
create_output_folder $FULLPATH

LOGFILE=${FULLPATH}/log.txt

# store the chip name
touch ${FULLPATH}/${CHIPNAME}

# store git diff
store_git_diff $FULLPATH >> $LOGFILE 2>&1

DATAPATH=${FULLPATH}/data
mkdir $DATAPATH

###################################################################################################
### initialise all power supplies
###################################################################################################

${ROOT_DIR}/common/pulser.py -1.0 # deactivate pulser which could lead to an input signal above VDDA and VDDD

${ROOT_DIR}/common/hameg2030.py ${HAMEG} 0 ${CURR[@]}

###################################################################################################
#### start the measurment itself
###################################################################################################
i_VBB=0
for VBB in "${VBB_LIST[@]}"
do
    # set the back-bias voltage
    echo "setting back-bias voltage to ${VBB}V" | tee -a $LOGFILE
    ${ROOT_DIR}/common/hameg2030.py ${HAMEG} 1 3 ${VBB}
    VBB_OLD=${VBB}
    sleep 1
    ${ROOT_DIR}/common/hameg2030.py ${HAMEG} 4
    if [ $? -eq 1 ]
    then
        echo "back-bias current too high, stopping measurement"
        ${ROOT_DIR}/common/hameg2030.py ${HAMEG} 4
        exit 1
    fi

    V_CLIP=${V_CLIP_LIST[${i_VBB}]}
    V_RESETD=${V_RESETD_LIST[${i_VBB}]}

    for I_THR in "${I_THR_LIST[@]}"
    do
        echo "I_THR="${I_THR}"DAC" | tee -a $LOGFILE
        for i_V_CASN in $(seq 0 $((${V_CASN_PER_VBB} - 1)))
        #for V_CASN in "${V_CASN_LIST[@]}"
        do
            V_CASN=${V_CASN_LIST[$(( i_VBB*${V_CASN_PER_VBB} + i_V_CASN ))]}
            V_CASN2=$(( ${V_CASN} + 12 ))
            echo "V_CASN="${V_CASN}"DAC" | tee -a $LOGFILE
            for MODE in "${MODE_LIST[@]}"
            do
                echo "READOUT MODE=$MODE" | tee -a $LOGFILE

                for TRG_FREQ in "${TRG_FREQ_LIST[@]}"
                do
                    echo "TRG_FREQ="${TRG_FREQ}"Hz" | tee -a $LOGFILE
                    ${ROOT_DIR}/common/pulser.py 1 ${TRG_FREQ} ${TRG_TRAIN_LENGTH} | tee -a $LOGFILE

                    for STROBEB in "${STROBEB_LIST[@]}"
                    do
                        if [ "${STROBEB}" -eq 0 ]
                        then
                            STROBEB=$(bc <<< "40000000/${TRG_FREQ}-10" )
                            echo "Continuous integration mode, STROBEB="$(bc <<< "${STROBEB}*25")"ns" | tee -a $LOGFILE
                        else
                            echo "Fixed length STROBEB="$(bc <<< "${STROBEB}*25")"ns" | tee -a $LOGFILE
                        fi


                        ###########################################################################
                        ### write config file
                        ###########################################################################
                        cd ${ROOT_DIR}
                        cat <<EOF  > Config.cfg
# First line has to be DEVICE (Values: CHIP, TELESCOPE, MODULE, STAVE, CHIPMOSAIC)
DEVICE CHIP

# as of firmware version 247e0611 the DAQboard version (v2 or v3) must be defined; 0 -> v2; 1 -> v3;
BOARDVERSION	1


ITHR	${I_THR}
VCASN2	${V_CASN2}
VCASN   ${V_CASN}
VCLIP	${V_CLIP}
VRESETD	${V_RESETD}
IDB 29
VCASP   86
IBIAS 64

STROBEDURATION ${STROBEB}

EOF
                        # READOUTMODE

                        mv Config.cfg ../

                        CURR_DATAPATH=$(printf "$DATAPATH/VBB-%0.1f/ITHR%0.3d/VCASN%0.3d/MODE%c/TRG_FREQ%0.6d/STROBEB%0.6d" \
                                               ${VBB} ${V_RST} ${I_THR} ${V_CASN} ${MODE} ${TRG_FREQ} ${STROBEB})
                        SUBFOLDER=${CURR_DATAPATH}/FHR
                        mkdir -p ${SUBFOLDER}
                        cp ../Config.cfg ${SUBFOLDER}


                        ###########################################################################
                        ### start the acquisition
                        ###########################################################################
                        cd ${ROOT_DIR}/../
                        ./test_noiseocc_ext  | tee -a $LOGFILE

                        cp -v Data/$(ls -1tr Data | grep NoiseOccupancyExt | tail -n 1) ${SUBFOLDER}
                        cp -v Data/$(ls -1tr Data | grep ScanConfig | tail -n 1) ${SUBFOLDER}
                    done
                done
            done
        done
    done
    let i_VBB+=1
done

###################################################################################################
### power down everything
###################################################################################################
${ROOT_DIR}/common/hameg2030.py ${HAMEG} 6 # turn off the PSU

git checkout ${ROOT_DIR}/../Config.cfg
