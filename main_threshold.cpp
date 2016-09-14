#include <unistd.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "USBHelpers.h"
#include "TConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"

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
   

      chip -> WriteRegister (0x1, 0x20); // config mode
      
      // PRST to chip
      myDAQBoard -> WriteRegister (0x400, 0xe4);
      // CMUDMU config
      chip -> WriteRegister (0xc, 0x60); // turn manchester encoding off etc, initial token=1, disable DDR
      // RORST to chip: to be always performed after write to CMUDMU?
      myDAQBoard -> WriteRegister (0x400, 0x63);
      // FROMU config 1
      //chip -> WriteRegister (0x4, 0x0); 
      //chip -> WriteRegister (0x4, 0x60); // analogue pulsing, enable test strobe (for automatic strobing after PULSE)
      chip -> WriteRegister (0x4, 0x20); // analogue pulsing, disable test strobe (for automatic strobing after PULSE)
      // FROMU config 2: STROBE duration
      chip -> WriteRegister (0x5, 40); // 10=>250ns
      // FROMU pulsing 1: delay between PULSE and STROBE
      //chip -> WriteRegister (0x6, 80); // 80=>2us
      // FROMU pulsing 2: PULSE duration
      chip -> WriteRegister (0x7, 1600); // 800=>40us

      sleep(1);
      // unmask all pixels 
      chip -> WriteRegister (0x500, 0x600); // Pixel CFG reg 1: write MASK, all rows 
      chip -> WriteRegister (0x501, 0x400); // Pixel CFG reg 2: all columns
      chip -> WriteRegister (0x502, 0x1);   // Pixel CFG reg 3: strobe on 
      chip -> WriteRegister (0x502, 0x0);   // Pixel CFG reg 3: strobe off
      // all regions
      //for (int ireg=0;ireg<32;ireg++) {
      //  chip -> WriteRegister((ireg<<11|2<<8|0),0xf3); // TODO: check what good for
      //}

    
      
      // configure dacs
      //chip -> WriteRegister (0x601, 0x75); 
      chip -> WriteRegister (0x602, 0x93); 
      //chip -> WriteRegister (0x603, 0x56); 
      chip -> WriteRegister (0x604, 57); // VCASN 0x32=50
      //chip -> WriteRegister (0x605, 0xff); // VPULSEH
      chip -> WriteRegister (0x605, 170); 
      //chip -> WriteRegister (0x606, 0x00);   // VPULSEL
      chip -> WriteRegister (0x606, 10);  
      chip -> WriteRegister (0x607, 77); // VCASN2 0x39=57
      //chip -> WriteRegister (0x608, 0x00); 
      //chip -> WriteRegister (0x609, 0x00); 
      //chip -> WriteRegister (0x60a, 0x00); 
      //chip -> WriteRegister (0x60b, 0x32); 
      //chip -> WriteRegister (0x60c, 0x40); 
      //chip -> WriteRegister (0x60d, 0x40); 
      //chip -> WriteRegister (0x60e, 0x33); 

      // triggered readout mode
      chip -> WriteRegister (0x1, 0x21); 
       
      // configure readout/pulse/trigger
      myDAQBoard->SetTriggerConfig(true, false, 50, 50); // enablePulse, enableTrigger, trigger(strobe)Delay (x25ns), pulseDelay (x12.5ns)
      // delay between pulse and trigger is the triggerDelay(x25ns) + puslseDelay(x12.5ns) + 50ns offset
      // pusleDelay has to be set to larger than 25 to not cause problems..
      myDAQBoard->SetTriggerSource(trigExt);


      std::cout << "start triggering in 2s" << std::endl;
      sleep(2);
  

      int n_bytes_data;
      unsigned char buffer[1024*4000]; // TODO is char better than unsigned char here?

      int n_bytes_header;
      int n_bytes_trailer;
      TBoardHeader boardInfo;

      // trigger n events
      int n_triggers = 1;

      // read events
      std::vector<TPixHit> *Hits = new std::vector<TPixHit>;
      int itrg = 0;
    

      for (int irow=100;irow<102;irow++) {
      
        // config mode
        chip -> WriteRegister (0x1, 0x20); 
        // set pulsing all -> false
        //pc.write_alpide_reg(chipid,0,5,1,1<<10)            # Pixel CFG reg 2: all columns
        //pc.write_alpide_reg(chipid,0,5,0,(1 if enable else 0)<<11|0<<10|1<<9) # Pixel CFG reg 1: write PULSE, all rows
        //pc.write_alpide_reg(chipid,0,5,2,1)                # Pixel CFG reg 3: strobe on
        //pc.write_alpide_reg(chipid,0,5,2,0)                # Pixel CFG reg 3: strobe off
        chip -> WriteRegister(0x501, 1<<10);
        chip -> WriteRegister(0x500, 0<<11|0<<10|1<<9);
        chip -> WriteRegister (0x502, 0x1);   // Pixel CFG reg 3: strobe on 
        chip -> WriteRegister (0x502, 0x0);   // Pixel CFG reg 3: strobe off
      
        // set pulsing row -> true
        //pc.write_alpide_reg(chipid,0,5,0,(1 if enable else 0)<<11|0<<10|0<<9|y)    # Pixel CFG reg 1: write PULSE, row y
        //pc.write_alpide_reg(chipid,0,5,1,1<<10)            # Pixel CFG reg 2: all columns
        //pc.write_alpide_reg(chipid,0,5,2,1)                # Pixel CFG reg 3: strobe on
        //pc.write_alpide_reg(chipid,0,5,2,0)                # Pixel CFG reg 3: strobe off
        chip -> WriteRegister(0x500, 1<<11|0<<10|0<<9|irow); // Pixel CFG reg 1: write PULSE, row irow
        chip -> WriteRegister(0x501, 1<<10);   // Pixel CFG reg 2: all colums
        chip -> WriteRegister (0x502, 0x1);   // Pixel CFG reg 3: strobe on 
        chip -> WriteRegister (0x502, 0x0);   // Pixel CFG reg 3: strobe off
       
        // triggered mode
        chip -> WriteRegister (0x1, 0x21); 
      
        // pulse!
        myDAQBoard->Trigger(n_triggers);
        
        itrg=0;
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
        for (int i=0; i<Hits->size(); i++) {
          std::cout << i << ":\t region: " << Hits->at(i).region << "\tdcol: " << Hits->at(i).dcol << "\taddres: " << Hits->at(i).address << std::endl; 
        }

        std::cout << "------------------- Row " << irow << " finished -------------------" << std::endl;
      
      }

    }
    else {
      std::cout << "Type cast failed" << std::endl;
    }

  }

  //myDAQBoard->ResetBoardFPGA(10);  
  delete myDAQBoard;
  
  return 0;
}
