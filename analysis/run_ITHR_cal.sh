#!/bin/bash
#Automatically runs tune, and then ...
cd ..
pwd
./test_tuneITHR
#get resulting filename from this...
#test_tune will write a Scan file name to filename.txt; get it from here
filename=$(<filename.txt)
pwd
mv Data/ITHRScan_* Data/IB_HIC_CAL_WB20/Threshold_cal  #this can be generalized
cd Data/IB_HIC_CAL_WB20/Threshold_cal
fnm="\"ITHRScan_"$filename"_Chip0_0.dat\""
echo $fnm
root ../../../analysis/All_FitThresholdTuneIB.C+\($fnm,false\)
#summary and fit files have been produced in Data/... directory.  Move the summary files...
