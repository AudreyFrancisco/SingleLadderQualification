#bin/bash

######################################################################################
#
# script for analyzing pulseshape measurements taken with run_pulseshape_scans.sh
# written by jacobus - j.w.van.hoorne@cern.ch
# and adapted by natalia :)
#
######################################################################################

# DIRECTORIES:
#---------------------------------------------------------
 
# Input:
if [ ! "$#" -eq 1 ]
then
    echo "Please provide the path to measurement folder!"
    exit 1
else
    MEAS_DIR=$1
    echo $MEAS_DIR
fi

if [ ! -d "$MEAS_DIR" ]
then
    echo "Data directory not found! please check!"
    exit 3
fi

# Analysis directory:
ANA_DIR=`pwd`



# FILES:
#---------------------------------------------------------
 
LOG=$MEAS_DIR/analysis_pulseshape.log
if [ -f "$LOG" ]
then
    rm $LOG
fi

VBB_FILE=$MEAS_DIR/settings/vbb_list.txt
if [ ! -f "$VBB_FILE" ]
then
    echo "Vbb file not found! please check!"
    exit 3
fi

ITHR_FILE=$MEAS_DIR/settings/ithr_list.txt
if [ ! -f "$ITHR_FILE" ]
then
    echo "Ithr file not found! please check!"
    exit 3
fi



# Loop over VBB, VCASN, ITHR
#---------------------------------------------------------

cd $MEAS_DIR
# Loop over VBBs:
for VBB in `cat $VBB_FILE`
do 
    VBB_DIR=$MEAS_DIR/$(printf "VBB-%0.1f" ${VBB}) 
    if [ ! -d "$VBB_DIR" ]
    then
        echo $VBB
        echo "Vbb directory not found! please check!"
        exit 3
    fi

    # Clean up the output folder:
    cd $VBB_DIR/pulseshape
    rm *.root
    rm PulselengthResults*

    # Loop over VCASN and ITHR:
    for f in `ls -1 PulselengthScan*.dat`
    do
        DATA_FILE=$VBB_DIR/pulseshape/$f
        if [ ! -f "$DATA_FILE" ]
        then
            echo $DATA_FILE
            echo "Data file not found! please check!"
            exit 3
        fi

        cd $ANA_DIR
        # Convert RAW to Root file:
        root -b -l <<EOF >> $LOG 
        .L helpers.cpp+g
        .L PulseshapeRawToHisto.C++
        .x PulseshapeRawToHisto.C+g("$DATA_FILE")
        .q
EOF
	wait
        RESULT_FILE="${DATA_FILE:0:${#DATA_FILE}-4}.root"

        # Analyze root file:
	root -b -l <<EOF >> $LOG 
        .L helpers.cpp+g
        .L PulseshapeAnalysis.C++
        .x PulseshapeAnalysis.C+g("$RESULT_FILE") 
        .q
EOF
        wait
    done
done

rm $MEAS_DIR/PulseshapeSummary.root
hadd $MEAS_DIR/PulseshapeSummary.root `find $MEAS_DIR -name "PulselengthResults_*.root"`
