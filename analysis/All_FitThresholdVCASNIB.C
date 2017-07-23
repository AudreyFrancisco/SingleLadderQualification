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
  char Prefix[100], fNameChip[100];
  
  int PrefixLength=strcspn(fName, "C"); //needs +"chipnum_rownum.dat"
  strncpy(Prefix, fName, PrefixLength);
  Prefix[PrefixLength]='\0';
  std::string Pre(Prefix);
  std::cout << "Prefix: " << Pre << std::endl;
  
  for(int i=0; i<9; i++) {
    runFit(i,Pre);
  }
  
  if(generateMap) {
    int IDstart = strcspn(Prefix, "_");
    std::string slice="FitValues" + Pre.substr(IDstart,15); //length-sensitive
    std::cout << "Map file prefix = " << slice << std::endl;
    const char * line = (".x ../../../analysis/ThresholdMapIB.C+(\"" + slice + "Chip0_0.dat\")").c_str();  //I /think/ this is right...add , false parameter to plot noise instead
    gROOT->ProcessLine(line); //arbitrary file
  }
  return 0;
}


