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
#include <cstdlib>
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


int main() {

  uint16_t address1 = 0x000E;
  uint16_t value1 = 0xFFFF;
  uint16_t address2 = 0x000F;
  uint16_t value2 = 0xFFFF;

  initSetup();

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  if (fBoards.size() == 1) {

    std::system("./poweron_chip.py");

    fChips.at(0)->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x60);

    sleep(0.1);

    fChips.at(0)->ReadRegister (address1, value1);
    fChips.at(0)->ReadRegister (address2, value2);

    std::cerr << "1st Readback 0x" << std::hex << value1 << std::dec
              << "\t0x"            << std::hex << value2 << std::dec << std::endl;

    if ((value1==0xaaa) && (value2==0xaa)) {
      if (myDAQBoard) {
        myDAQBoard->PowerOff();
        delete myDAQBoard;
      }
      return 0;
    }

    value1 = 0xFFFF;
    value2 = 0xFFFF;

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);

    sleep(1.);

    fChips.at(0)->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x60);

    fChips.at(0)->ReadRegister (address1, value1);
    fChips.at(0)->ReadRegister (address2, value2);

    std::cerr << "2nd Readback 0x" << std::hex << value1 << std::dec
              << "\t0x"            << std::hex << value2 << std::dec << std::endl;
  }

  if (myDAQBoard) {
    myDAQBoard->PowerOff();
    delete myDAQBoard;
  }

  if ((value1==0xaaa) && (value2==0xaa)) return 0;
  else return 1;
}
