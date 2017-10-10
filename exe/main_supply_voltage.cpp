// Template to prepare standard test routines
// ==========================================
//
// After successful call to initSetup() the elements of the setup are accessible in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1 readout board, i.e. fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for staves and modules,
// all chip accesses should be done with a loop over all elements of the chip vector.
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);
// For an example how to access board-specific functions see the power off at the end of main.
//
// The functions that should be modified for the specific test are configureChip() and main()


#include <unistd.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include "TConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"

#include <iostream>
#include <iomanip>


TConfig* config;
std::vector <TReadoutBoard *> fBoards;
TBoardType boardType;
std::vector <TAlpide *> fChips;
TReadoutBoardDAQ *myDAQBoard;

unsigned int mySampleRepetition = 50;


int configureChip(TAlpide *chip) {
  // put all chip configurations before the start of the test here
  chip->WriteRegister (Alpide::REG_MODECONTROL,   0x20);
  chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x60);
  return 0;
}


int main(int argc, char** argv) {
  decodeCommandParameters(argc, argv);
  initSetup(config, &fBoards, &boardType, &fChips);

  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  if (fBoards.size() == 1) {

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    if ((myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0)))) {
      for (unsigned int i = 0; i < fChips.size(); i ++) {
        configureChip (fChips.at(i));
      }
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);

    std::cout << std::endl << std::endl;
    std::cout << std::setprecision(4);

    for (unsigned int i = 0; i < fChips.size(); i ++) {
      std::cout << std::endl << std::endl;
      std::cout << "== Chip " << i << std::endl;
      fChips.at(i)->CalibrateADC();

      float AVDD_direct = 0.;
      float AVDD_VTEMP  = 0.;
      float DVDD_direct = 0.;

      bool AVDD_saturated = false;
      bool DVDD_saturated = false;

      uint16_t theResult = 0;

      // AVDD direct
      fChips.at(i)->SetTheDacMonitor(Alpide::REG_ANALOGMON);
      fChips.at(i)->SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_AVDD, Alpide::COMP_296uA, Alpide::RAMP_1us);
      usleep(5000);
      for (unsigned int repetition = 0; repetition < mySampleRepetition; ++repetition) {
        fBoards.at(0)->SendOpCode (Alpide::OPCODE_ADCMEASURE, fChips.at(i));
        usleep(5000);
        fChips.at(i)->ReadRegister(Alpide::REG_ADC_AVSS, theResult);
        if (theResult == 1055) AVDD_saturated = true;
        AVDD_direct += 2. * ((float)theResult - (float)(fChips.at(i)->GetADCBias())) * 0.823e-3; // first approximation
      }
      AVDD_direct /= mySampleRepetition;
      std::cout << "AVDD (direct measurement): " << AVDD_direct << "V";
      if (AVDD_saturated)
        std::cout << ", out-of-range (>1.72V)!";
      std::cout << std::endl;
      if (AVDD_direct < 1.55)
        std::cout << "AVDD below 1.55V, indirect measurement unreliable" << std::endl;

      // AVDD via VTEMP @ 200
      fChips.at(i)->WriteRegister (Alpide::REG_VTEMP, 200);
      usleep(5000);

      for (unsigned int repetition = 0; repetition < mySampleRepetition; ++repetition) {
        // calculated AVDD based on VTEMP
        AVDD_VTEMP += fChips.at(i)->ReadDACVoltage(Alpide::REG_VTEMP) / 0.75;
      }
      AVDD_VTEMP /= mySampleRepetition;
      std::cout << "AVDD (via VTEMP @ 200 DAC): " << AVDD_VTEMP << "V" << std::endl;


      fChips.at(i)->SetTheDacMonitor(Alpide::REG_ANALOGMON);
      fChips.at(i)->SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_DVDD, Alpide::COMP_296uA, Alpide::RAMP_1us);
      usleep(5000);
      for (unsigned int repetition = 0; repetition < mySampleRepetition; ++repetition) {
        fBoards.at(0)->SendOpCode (Alpide::OPCODE_ADCMEASURE, fChips.at(i));
        usleep(5000);
        fChips.at(i)->ReadRegister(Alpide::REG_ADC_AVSS, theResult);
        if (theResult == 1055) DVDD_saturated = true;
        DVDD_direct += 2. * ((float)theResult - (float)(fChips.at(i)->GetADCBias())) * 0.823e-3; // first approximation
      }
      DVDD_direct /= mySampleRepetition;
      std::cout << "DVDD (direct measurement): " << DVDD_direct << "V";
      if (DVDD_saturated)
        std::cout << ", out-of-range (>1.72V)!";
      std::cout << std::endl;
    }

    std::cout << std::endl << std::endl;

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
