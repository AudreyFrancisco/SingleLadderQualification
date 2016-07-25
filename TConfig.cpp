#include "TConfig.h" 
#include "TBoardConfigDAQ.h"

//construct Config from config file
TConfig::TConfig (const char *fName) {
}


// construct Config in the application using only number of boards and number of chips / vector of chip Ids
// for the time being use one common config for all board types (change this?)
TConfig::TConfig (int nBoards, std::vector <int> chipIds) {
  for (int iboard = 0; iboard < nBoards; iboard ++) {
    fBoardConfigs.push_back (new TBoardConfigDAQ());
  }
  for (int ichip = 0; ichip < chipIds.size(); ichip ++) {
    fChipConfigs.push_back (new TChipConfig(chipIds.at(ichip)));
  } 
}


// construct a config for a single chip setup (one board and one chip only)
TConfig::TConfig (int chipId) {
  fBoardConfigs.push_back (new TBoardConfigDAQ());
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


// write config to file, has to call same function for all sub-configs (chips and boards)
void TConfig::WriteToFile (const char *fName) {
}
