// Template to prepare standard test routines
// ==========================================
//
// The template is intended to prepare scans that work in the same way for the three setup types
//   - single chip with DAQ board
//   - IB stave with MOSAIC
//   - OB module with MOSAIC
// The setup type has to be set with the global variable fSetupType
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


// definition of standard setup types: 
//   - single chip with DAQ board
//   - IB stave with MOSAIC
//   - OB module with MOSAIC

typedef enum {setupSingle, setupIB, setupOB} TSetupType;

// chip ID that is used in case of single chip setup
const int singleChipId = 16;

// module ID that is used for outer barrel modules 
// (1 will result in master chip IDs 0x10 and 0x18, 2 in 0x20 and 0x28 ...)
const int moduleId = 1;

TBoardType fBoardType;
TSetupType fSetupType = setupSingle;

std::vector <TReadoutBoard *> fBoards;
std::vector <TAlpide *>       fChips;

TConfig *fConfig;


int powerOn (TReadoutBoardDAQ *aDAQBoard) {
  int overflow;

  if (aDAQBoard -> PowerOn (overflow)) std::cout << "LDOs are on" << std::endl;
  else std::cout << "LDOs are off" << std::endl;
  std::cout << "Version = " << std::hex << aDAQBoard->ReadFirmwareVersion() << std::dec << std::endl;
  aDAQBoard -> SendOpCode (Alpide::OPCODE_GRST);
  //sleep(1); // sleep necessary after GRST? or PowerOn?

  std::cout << "Analog Current  = " << aDAQBoard-> ReadAnalogI() << std::endl;
  std::cout << "Digital Current = " << aDAQBoard-> ReadDigitalI() << std::endl; 
}


// Setup definition for outer barrel module with MOSAIC
//    - module ID (3 most significant bits of chip ID) defined by moduleId 
//      (usually 1) 
//    - chips connected to two different control interfaces
//    - masters send data to two different receivers (0 and 1)
//    - receiver number for slaves set to -1 (not connected directly to receiver)
//      (this ensures that a receiver is disabled only if the connected master is disabled)
int initSetupOB() {
  std::vector <int> chipIDs;
  int offset = (moduleId & 0x7) << 4;
  for (int i = 0 + offset; i < 7  + offset; i++) chipIDs.push_back(i);
  for (int i = 8 + offset; i < 15 + offset; i++) chipIDs.push_back(i);

  fConfig       = new TConfig (1, chipIDs);
  fBoardType    = boardMOSAIC;

  fBoards.push_back (new TReadoutBoardMOSAIC((TBoardConfigMOSAIC*)fConfig->GetBoardConfig(0)));

  for (int i = 0; i < fConfig->GetNChips(); i++) {
    fChips.push_back(new TAlpide(fConfig->GetChipConfig(chipIDs.at(i))));
    fChips.at(i) -> SetReadoutBoard(fBoards.at(0));
    if (chipIDs.at(i) < 7 + offset) { // first master-slave row
      if (chipIDs.at(i) & 0x7) {        // slave
        fBoards.at(0)-> AddChip        (chipIDs.at(i), 0, -1);
      }
      else {                            // master
        fBoards.at(0)-> AddChip        (chipIDs.at(i), 0, 0);
      }
    }
    else {                    // second master-slave row
      if (chipIDs.at(i) & 0x7) {        // slave
        fBoards.at(0)-> AddChip        (chipIDs.at(i), 1, -1);
      }                                
      else {                            // master
        fBoards.at(0)-> AddChip        (chipIDs.at(i), 1, 1);
      }
    }
  }
}


// Setup definition for inner barrel stave with MOSAIC
//    - all chips connected to same control interface
//    - each chip has its own receiver, mapping defined in RCVMAP
int initSetupIB() {
  std::vector <int> chipIDs;
  for (int i = 0; i < 9; i++) chipIDs.push_back(i);

  int RCVMAP [] = { 3, 5, 7, 8, 6, 4, 2, 1, 0 };
  fConfig       = new TConfig (1, chipIDs);
  fBoardType    = boardMOSAIC;


  fBoards.push_back (new TReadoutBoardMOSAIC((TBoardConfigMOSAIC*)fConfig->GetBoardConfig(0)));

  for (int i = 0; i < fConfig->GetNChips(); i++) {
    fChips.push_back(new TAlpide(fConfig->GetChipConfig(chipIDs.at(i))));
    fChips.at(i) -> SetReadoutBoard(fBoards.at(0));
    fBoards.at(0)-> AddChip        (chipIDs.at(i), 0, RCVMAP[i]);
  }
}


int initSetupSingle() {
  TReadoutBoardDAQ  *myDAQBoard = 0;

  fConfig    = new TConfig (singleChipId);
  fBoardType = boardDAQ;
  
  InitLibUsb(); 
  //  The following code searches the USB bus for DAQ boards, creates them and adds them to the readout board vector: 
  //  TReadoutBoard *readoutBoard = new TReadoutBoardDAQ(device, config);
  //  board.push_back (readoutBoard);
  FindDAQBoards (fConfig, fBoards);
  std::cout << "Found " << fBoards.size() << " DAQ boards" << std::endl;
  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  if (fBoards.size() != 1) {
    std::cout << "Error in creating readout board object" << std::endl;
    return -1;
  }

  // create chip object and connections with readout board
  fChips. push_back(new TAlpide (fConfig->GetChipConfig(singleChipId)));
  fChips. at(0) -> SetReadoutBoard (fBoards.at(0));
  fBoards.at(0) -> AddChip         (singleChipId, 0, 0);

  powerOn(myDAQBoard);

}


int initSetup() {
  switch (fSetupType) 
    {
    case setupSingle: 
      initSetupSingle();
      break;
    case setupIB:
      initSetupIB();
      break;
    case setupOB:
      initSetupOB();
      break;
    default: 
      std::cout << "Unknown setup type, doing nothing" << std::endl;
      return -1;
    }
  return 0;
}


int configureChip(TAlpide *chip) {
  // put all chip configurations before the start of the test here
}


int main() {

  initSetup();

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
  if (fBoards.size() == 1) {
     
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (int i = 0; i < fChips.size(); i ++) {
      configureChip (fChips.at(i));
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     


    // put your test here... 


    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
