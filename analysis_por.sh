#!/bin/bash

ROOT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd ) # determine where this script is located

cd ${ROOT_DIR}/Data
for f in $(find . -name "*log*.txt") ; 
do 
    echo $f ; 
    GOOD=$(cat $f | grep Readback | grep 0xaaa | grep 0xaa | wc -l )
    BAD=$(cat $f | grep Readback | wc -l )
    echo "Status: ${GOOD}/${BAD}"
    echo "Suspicious: "
    cat $f | grep -C3 0xffff ;
    echo "" ;
 done
