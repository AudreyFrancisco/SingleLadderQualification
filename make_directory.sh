#!/bin/bash

# Author: Andry R. (CEA Saclay)
# Make subdirectories into Data for the HIC that will be tested
#
# After your tests, compress the ladder_func subdirectory and copy the archive to 
# the cernbox directory /eos/project/m/mft/WP2_Ladder/HICXX/
# where XX is the number of the HIC that you just tested

# print to stdout
echo -n "Enter the hic number (for e.g. 82)"

# read user input i.e. hic number
read NB
echo "You have chosen hic # " $NB
echo 

# create subdirectory structure for that hic
if [ -d Data/hic$NB ]; then
	echo "Directory Data/hic${NB} already exists"
else 
	mkdir -pv Data/hic${NB}/ladder_func/fifo/
	mkdir -pv Data/hic${NB}/ladder_func/digitalscan/allchips/
	mkdir -pv Data/hic${NB}/ladder_func/thresholdscan/allchips/bb0/
	mkdir -pv Data/hic${NB}/ladder_func/thresholdscan/allchips/bb3/
	echo "Done"
fi 

