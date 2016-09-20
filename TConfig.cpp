#include "TConfig.h" 
#include "TBoardConfigDAQ.h"
#include "TBoardConfigMOSAIC.h"
#include <iostream>

//construct Config from config file
TConfig::TConfig (const char *fName) {
  ReadConfigFile (fName);
}


// construct Config in the application using only number of boards and number of chips / vector of chip Ids
// for the time being use one common config for all board types (change this?)
TConfig::TConfig (int nBoards, std::vector <int> chipIds, TBoardType boardType) {
  Init(nBoards, chipIds, boardType);
}


// construct a config for a single chip setup (one board and one chip only)
TConfig::TConfig (int chipId, TBoardType boardType) {
  Init(chipId, boardType);
}


void TConfig::Init (int nBoards, std::vector <int> chipIds, TBoardType boardType) {
  for (int iboard = 0; iboard < nBoards; iboard ++) {
    if (boardType == boardDAQ) {
      fBoardConfigs.push_back (new TBoardConfigDAQ());
    } 
    else if (boardType == boardMOSAIC) {
      fBoardConfigs.push_back (new TBoardConfigMOSAIC());
    }
    else {
      std::cout << "TConfig: Unknown board type" << std::endl;
    }
  }
  for (int ichip = 0; ichip < chipIds.size(); ichip ++) {
    fChipConfigs.push_back (new TChipConfig(chipIds.at(ichip)));
  } 
}


void TConfig::Init (int chipId, TBoardType boardType) {
  if (boardType == boardDAQ) {
    fBoardConfigs.push_back (new TBoardConfigDAQ());
  } 
  else if (boardType == boardMOSAIC) {
    fBoardConfigs.push_back (new TBoardConfigMOSAIC());
  }
  else {
    std::cout << "TConfig: Unknown board type" << std::endl;
  }

  fChipConfigs. push_back (new TChipConfig (chipId));
}

// getter functions for chip and board config
TChipConfig *TConfig::GetChipConfig  (int chipId) {
  for (int i = 0; i < fChipConfigs.size(); i++) {
    if (fChipConfigs.at(i)->GetChipId() == chipId) 
      return fChipConfigs.at(i);
  }
  // throw exception here.
  return 0;
}


TBoardConfig *TConfig::GetBoardConfig (int iBoard){
  if (iBoard < fBoardConfigs.size()) {
    return fBoardConfigs.at(iBoard);
  }
  else {  // throw exception
    return 0;
  }
}


void TConfig::ReadConfigFile (const char *fName) 
{
  FILE *fp = fopen (fName, "r");

  if (!fp) {
    std::cout << "WARNING: Config file " << fName << " not found, using default configuration." << std::endl;
    return;
  }

}



// write config to file, has to call same function for all sub-configs (chips and boards)
void TConfig::WriteToFile (const char *fName) {
}
