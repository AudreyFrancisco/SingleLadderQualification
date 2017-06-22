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

bool Verbose = false;
int  fErrCount0;
int  fErrCount5;
int  fErrCountf;

int fEnabled = 0;  // variable to count number of enabled chips; leave at 0

int fTotalErr;

int configureChip(TAlpide *chip) {
  // put all chip configurations before the start of the test here
  chip->WriteRegister (Alpide::REG_MODECONTROL,   0x20);
  if (fConfig->GetDeviceType() == TYPE_CHIP)
    chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x60);

  return 0;
}


void WriteMem (TAlpide *chip, int ARegion, int AOffset, int AValue) {
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "WriteMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;
  
  uint16_t LowVal  = AValue & 0xffff;
  uint16_t HighVal = (AValue >> 16) & 0xff;

  int err = chip->WriteRegister (LowAdd,  LowVal);
  if (err >= 0) err = chip->WriteRegister (HighAdd, HighVal);
 
  if (err < 0) {
    std::cout << "Cannot write chip register. Exiting ... " << std::endl;
    exit (1);
  }
 
}


void ReadMem (TAlpide *chip, int ARegion, int AOffset, int &AValue) {
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "ReadMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;
 
  uint16_t LowVal, HighVal;

  int err = chip->ReadRegister (LowAdd, LowVal);
  if (err >= 0) err = chip->ReadRegister (HighAdd, HighVal);

  if (err < 0) {
    std::cout << "Cannot read chip register. Exiting ... " << std::endl;
    exit (1);
  }

  // Note to self: if you want to shorten the following lines, 
  // remember that HighVal is 16 bit and (HighVal << 16) will yield 0 
  // :-)
  AValue = (HighVal & 0xff);
  AValue <<= 16;
  AValue |= LowVal;
}


bool MemReadback (TAlpide *chip, int ARegion, int AOffset, int AValue) {
  int Value;

  WriteMem (chip, ARegion, AOffset, AValue); 
  ReadMem  (chip, ARegion, AOffset, Value);
 
  if (Value != AValue) {
    if (Verbose) {
      std::cout << "Error in mem " << ARegion << "/0x" << std::hex << AOffset << ": wrote " << AValue << ", read: " << Value << std::dec << std::endl;
    }
    return false;
  }
  return true;
}


void MemTest (TAlpide *chip, int ARegion, int AOffset) {
  if (! MemReadback (chip, ARegion, AOffset, 0x0))      fErrCount0 ++;
  if (! MemReadback (chip, ARegion, AOffset, 0x555555)) fErrCount5 ++;
  if (! MemReadback (chip, ARegion, AOffset, 0xffffff)) fErrCountf ++;
}


int main(int argc, char** argv) {

  decodeCommandParameters(argc, argv);

  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
  if (fBoards.size() == 1) {
     
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (int i = 0; i < fChips.size(); i ++) { 
      configureChip (fChips.at(i));
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

    fTotalErr  = 0;

    for (int ichip = 0; ichip < fChips.size(); ichip++) {
      if (! fChips.at(ichip)->GetConfig()->IsEnabled()) continue;
      
      fEnabled ++;

      std::cout << std::endl << "Doing FIFO test on ControlInterface " << fChips.at(ichip)->GetConfig()->GetCtrInt( )<< "  chip ID " << fChips.at(ichip)->GetConfig()->GetChipId() << std::endl;
      // Reset error counters
      fErrCount0 = 0;
      fErrCount5 = 0;
      fErrCountf = 0;

      // Do the loop over all memories
      for (int ireg = 0; ireg < 32; ireg++) {
        std::cout << "FIFO scan: region " << ireg << std::endl;
        for (int iadd = 0; iadd < 128; iadd ++) {
          MemTest (fChips.at(ichip), ireg, iadd);
        }
      }

      // Output result
      std::cout << "Test finished: error counters: " << std::endl; 
      std::cout << "  pattern 0x0:      " << fErrCount0 << std::endl;
      std::cout << "  pattern 0x555555: " << fErrCount5 << std::endl;
      std::cout << "  pattern 0xffffff: " << fErrCountf << std::endl;
      std::cout << "(total number of tested memories: 32 * 128 = 4096)" << std::endl;

      if (fErrCount0 + fErrCount5 + fErrCountf > 0) 
        std::cout << "Set <Verbose> in source code to get single errors" << std::endl;
      fTotalErr += fErrCount0 + fErrCount5 + fErrCountf;
    }

    std::cout << std::endl << "Total error count (all chips): " << fTotalErr << std::endl << std::endl;

    std::cout << fEnabled << " chips were enabled for scan." << std::endl << std::endl << std::endl;

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
