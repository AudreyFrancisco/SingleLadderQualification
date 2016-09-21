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


int myVCASN   = 57;
int myITHR    = 51;
int myVCASN2  = 64;
int myVCLIP   = 0;
int myVRESETD = 147;

int myStrobeLength = 80;      // strobe length in units of 25 ns
int myStrobeDelay  = 0;
int myPulseLength  = 500;

int myPulseDelay   = 40;
int myNTriggers    = 50;
//int myMaskStages   = 164;    // full: 8192
int myMaskStages   = 1; 

int myChargeStart  = 0;
int myChargeStop   = 50;   // if > 100 points, increase array sizes

int HitData     [100][512][1024];
int ChargePoints[100];


void ClearHitData() {
  for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
    ChargePoints[icharge-myChargeStart] = icharge;
    for (int icol = 0; icol < 512; icol ++) {
      for (int iaddr = 0; iaddr < 1024; iaddr ++) {
        HitData[icharge-myChargeStart][icol][iaddr] = 0;
      }
    }
  }
}


void CopyHitData(std::vector <TPixHit> *Hits, int charge) {
  for (int ihit = 0; ihit < Hits->size(); ihit ++) {
    HitData[charge-myChargeStart][Hits->at(ihit).dcol + Hits->at(ihit).region * 16][Hits->at(ihit).address] ++;
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
      HasData = false;
      for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
        if (HitData[icharge - myChargeStart][icol][iaddr] > 0) HasData = true;
      }
      
      if (HasData) {
        for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
          fprintf(fp, "%d %d %d %d\n", icol, iaddr, icharge, HitData[icharge - myChargeStart][icol][iaddr]);
	}
      }
    }
  }
  fclose (fp);
}


// initialisation of Fromu
int configureFromu(TAlpide *chip) {
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x20);            // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  myStrobeLength);  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, myStrobeDelay);   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength);   // fromu pulsing 2: pulse length 
}


// initialisation of chip DACs
int configureDACs(TAlpide *chip) {
  chip->WriteRegister (Alpide::REG_VPULSEH, 170);
  chip->WriteRegister (Alpide::REG_VPULSEL, 169);
  chip->WriteRegister (Alpide::REG_VRESETD, myVRESETD);
  chip->WriteRegister (Alpide::REG_VCASN,   myVCASN);
  chip->WriteRegister (Alpide::REG_VCASN2,  myVCASN2);
  chip->WriteRegister (Alpide::REG_VCLIP,   myVCLIP);
  chip->WriteRegister (Alpide::REG_ITHR,    myITHR);
}


// initialisation of fixed mask
int configureMask(TAlpide *chip) {
  // unmask all pixels 
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK, true);
}


// setting of mask stage during scan
int configureMaskStage(TAlpide *chip, int istage) {
  int row    = istage / 4;
  int region = istage % 4;

  //uint32_t regionmod = 0x08080808 >> region;

  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   true);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);

  //AlpideConfig::WritePixRegRow (chip, Alpide::PIXREG_MASK,   false, row);
  //AlpideConfig::WritePixRegRow (chip, Alpide::PIXREG_SELECT, true,  row);

  //chip->WriteRegister (Alpide::REG_REGDISABLE_LOW,  (uint16_t) regionmod);
  //chip->WriteRegister (Alpide::REG_REGDISABLE_HIGH, (uint16_t) regionmod);

  for (int icol = 0; icol < 1024; icol += 8) {
    AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_MASK,   false, istage % 1024, icol + istage / 1024);
    AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_SELECT, true,  istage % 1024, icol + istage / 1024);   
  }

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

void WriteScanConfig(const char *fName, TAlpide *chip, TReadoutBoardDAQ *daqBoard) {
  char Config[1000];
  FILE *fp = fopen(fName, "w");

  chip     -> DumpConfig("", false, Config);
  std::cout << Config << std::endl;
  fprintf(fp, "%s\n", Config);
  daqBoard -> DumpConfig("", false, Config);
  fprintf(fp, "%s\n", Config);
  std::cout << Config << std::endl;
    
  fclose(fp);
}



void scan() {   
  unsigned char         buffer[1024*4000]; 
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;


  for (int istage = 0; istage < myMaskStages; istage ++) {
    std::cout << "Mask stage " << istage << std::endl;
    for (int i = 0; i < fChips.size(); i ++) {
      configureMaskStage (fChips.at(i), istage);
    }
    configureMaskStage (fChips.at(0), istage);

    for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
      //std::cout << "Charge = " << icharge << std::endl;
      fChips.at(0)->WriteRegister (Alpide::REG_VPULSEL, 170 - icharge);
      fBoards.at(0)->Trigger(myNTriggers);

      int itrg = 0;
      while(itrg < myNTriggers) {
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
      CopyHitData(Hits, icharge);
    }
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

  char Suffix[20], fName[100], Config[1000];

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

    sprintf(fName, "Data/ThresholdScan_%s.dat", Suffix);
    WriteDataToFile (fName, true);
    sprintf(fName, "Data/ScanConfig_%s.cfg", Suffix);
    WriteScanConfig (fName, fChips.at(0), myDAQBoard);

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
