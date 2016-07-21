#include "TAlpide.h"
#include "TReadoutBoard.h"
#include "TDAQBoard.h"
#include "USBHelpers.h"

int main() {
  // if (config->BoardType == DAQBoard)
  // 
  //  The following code searches the USB bus for DAQ boards, creates them and adds them to the readout board vector: 
  //  TReadoutBoard *readoutBoard = new TDAQBoard(device, config);
  //  board.push_back (readoutBoard);
  std::vector <TReadoutBoard *> boards;
  InitLibUsb(); 
  FindDAQBoards (0 /* config */, boards);

  std::cout << "found " << boards.size() << " DAQ boards" << std::endl;
  
  if (boards.size() == 1) {
    TAlpide *chip = new TAlpide (0 /* config */, 0 /* chipId */);
    chip -> SetReadoutBoard (boards.at(0));
    boards.at(0) -> AddChip (0, 0, 0);

    chip -> WriteRegister (Alpide::REG_COMMAND, 0x0);
  }

  return 0;
}
