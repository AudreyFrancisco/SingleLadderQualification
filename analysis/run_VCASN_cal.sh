#!/bin/bash
#this automatically runs test_tuneVCASN, moves the output files, and then runs the
#corresponding fit routines.
cd ..
./test_tuneVCASN
#get resulting filename from this...
#test_tune will write a Scan file name to filename.txt; get it from here
filename=$(<filename.txt)
mv Data/ITHRScan_* Data/IB_HIC_CAL_WB20/Threshold_Vcal
cd Data/IB_HIC_CAL_WB20/Threshold_Vcal
fnm="\"VcASNScan_"$filename"_Chip0_0.dat\""
echo $fnm
root ../../../analysis/All_FitThresholdTuneVCASNIB.C+\($fnm,false\)

