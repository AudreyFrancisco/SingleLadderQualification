#include "TConfig.h" 

//construct Config from config file
TConfig::TConfig (const char *fName) {
}


// construct Config in the application using only number of boards and number of chips
TConfig::TConfig (int nBoards, int nChips) {
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
