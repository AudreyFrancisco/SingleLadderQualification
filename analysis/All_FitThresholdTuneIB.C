/* Runs FitThreshold for every chip; current loop bounds are for middle detector layer.
   If the second parameter is true (default), the ThresholdMap is plotted. */
#include "TROOT.h"
#include <string>
#include <iostream>
#include <fstream>

//NOTE:  This should be called from the Data/... directory where the data files are located, NOT from /analysis.
//same as other routine, but inner barrel instead of outer.
//Specifically for use after thresh calibration

void runFit(int chipNum, std::string prefix) {
  //This might not be the cleanest way to do this, but it works.  Streamline later?
  std::string name = prefix + "Chip" + std::to_string(chipNum) + "_0.dat";
  //Write to file.  Need to manually give it ITH and VCASN??
  //   - BUT ITH varies constantly...?
  std::string commandStr = ".x ../../../analysis/FitThresholdTune.C+(\"" + name + "\", true)";
  
  const char * input = commandStr.c_str();
  std::cout << "CHIP " << chipNum << ":" << std::endl;
  std::cout << input << std::endl;
  std::ifstream f(name.c_str());
  if(f.good()) { //if file exists
    gROOT->ProcessLine(input);
  } else {
    std::cout << "Input file does not exist." << std::endl;
  }
}

int All_FitThresholdTuneIB(const char *fName, bool generateMap=true) {  //Give it an arbitrary data file name...
  char Prefix[100], fNameChip[100];
  
  int PrefixLength=strcspn(fName, "C"); //needs +"chipnum_rownum.dat"
  strncpy(Prefix, fName, PrefixLength);
  Prefix[PrefixLength]='\0';
  std::string Pre(Prefix);
  std::cout << Pre << std::endl;
  
  for(int i=0; i<9; i++) {
    runFit(i,Pre);
  }
  
  /*for(int i=8; i<15; i++) {
    suffix = 0;
    runFit(i,Pre,suffix);
  }*/
  if(generateMap) {
    std::string slice="Plotting FitValues" + Pre.substr(13,15); //length-sensitive...
    std::cout << slice << std::endl;
    const char * line = (".x ../../../analysis/ThresholdMapIB.C+(\"" + slice + "Chip0_0.dat\", false)").c_str();  //I /think/ this is right...
    gROOT->ProcessLine(line); //arbitrary file
  }
  return 0;
}


