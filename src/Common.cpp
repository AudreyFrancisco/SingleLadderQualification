#include <iomanip>

#include "Common.h"

#include "THisto.h"


std::string common::GetFileName(TChipIndex aChipIndex, 
				std::string suffix){
  std::string fileName;
  fileName+= suffix;
  fileName+= "-B";
  fileName+= std::to_string(aChipIndex.boardIndex); 
  fileName+= "-Rx";
  fileName+= std::to_string(aChipIndex.dataReceiver);
  fileName+= "-chip";
  fileName+= std::to_string(aChipIndex.chipId);
  fileName+= ".dat";
  
  return fileName;
}


int common::GetChipIntIndex(TChipIndex aChipIndex){
  
  int intIndexDummy = (aChipIndex.boardIndex << 8) | (aChipIndex.dataReceiver << 4) | (aChipIndex.chipId & 0xf);
  
  return intIndexDummy;
}

common::TChipIndex common::GetChipIndex(int aIntIndex){
  
  TChipIndex index;
  index.boardIndex   = (aIntIndex >> 8);
  index.dataReceiver = (aIntIndex >> 4) & 0xf;
  index.chipId       =  aIntIndex       & 0xf;
  
  return index;
}


bool common::HitBelongsToChip(TChipIndex aChipIndex, TPixHit aHit) 
{
  if ((aChipIndex.boardIndex   == aHit.boardIndex) &&
      (aChipIndex.dataReceiver == aHit.channel) && 
      (aChipIndex.chipId       == aHit.chipId)) return true;
  return false;
}


int common::FindIndexForHit (std::vector <TChipIndex> aChipList, TPixHit aHit) 
{
  for (int ichip = 0; ichip < aChipList.size(); ichip++) {
    if (HitBelongsToChip(aChipList.at(ichip), aHit)) return ichip;
  }
  return -1;
}


std::vector <common::TChipIndex> common::GetChipList (TScanHisto* aScanHisto){
  
  std::vector <TChipIndex> chipList;
  
  std::map<int, THisto> histoMap_dummy= aScanHisto->GetHistoMap();
  
  for (std::map<int, THisto>::iterator it=histoMap_dummy.begin();
       it!=histoMap_dummy.end(); 
       ++it) {
    
    int        intIndex = it->first;
    TChipIndex index;
    index.boardIndex   = (intIndex >> 8);
    index.dataReceiver = (intIndex >> 4) & 0xf;
    index.chipId       =  intIndex       & 0xf;
    chipList.push_back(index);
  }
  
  return chipList;
}
