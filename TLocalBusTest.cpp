#include <iostream>
#include "TLocalBusTest.h"


TLocalBusTest::TLocalBusTest (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue) 
  : TScan (config, chips, boards, histoQue) 
{
  int i = 0;
}


int TLocalBusTest::GetChipById (std::vector <TAlpide*> chips, int id) 
{
  for (int i = 0; i < chips.size(); i++) {
    if (chips.at(i)->GetConfig()->GetChipId() == id) return i;
  }

  return -1;
}


int TLocalBusTest::FindDaisyChains(std::vector <TAlpide *> chips) 
{
  int totalChips = 0;
  int iChip      = 0;
  std::vector <TAlpide *> daisyChain;

  while (totalChips < chips.size())
  {
    // find next enabled chip
    for (iChip = totalChips; (iChip < chips.size()) && (!(chips.at(iChip)->GetConfig()->IsEnabled())); iChip++) {
      totalChips ++;
    }
     
    // go back through daisy chain until next chip would be iChip
    int chipId = chips.at(iChip)->GetConfig()->GetChipId();
    int iiChip = iChip;

    while (chips.at(iiChip)->GetConfig()->GetPreviousId() != chipId); {
      int previousId = chips.at(iiChip)->GetConfig()->GetPreviousId();
      iiChip         = GetChipById (chips, previousId);
      if (iiChip < 0) {
	std::cout << "Something went wrong, Did not find chip Id" << previousId << std::endl;
        exit(1);
      }
      totalChips ++;
      daisyChain.push_back(chips.at(iiChip));
    }
    m_daisyChains.push_back(daisyChain);
    daisyChain.clear();    
  }

}


void TLocalBusTest::Execute() 
{
}
