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


int myStrobeLength = 40;      // strobe length in units of 25 ns
int myStrobeDelay  = 10;
int myPulseLength  = 1000;

int myPulseDelay   = 50;
int myNTriggers    = 50;
int myMaskStages   = 2048;

int HitData[512][1024];


void ClearHitData() {
  for (int icol = 0; icol < 512; icol ++) {
    for (int iaddr = 0; iaddr < 1024; iaddr ++) {
      HitData[icol][iaddr] = 0;
    }
  }
}


void CopyHitData(std::vector <TPixHit> *Hits) {
  for (int ihit = 0; ihit < Hits->size(); ihit ++) {
    HitData[Hits->at(ihit).dcol + Hits->at(ihit).region * 16][Hits->at(ihit).address] ++;
  }
  Hits->clear();
}


void WriteDataToFile (const char *fName, bool Recreate) {
  FILE *fp;
  bool  HasData;
  if (Recreate) fp = fopen(fName, "w");
  else          fp = fopen(fName, "a");

  for (int icol = 0; icol < 512; icol ++) {
    for (int iaddr = 0; iaddr < 1024; iaddr ++) {
      if (HitData[icol][iaddr] > 0) {
        fprintf(fp, "%d %d %d\n", icol, iaddr, HitData[icol][iaddr]);
      }
    }
  }
  fclose (fp);
}


// initialisation of Fromu
int configureFromu(TAlpide *chip) {
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x0);             // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  myStrobeLength);  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, myStrobeDelay);   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength);   // fromu pulsing 2: pulse length 
}


// setting of mask stage during scan
int configureMaskStage(TAlpide *chip, int istage) {
  int row    = istage / 4;
  int region = istage % 4;

  uint32_t regionmod = 0x77777777 >> region;

  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   true);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);

  AlpideConfig::WritePixRegRow (chip, Alpide::PIXREG_MASK,   false, row);
  AlpideConfig::WritePixRegRow (chip, Alpide::PIXREG_SELECT, true,  row);

  chip->WriteRegister (Alpide::REG_REGDISABLE_LOW,  (uint16_t) regionmod);
  chip->WriteRegister (Alpide::REG_REGDISABLE_HIGH, (uint16_t) regionmod);

  for (uint16_t ireg = 0; ireg < 32; ireg ++) {
    uint16_t Address = Alpide::REG_DCOL_DISABLE_BASE | (ireg << 11);
    chip->WriteRegister (Address, 0);
  }


  //for (int icol = 0; icol < 1024; icol += 4) {
  //  AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_MASK,   false, istage % 1024, icol + istage / 1024);
  //  AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_SELECT, true,  istage % 1024, icol + istage / 1024);
  // 
  //}

}


int configureChip(TAlpide *chip) {
  AlpideConfig::BaseConfig(chip);

  configureFromu(chip);
  chip->WriteRegister (Alpide::REG_MODECONTROL, 0x21); // strobed readout mode


}


void scan() {   
  unsigned char         buffer[1024*4000]; 
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer, errors8b10b = 0;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;


  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));

  if (myMOSAIC) {
    myMOSAIC->StartRun();
  }


  for (int istage = 0; istage < myMaskStages; istage ++) {
    std::cout << "Mask stage " << istage << std::endl;
    for (int i = 0; i < fChips.size(); i ++) {
      configureMaskStage (fChips.at(i), istage);
    }
    fBoards.at(0)->Trigger(myNTriggers);
    int itrg = 0;
    while(itrg < myNTriggers) {
      if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
        usleep(100);
        continue;
      }
      else {

        //std::cout << "received Event" << itrg << " with length " << n_bytes_data << std::endl; 
        //for (int iByte=0; iByte<n_bytes_data; ++iByte) {
        //  std::cout << std::hex << (int)(uint8_t)buffer[iByte] << std::dec;
	// }
        //std::cout << std::endl;
            
        // decode DAQboard event
        BoardDecoder::DecodeEvent(fBoards.at(0)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
        if (boardInfo.decoder10b8bError) errors8b10b++;
        // decode Chip event
        int n_bytes_chipevent=n_bytes_data-n_bytes_header-n_bytes_trailer;
        AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits);
        //std::cout << "total number of hits found: " << Hits->size() << std::endl;

        itrg++;

      }
    } 
    
    //std::cout << "Hit pixels: " << std::endl;
    //for (int i=0; i<Hits->size(); i++) {
    //  std::cout << i << ":\t region: " << Hits->at(i).region << "\tdcol: " << Hits->at(i).dcol << "\taddres: " << Hits->at(i).address << std::endl; 
    //}
    CopyHitData(Hits);
  }
  if (myMOSAIC) {
    myMOSAIC->StopRun();
    std::cout << "Total number of 8b10b decoder errors: " << errors8b10b << std::endl;
  }


}


int main() {
  initSetup();

  char Suffix[20], fName[100];

  ClearHitData();
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
  if (fBoards.size() == 1) {
     
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (int i = 0; i < fChips.size(); i ++) {
      configureChip (fChips.at(i));
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

    // put your test here... 
    if (fBoards.at(0)->GetConfig()->GetBoardType() == boardMOSAIC) {
      fBoards.at(0)->SetTriggerConfig (true, true, 100, 1000);//myStrobeDelay, myPulseDelay);
      fBoards.at(0)->SetTriggerSource (trigInt);
    }
    else if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
      fBoards.at(0)->SetTriggerConfig (true, false, myStrobeDelay, myPulseDelay);
      fBoards.at(0)->SetTriggerSource (trigExt);
    }
    scan();

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }


  sprintf(fName, "Data/DigitalScan_%s.dat", Suffix);
  WriteDataToFile (fName, true);
  return 0;
}
