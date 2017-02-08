#!/bin/bash

###---------------------------------------------------
##  Macro for easier running of VD alignment
##  
##  Written by Miljenko Suljic, m.suljic@cern.ch
###---------------------------------------------------

if  [ "$1" == "--help" ] || [ "$1" == "-h" ]
then
    echo "-------------------------------------------------------"
    echo "Optional arguments:"
    echo "  ROOT flags "
    echo "-------------------------------------------------------"
    exit 1
fi

# paths
DIR_ANALYSIS=${BASH_SOURCE%/*}
DIR_MACROS=$(readlink -f $DIR_ANALYSIS/na61)/
DIR_CLASSES=$(readlink -f $DIR_ANALYSIS/classes)/

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

root -l $3 $4 "$DIR_CLASSES/load_classes.C" "alignment_vd.C+"
