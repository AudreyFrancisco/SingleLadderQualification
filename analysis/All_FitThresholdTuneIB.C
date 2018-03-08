/* Runs FitThreshold for every chip; current loop bounds are for middle detector layer.
   If the second parameter is true (default), the ThresholdMap is plotted. */
#include "TROOT.h"
#include <fstream>
#include <iostream>
#include <string>

// NOTE:  This should be called from the Data/... directory where the data files are located, NOT
// from /analysis.
// same as other routine, but inner barrel instead of outer.
// Specifically for use after thresh calibration

void runFit(int chipNum, std::string prefix)
{
  // This might not be the cleanest way to do this, but it works.  Streamline later?
  std::string name = prefix + "Chip" + std::to_string(chipNum) + "_0.dat";
  // Write to file.  Need to manually give it ITH and VCASN??
  //   - BUT ITH varies constantly...?
  std::string commandStr = ".x ../../../analysis/FitThresholdTune.C+(\"" + name + "\", true)";

  const char *input = commandStr.c_str();
  std::cout << "CHIP " << chipNum << ":" << std::endl;
  std::cout << input << std::endl;
  std::ifstream f(name.c_str());
  if (f.good()) { // if file exists
    gROOT->ProcessLine(input);
  }
  else {
    std::cout << "Input file does not exist." << std::endl;
  }
}

int All_FitThresholdTuneIB(const char *fName, bool generateMap = true,
                           bool thresholdNotNoise = false)
{ // Give it an arbitrary data file name...
  std::string Prefix = fName;
  Prefix.erase(Prefix.rfind("Chip"));
  std::cout << "Prefix: " << Prefix << std::endl;

  char fNameChip[100];

  for (int i = 0; i < 9; i++) {
    runFit(i, Prefix);
  }

  if (generateMap) {
    std::string slice = Prefix.substr(0, Prefix.rfind("/") + 1) + "FitValues" +
                        Prefix.substr(Prefix.rfind("ThresholdScan_") + 13, 15);

    std::string line = ".x ThresholdMapIB.C+g(\"" + slice + "Chip0_0.dat\"," +
                       std::to_string(thresholdNotNoise) + ")";
    gROOT->ProcessLine(line.c_str());
  }
  return 0;
}
