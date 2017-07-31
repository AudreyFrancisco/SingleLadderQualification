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



TBoardType fBoardType;
std::vector <TReadoutBoard *> fBoards;
std::vector <TAlpide *>       fChips;
TConfig *fConfig;


int fEnabled = 0;  // variable to count number of enabled chips; leave at 0


int configureChip(TAlpide *chip) {
  // put all chip configurations before the start of the test here
  chip->WriteRegister (Alpide::REG_MODECONTROL,   0x20);
  if (fConfig->GetDeviceType() == TYPE_CHIP)
    chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x60);

  return 0;
}


int main(int argc, char** argv) {

  decodeCommandParameters(argc, argv);

  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  if (fBoards.size() == 1) {

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (unsigned int i = 0; i < fChips.size(); i ++) {
      configureChip (fChips.at(i));
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);

    for (unsigned int ichip = 0; ichip < fChips.size(); ichip++) {
      if (! fChips.at(ichip)->GetConfig()->IsEnabled()) continue;

      fEnabled ++;

    }
    std::cout << fEnabled << " chips were enabled for scan." << std::endl << std::endl << std::endl;

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
