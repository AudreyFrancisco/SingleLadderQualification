/* Runs FitThreshold for every chip; current loop bounds are for middle detector layer. */

//#include "FitThresholds.h"
//#include "ThresholdMap.C"
//#include "ThresholdMapIP.C"
#include <string>
#include <iostream>

void runFit(int chipNum, int suff, int groupNum) {
  std::string commandStr = ".x FitThresholds.C+(\"../Data/OB_HIC_" + std::to_string(groupNum) + "/Threshold/"
                 + "ThresholdScan_170619_112554_Chip" + std::to_string(chipNum) + "_" + std::to_string(suff) + ".dat\")";
  const char * input = commandStr.c_str();
  std::cout << " CHIP " << chipNum << "_" << suff << ":" << std::endl;
  std::cout << input << std::endl;
  //gROOT->ProcessLine(input);
}

int All_ThresholdMap() {
  int dataNum=60;
  bool generateMap=true; //set to false if map not desired
  int suffix;
  
  for(int i=3; i<6; i++) {
    suffix = 1;
    runFit(i,suffix,dataNum);
    //automatic data analysis??
  }
  
  for(int i=8; i<10; i++) {
    suffix = 0;
    //runFit(i,suffix,dataNum);
  }
  if(generateMap) {
    //ADD ThresholdMap stuff here if necessary
  }
  return 0;
}


