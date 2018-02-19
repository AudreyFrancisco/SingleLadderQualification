// Template to prepare standard test routines
// ==========================================
//
// After successful call to initSetup() the elements of the setup are accessible in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1 readout board, i.e.
// fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for staves and modules,
// all chip accesses should be done with a loop over all elements of the chip vector.
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);
// For an example how to access board-specific functions see the power off at the end of main.
//
// The functions that should be modified for the specific test are configureChip() and main()

#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TAlpide.h"
#include "TChipConfig.h"
#include "TConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include <string.h>
#include <unistd.h>

TBoardType                   fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *>       fChips;
TConfig *                    fConfig;

std::vector<int> fEnPerBoard;

int myVCASN   = 57;
int myITHR    = 51;
int myVCASN2  = 64;
int myVCLIP   = 0;
int myVRESETD = 147;

int myStrobeLength = 20; // strobe length in units of 25 ns
int myStrobeDelay  = 10;
int myPulseLength  = 4000;

int myPulseDelay = 40;
// int myNTriggers  = 1000000;
int myNTriggers = 200;
// int myNTriggers    = 100;


char fNameRaw[1024];

int HitData[10][16][512][1024];

void ClearHitData()
{
  for (int imod = 0; imod < 10; imod++) {
    for (int ichip = 0; ichip < 16; ichip++) {
      for (int icol = 0; icol < 512; icol++) {
        for (int iaddr = 0; iaddr < 1024; iaddr++) {
          HitData[imod][ichip][icol][iaddr] = 0;
        }
      }
    }
  }
}

void CopyHitData(std::vector<TPixHit> *Hits)
{
  for (unsigned int ihit = 0; ihit < Hits->size(); ihit++) {
    int chipId  = Hits->at(ihit).chipId & 0xf;
    int modId   = (chipId >> 4) & 0x7;
    int dcol    = Hits->at(ihit).dcol;
    int region  = Hits->at(ihit).region;
    int address = Hits->at(ihit).address;
    HitData[modId][chipId][dcol + region * 16][address]++;
  }
  Hits->clear();
}

bool HasDataChip(int chipId)
{
  for (int imod = 0; imod < 10; imod++) {
    for (int icol = 0; icol < 512; icol++) {
      for (int iaddr = 0; iaddr < 1024; iaddr++) {
        if (HitData[imod][chipId][icol][iaddr] > 0) return true;
      }
    }
  }
  return false;
}

void WriteRawData(FILE *fp, std::vector<TPixHit> *Hits, int oldHits, TBoardHeader boardInfo)
{
  int ChipId, dcol, address, event;
  for (unsigned int ihit = oldHits; ihit < Hits->size(); ihit++) {
    ChipId  = Hits->at(ihit).chipId;
    dcol    = Hits->at(ihit).dcol + Hits->at(ihit).region * 16;
    address = Hits->at(ihit).address;
    if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
      event = boardInfo.eventId;
    }
    else {
      event = boardInfo.eoeCount;
    }
    fprintf(fp, "%d %d %d %d\n", event, ChipId, dcol, address);
  }
}

void WriteDataToFile(const char *fName, bool Recreate)
{
  FILE *fp;

  char fNameChip[100];
  char fNameTemp[100];

  sprintf(fNameTemp, "%s", fName);
  strtok(fNameTemp, ".");

  for (unsigned int imod = 0; imod < 10; imod++) {
    for (unsigned int ichip = 0; ichip < fChips.size(); ichip++) {
      int chipId = fChips.at(ichip)->GetConfig()->GetChipId() & 0xf;
      int modId  = (chipId >> 4) & 0x7;
      if (!HasDataChip(chipId)) continue; // write files only for chips with data
      if (fChips.size() > 1) {
        sprintf(fNameChip, "%s_Chip%d_%d.dat", fNameTemp, modId, chipId);
      }
      else {
        sprintf(fNameChip, "%s.dat", fNameTemp);
      }
      std::cout << "Writing data to file " << fNameChip << std::endl;
      if (Recreate) {
        fp       = fopen(fNameChip, "w");
        Recreate = false;
      }
      else
        fp = fopen(fNameChip, "a");
      for (int icol = 0; icol < 512; icol++) {
        for (int iaddr = 0; iaddr < 1024; iaddr++) {
          if (HitData[imod][ichip][icol][iaddr] > 0) {
            fprintf(fp, "%d %d %d %d %d\n", modId, chipId, icol, iaddr,
                    HitData[imod][ichip][icol][iaddr]);
          }
        }
      }
      if (fp) fclose(fp);
    }
  }
}

// initialisation of Fromu
int configureFromu(TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,
                      0x0); // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2, myStrobeLength); // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, myStrobeDelay); // fromu pulsing 1: delay pulse -
                                                                  // strobe (not used here, since
                                                                  // using external strobe)
  //  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength);   // fromu pulsing 2: pulse
  // length
  return 0;
}

// initialisation of fixed mask
int configureMask(TAlpide *chip)
{
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_MASK, false);
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_SELECT, false);
  return 0;
}

int configureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  configureFromu(chip);
  configureMask(chip);
  AlpideConfig::ConfigureCMU(chip);

  chip->WriteRegister(Alpide::REG_MODECONTROL, 0x21); // strobed readout mode
  return 0;
}

void WriteScanConfig(const char *fName, TAlpide *chip, TReadoutBoardDAQ *daqBoard)
{
  char  Config[1000];
  FILE *fp = fopen(fName, "w");

  chip->DumpConfig("", false, Config);
  // std::cout << Config << std::endl;
  fprintf(fp, "%s\n", Config);
  if (daqBoard) daqBoard->DumpConfig("", false, Config);
  fprintf(fp, "%s\n", Config);
  // std::cout << Config << std::endl;

  fprintf(fp, "\n");

  fprintf(fp, "NTRIGGERS %i\n", myNTriggers);

  fclose(fp);
}

void WriteChipList(const char *fName, bool Recreate)
{
  FILE *fp;

  if (Recreate) {
    fp       = fopen(fName, "w");
    Recreate = false;
  }
  else
    fp = fopen(fName, "a");
  for (unsigned int i = 0; i < fConfig->GetNChips(); i++) {
    TChipConfig *chipConfig = fConfig->GetChipConfig(i);
    int          chipId     = chipConfig->GetChipId();
    fprintf(fp, " %d\n", chipId);
  }
  if (fp) fclose(fp);
}

void scan()
{
  unsigned char buffer[1024 * 4000];
  int           n_bytes_data, n_bytes_header, n_bytes_trailer, oldHits;
  int           prioErrors;
  TBoardHeader  boardInfo;

  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  FILE *rawFile = fopen(fNameRaw, "w");

  int nTrains, nRest, nTrigsPerTrain = 1;

  nTrains = myNTriggers / nTrigsPerTrain;
  nRest   = myNTriggers % nTrigsPerTrain;

  std::cout << "NTriggers: " << myNTriggers << std::endl;
  std::cout << "NTriggersPerTrain: " << nTrigsPerTrain << std::endl;
  std::cout << "NTrains: " << nTrains << std::endl;
  std::cout << "NRest: " << nRest << std::endl;

  for (const auto &rBoard : fBoards)
    rBoard->StartRun();

  for (unsigned int i = 0; i < fConfig->GetNChips(); i++) {
    TChipConfig *chipConfig = fConfig->GetChipConfig(i);
    std::cout << "chipId  " << chipConfig->GetChipId() << std::endl;

    int count;
    for (int itrain = 0; itrain <= nTrains; itrain++) {
      std::cout << "Train: " << itrain << std::endl;

      for (const auto &rBoard : fBoards) {
        rBoard->Trigger(nTrigsPerTrain);
      }


      for (unsigned int ib = 0; ib < fBoards.size(); ++ib) {
        int itrg = 0;
        count    = 0;

        int fEnabled = fEnPerBoard.at(ib);
        while (itrg < nTrigsPerTrain * fEnabled) {
          if (fBoards.at(ib)->ReadEventData(n_bytes_data, buffer) ==
              -1) { // no event available in buffer yet, wait a bit
            usleep(100);
            if (count > 50) break;
            count++;
            continue;
          }
          else {
            // decode event
            BoardDecoder::DecodeEvent(fBoards.at(ib)->GetConfig()->GetBoardType(), buffer,
                                      n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
            // decode Chip event
            int n_bytes_chipevent = n_bytes_data - n_bytes_header - n_bytes_trailer;
            oldHits               = Hits->size();
            AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits, 0,
                                       boardInfo.channel, prioErrors);
            WriteRawData(rawFile, Hits, oldHits, boardInfo);
            itrg++;
          }
        }
      }
      // std::cout << "Number of hits: " << Hits->size() << std::endl;
      CopyHitData(Hits);
    }
  }

  std::cout << std::endl;
  for (const auto &rBoard : fBoards) {
    TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(rBoard);

    if (myMOSAIC) {
      myMOSAIC->StopRun();
    }
  }
  fclose(rawFile);
}

int main(int argc, char **argv)
{

  decodeCommandParameters(argc, argv);
  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  char Suffix[30], fName[1000];

  ClearHitData();
  time_t     t   = time(0); // get time now
  struct tm *now = localtime(&t);
  sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday,
          now->tm_hour, now->tm_min, now->tm_sec);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(fBoards.at(0));
  if (fBoards.size()) {

    for (const auto &rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_GRST);
      rBoard->SendOpCode(Alpide::OPCODE_PRST);
    }

    for (const auto &rChip : fChips) {
      if (!rChip->GetConfig()->IsEnabled()) continue;
      configureChip(rChip);
    }

    for (const auto &rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_RORST);
    }

    for (const auto &rBoard : fBoards) {

      rBoard->SendOpCode(Alpide::OPCODE_RORST);
      // put your test here...
      if (rBoard->GetConfig()->GetBoardType() == boardDAQ) {
        // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns *
        // strobe delay
        // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
        rBoard->SetTriggerConfig(true, false, 0,
                                 2 * rBoard->GetConfig()->GetParamValue("STROBEDELAYBOARD"));
        rBoard->SetTriggerSource(trigExt);
      }
      else {
        rBoard->SetTriggerConfig(true, true, rBoard->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                                 rBoard->GetConfig()->GetParamValue("PULSEDELAY"));
        rBoard->SetTriggerSource(trigInt);
      }
    }
  }

  sprintf(fNameRaw, "Data/SourceRaw_%s.dat", Suffix);
  scan();

  sprintf(fName, "Data/Source_%s.dat", Suffix);
  WriteDataToFile(fName, true);

  sprintf(fName, "Data/ScanConfig_%s.cfg", Suffix);
  WriteScanConfig(fName, fChips.at(0), myDAQBoard);

  sprintf(fName, "Data/ChipList_%s.dat", Suffix);
  WriteChipList(fName, true);

  if (myDAQBoard) {
    myDAQBoard->PowerOff();
    delete myDAQBoard;
  }


  // sprintf(fName, "Data/NoiseOccupancy_%s.dat", Suffix);
  // WriteDataToFile (fName, true);
  return 0;
}
