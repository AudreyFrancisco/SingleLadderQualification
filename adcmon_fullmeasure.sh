#!/bin/bash

mkdir Data/$1
adcmon_temperature.sh > Data/$1/temp.txt
test_adcmon mode=manual -e -f Data/$1/scanIntDac
test_adcmon mode=manual -s input=13 -f Data/$1/adcmon_eval
test_adcmon mode=manual -s input=14 -f Data/$1/adcmon_eval
test_adcmon mode=manual -p -f Data/$1/scan_preset_64
del Data/$1/scan_preset_64*.dat
cd Data
tar -zcvf $1.tar.gz $1
cd ..
echo "finished"
