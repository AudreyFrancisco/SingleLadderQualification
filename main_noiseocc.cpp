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


TBoardType fBoardType = boardDAQ; 
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
  uint16_t          status;

      chip -> WriteRegister (0x1, 0x20); // config mode
      
      // CMUDMU config
      chip -> WriteRegister (0xc, 0x60); // turn manchester encoding off etc..
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
      chip -> WriteRegister (0x1, 0x21); 
      
      
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
}


int main() {
  uint32_t          version;

  initSetup();

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
  if (fBoards.size() == 1) {
     
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (int i = 0; i < fChips.size(); i ++) {
      configureChip (fChips.at(i));
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     
      //usleep(10000);
      //// start tigger
      //myDAQBoard -> WriteRegister (0x302, 0xd); 
      //// stop tigger
      //myDAQBoard -> WriteRegister (0x303, 0xd); 
      //// start tigger
      //myDAQBoard -> WriteRegister (0x302, 0xd); 
      //// stop tigger
      //myDAQBoard -> WriteRegister (0x303, 0xd); 

      /**/ //test trigger train
      //myDAQBoard->GetBoardConfig()->SetStrobeDelay(40); // 40*25ns + 175 ns (offset by fw) => ~1MHz
      //myDAQBoard->GetBoardConfig()->SetStrobeDelay(100); // 100*25ns + 175 ns (offset by fw) => ~400kHz
      //myDAQBoard->GetBoardConfig()->SetStrobeDelay(400); //  => ~100kHz
      //myDAQBoard->GetBoardConfig()->SetStrobeDelay(1000); // ) => ~40kHz during bunch train
      //myDAQBoard->GetBoardConfig()->SetStrobeDelay(4000); // 4000*25ns + 175 ns (offset by fw) => ~10kHz
      //myDAQBoard->GetBoardConfig()->SetStrobeDelay(40000); // 40000*25ns + 175 ns (offset by fw) => ~1kHz
      //myDAQBoard->ReadAllRegisters();
      //myDAQBoard->WriteTriggerModuleConfigRegisters(); 

      //std::cout << "start trigger in 3s" << std::endl;
      //sleep(3);
      //myDAQBoard -> WriteRegister (0x302, 0xd); // StartTrigger
      //sleep(1);
      //myDAQBoard -> WriteRegister (0x303, 0xd); // StopTrigger

      std::cout << "start trigger in 2s" << std::endl;
      sleep(2);
  
      //myDAQBoard->StartTrigger();
      //std::cout << "triggers sent" << std::endl;

      //sleep(2);
      /**/


      // trigger n events
      myDAQBoard->SetTriggerConfig(true, false, 1000, 75); // enablePulse, enableTrigger, trigger(strobe)Delay, pulseDelay

      int n_triggers = 10;
      fBoards.at(0)->Trigger(n_triggers);



      // read events
      int n_bytes_data;
      unsigned char buffer[1024*4000]; // TODO is char better than unsigned char here?
      int n_bytes_header;
      int n_bytes_trailer;
      TBoardHeader boardInfo;
      std::vector<TPixHit> *Hits = new std::vector<TPixHit>;
      int itrg = 0;
      //for (int itrg=0; itrg<n_triggers; itrg++) {
      while(itrg<n_triggers) {
        if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
          usleep(100);
          continue;
        }
        else {
          std::cout << "received Event" << itrg << " with length " << n_bytes_data << std::endl; 
          for (int iByte=0; iByte<n_bytes_data; ++iByte) {
            std::cout << std::hex << (int)(uint8_t)buffer[iByte] << std::dec;
          }
          std::cout << std::endl;
          
          // decode DAQboard event
          BoardDecoder::DecodeEvent(boardDAQ, buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
          // decode Chip event
          int n_bytes_chipevent=n_bytes_data-n_bytes_header-n_bytes_trailer;
          AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits);
          std::cout << "total number of hits found: " << Hits->size() << std::endl;

          itrg++;

        }
      } 
     
      if (myDAQBoard) {
        if (myDAQBoard->GetEventBufferLength()!=0) { 
          std::cout << "WARNING: still events in buffer, not expected at this point!" << std::endl; 
        }
        else {
          std::cout << n_triggers << " triggers read" << std::endl;
        }
      }
      std::cout << "Hit pixels: " << std::endl;
      //for (int i=0; i<Hits->size(); i++) {
      //  std::cout << i << ":\t region: " << Hits->at(i).region << "\tdcol: " << Hits->at(i).dcol << "\taddres: " << Hits->at(i).address << std::endl; 
      //}

      //std::cout << "Powering off board" << std::endl;
      //myDAQBoard -> PowerOff();

    }



  //myDAQBoard->ResetBoardFPGA(10);  
  delete myDAQBoard;
  
  return 0;
}
