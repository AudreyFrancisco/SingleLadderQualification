/* Runs FitThreshold for every chip; current loop bounds are for middle detector layer.
   If the second parameter is true (default), the ThresholdMap is plotted. */
#include "TROOT.h"
#include <iostream>
#include <string>

// NOTE:  This should be called from the Data/... directory where the data files are located, NOT
// from /analysis.

void runFit(int chipNum, std::string prefix, int suff)
{
  // This might not be the cleanest way to do this, but it works.  Streamline later?
  std::string commandStr = ".x ../../../analysis/FitThresholds.C+(\"" + prefix + "Chip" +
                           std::to_string(chipNum) + "_" + std::to_string(suff) + ".dat\")";


  const char *input = commandStr.c_str();
  std::cout << "CHIP " << chipNum << "_" << suff << ":" << std::endl;
  std::cout << input << std::endl;
  gROOT->ProcessLine(input);
}

int All_FitThresholds(const char *fName, bool generateMap = true)
{ // Give it an arbitrary data file name...
  std::string Prefix = fName;
  Prefix.erase(Prefix.rfind("Chip"));
  std::cout << "Prefix: " << Prefix << std::endl;

  char fNameChip[100];

  int suffix;

  for (int i = 0; i < 7; i++) {
    suffix = 1;
    runFit(i, Prefix, suffix);
  }

  for (int i = 8; i < 15; i++) {
    suffix = 0;
    runFit(i, Prefix, suffix);
  }
  if (generateMap) {
    std::string slice = Prefix.substr(0, Prefix.rfind("/") + 1) + "FitValues" +
                        Prefix.substr(Prefix.rfind("ThresholdScan_") + 13, 15);
    const char *line =
        (".x ../../../analysis/All_ThresholdMap.C+(\"" + slice + "Chip0_1.dat\", true)").c_str();
    gROOT->ProcessLine(line); // arbitrary file
  }
  return 0;
}
