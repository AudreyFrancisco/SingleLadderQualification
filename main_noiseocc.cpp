#include <unistd.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "USBHelpers.h"
#include "TConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"


TBoardType fBoardType = boardDAQ; 
std::vector <TReadoutBoard *> boards;
TConfig *config = new TConfig (16);


int powerOn () {
}


int initSetup() {
  TConfig *config = new TConfig (16, fBoardType);
  
  if (fBoardType == boardDAQ) {
    InitLibUsb(); 
    FindDAQBoards (config, boards);

    std::cout << "found " << boards.size() << " DAQ boards" << std::endl;
  }
  else if (fBoardType == boardMOSAIC) {
  }


}


int main() {
  uint16_t          status;
  uint32_t          version;
  int               overflow;
  TReadoutBoardDAQ  *myDAQBoard;


  // if (config->BoardType == DAQBoard)
  // 
  //  The following code searches the USB bus for DAQ boards, creates them and adds them to the readout board vector: 
  //  TReadoutBoard *readoutBoard = new TReadoutBoardDAQ(device, config);
  //  board.push_back (readoutBoard);

  
  if (boards.size() == 1) {
    TAlpide *chip   = new TAlpide (config->GetChipConfig(16));
    chip         -> SetReadoutBoard (boards.at(0));
    boards.at(0) -> AddChip (0, 0, 0);
    
    myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (boards.at(0));
    if (myDAQBoard) {
      if (myDAQBoard -> PowerOn (overflow)) std::cout << "LDOs are on" << std::endl;
      else std::cout << "LDOs are off" << std::endl;
      //myDAQBoard->ReadRegister (0x602, version); // read firmware version
      //std::cout << "Version = " << std::hex << version << std::dec << std::endl;
      std::cout << "Version = " << std::hex << myDAQBoard->ReadFirmwareVersion() << std::dec << std::endl;
      //myDAQBoard->WriteRegister (0x402, 3); // disable manchester encoding in DAQboard
      //myDAQBoard->WriteRegister (0x500, 0x0220);
      myDAQBoard -> SendOpCode (Alpide::OPCODE_GRST);
      //sleep(1); // sleep necessary after GRST? or PowerOn?

      std::cout << "Analog Current = " << myDAQBoard-> ReadAnalogI() << std::endl;
      std::cout << "Digital Current = " << myDAQBoard-> ReadDigitalI() << std::endl;

      /* test markus
      //std::cout << "After Write register " << std::endl;
      //chip -> ReadRegister (Alpide::REG_IBIAS, status);
      //std::cout << "IBias register value: 0x" << std::hex << status << std::dec << std::endl;
      //chip->WriteRegister (Alpide::REG_IBIAS, 0);
      //sleep(1);
      //std::cout << "Analog Current = " << myDAQBoard-> ReadAnalogI() << std::endl;

      //AlpideConfig::ApplyStandardDACSettings (chip, 0);
      //chip -> ReadRegister (Alpide::REG_VRESETD, status);

      //std::cout << "Control register value: 0x" << std::hex << status << std::dec << std::endl;

      //chip -> ReadRegister (Alpide::REG_IBIAS, status);
      //std::cout << "IBias register value: 0x" << std::hex << status << std::dec << std::endl;

      */ 
      
    
   
/*
      //myDAQBoard -> ReadAllRegisters();
      //chip -> ReadAllRegisters();
      usleep(10000);

      int n_bytes;
      char buffer[1024*4000];

      int n_triggers = 5;
      myDAQBoard->Trigger(n_triggers);
      //for (int itrg=0; itrg<n_triggers; itrg++) {
      //  myDAQBoard->ReadEventData(n_bytes, buffer);
      //} 

      //sleep(5); 

      //libusb_exit(fContext);
*/
      
   

      chip -> WriteRegister (0x1, 0x20); // config mode
      
      // PRST
      myDAQBoard -> WriteRegister (0x400, 0xe4);
      // CMUDMU config
      chip -> WriteRegister (0xc, 0x60); // turn manchester encoding off etc..
      // RORST: to be always performed after write to CMUDMU?
      myDAQBoard -> WriteRegister (0x400, 0x63);
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
      myDAQBoard->GetBoardConfig()->SetStrobeDelay(1000); // ) => ~40kHz during bunch train
      //myDAQBoard->GetBoardConfig()->SetStrobeDelay(4000); // 4000*25ns + 175 ns (offset by fw) => ~10kHz
      //myDAQBoard->GetBoardConfig()->SetStrobeDelay(40000); // 40000*25ns + 175 ns (offset by fw) => ~1kHz
      //myDAQBoard->ReadAllRegisters();
      myDAQBoard->WriteTriggerModuleConfigRegisters(); 

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
      myDAQboard->SetTriggerConfig(true, true, 1000, 75); // enablePulse, enableTrigger, trigger(strobe)Delay, pulseDelay

      int n_triggers = 10;
      myDAQBoard->Trigger(n_triggers);



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
        if (myDAQBoard->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
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
      if (myDAQBoard->GetEventBufferLength()!=0) { 
        std::cout << "WARNING: still events in buffer, not expected at this point!" << std::endl; 
      }
      else {
        std::cout << n_triggers << " triggers read" << std::endl;
      }
      std::cout << "Hit pixels: " << std::endl;
      //for (int i=0; i<Hits->size(); i++) {
      //  std::cout << i << ":\t region: " << Hits->at(i).region << "\tdcol: " << Hits->at(i).dcol << "\taddres: " << Hits->at(i).address << std::endl; 
      //}

      //std::cout << "Powering off board" << std::endl;
      //myDAQBoard -> PowerOff();

    }
    else {
      std::cout << "Type cast failed" << std::endl;
    }

  }

  //myDAQBoard->ResetBoardFPGA(10);  
  delete myDAQBoard;
  
  return 0;
}
