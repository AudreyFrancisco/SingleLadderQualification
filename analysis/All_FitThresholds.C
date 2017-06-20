/* Runs FitThreshold for every chip; current loop bounds are for middle detector layer.
   If the second parameter is true (default), the ThresholdMap is plotted. */
#include "TROOT.h"
#include <string>
#include <iostream>

//NOTE:  This should be called from the Data/... directory where the data files are located, NOT from /analysis.

void runFit(int chipNum, std::string prefix, int suff) {
  //This might not be the cleanest way to do this, but it works.  Streamline later?
  std::string commandStr = ".x ../../../analysis/FitThresholds.C+(\"" + prefix
                 + "_Chip" + std::to_string(chipNum) + "_" + std::to_string(suff) + ".dat\")";
  
  
  const char * input = commandStr.c_str();
  std::cout << "CHIP " << chipNum << "_" << suff << ":" << std::endl;
  std::cout << input << std::endl;
  gROOT->ProcessLine(input);
}

int All_FitThresholds(const char *fName, bool generateMap=true) {  //Give it an arbitrary data file name...
  char Prefix[100], fNameChip[100];
  
  int PrefixLength=strcspn(fName, "C"); //needs +"chipnum_rownum.dat"
  strncpy(Prefix, fName, PrefixLength);
  Prefix[PrefixLength]='\0';
  std::string Pre(Prefix);
  std::cout << Pre << std::endl;
  int suffix;
  
  for(int i=0; i<7; i++) {
    suffix = 1;
    runFit(i,Pre,suffix);
  }
  
  for(int i=8; i<15; i++) {
    suffix = 0;
    runFit(i,Pre,suffix);
  }
  if(generateMap) {
    std::string slice="FitValues" + Pre.substr(13,15); //length-sensitive...
    std::cout << slice << std::endl;
    const char * line = (".x ../../../analysis/All_ThresholdMap.C+(\"" + slice + "Chip0_1.dat\", true)").c_str();
    gROOT->ProcessLine(line); //arbitrary file
  }
  return 0;
}


