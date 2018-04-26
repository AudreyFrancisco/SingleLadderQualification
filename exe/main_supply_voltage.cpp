// Template to prepare standard test routines
// ==========================================
//
// After successful call to initSetup() the elements of the setup are accessible in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1 readout board, i.e.
// fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for staves and modules,
// all chip accesses should be done with a loop over all elements of the chip vector.
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);
// For an example how to access board-specific functions see the power off at the end of main.
//
// The functions that should be modified for the specific test are configureChip() and main()

#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TAlpide.h"
#include "TConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include <unistd.h>

#include <iomanip>
#include <iostream>

TConfig *                    config;
std::vector<TReadoutBoard *> fBoards;
TBoardType                   boardType;
std::vector<TAlpide *>       fChips;

unsigned int mySampleRepetition = 30;

int configureChip(TAlpide *chip)
{
  // put all chip configurations before the start of the test here
  chip->WriteRegister(Alpide::REG_MODECONTROL, 0x20);
  chip->WriteRegister(Alpide::REG_CMUDMU_CONFIG, 0x60);
  AlpideConfig::ConfigureCMU(chip);
  return 0;
}

int main(int argc, char **argv)
{
  decodeCommandParameters(argc, argv);
  initSetup(config, &fBoards, &boardType, &fChips);

  if (fBoards.size()) {

    for (const auto &rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_GRST);
      rBoard->SendOpCode(Alpide::OPCODE_PRST);
    }

    for (const auto &rChip : fChips) {
      if (!rChip->GetConfig()->IsEnabled()) continue;
      configureChip(rChip);
    }

    for (const auto &rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_RORST);
    }

    std::cout << std::endl << std::endl;
    std::cout << std::setprecision(4);

    for (unsigned int i = 0; i < fChips.size(); i++) {
      if (!fChips.at(i)->GetConfig()->IsEnabled()) continue;
      std::cout << std::endl << std::endl;
      std::cout << "== Chip " << i << ", ID: " << fChips.at(i)->GetConfig()->GetChipId()
                << std::endl;
      fChips.at(i)->CalibrateADC();

      float AVDD_direct = 0.;
      float AVDD_VTEMP  = 0.;
      float DVDD_direct = 0.;

      bool AVDD_saturated = false;
      bool DVDD_saturated = false;

      uint16_t theResult = 0;

      // AVDD direct
      for (unsigned int repetition = 0; repetition < mySampleRepetition; ++repetition) {
        fChips.at(i)->SetTheDacMonitor(Alpide::REG_ANALOGMON);
        fChips.at(i)->SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_AVDD,
                                            Alpide::COMP_296uA, Alpide::RAMP_1us);
        usleep(5000);
        fChips.at(i)->GetReadoutBoard()->SendCommand(Alpide::COMMAND_ADCMEASURE, fChips.at(i));
        usleep(5000);
        fChips.at(i)->ReadRegister(Alpide::REG_ADC_AVSS, theResult);
        if (theResult == 1055) AVDD_saturated = true;
        AVDD_direct += 2. * ((float)theResult - (float)(fChips.at(i)->GetADCOffset())) *
                       0.823e-3; // first approximation
      }
      AVDD_direct /= mySampleRepetition;
      std::cout << "AVDD (direct measurement): " << AVDD_direct << "V";
      if (AVDD_saturated) std::cout << ", out-of-range (>1.72V)!";
      std::cout << std::endl;
      if (AVDD_direct < 1.55 && AVDD_direct > 0.)
        std::cout << "AVDD below 1.55V, indirect measurement unreliable" << std::endl;

      // AVDD via VTEMP @ 200
      fChips.at(i)->WriteRegister(Alpide::REG_VTEMP, 200);
      usleep(5000);

      for (unsigned int repetition = 0; repetition < mySampleRepetition; ++repetition) {
        // calculated AVDD based on VTEMP
        AVDD_VTEMP += fChips.at(i)->ReadDACVoltage(Alpide::REG_VTEMP) / 0.772 + 0.023;
      }
      AVDD_VTEMP /= mySampleRepetition;
      std::cout << "AVDD (via VTEMP @ 200 DAC): " << AVDD_VTEMP << "V" << std::endl;

      fChips.at(i)->SetTheDacMonitor(Alpide::REG_ANALOGMON);
      fChips.at(i)->SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_DVDD, Alpide::COMP_296uA,
                                          Alpide::RAMP_1us);
      usleep(5000);
      for (unsigned int repetition = 0; repetition < mySampleRepetition; ++repetition) {
        fChips.at(i)->SetTheDacMonitor(Alpide::REG_ANALOGMON);
        fChips.at(i)->SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_AVDD,
                                            Alpide::COMP_296uA, Alpide::RAMP_1us);
        usleep(5000);
        fChips.at(i)->GetReadoutBoard()->SendCommand(Alpide::COMMAND_ADCMEASURE, fChips.at(i));
        usleep(5000);
        fChips.at(i)->ReadRegister(Alpide::REG_ADC_AVSS, theResult);
        if (theResult == 1055) DVDD_saturated = true;
        DVDD_direct += 2. * ((float)theResult - (float)(fChips.at(i)->GetADCOffset())) *
                       0.823e-3; // first approximation
      }
      DVDD_direct /= mySampleRepetition;
      std::cout << "DVDD (direct measurement): " << DVDD_direct << "V";
      if (DVDD_saturated) std::cout << ", out-of-range (>1.72V)!";
      std::cout << std::endl;
    }

    std::cout << std::endl << std::endl;

    for (const auto &rBoard : fBoards) {
      TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(rBoard);
      if (myDAQBoard) {
        myDAQBoard->PowerOff();
        delete myDAQBoard;
      }
    }
  }

  return 0;
}
