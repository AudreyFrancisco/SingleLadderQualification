#!/bin/bash
#This script:
# Runs tuneVCASN and the corresponding fit...
#   - ...with a certain step size...
#   - ...then goes through the resulting summary files, calculates VCASN_mean from the 8/9 values there, and...
# Runs tuneITHR and the corresponding fit...
#   - ...also with a certain step size...
#   - tuneITHR accepts the VCASN_mean value via the command line now.
# Runs ITHR and the corresponding fit (the FULL scan)
#   - The final summary files are left in the directory for later analysis.

stepSize=$1  #let's start with 8.  Probably shouldn't exceed 128?
echo "BEGINNING CALIBRATION: step size = " $stepSize
cd ..
./test_tuneVCASN $stepSize
#get resulting filename from this...
#test_tune will write a Scan file name to filename.txt; get it from here
filename=$(<filename.txt)
rm filename.txt
mv Data/VcASNScan_* Data/IB_HIC_CAL_WB20/Threshold_Vcal  #this can be generalized
cd Data/IB_HIC_CAL_WB20/Threshold_Vcal
fnm="\"VcASNScan_"$filename"_Chip0_0.dat\""
echo "tuneVCASN filename: " $fnm
root -q ../../../analysis/All_FitThresholdVCASNIB.C+\($fnm,false\)
#summary and fit files have been produced in Data/... directory.  Delete the data files...
rm VcASNScan*
rm FitValues*
#...and compute the mean VCASN from the summary files.
fnm="\"ThresholdSummary_"$filename"_Chip0_0.dat\""
root -q ../../../analysis/GetThresholdMean.C+\(9,$fnm\)
#find the result in vcasn_avg.txt
vcasn=$(<vcasn_avg.txt)
echo "VCASN_avg = " $vcasn
rm vcasn_avg.txt

cd ../../..
./test_tuneITHR $stepSize $vcasn
filename=$(<filename.txt)  #get new scan prefix
rm filename.txt
mv Data/ITHRScan_* Data/IB_HIC_CAL_WB20/Threshold_cal
cd Data/IB_HIC_CAL_WB20/Threshold_cal
fnm="\"ITHRScan_"$filename"_Chip0_0.dat\""
echo "tuneITHR filename: " $fnm
root -q ../../../analysis/All_FitThresholdTuneIB.C+\($fnm,false\)
rm ITHRScan_*
rm FitValues*
#  ...and move the new Summary files (containing tuned ITHR)...
mv ThresholdSummary* ../../..
cd ../../..

#  Now run the full ITHR scan:
fnm="ThresholdSummary_"$filename"_Chip0_0.dat"
echo $fnm
./test_ITHRthreshold $fnm $vcasn
filename=$(<filename.txt)
rm filename.txt
rm ThresholdSummary*
mv Data/ThresholdScan* Data/IB_HIC_CAL_WB20/Threshold_post
cd Data/IB_HIC_CAL_WB20/Threshold_post
fnm="\"ThresholdScan_"$filename"_Chip0_0.dat\""
echo "ITHR filename: " $fnm
root -q ../../../analysis/All_FitThresholdsIB.C+\($fnm,false\)
#  ...and remove all uneeded files to free up memory.  Plot and compute RMS separately.
rm ThresholdScan*
rm FitValues*

echo "FINISHED: step size = " $stepSize ", vcasn_avg = " $vcasn ", file prefix " $filename


