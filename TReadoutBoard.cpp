#include "TReadoutBoard.h"


TReadoutBoard::TReadoutBoard (TBoardConfig *config) 
{
  fNChips = 0;
  //fChipPositions (0);
  fBoardConfig = config;
}



int TReadoutBoard::AddChip (uint8_t chipId, int controlInterface, int receiver) 
{
  if (GetControlInterface (chipId) >= 0) {
    // throw exception -> duplicate chip id
    return -1; 
  }

  TChipPos newChip;
  newChip.chipId           = chipId; 
  newChip.controlInterface = controlInterface;
  newChip.receiver         = receiver;

  fChipPositions.push_back(newChip);
  fNChips ++;
  return 0;
}


int TReadoutBoard::GetControlInterface (uint8_t chipId) 
{
  for (int i = 0; i < fChipPositions.size(); i ++) {
    if (fChipPositions.at(i).chipId == chipId) return fChipPositions.at(i).controlInterface;
  }
  return -1;  // throw exception -> non-existing chip
}


int TReadoutBoard::GetReceiver(uint8_t chipId)
{
  for (int i = 0; i < fChipPositions.size(); i ++) {
    if (fChipPositions.at(i).chipId == chipId) return fChipPositions.at(i).receiver;
  }
  return -1;  // throw exception -> non-existing chip
}
