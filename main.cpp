#include <unistd.h>
#include "TAlpide.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "USBHelpers.h"
#include "TConfig.h"

int main() {
  uint16_t          status;
  uint32_t          version;
  int               overflow;
  TReadoutBoardDAQ  *myDAQBoard;
  TConfig *config = new TConfig (16);

  // if (config->BoardType == DAQBoard)
  // 
  //  The following code searches the USB bus for DAQ boards, creates them and adds them to the readout board vector: 
  //  TReadoutBoard *readoutBoard = new TReadoutBoardDAQ(device, config);
  //  board.push_back (readoutBoard);
  std::vector <TReadoutBoard *> boards;
  InitLibUsb(); 
  FindDAQBoards (config, boards);

  std::cout << "found " << boards.size() << " DAQ boards" << std::endl;
  
  if (boards.size() == 1) {
    TAlpide *chip   = new TAlpide (config, 16);
    chip         -> SetReadoutBoard (boards.at(0));
    boards.at(0) -> AddChip (0, 0, 0);
    
    myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (boards.at(0));
    if (myDAQBoard) {
      if (myDAQBoard -> PowerOn (overflow)) std::cout << "LDOs are on" << std::endl;
      else std::cout << "LDOs are off" << std::endl;
      myDAQBoard->ReadRegister (0x602, version); 
      std::cout << "Version = " << std::hex << version << std::dec << std::endl;
      myDAQBoard->WriteRegister (0x402, 3);
      myDAQBoard->WriteRegister (0x500, 0x0220);
      myDAQBoard -> SendOpCode (Alpide::OPCODE_GRST);
      std::cout << "Analog Current = " << myDAQBoard-> ReadAnalogI() << std::endl;
      std::cout << "Digital Current = " << myDAQBoard-> ReadDigitalI() << std::endl;
      
      //chip -> WriteRegister (Alpide::REG_MODECONTROL, 0x20);
      //std::cout << "After Write register " << std::endl;
      chip -> ReadRegister (Alpide::REG_IBIAS, status);
      std::cout << "IBias register value: 0x" << std::hex << status << std::dec << std::endl;
      chip->WriteRegister (Alpide::REG_IBIAS, 0);
      sleep(1);
      std::cout << "Analog Current = " << myDAQBoard-> ReadAnalogI() << std::endl;

      chip -> ReadRegister (Alpide::REG_MODECONTROL, status);

      std::cout << "Control register value: 0x" << std::hex << status << std::dec << std::endl;

      chip -> ReadRegister (Alpide::REG_IBIAS, status);
      std::cout << "IBias register value: 0x" << std::hex << status << std::dec << std::endl;

      //libusb_exit(fContext);
    }
    else {
      std::cout << "Type cast failed" << std::endl;
    }

  }

  return 0;
}
