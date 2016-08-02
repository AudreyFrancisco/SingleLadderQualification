//
//  ThresholdScan.cpp
//
// example file for a threshold scan. The code currently assumes one DAQ board with one chip;
// however, for the more general case only InitSetup needs to be adapted



#include <stdio.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "USBHelpers.h"
#include "TConfig.h"

std::vector <TReadoutBoard *> fBoards;
std::vector <TAlpide *>       fChips;
TConfig                      *fConfig;


bool InitSetup() {
  TReadoutBoardDAQ *myDAQBoard;
  int               overflow;
  
  InitLibUsb();
  FindDAQBoards (fConfig, fBoards);
  
  if (boards.size() != 1) { 
    std::cout << "Problem in finding DAQ board" << std::endl;
    return false;
  }
  
  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (boards.at(0));
  
  if (! myDAQBoard->PowerOn(overflow) ) {
    std::cout << "LDOs did not switch on" << std::endl;
    return false;
  }
  
  return true;
}


int main() {
  if (!InitSetup()) exit(1);

  // pre-loop initialisation
  for (std::vector<TAlpide*>::iterator ichip = fChips.begin(); ichip != fChips.end(); ichip++) {
    AlpideConfig::Init                     (ichip);
    AlpideConfig::ApplyStandardDACSettings (ichip, 0.0);      // 0.0 = no back bias
    AlpideConfig::ConfigureFromu           (ichip, Alpide::PT_ANALOGUE, false);
    AlpideConfig::WritePixRegAll           (ichip, PIXREG_MASK, false);
    AlpideConfig::WritePixRegAll           (ichip, PIXREG_SELECT, false);
    ichip->WriteRegister                   (Alpide::REG_VPULSEH, 170);
  }
  
  // boards.at(0) -> SetTriggerConfig
  for (int irow = 0; irow < 512; irow ++) {
    // row selection
    for (std::vector<TAlpide*>::iterator ichip = fChips.begin(); ichip != fChips.end(); ichip++) {
      if (irow > 0) AlpideConfig::WritePixRegRow (ichip, PIXREG_SELECT, false, irow -1);
      AlpideConfig::WritePixRegRow (ichip, PIXREG_SELECT, true, irow);
    }
    for (int icharge = 0; icharge < 50; icharge ++) {
      // charge selection
      for (std::vector<TAlpide*>::iterator ichip = fChips.begin(); ichip != fChips.end(); ichip++) {
        ichip->WriteRegister (Alpide::REG_VPULSEL, 170 - icharge);
      }
      //boards.at(0)->Trigger
      //boards.at(0)->GetEventdata
      //Decode...
    }
  }
  
}