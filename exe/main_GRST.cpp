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

TBoardType fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *> fChips;
TConfig *fConfig;

int main(int argc, char **argv) {

  decodeCommandParameters(argc, argv);

  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  if (fBoards.size()) { // Yasser (change to use more than one board)

    for (const auto &rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_GRST);
      rBoard->SendOpCode(Alpide::OPCODE_PRST);
      rBoard->SendOpCode(Alpide::OPCODE_RORST);

      TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(rBoard);
      if (myDAQBoard) {
        myDAQBoard->PowerOff();
        delete myDAQBoard;
      }
    }
  }

  return 0;
}
