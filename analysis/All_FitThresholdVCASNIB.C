/* Runs FitThreshold for every chip; current loop bounds are for middle detector layer.
   If the second parameter is true (default), the ThresholdMap is plotted. */
#include "TROOT.h"
#include <string>
#include <iostream>
#include <fstream>

//NOTE:  This should be called from the Data/... directory where the data files are located, NOT from /analysis.
//same as other routine, but inner barrel instead of outer.

void runFit(int chipNum, std::string prefix) {
  //This might not be the cleanest way to do this, but it works.  Streamline later?
  std::string name = prefix + "Chip" + std::to_string(chipNum) + "_0.dat";
  std::string commandStr = ".x ../../../analysis/FitThresholdTuneVCASNIB.C+(\""+ name + "\", true, 0, 0, false)";

  const char * input = commandStr.c_str();
  std::cout << "CHIP " << chipNum << ":" << std::endl;
  std::cout << input << std::endl;
  std::ifstream f(name.c_str());
  if(!f.good()) {
    std::cout << "Input file not found." << std::endl;
  } else {
    gROOT->ProcessLine(input);
  }
}

int All_FitThresholdVCASNIB(const char *fName, bool generateMap=true) {  //Give it an arbitrary data file name...

  std::string Prefix = fName;
  Prefix.erase(Prefix.rfind("Chip"));
  std::cout << "Prefix: " << Prefix << std::endl;

  for(int i=0; i<9; i++) {
    runFit(i,Prefix);
  }

  if(generateMap) {
    std::string slice= Prefix.substr(0, Prefix.rfind("/")+1) + "FitValues" + Prefix.substr(Prefix.rfind("ThresholdScan_")+13, 15);
    std::string line = ".x ThresholdMapIB.C+g(\"" + slice + "Chip0_0.dat\"," + std::to_string(thresholdNotNoise) + ")";
    gROOT->ProcessLine(line.c_str()); //arbitrary file
  }
  return 0;
}
