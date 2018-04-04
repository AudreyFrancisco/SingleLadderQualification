#!/bin/bash -x

#
# Author: Andry R. (CEA Saclay)
# Use this to perform a clean compilation of all sources
# Here, the PC has 4 CPU cores (adapt to your own PC)
# 

echo '---------------------------------------'
pwd
echo
make clean
make -j4
make -j4 lib
make -j4 lib_analysis
echo '---------------------------------------'
cd GUI
pwd
echo 
qmake-qt5 -makefile
make -j4
echo '---------------------------------------'
cd ../MosaicSrc/powerboard
pwd
echo ''
make clean
make -j4
echo '---------------------------------------'
cd GUI
pwd
echo 
qmake-qt5 -makefile
make clean
make -j4
echo '---------------------------------------'
cd ../../../
pwd
echo 
make format-check
