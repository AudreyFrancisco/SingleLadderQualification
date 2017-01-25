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

if [ "$#" -lt 2 ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]
then
    echo "-------------------------------------------------------"
    echo "Either number of arguments is not correct"
    echo " or you have requested help... "
    echo "Required arugments:"
    echo "  1) path to dir with event tree"
    echo "  2) path to file with VD tracks"
    echo "Optional:"
    echo "  ROOT flags "
    echo "-------------------------------------------------------"
    exit 1
fi

# paths
DIR_ANALYSIS=${BASH_SOURCE%/*}
DIR_MACROS=$(readlink -f $DIR_ANALYSIS/na61)/
DIR_CLASSES=$(readlink -f $DIR_ANALYSIS/classes)/
DIR_RAW=$(readlink -f $1)/

FILE_VD=$(readlink -f $2)

if [ -f "$DIR_RAW/crown" ]
then
    CROWN=`cat $DIR_RAW/crown`
else
    echo "ERROR: No CROWN information in $DIR_RAW"
    exit 2
fi
DIR_RESULTS=$(readlink -f $DIR_RAW/results)_cr${CROWN}/
DIR_LOGS=$(readlink -f $DIR_RAW/logs)/

echo "---------------------------------"
echo "Target directory:  $DIR_RAW"
echo "VD tracks file:    $FILE_VD"
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
.L prealignment_vd.C+g
.q
EOF

FILE_ROOT="event_tree.root"

cd $DIR_MACROS

root -l $3 $4 "$DIR_CLASSES/load_classes.C" "prealignment_vd.C+(\"$DIR_RAW/$FILE_ROOT\", \"$FILE_VD\", \"$DIR_RESULTS\", \"cr$CROWN\")" | tee $DIR_LOGS/prealignment_vd.log

#root -l ~/.tbr.C $DIR_RESULTS

echo "Finished processing directory!"
