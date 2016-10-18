#!/bin/bash

if [ "$#" -lt 1 ]
then
    echo "Path to SourceRaw_*_*.dat file needed!"
fi

FILE_DATA=$(readlink -f $1)
DIR_MON=$(readlink -f ${BASH_SOURCE%/*})
DIR_CLASSES=$(readlink -f $DIR_MON/../classes)/

cd $DIR_CLASSES
root -l -b -q "compile_classes.C"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DIR_CLASSES

#root -l -b -q << EOF 
#  .L AliPALPIDEFSRawStreamMS.cpp++
#EOF

cd $DIR_MON

root -l monitoring_main.C\(\"$FILE_DATA\"\)
