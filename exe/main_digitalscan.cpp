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
#include "TConfig.h"
#include "THisto.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include <string.h>
#include <unistd.h>

// !!! NOTE: Scan parameters are now set via Config file

TBoardType                   fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *>       fChips;
TConfig *                    fConfig;

int myNTriggers;
int myMaskStages;
int myPixPerRegion;
int maxTrigsPerTrain;

std::vector<int> fEnPerBoard; // variable to count number of enabled chips per board;

TScanHisto *fScanHisto;

const unsigned int kNdcol = 512;
const unsigned int kNaddr = 1024;

template <class T> T Sum(std::vector<T> __v, size_t __len)
{
  T ret = __v.at(0);
  __len = (__len > __v.size()) ? __v.size() : __len;
  for (size_t i = 1; i < __len; ++i)
    ret += __v.at(i);

  return ret;
}

void CreateScanHisto()
{
  common::TChipIndex id;
  fScanHisto = new TScanHisto();

  THisto histo("DigScanHisto", "DigScanHisto", kNdcol, 0, kNdcol - 1, kNaddr, 0,
               kNaddr); // dcol, address;

  for (unsigned int iboard = 0; iboard < fBoards.size(); iboard++) {
    for (unsigned int ichip = 0; ichip < fChips.size(); ichip++) {
      if ((fChips.at(ichip)->GetConfig()->IsEnabled()) &&
          (fChips.at(ichip)->GetReadoutBoard() == fBoards.at(iboard))) {
        id.boardIndex   = iboard;
        id.dataReceiver = fChips.at(ichip)->GetConfig()->GetParamValue("RECEIVER");
        id.chipId       = fChips.at(ichip)->GetConfig()->GetChipId();

        fScanHisto->AddHisto(id, histo);
      }
    }
  }
  std::cout << "CreateHisto: generated map with " << fScanHisto->GetSize() << " elements"
            << std::endl;
  return;
}

void InitScanParameters()
{
  myMaskStages     = fConfig->GetScanConfig()->GetParamValue("NMASKSTAGES");
  myPixPerRegion   = fConfig->GetScanConfig()->GetParamValue("PIXPERREGION");
  myNTriggers      = fConfig->GetScanConfig()->GetParamValue("NINJ");
  maxTrigsPerTrain = fConfig->GetScanConfig()->GetParamValue("MAXNTRIGTRAIN");
}

void FillHisto(int board, std::vector<TPixHit> *Hits)
{
  common::TChipIndex idx;
  idx.boardIndex = board;

  for (const auto &hit : *Hits) {
    idx.dataReceiver = hit.channel;
    idx.chipId       = hit.chipId;
    int dcol         = hit.dcol + 16 * hit.region;
    int addr         = hit.address;

    if ((hit.channel < 0) || (hit.chipId < 0) || (dcol < 0) || (dcol > 511) || (hit.region < 0) ||
        (addr < 0) || (addr > 1023)) {
      printf("%d %d %d %d %d \n", idx.dataReceiver, idx.chipId, dcol, hit.region, addr);
      std::cout << "Bad pixel coordinates ( <0), skipping hit" << std::endl;
      abort();
    }
    else {
      fScanHisto->Incr(idx, dcol, addr);
    }
  }
  Hits->clear();
  return;
}

bool HasData(const common::TChipIndex &idx)
{
  for (unsigned int icol = 0; icol < kNdcol; icol++) {
    for (unsigned int iaddr = 0; iaddr < kNaddr; iaddr++) {
      if ((*fScanHisto)(idx, icol, iaddr) > 0) return true;
    }
  }

  return false;
}

void WriteDataToFile(const char *fName, bool Recreate)
{
  char  fNameChip[100];
  FILE *fp;

  char fNameTemp[100];
  sprintf(fNameTemp, "%s", fName);
  strtok(fNameTemp, ".");
  for (unsigned int ib = 0; ib < fBoards.size(); ++ib) {
    for (unsigned int ichip = 0; ichip < fChips.size(); ichip++) {
      if (!fChips.at(ichip)->GetConfig()->IsEnabled() ||
          fChips.at(ichip)->GetReadoutBoard() != fBoards.at(ib))
        continue;

      int chipId = fChips.at(ichip)->GetConfig()->GetChipId() & 0x7F;
      std::cout << "ichip = " << ichip << " with chipId = " << chipId << std::endl;
      int channel = fBoards.at(ib)->GetReceiver(chipId); // Get receiver

      common::TChipIndex idx;
      idx.boardIndex   = ib;
      idx.dataReceiver = channel;
      idx.chipId       = chipId;

      if (!HasData(idx)) continue; // write files only for chips with data

      if (fChips.size() > 1) {
        sprintf(fNameChip, "%s_Mod%d-Chip%d.dat", fNameTemp, (chipId & 0x70) >> 4, chipId & 0xF);
      }
      else {
        sprintf(fNameChip, "%s.dat", fNameTemp);
      }
      std::cout << "Writing data to file " << fNameChip << std::endl;

      if (Recreate)
        fp = fopen(fNameChip, "w");
      else
        fp = fopen(fNameChip, "a");
      for (unsigned int icol = 0; icol < kNdcol; icol++) {
        for (unsigned int iaddr = 0; iaddr < kNaddr; iaddr++) {
          double hits = (*fScanHisto)(idx, icol, iaddr);
          if (hits > 0) {
            fprintf(fp, "%d %d %d\n", icol, iaddr, (int)hits);
          }
        }
      }
      if (fp) fclose(fp);
    }
  }
  fScanHisto->Clear();
}

// initialisation of Fromu
int configureFromu(TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,
                      0x0); // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(
      Alpide::REG_FROMU_CONFIG2,
      chip->GetConfig()->GetParamValue("STROBEDURATION")); // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1,
                      chip->GetConfig()->GetParamValue("STROBEDELAYCHIP")); // fromu pulsing 1:
                                                                            // delay pulse - strobe
                                                                            // (not used here, since
                                                                            // using external
                                                                            // strobe)
  chip->WriteRegister(
      Alpide::REG_FROMU_PULSING2,
      chip->GetConfig()->GetParamValue("PULSEDURATION")); // fromu pulsing 2: pulse length
  return 0;
}

int configureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  configureFromu(chip);

  AlpideConfig::ConfigureCMU(chip);

  return 0;
}

void scan()
{
  unsigned char buffer[1024 * 4000];
  int           n_bytes_data, n_bytes_header, n_bytes_trailer, errors8b10b = 0, nClosedEvents = 0;
  int           nBad       = 0;
  int           nSkipped   = 0;
  int           prioErrors = 0;
  TBoardHeader  boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  int nTrigsPerTrain = (maxTrigsPerTrain > 0) ? maxTrigsPerTrain : myNTriggers;
  while ((myNTriggers % nTrigsPerTrain) != 0)
    nTrigsPerTrain--;
  int nTrains = myNTriggers / nTrigsPerTrain;

  std::cout << "Doing " << nTrains << " with " << nTrigsPerTrain << " triggers each." << std::endl;

  for (const auto &rBoard : fBoards)
    rBoard->StartRun();

  for (int istage = 0; istage < myMaskStages; istage++) {
    std::cout << std::endl << "Mask stage " << istage << std::endl;
    for (unsigned int i = 0; i < fChips.size(); i++) {
      if (!fChips.at(i)->GetConfig()->IsEnabled()) continue;
      AlpideConfig::ConfigureMaskStage(fChips.at(i), myPixPerRegion, istage);
    }

    for (int itrain = 0; itrain < nTrains; itrain++) {
      std::cout << "Trigger train " << itrain << std::endl;

      // Send triggers for all boards
      for (const auto &rBoard : fBoards) {
        // uint16_t Value;
        // fChips.at(0)->ReadRegister(Alpide::REG_CMUDMU_CONFIG, Value);
        // std::cout << "CMU DMU Config: 0x" << std::hex << Value << std::dec << std::endl;
        // fChips.at(0)->ReadRegister(Alpide::REG_FROMU_STATUS1, Value);
        // std::cout << "Trigger counter before: " << Value << std::endl;
        rBoard->Trigger(nTrigsPerTrain);
        // fChips.at(0)->ReadRegister(Alpide::REG_FROMU_STATUS1, Value);
        // std::cout << "Trigger counter after: " << Value << std::endl;

        // rBoard->SendOpCode(Alpide::OPCODE_DEBUG);
        // AlpideConfig::PrintDebugStream(fChips.at(0));
      }

      // Read data for all boards
      for (unsigned int ib = 0; ib < fBoards.size(); ++ib) {
        int itrg      = 0;
        int nTrials   = 0;
        int MAXTRIALS = 3;

        int fEnabled = fEnPerBoard.at(ib);
        while (itrg < nTrigsPerTrain * fEnabled) {
          if (fBoards.at(ib)->ReadEventData(n_bytes_data, buffer) ==
              -1) { // no event available in buffer yet, wait a bit
            usleep(100);
            nTrials++;
            if (nTrials == MAXTRIALS) {
              std::cout << "Reached " << nTrials
                        << " timeouts, giving up on this point (board = " << ib
                        << ", itrg = " << itrg << ")." << std::endl;
              itrg = nTrigsPerTrain * fEnabled;
              nSkipped++;
              nTrials = 0;
            }
            continue;
          }
          else {
            // std::cout << "received Event" << itrg << " with length " << n_bytes_data <<
            // std::endl;
            // for (int iByte=0; iByte<n_bytes_data; ++iByte) {
            //  printf ("%02x ", (int) buffer[iByte]);
            //}
            // std::cout << std::endl;
            // decode board event
            BoardDecoder::DecodeEvent(fBoards.at(ib)->GetConfig()->GetBoardType(), buffer,
                                      n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
            // std::cout << "Closed data counter: " <<  boardInfo.eoeCount << std::endl;
            if (boardInfo.eoeCount) {
              nClosedEvents = boardInfo.eoeCount;
            }
            else {
              nClosedEvents = 1;
            }
            if (boardInfo.decoder10b8bError) errors8b10b++;
            // decode Chip event
            int n_bytes_chipevent = n_bytes_data - n_bytes_header - n_bytes_trailer;
            if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits, 0,
                                            boardInfo.channel, prioErrors)) {
              std::cout << "Found bad event " << std::endl;
              nBad++;
              if (nBad > 10) continue;
              FILE *fDebug = fopen("DebugData.dat", "a");
              for (int iByte = 0; iByte < n_bytes_data; ++iByte) {
                fprintf(fDebug, "%02x ", (int)buffer[iByte]);
              }
              fclose(fDebug);
            }
            // std::cout << "Does chipId contains modId??? " << Hits->back().chipId << " " <<
            // ((Hits->back().chipId >> 4) & 0x7) << std::endl;
            // std::cout << "total number of hits found: " << Hits->size() << std::endl;
            itrg += nClosedEvents;
          }
        }
        // std::cout << "Hit pixels: " << std::endl;
        // for (int i=0; i<Hits->size(); i++) {
        //  std::cout << i << ":\t region: " << Hits->at(i).region << "\tdcol: " << Hits->at(i).dcol
        // << "\taddres: " << Hits->at(i).address << std::endl;
        //}
        FillHisto(ib, Hits);
      }
    }
  }

  std::cout << std::endl;
  for (const auto &rBoard : fBoards) {
    TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(rBoard);

    if (myMOSAIC) {
      myMOSAIC->StopRun();
      std::cout << "Total number of 8b10b decoder errors: " << errors8b10b << std::endl;
    }
  }
  std::cout << "Number of corrupt events:             " << nBad << std::endl;
  std::cout << "Number of skipped points:             " << nSkipped << std::endl;
  std::cout << "Priority encoder errors:              " << prioErrors << std::endl;
  std::cout << std::endl;
  int sum_of_en = Sum(fEnPerBoard, fEnPerBoard.size());
  std::cout << sum_of_en << " chips were enabled for scan." << std::endl << std::endl;
}

int main(int argc, char **argv)
{

  decodeCommandParameters(argc, argv);
  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  sleep(1);
  char Suffix[20], fName[100];

  InitScanParameters();
  CreateScanHisto();
  time_t     t   = time(0); // get time now
  struct tm *now = localtime(&t);
  sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday,
          now->tm_hour, now->tm_min, now->tm_sec);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(fBoards.at(0));

  if (fBoards.size()) {

    fEnPerBoard.resize(fBoards.size());
    for (unsigned int ib = 0; ib < fBoards.size(); ++ib) {

      fBoards.at(ib)->SendOpCode(Alpide::OPCODE_GRST);
      fBoards.at(ib)->SendOpCode(Alpide::OPCODE_PRST);

      for (unsigned int i = 0; i < fChips.size(); i++) {
        if (fChips.at(i)->GetConfig()->IsEnabled() &&
            fChips.at(i)->GetReadoutBoard() == fBoards.at(ib))
          fEnPerBoard.at(ib)++;
      }
    }
    for (unsigned int i = 0; i < fChips.size(); i++) {
      if (fChips.at(i)->GetConfig()->IsEnabled()) {
        std::cout << "Configuring chip " << i
                  << ", chip ID = " << fChips.at(i)->GetConfig()->GetChipId() << std::endl;
        configureChip(fChips.at(i));
      }
      else if (fChips.at(i)->GetConfig()->HasEnabledSlave()) {
        std::cout << "Configuring PLL of chip " << i
                  << ", chip ID = " << fChips.at(i)->GetConfig()->GetChipId() << std::endl;
        AlpideConfig::BaseConfigPLL(fChips.at(i));
      }
    }
    int sum_of_en = Sum(fEnPerBoard, fEnPerBoard.size());
    std::cout << "Found " << sum_of_en << " enabled chips" << std::endl;

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

    scan();

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  sprintf(fName, "Data/DigitalScan_%s.dat", Suffix);
  WriteDataToFile(fName, true);
  return 0;
}
