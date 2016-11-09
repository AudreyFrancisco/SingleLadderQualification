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

int myStrobeDelay  = 0;
int myPulseLength  = 500;

int myPulseDelay   = 50;
//int myNTriggers    = 1000000;
int myNTriggers    = 100000;
//int myNTriggers    = 100;


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
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  chip->GetConfig()->GetStrobeDuration());  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, myStrobeDelay);   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  //  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength);   // fromu pulsing 2: pulse length
}


// initialisation of fixed mask
int configureMask(TAlpide *chip) {
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   false);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);
}


int configureChip(TAlpide *chip) {
  AlpideConfig::BaseConfig(chip);

  configureFromu(chip);
  configureMask (chip);

  int readout_mode = (chip->GetConfig()->GetReadoutMode()) ? 0x2 : 0x1;
  chip->WriteRegister (Alpide::REG_MODECONTROL, 0x20 | (readout_mode & 0x3)); // strobed readout mode
}

void WriteScanConfig(const char *fName, TAlpide *chip, TReadoutBoardDAQ *daqBoard) {
  char Config[1000];
  FILE *fp = fopen(fName, "w");

  chip     -> DumpConfig("", false, Config);
  //std::cout << Config << std::endl;
  fprintf(fp, "%s\n", Config);
  if (daqBoard) daqBoard -> DumpConfig("", false, Config);
  fprintf(fp, "%s\n", Config);
  //std::cout << Config << std::endl;

  fprintf(fp, "\n", Config);

  fprintf(fp, "NTRIGGERS %i\n", myNTriggers);

  fclose(fp);
}


void scan() {
  unsigned char         buffer[1024*4000];
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  int nTrains, nRest, nTrigsThisTrain, nTrigsPerTrain = 1000;

  nTrains = myNTriggers / nTrigsPerTrain;
  nRest   = myNTriggers % nTrigsPerTrain;

  std::cout << "NTriggers: " << myNTriggers << std::endl;
  std::cout << "NTriggersPerTrain: " << nTrigsPerTrain << std::endl;
  std::cout << "NTrains: " << nTrains << std::endl;
  std::cout << "NRest: " << nRest << std::endl;

  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));
  TReadoutBoardDAQ *myDAQboard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  int nEventsDecoded = 0;


  if (myMOSAIC) {
    myMOSAIC->StartRun();
  }
  if (myDAQboard) {
    myDAQboard->StartTrigger();
  }
  for (int itrain = 0; itrain <= nTrains; itrain ++) {
    std::cout << "Train: " << itrain << std::endl;
    if (itrain == nTrains) {
      nTrigsThisTrain = nRest;
    }
    else {
      nTrigsThisTrain = nTrigsPerTrain;
    }

    fBoards.at(0)->Trigger(-nTrigsThisTrain);

    int itrg = 0;
    while (itrg < nTrigsThisTrain) {
      if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
        std::cout << "No event in buffer but triggers where issued!" << std::endl;
        usleep(10);
      }
      else {
        // decode DAQboard event
        BoardDecoder::DecodeEvent(fBoards.at(0)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
        // decode Chip event
        int n_bytes_chipevent=n_bytes_data-n_bytes_header-n_bytes_trailer;
        AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits);

        //if (Hits->size()>0) std::cout << "Number of hits: " << Hits->size() << std::endl;
        CopyHitData(Hits);

        nEventsDecoded++;
        itrg++;
      }
    }
  }

  if (myMOSAIC) {
    myMOSAIC->StopRun();
  }
  if (myDAQboard) {
    myDAQboard->StopTrigger();
    while(myDAQboard->GetStatusReadData()!=-3) fBoards.at(0)->Trigger(-nTrigsPerTrain);
  }
  std::cout << nEventsDecoded << " events were decoded " << std::endl;
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
      return -1;
    }
    else if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
      fBoards.at(0)->SetTriggerConfig (true, false, myStrobeDelay, myPulseDelay);
      fBoards.at(0)->SetTriggerSource (trigExt);
      TBoardConfigDAQ* cnf = (TBoardConfigDAQ*)(fBoards.at(0)->GetConfig());
      cnf->SetStrobePulseSeq(0);
      cnf->SetPktBasedROEnable(true);
      cnf->SetNTriggers(0);
      myDAQBoard->WriteReadoutModuleConfigRegisters();
      myDAQBoard->WriteResetModuleConfigRegisters();
      myDAQBoard->WriteTriggerModuleConfigRegisters();
    }

    scan();

    sprintf(fName, "Data/NoiseOccupancyExt_%s.dat", Suffix);
    WriteDataToFile (fName, true);

    sprintf(fName, "Data/ScanConfig_%s.cfg", Suffix);
    WriteScanConfig (fName, fChips.at(0), myDAQBoard);

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  //sprintf(fName, "Data/NoiseOccupancy_%s.dat", Suffix);
  //WriteDataToFile (fName, true);
  return 0;
}
