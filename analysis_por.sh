#!/bin/bash

ROOT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd ) # determine where this script is located

cd ${ROOT_DIR}/Data
for f in $(find . -name "*log*.txt" | grep -v result | sort) ;
do
    GOOD=$(cat $f | grep -E 'Readback.*0xaaa.*0xaa' | wc -l )
    TOTAL=$(cat $f | grep Readback | wc -l )
    LDO_OFF=$(cat $f | grep "LDOs are off" | wc -l)
    BAD=$(cat $f | grep -E 'Readback.*0x.*0x' | grep -v -E 'Readback.*0xaaa.*0xaa' | wc -l )
    BAD=$(( ${BAD} - ${LDO_OFF} ))
    if [[ "${TOTAL}" -ne "$((GOOD + LDO_OFF))" ]]
    then
	echo -e "$f\tStatus: ${GOOD}/${TOTAL}/${LDO_OFF}/${BAD} - BAD"
	echo "Suspicious: "
	#cat $f | egrep -C3 -E 'Readback.*0x' ;
	cat $f | egrep -A1 "received chipID" ;
    else
	echo -e "$f\tStatus: ${GOOD}/${TOTAL}/${LDO_OFF}/${BAD} - OK"
    fi
    echo " "
 done
