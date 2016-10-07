#!/bin/bash
# 1. Create ProgressBar function
# 1.1 Input is currentState($1) and totalState($2)
				
# Variables
_start=0
				
# This accounts as the "totalState" variable for the ProgressBar function
_end=12
				
dateinit=`date +%s`

for number in $(seq ${_start} ${_end})
do
	datecur=`date +%s`
	echo -n "$(($datecur-$dateinit)) "
#	test_adcmon mode=manual -v input=21 | awk '/#Float Value/ {print $0}' | tail -1
	test_adcmon mode=manual -v input=21 | awk 'BEGIN {FS="\t"}; /#Float Value/ || /#Value/ {printf("%s ",$2)}; END {print ""}' | awk '{printf("%s %s\n",$3,$4)}'

	sleep 5
done
del adcmon_acq*.dat
printf '\nFinished!\n'
