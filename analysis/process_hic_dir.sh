#!/bin/bash

###---------------------------------------------------
##  Macro for processing directory containing
##  RawHits_*_*.dat files.
##  Runs CSA on each file and saves clusters in
##  RawHits_*_*_tree.root files. Afterwards performs
##  an anlysis on the combined data.
##  
##  Written by Miljenko Suljic, m.suljic@cern.ch
##  Following srs-software concept.
##  (thanks to Felix & Jacobus)
###---------------------------------------------------

if [ "$#" -lt 1 ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]
then
    echo "-------------------------------------------------------"
    echo "Either number of arguments is not correct"
    echo " or you have requested help... "
    echo "Required arugments:"
    echo "  1) path to dir with raw files"
    echo "Optional:"
    echo "  2) masks dir name (0 for none - default)"
    echo "  3) find interesting events? 0 = no (default), 1 = yes"
    echo "  4) recreate cluster trees?  0 = no (default), 1 = yes"
    echo "-------------------------------------------------------"
    exit 1
fi

# variables
CROWN=2

# paths
DIR_ANALYSIS=${BASH_SOURCE%/*}
DIR_MACROS=$(readlink -f $DIR_ANALYSIS/clustering)/
DIR_CLASSES=$(readlink -f $DIR_ANALYSIS/classes)/
DIR_RAW=$(readlink -f $1)/
DIR_RESULTS=$(readlink -f $DIR_RAW/results)_cr${CROWN}/
DIR_LOGS=$(readlink -f $DIR_RAW/logs)/

# parallel processing
MAXJOBS=3

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

if [ "$#" -ge 3 ]
then
    FLAG_INTERESTING=$3
else
    FLAG_INTERESTING=0
fi

if [ "$#" -ge 4 ]
then
    FLAG_RECREATE=$4
else
    FLAG_RECREATE=0
fi

mkdir -p $DIR_RESULTS
mkdir -p $DIR_LOGS

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
.L csa_hic.C+g
.L interesting_events_hic.C+g
.L analysis_basic_hic.C+g
.q
EOF

njobs=0

cd $DIR_RAW
FILE_RAW=${PWD##*/}"_Chip%i.dat"
FILE_ROOT=${PWD##*/}"_tree.root"
FILE_LOG=${PWD##*/}"_csa.log"
echo $FILE_RAW

cd $DIR_MACROS

[ -f "$DIR_RAW/crown" ] && LAST_CSA_CROWN=`cat $DIR_RAW/crown` || LAST_CSA_CROWN=0
if [ ! -f "$DIR_RAW/$FILE_ROOT" ] || [ "$FLAG_RECREATE" -eq 1 ] || [ "$LAST_CSA_CROWN" != "$CROWN" ]
then
    echo "Processing $FILE_RAW"
    root -l -b -q "$DIR_CLASSES/load_classes.C" "csa_hic.C+g(\"$DIR_RAW/$FILE_RAW\", \"$DIR_RAW/$FILE_ROOT\", \"$DIR_MASK\", $CROWN)" 2>&1 | tee $DIR_LOGS/$FILE_LOG
fi

#valgrind --leak-check=full --show-leak-kinds=all --suppressions=/opt/root/v5-34-36/etc/valgrind-root.supp  root -l -b -q
#exit 0

echo $CROWN > $DIR_RAW/crown

if [ "$FLAG_INTERESTING" -eq 1 ]; then
    root -l -b -q "$DIR_CLASSES/load_classes.C" "interesting_events_hic.C+(\"$DIR_RAW/$FILE_ROOT\", \"$DIR_RESULTS/interesting_events_hic.root\")" | tee $DIR_LOGS/interesting_events_hic.log 2>&1
else
    echo "Not searching for interesting events!"
fi

root -l "$DIR_CLASSES/load_classes.C" "analysis_basic_hic.C+(\"$DIR_RAW/$FILE_ROOT\", \"$DIR_RESULTS\", \"cr$CROWN\")" | tee $DIR_LOGS/analysis_basic_chip${i}.log

echo "Finished processing directory!"
