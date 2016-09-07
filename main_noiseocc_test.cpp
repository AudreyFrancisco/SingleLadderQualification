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
//
// Note that the chip configuration itself is slightly different for DAQ board and MOSAIC 
// (serial vs parallel out, Manchester encoding ...) 

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

const int singleChipId = 16;


TBoardType fBoardType;
TSetupType fSetupType = setupIB;

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


// Prepare setup for outer barrel module. This assumes that the module Id is 001, i.e. the chipIDs are 
// 16 - 22 for the first master and 24 - 30 for the second master 

int initSetupOB() {
  std::vector <int> chipIDs;
  for (int i = 16; i < 23; i++) chipIDs.push_back(i);
  for (int i = 24; i < 31; i++) chipIDs.push_back(i);

  fConfig       = new TConfig (1, chipIDs);
  fBoardType    = boardMOSAIC;

  fBoards.push_back (new TReadoutBoardMOSAIC((TBoardConfigMOSAIC*)fConfig->GetBoardConfig(0)));

  for (int i = 0; i < fConfig->GetNChips(); i++) {
    fChips.push_back(new TAlpide(fConfig->GetChipConfig(chipIDs.at(i))));
    fChips.at(i) -> SetReadoutBoard(fBoards.at(0));
    if (chipIDs.at(i) < 23) { // first master
      fBoards.at(0)-> AddChip        (chipIDs.at(i), 0, 0);
    }
    else {                    // second master
      fBoards.at(0)-> AddChip        (chipIDs.at(i), 1, 1);
    }
  }
}


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
  uint16_t          status;
      chip -> WriteRegister (0x1, 0x20); // config mode
      
      // CMUDMU config
      chip -> WriteRegister (0xc, 0x20); 
      // RORST: to be always performed after write to CMUDMU?
 
      // FROMU config 1
      chip -> WriteRegister (0x4, 0x0); 
      sleep(1);
      // unmask all pixels 
      chip -> WriteRegister (0x500, 0x600); 
      chip -> WriteRegister (0x501, 0x400); 
      chip -> WriteRegister (0x502, 0x1); 
      chip -> WriteRegister (0x502, 0x0); 
      //// configure readout
      //myDAQBoard -> WriteRegister (0x200, 0x1911); 
      //// configure trigger
      //myDAQBoard -> WriteRegister (0x300, 0x4); 
      //myDAQBoard -> WriteRegister (0x301, 0x510001); 
      //myDAQBoard -> WriteRegister (0x304, 0x1a); 
      // strobeB duration
      chip -> WriteRegister (0x5, 0xa); // 10*25ns
      // triggered readout mode
      
      // configure dacs
      chip -> WriteRegister (0x601, 0x75); 
      chip -> WriteRegister (0x602, 0x93); 
      chip -> WriteRegister (0x603, 0x56); 
      chip -> WriteRegister (0x604, 0x3c); // VCASN 0x32=50
      chip -> WriteRegister (0x605, 0xff); 
      chip -> WriteRegister (0x606, 0x00); 
      chip -> WriteRegister (0x607, 0x48); // VCASN2 0x39=57
      chip -> WriteRegister (0x608, 0x00); 
      chip -> WriteRegister (0x609, 0x00); 
      chip -> WriteRegister (0x60a, 0x00); 
      chip -> WriteRegister (0x60b, 0x32); 
      chip -> WriteRegister (0x60c, 0x40); 
      chip -> WriteRegister (0x60d, 0x40); 
      chip -> WriteRegister (0x60e, 0x33); 
      //chip -> WriteRegister (0xc, 0x60); 
      chip -> ReadRegister (0x605, status);
      std::cout << "VPULSEH register value: 0x" << std::hex << status << std::dec << std::endl; 
      chip -> ReadRegister (0x60d, status);
      std::cout << "IBIAS register value: 0x" << std::hex << status << std::dec << std::endl;

      chip->WriteRegister (0xe, 0x141);   // DTU_CFG
      chip->WriteRegister (0xe, 0x041); 

      usleep(1000);		// wait 1ms
      chip->WriteRegister (0xf, 0xaaf);     // DTU_DAC: PLL:0 Driver:0x0a Pre-emphasis:0	bits 0-3 charge pump current, 4-7 hs line driver current, 8-11 preemph

      chip->WriteRegister (0x12, 0x00); 	// DTU_TEST1: Normal mode	
      chip->WriteRegister (0x11, 0x0101);     // DTU_PLL_LOCK2, 0x0101); 
      chip -> WriteRegister (0x1, 0x21); 
       
}


int main() {

  initSetup();

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
  if (fBoards.size() == 1) {
     
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    uint16_t Value;
       for (int i = 0; i < fChips.size(); i++) {
          fChips.at(i)->WriteRegister (0x60d, 10);
          try {
            fChips.at(i)->ReadRegister (0x60d, Value);
    	    std::cout << "Chip ID " << fChips.at(i)->GetConfig()->GetChipId() << ", Value = 0x" << std::hex << (int) Value << std::dec << std::endl;
	  }
          catch (exception &e) {
    	    std::cout << "Chip ID " << fChips.at(i)->GetConfig()->GetChipId() << ", not answering, exception: " << e.what() << std::endl;
	  }
	}


    for (int i = 0; i < fChips.size(); i ++) {
      std::cout << "Configuring chip " << i << std::endl;
      configureChip (fChips.at(i));
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

    fBoards.at(0)->SetTriggerConfig(false, true, 100, 1000);
    fBoards.at(0)->SetTriggerSource (TTriggerSource::trigInt);

	int numberOfReadByte; // the bytes of row event
	unsigned char *theBuffer; // the buffer containing the event

	int enablePulse, enableTrigger, triggerDelay, pulseDelay, nTriggers; // variables that define the trigger/pulse

	theBuffer = (unsigned char*) malloc(200 * 1024); // allocates 200 kilobytes ...

	bool isDataTackingEnd = false; // break the execution of read polling
	int returnCode = 0;
	int timeoutLimit = 10; // ten seconds



    ((TReadoutBoardMOSAIC *)fBoards.at(0))->StartRun(); // Activate the data taking ...

    fBoards.at(0)->Trigger(1); // Preset end start the trigger

	while(!isDataTackingEnd) { // while we don't receive a timeout
	  returnCode = fBoards.at(0)->ReadEventData(numberOfReadByte, theBuffer);
		if(returnCode != 0) { // we have some thing
			std::cout << "Read an event !  Dimension :" << numberOfReadByte << std::endl;   // Consume the buffer ...
			usleep(20000); // wait
		} else { // read nothing is finished ?
			if(timeoutLimit-- == 0) isDataTackingEnd = true;
			sleep(1);
		}
	}

	((TReadoutBoardMOSAIC *)fBoards.at(0))->StopRun(); // Stop run


    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
