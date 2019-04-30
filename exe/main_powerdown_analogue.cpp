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

TConfig *                    config;
std::vector<TReadoutBoard *> fBoards;
TBoardType                   boardType;
std::vector<TAlpide *>       fChips;

int main(int argc, char **argv)
{
  time_t     t   = time(0); // get time now
  struct tm *now = localtime(&t);
  char       Suffix[80];
  snprintf(Suffix, 80, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1,
           now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  decodeCommandParameters(argc, argv);
  initSetup(config, &fBoards, &boardType, &fChips);

  if (fBoards.size()) {

    for (const auto &rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_GRST);
      rBoard->SendOpCode(Alpide::OPCODE_PRST);
    }

    for (const auto &rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_RORST);
    }

    for (unsigned int i = 0; i < fChips.size(); i++) {
      /*
      fChips.at(i)->WriteRegister(Alpide::REG_VRESETP, 0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_VRESETD, 0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_VCASP,   0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_VCASN,   0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_VPULSEH, 0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_VPULSEL, 0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_VCASN2,  0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_VCLIP,   0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_VTEMP,   0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_IAUX2,   0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_IRESET,  0x0);
      fChips.at(i)->WriteRegister(Alpide::REG_IDB,     0x0);
      */
      fChips.at(i)->WriteRegister(Alpide::REG_IBIAS, 0x0);
      // fChips.at(i)->WriteRegister(Alpide::REG_ITHR,    0x0);
    }

    std::cout << std::endl << std::endl;
    std::cout << "Analogue current should now be below 50mA!" << std::endl;
    std::cout << std::endl << std::endl;

    for (const auto &rBoard : fBoards) {
      TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(rBoard);
      if (myMOSAIC) myMOSAIC->enableControlInterfaces(false);
    }
  }

  return 0;
}
