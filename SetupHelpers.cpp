#include <iostream>
#include "SetupHelpers.h"
#include "USBHelpers.h"

int fSingleChipId;
int fModuleId;

TBoardType fBoardType;
TSetupType fSetupType;


std::vector <TReadoutBoard *> fBoards;
std::vector <TAlpide *>       fChips;

TConfig *fConfig;



// Setup definition for outer barrel module with MOSAIC
//    - module ID (3 most significant bits of chip ID) defined by moduleId 
//      (usually 1) 
//    - chips connected to two different control interfaces
//    - masters send data to two different receivers (0 and 1)
//    - receiver number for slaves set to -1 (not connected directly to receiver)
//      (this ensures that a receiver is disabled only if the connected master is disabled)
int initSetupOB() {
  std::vector <int> chipIDs;
  int offset = (fModuleId & 0x7) << 4;
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

  fConfig    = new TConfig (fSingleChipId);
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
  fChips. push_back(new TAlpide (fConfig->GetChipConfig(fSingleChipId)));
  fChips. at(0) -> SetReadoutBoard (fBoards.at(0));
  fBoards.at(0) -> AddChip         (fSingleChipId, 0, 0);

  powerOn(myDAQBoard);

}


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

