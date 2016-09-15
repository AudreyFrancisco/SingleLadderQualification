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
#include "SetupHelpers.h"


int myStrobeLength = 10;      // strobe length in units of 25 ns
int myStrobeDelay  = 0;
int myPulseLength  = 500;

int myPulseDelay   = 50;
int myNTriggers    = 1000000;


int HitData     [512][1024];


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
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x0);            // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  myStrobeLength);  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, myStrobeDelay);   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  //  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength);   // fromu pulsing 2: pulse length 
}


// initialisation of chip DACs
int configureDACs(TAlpide *chip) {
  chip->WriteRegister (Alpide::REG_VRESETD, 147);
  //  chip->WriteRegister (Alpide::REG_ITHR, 51);
}


// initialisation of fixed mask
int configureMask(TAlpide *chip) {
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   false);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);
}


int configureChip(TAlpide *chip) {
  // put all chip configurations before the start of the test here
  chip->WriteRegister (Alpide::REG_MODECONTROL, 0x20); // set chip to config mode
  if (fSetupType == setupSingle) {
    chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x30); // CMU/DMU config: turn manchester encoding off etc, initial token=1, disable DDR
  }
  else {
    chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x10); // CMU/DMU config: same as above, but manchester on
  }

  configureFromu(chip);
  configureDACs (chip);
  configureMask (chip);


  chip->WriteRegister (Alpide::REG_MODECONTROL, 0x21); // strobed readout mode


}


void scan() {   
  unsigned char         buffer[1024*4000]; 
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  int nTrains, nRest, nTrigsThisTrain, nTrigsPerTrain = 100;

  nTrains = myNTriggers / nTrigsPerTrain;
  nRest   = myNTriggers % nTrigsPerTrain;

  for (int itrain = 0; itrain <= nTrains; itrain ++) {
    if (itrain == nTrains) {
      nTrigsThisTrain = nRest;
    }
    else {
      nTrigsThisTrain = nTrigsPerTrain;
    }

      fBoards.at(0)->Trigger(nTrigsThisTrain);

      int itrg = 0;
      while(itrg < nTrigsThisTrain) {
        if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
          usleep(100);
          continue;
        }
        else {
          // decode DAQboard event
          BoardDecoder::DecodeEvent(boardDAQ, buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
          // decode Chip event
          int n_bytes_chipevent=n_bytes_data-n_bytes_header-n_bytes_trailer;
          AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits);

          itrg++;
        }
      } 
      //std::cout << "Number of hits: " << Hits->size() << std::endl;
      CopyHitData(Hits);
  }
}


int main() {
  // chip ID that is used in case of single chip setup
  fSingleChipId = 16;

  // module ID that is used for outer barrel modules 
  // (1 will result in master chip IDs 0x10 and 0x18, 2 in 0x20 and 0x28 ...)
  fModuleId = 1;

  fSetupType = setupSingle;

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
    fBoards.at(0)->SetTriggerConfig (true, false, myStrobeDelay, myPulseDelay);
    fBoards.at(0)->SetTriggerSource (trigExt);

    scan();

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  sprintf(fName, "Data/NoiseOccupancy_%s.dat", Suffix);
  WriteDataToFile (fName, true);
  return 0;
}
