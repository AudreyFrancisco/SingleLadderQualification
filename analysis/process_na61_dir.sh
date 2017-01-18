#!/bin/bash

###---------------------------------------------------
##  Macro for processing directory containing
##  NoiseOccupancy_*_*_Chip*.dat files.
##  Runs CSA on all files and saves clusters in
##  NoiseOccupancy_*_*_tree.root file. Afterwards performs
##  an anlysis on the combined data.
##  
##  Written by Miljenko Suljic, m.suljic@cern.ch
##  Following srs-software concept.
###---------------------------------------------------

if [ "$#" -lt 1 ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]
then
    echo "-------------------------------------------------------"
    echo "Either number of arguments is not correct"
    echo " or you have requested help... "
    echo "Required arugments:"
    echo "  1) path to dir with event tree"
    echo "Optional:"
    echo "  2) masks dir name (0 for none - default)"
    echo "-------------------------------------------------------"
    exit 1
fi

# paths
DIR_ANALYSIS=${BASH_SOURCE%/*}
DIR_MACROS=$(readlink -f $DIR_ANALYSIS/na61)/
DIR_CLASSES=$(readlink -f $DIR_ANALYSIS/classes)/
DIR_RAW=$(readlink -f $1)/
if [ -f "$DIR_RAW/crown" ]
then
    CROWN=`cat $DIR_RAW/crown`
else
    echo "ERROR: No CROWN information in $DIR_RAW"
    exit 2
fi
DIR_RESULTS=$(readlink -f $DIR_RAW/results)_cr${CROWN}/
DIR_LOGS=$(readlink -f $DIR_RAW/logs)/

if [ "$#" -ge 2 ] 
then
    if [ "$2" == "NULL" ] || [ "$2" == "0" ]
    then
        DIR_MASK=""
    else
        DIR_MASK=$(readlink -f $2)/
    fi
else
    DIR_MASK=""
fi

echo "---------------------------------"
echo "Target directory:  $DIR_RAW"
echo "Mask filename:     $DIR_MASK"
echo "Results directory: $DIR_RESULTS"
echo "Dir with macros:   $DIR_MACROS"
echo "Dir with classes:  $DIR_CLASSES"
echo "---------------------------------"

# compile classes
cd $DIR_CLASSES
#rm *.so *.d
root -l -b -q "compile_classes.C"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DIR_CLASSES

# compile macros
cd $DIR_MACROS
#rm *.so *.d
root -l -b <<EOF
.x $DIR_CLASSES/load_classes.C
.L alignment_vd.C+g
.q
EOF

FILE_ROOT="event_tree.root"

cd $DIR_MACROS

root -l "$DIR_CLASSES/load_classes.C" "alignment_vd.C+(\"$DIR_RAW/$FILE_ROOT\", \"$DIR_RESULTS\", \"cr$CROWN\")" | tee $DIR_LOGS/alignment_vd.log

echo "Finished processing directory!"
