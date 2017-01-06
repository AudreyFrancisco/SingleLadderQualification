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
  fBoardType                      = boardMOSAIC;
  TBoardConfigMOSAIC *boardConfig = (TBoardConfigMOSAIC*) fConfig->GetBoardConfig(0);

  boardConfig->SetInvertedData (true);
  boardConfig->SetSpeedMode    (Mosaic::RCV_RATE_400);

  fBoards.push_back (new TReadoutBoardMOSAIC(boardConfig));

  for (int i = 0; i < fConfig->GetNChips(); i++) {
    TChipConfig *chipConfig = fConfig   ->GetChipConfig(i);
    int          chipId     = chipConfig->GetChipId    ();
    int          control    = chipConfig->GetParamValue("CONTROLINTERFACE");
    int          receiver   = chipConfig->GetParamValue("RECEIVER");

    fChips.push_back(new TAlpide(chipConfig));
    fChips.at(i) -> SetReadoutBoard(fBoards.at(0));
    if (i < 7) {              // first master-slave row
      if (receiver < 0) receiver = 9;
      if (control  < 0) control  = 1;
    }
    else {                    // second master-slave row
      if (receiver < 0) receiver = 0;
      if (control  < 0) control  = 0;
    }
    fBoards.at(0)-> AddChip        (chipId, control, receiver);
  }
  std::cout << "Checking control interfaces." << std::endl;
  int nWorking = CheckControlInterface();
  std::cout << "Found " << nWorking << " working chips" << std::endl;
  sleep(5);
  MakeDaisyChain();
}


int initSetupHalfStave() {
  fBoardType = boardMOSAIC;
  
}


// Make the daisy chain for OB readout, based on enabled chips
// i.e. to be called after CheckControlInterface
void MakeDaisyChain() {
  for (int i = 0; i < fChips.size(); i++) {
    if (!fChips.at(i)->GetConfig()->IsEnabled()) continue;
    int chipId   = fChips.at(i)->GetConfig()->GetChipId();
    int previous = -1;
    if (!(chipId & 0x7)) {   // Master, has initial token, previous chip is last enabled chip in row
      fChips.at(i)->GetConfig()->SetInitialToken(true);
      for (int iprev = chipId + 6; (iprev > chipId) && (previous == -1); iprev--) { 
        if (fConfig->GetChipConfigById(iprev)->IsEnabled()) {
          previous = iprev; 
	}
      }
      if (previous == -1) {
        fChips.at(i)->GetConfig()->SetPreviousId(chipId);
      }
      else {
         fChips.at(i)->GetConfig()->SetPreviousId(previous);
      }
    }
    else {    // Slave, does not have token, previous chip is last enabled chip before
      fChips.at(i)->GetConfig()->SetInitialToken(false);
      for (int iprev = chipId - 1; (iprev >= (chipId & 0x78)) && (previous == -1); iprev--) {
        if (fConfig->GetChipConfigById(iprev)->IsEnabled()) {
          previous = iprev; 
	}
      }
      if (previous == -1) {
        fChips.at(i)->GetConfig()->SetPreviousId(chipId);
      }
      else {
         fChips.at(i)->GetConfig()->SetPreviousId(previous);
      }
    }
    std::cout << "Chip Id " << chipId << ", token = " << (bool) fChips.at(i)->GetConfig()->GetInitialToken() << ", previous = " << fChips.at(i)->GetConfig()->GetPreviousId() << std::endl;
  }
}


// Try to communicate with all chips, disable chips that are not answering
int CheckControlInterface() {
  uint16_t Value;
  int      nWorking = 0;

  for (int i = 0; i < fChips.size(); i++) {
    if (!fChips.at(i)->GetConfig()->IsEnabled()) continue;
    fChips.at(i)->WriteRegister (0x60d, 10);
    try {
      fChips.at(i)->ReadRegister (0x60d, Value);
      std::cout << "Chip ID " << fChips.at(i)->GetConfig()->GetChipId() << ", Value = 0x" << std::hex << (int) Value << std::dec << std::endl;
      nWorking ++;
    }
    catch (exception &e) {
      std::cout << "Chip ID " << fChips.at(i)->GetConfig()->GetChipId() << ", not answering, disabling." << std::endl;
      fChips.at(i)->GetConfig()->SetEnable(false);
    }
  }
  return nWorking;
}


// Setup definition for inner barrel stave with MOSAIC
//    - all chips connected to same control interface
//    - each chip has its own receiver, mapping defined in RCVMAP
int initSetupIB() {
  int RCVMAP []                   = { 3, 5, 7, 8, 6, 4, 2, 1, 0 };
  fBoardType                      = boardMOSAIC;
  TBoardConfigMOSAIC *boardConfig = (TBoardConfigMOSAIC*) fConfig->GetBoardConfig(0);

  boardConfig->SetInvertedData (false);

  Mosaic::TReceiverSpeed speed; 

  switch (fConfig->GetChipConfig(0)->GetParamValue("LINKSPEED")) {
  case 400: 
    speed = Mosaic::RCV_RATE_400;
    break;
  case 600: 
    speed = Mosaic::RCV_RATE_600;
    break;
  case 1200: 
    speed = Mosaic::RCV_RATE_1200;
    break;
  default: 
    std::cout << "Warning: invalid link speed, using 1200" << std::endl;
    speed = Mosaic::RCV_RATE_1200;
    break;
  }
  std::cout << "Speed mode = " << speed << std::endl;
  boardConfig->SetSpeedMode    (speed);

  fBoards.push_back (new TReadoutBoardMOSAIC(boardConfig));

  for (int i = 0; i < fConfig->GetNChips(); i++) {
    TChipConfig *chipConfig = fConfig->GetChipConfig(i);
    int          control    = chipConfig->GetParamValue("CONTROLINTERFACE");
    int          receiver   = chipConfig->GetParamValue("RECEIVER");
    fChips.push_back(new TAlpide(chipConfig));
    fChips.at(i) -> SetReadoutBoard(fBoards.at(0));

    if (control  < 0) control  = 0;
    if (receiver < 0) receiver = RCVMAP[i];

    fBoards.at(0)-> AddChip        (chipConfig->GetChipId(), control, receiver);
  }

  std::cout << "Checking control interfaces." << std::endl;
  int nWorking = CheckControlInterface();
  std::cout << "Found " << nWorking << " working chips" << std::endl;
}


int initSetupSingleMosaic() {
  TChipConfig        *chipConfig  = fConfig->GetChipConfig(0);
  fBoardType                      = boardMOSAIC;
  TBoardConfigMOSAIC *boardConfig = (TBoardConfigMOSAIC*) fConfig->GetBoardConfig(0);
  int                 control     = chipConfig->GetParamValue("CONTROLINTERFACE");
  int                 receiver    = chipConfig->GetParamValue("RECEIVER");

  if (receiver < 0) receiver = 3;   // HSData is connected to pins for first chip on a stave
  if (control  < 0) control  = 0; 

  boardConfig->SetInvertedData (false);
  boardConfig->SetSpeedMode    (Mosaic::RCV_RATE_400);

  fBoards.push_back (new TReadoutBoardMOSAIC(boardConfig));

  fChips. push_back(new TAlpide(chipConfig));
  fChips. at(0) -> SetReadoutBoard(fBoards.at(0));
  fBoards.at(0) -> AddChip        (chipConfig->GetChipId(), control, receiver);
}


int initSetupSingle() {
  TReadoutBoardDAQ  *myDAQBoard = 0;
  TChipConfig       *chipConfig = fConfig->GetChipConfig(0);
  fBoardType                    = boardDAQ;
  // values for control interface and receiver currently ignored for DAQ board
  int               control     = chipConfig->GetParamValue("CONTROLINTERFACE");
  int               receiver    = chipConfig->GetParamValue("RECEIVER");
  
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

  // for Cagliari DAQ board disable DDR and Manchester encoding
  chipConfig->SetEnableDdr         (false);
  chipConfig->SetDisableManchester (true);

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
  fConfig = new TConfig (configFileName);  

  switch (fConfig->GetDeviceType())
    {
    case TYPE_CHIP: 
      initSetupSingle();
      break;
    case TYPE_IBHIC:
      initSetupIB();
      break;
    case TYPE_OBHIC:
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

