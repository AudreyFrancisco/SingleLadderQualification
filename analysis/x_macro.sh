#!/bin/bash

### ----------------------------------------------------------
##  Simple script that runs different analysis macros
##  Uses (almost) the same commands as 'runTests'
##     and executes a corresponding analysis macro
##  First parameter is the path to the datafile
##
##  e.g. ./x_macro.sh GEN /path/to/SouceScan_*_*.dat 
##       plots source hit map
##  e.g. ./x_macro.sh THRESHOLD /path/to/ThresholdScan_*_*.sh
##       plots scan results and calculates thresholds
##
##  Written by Miljenko Suljic, m.suljic@cern.ch
### ----------------------------------------------------------

if [ "$#" -lt 1 ] || [ "$1" = "--help" ];
then
    echo "----------------------------------------------------------------------"
    echo "Usage: ./x_macro.sh cmd PAR1 PAR2 PAR3 ... "
    echo "--help       - this message"
    echo "All the following commands take data file path as the first parameter:"
    echo "GEN[ERAL]    - plots data in the format <Doublecol Address NHits>"
    echo "               (produced by e.g. SCANDIGITAL, NOISEOCC)"
    echo "SCANDACS     - plots data produced by SCANDACS"
    echo "NOISEOCCSCAN - plots data produced by NOISEOCCSCAN"
    echo "THRESHOLD    - plots data produced by THRESHOLD"
    echo "               [PAR2/3/4/5/6] - n_sectors/ChiSqCut/WriteToFile/ITH/VCASN/saveCanvas,"
    echo "               see pALPIDEfsThresholdPlots.C for more details"
    echo "RAWHITS      - plots data from RawHits_*_*.dat"
    echo "----------------------------------------------------------------------"
    exit 1
fi

if [ "$#" -ge 2 ] 
then
    DATA_FILE=$2
    if [ ! -f $DATA_FILE ]
    then
	    echo "Data file does not exist!"
	    exit 2
    fi
else
    DATA_FILE=""
fi

if   [ "$1" = "GEN" ] || [ "$1" = "GENERAL" ]; then 
    root -l 'classes/helpers.cpp+' 'pALPIDEfsRawToPlot.C+("'$DATA_FILE'")'
elif [ "$1" = "SCANDACS" ]; then
    root -l scanDACs.C+
elif [ "$1" = "NOISEOCCSCAN" ]; then
    root -l 'classes/helpers.cpp+' 'noiseOccScan2D.C+("'$DATA_FILE'")'
elif [ "$1" = "THRESHOLD" ]; then
    [ "$#" -ge 3 ] && MPAR2=$3 || MPAR2="1"
    [ "$#" -ge 4 ] && MPAR3=$4 || MPAR3="-5."
    [ "$#" -ge 5 ] && MPAR4=$5 || MPAR4="0"
    [ "$#" -ge 5 ] && MPAR5=$6 || MPAR5="0"
    [ "$#" -ge 5 ] && MPAR6=$7 || MPAR6="0"
    [ "$#" -ge 5 ] && MPAR7=$8 || MPAR7="1"
    root -l 'classes/helpers.cpp+' 'pALPIDEfsThresholdPlots.C+("'$DATA_FILE'", '$MPAR2', '$MPAR3', '$MPAR4', '$MPAR5', '$MPAR6', '$MPAR7')'
elif [ "$1" = "RAWHITS" ]; then
    root -l 'classes/helpers.cpp+' 'RawHitsPlots.C+("'$DATA_FILE'")'
else
    echo "----------------------------------"
    echo "No macro associated to command $1"
    echo "See --help for the list of macros."
    echo "----------------------------------"
    exit 3
fi
