#include <iostream>
#include "SetupHelpers.h"
#include "USBHelpers.h"


TBoardType fBoardType;


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
  fBoardType    = boardMOSAIC;

  fBoards.push_back (new TReadoutBoardMOSAIC((TBoardConfigMOSAIC*)fConfig->GetBoardConfig(0)));

  for (int i = 0; i < fConfig->GetNChips(); i++) {
    TChipConfig *chipConfig = fConfig   ->GetChipConfig(i);
    int          chipId     = chipConfig->GetChipId    ();

    fChips.push_back(new TAlpide(chipConfig));
    fChips.at(i) -> SetReadoutBoard(fBoards.at(0));
    if (i < 7) { // first master-slave row
      if (chipId & 0x7) {        // slave
        fBoards.at(0)-> AddChip        (chipId, 0, -1);
      }
      else {                            // master
        fBoards.at(0)-> AddChip        (chipId, 0, 0);
      }
    }
    else {                    // second master-slave row
      if (chipId & 0x7) {        // slave
        fBoards.at(0)-> AddChip        (chipId, 1, -1);
      }                                
      else {                            // master
        fBoards.at(0)-> AddChip        (chipId, 1, 1);
      }
    }
  }
}


// Setup definition for inner barrel stave with MOSAIC
//    - all chips connected to same control interface
//    - each chip has its own receiver, mapping defined in RCVMAP
int initSetupIB() {
  int RCVMAP [] = { 3, 5, 7, 8, 6, 4, 2, 1, 0 };
  fBoardType    = boardMOSAIC;

  fBoards.push_back (new TReadoutBoardMOSAIC((TBoardConfigMOSAIC*)fConfig->GetBoardConfig(0)));

  for (int i = 0; i < fConfig->GetNChips(); i++) {
    TChipConfig *chipConfig = fConfig->GetChipConfig(i);
    fChips.push_back(new TAlpide(chipConfig));
    fChips.at(i) -> SetReadoutBoard(fBoards.at(0));
    fBoards.at(0)-> AddChip        (chipConfig->GetChipId(), 0, RCVMAP[i]);
  }
}


int initSetupSingleMosaic() {
  int          ReceiverId = 4;  // HSData is connected to pins for first chip on a stave
  TChipConfig *chipConfig = fConfig->GetChipConfig(0);
  fBoardType              = boardMOSAIC;

  fBoards.push_back (new TReadoutBoardMOSAIC((TBoardConfigMOSAIC*)fConfig->GetBoardConfig(0)));

  fChips. push_back(new TAlpide(chipConfig));
  fChips. at(0) -> SetReadoutBoard(fBoards.at(0));
  fBoards.at(0) -> AddChip        (chipConfig->GetChipId(), 0, ReceiverId);
}


int initSetupSingle() {
  TReadoutBoardDAQ  *myDAQBoard = 0;
  TChipConfig       *chipConfig = fConfig->GetChipConfig(0);
  fBoardType                    = boardDAQ;
  
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
  fChips. push_back(new TAlpide (chipConfig));
  fChips. at(0) -> SetReadoutBoard (fBoards.at(0));
  fBoards.at(0) -> AddChip         (chipConfig->GetChipId(), 0, 0);

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
  std::cout << "Temperature     = " << aDAQBoard-> ReadTemperature() << std::endl; 
}


int initSetup(const char *configFileName) {
  fConfig = new TConfig ("Config.cfg");  
  switch (fConfig->GetDeviceType())
    {
    case TYPE_CHIP: 
      initSetupSingle();
      break;
    case TYPE_STAVE:
      initSetupIB();
      break;
    case TYPE_MODULE:
      initSetupOB();
      break;
    case TYPE_CHIP_MOSAIC: 
      initSetupSingleMosaic();
      break;
    default: 
      std::cout << "Unknown setup type, doing nothing" << std::endl;
      return -1;
    }
  return 0;
}

