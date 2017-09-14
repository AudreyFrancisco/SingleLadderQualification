#!/bin/bash

#################################################################
### GROUND OFFSET
#################################################################


VGO_LIST=($(seq 0.0 0.01 0.3))
VBB_LIST=($(seq 0.0 0.01 0.3))

#################################################################
### ENVIRONMENT
#################################################################
# script dir
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# dir with pALPIDEfs software
ROOT_DIR=$SCRIPT_DIR/../../
ANA_DIR=$ROOT_DIR/../analysis/

# initialise all power supplies
$SCRIPT_DIR/powerOff.sh > /dev/null 2>&1
sleep 2
$SCRIPT_DIR/powerOn.sh
sleep 1


cd $SCRIPT_DIR
#################################################################
### create folder structure and parameter lists
#################################################################
DATE_TIME=`date +%Y%m%d_%H%M%S`

# create folder structure and write list files
OUT_FOLDER=groundOffset_${DATE_TIME}
DATA_DIR=$ROOT_DIR/Data/$OUT_FOLDER
if [ ! -d "$DATA_DIR" ]
then
    mkdir -p $DATA_DIR 
fi

LOG=$DATA_DIR/run.log
if [ -f "$LOG" ]
then
    rm $LOG
fi

cat <<EOF >> $SCRIPT_DIR/Config.cfg
# Software
DEVICE       IBHIC 
NMASKSTAGES  51
PIXPERREGION 32

# Readout System
ADDRESS 192.168.168.250
CONTROLLATENCYMODE 0
POLLINGDATATIMEOUT 500
DATALINKPOLARITY 0
DATALINKSPEED 12

# Chip
PLLSTAGES 0
LINKSPEED 600

ITHR	50
VCASN2  57
VCASN   50
VCLIP   0
VRESETD	117
IDB 29
VCASP   86
IBIAS 64
VPULSEH 170

EOF

#################################################################
### loop over parameters
#################################################################
cd $ROOT_DIR

for VGO in "${VGO_LIST[@]}"
do
    $SCRIPT_DIR/hameg.py 2 4 ${VGO} 0.020      | tee -a $LOG
    sleep 1
    $SCRIPT_DIR/hameg.py 3                     | tee -a $LOG
    ./test_threshold -c $SCRIPT_DIR/Config.cfg | tee -a $LOG
    $SCRIPT_DIR/hameg.py 3                     | tee -a $LOG
    OUT_FILE=$(ls -1lr $ROOT_DIR/Data/Threshold*.dat | tail -1)
    SUBFOLDER=$(printf "$DATA_DIR/VGO%0.3f" ${VGO})
    mkdir -p ${SUBFOLDER}
    mv -v ${OUT_FILE} ${SUBFOLDER}             | tee -a $LOG
    echo ${VGO} >> ${SUBFOLDER}/VGO.txt
done
$SCRIPT_DIR/hameg.py 2 4 0.0 0.020             | tee -a $LOG

for VBB in "${VBB_LIST[@]}"
do
    $SCRIPT_DIR/hameg.py 2 3 ${VBB} 0.020      | tee -a $LOG
    sleep 1
    $SCRIPT_DIR/hameg.py 3                     | tee -a $LOG
    ./test_threshold -c $SCRIPT_DIR/Config.cfg | tee -a $LOG
    $SCRIPT_DIR/hameg.py 3                     | tee -a $LOG
    OUT_FILE=$(ls -1lr $ROOT_DIR/Data/Threshold*.dat | tail -1)
    SUBFOLDER=$(printf "$DATA_DIR/VBB%0.3f" ${VBB})
    mkdir -p ${SUBFOLDER}
    mv -v ${OUT_FILE} ${SUBFOLDER}                | tee -a $LOG
    echo ${VBB} >> ${SUBFOLDER}/VBB.txt
done
$SCRIPT_DIR/hameg.py 2 3 0.0 0.020             | tee -a $LOG


# power off DAQ board 
$SCRIPT_DIR/powerOff.sh                        | tee -a $LOG

echo "done."



