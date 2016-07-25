#include "TChipConfig.h"

TChipConfig::TChipConfig (int chipId, const char *fName) {
  fChipId = chipId;
  // fill default values from header file 

  if (fName) {
    // read information from file
  }
}
