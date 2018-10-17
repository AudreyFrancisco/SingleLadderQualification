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
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include <string.h>
#include <unistd.h>
#include <unordered_map>

TBoardType                   fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *>       fChips;
TConfig *                    fConfig;
std::unordered_map<int, int> fReceivers;

int myVCASN   = 57;
int myITHR    = 50;
int myVCASN2  = 64;
int myVCLIP   = 0;
int myVRESETD = 147;

int myStrobeLength = 20; // strobe length in units of 25 ns
int myStrobeDelay  = 10;
int myPulseLength  = 4000;

int myPulseDelay = 40;
// int myNTriggers    = 1000000;
int myNTriggers = 1000000;
// int myNTriggers    = 100;

int fEnabled = 0; // variable to count number of enabled chips; leave at 0

int HitData[16][512][1024];

void ClearHitData()
{
  for (int ichip = 0; ichip < 16; ichip++) {
    for (int icol = 0; icol < 512; icol++) {
      for (int iaddr = 0; iaddr < 1024; iaddr++) {
        HitData[ichip][icol][iaddr] = 0;
      }
    }
  }
}

void CopyHitData(std::vector<TPixHit> *Hits)
{
  for (unsigned int ihit = 0; ihit < Hits->size(); ihit++) {
    int chipId  = Hits->at(ihit).chipId;
    int dcol    = Hits->at(ihit).dcol;
    int region  = Hits->at(ihit).region;
    int address = Hits->at(ihit).address;
    if ((chipId < 0) || (dcol < 0) || (region < 0) || (address < 0)) {
      std::cout << "Bad pixel coordinates ( <0), skipping hit" << std::endl;
    }
    else {
      HitData[chipId][dcol + region * 16][address]++;
    }
  }
  Hits->clear();
}

bool HasData(int chipId)
{
  for (int icol = 0; icol < 512; icol++) {
    for (int iaddr = 0; iaddr < 1024; iaddr++) {
      if (HitData[chipId][icol][iaddr] > 0) return true;
    }
  }
  return false;
}

void WriteDataToFile(const char *fName, bool Recreate)
{
  char  fNameChip[120];
  FILE *fp;

  char fNameTemp[100];
  sprintf(fNameTemp, "%s", fName);
  strtok(fNameTemp, ".");

  for (unsigned int ichip = 0; ichip < fChips.size(); ichip++) {
    int chipId = fChips.at(ichip)->GetConfig()->GetChipId() & 0xf;
    if (!HasData(chipId)) continue; // write files only for chips with data
    if (fChips.size() > 1) {
      sprintf(fNameChip, "%s_Chip%d.dat", fNameTemp, chipId);
    }
    else {
      sprintf(fNameChip, "%s.dat", fNameTemp);
    }
    std::cout << "Writing data to file " << fNameChip << std::endl;

    if (Recreate)
      fp = fopen(fNameChip, "w");
    else
      fp = fopen(fNameChip, "a");

    for (int icol = 0; icol < 512; icol++) {
      for (int iaddr = 0; iaddr < 1024; iaddr++) {
        if (HitData[ichip][icol][iaddr] > 0) {
          fprintf(fp, "%d %d %d\n", icol, iaddr, HitData[ichip][icol][iaddr]);
        }
      }
    }
    if (fp) fclose(fp);
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
  // chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength);   // fromu pulsing 2: pulse
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
  // chip->WriteRegister (Alpide::REG_MODECONTROL, 0x21); // strobed readout mode

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

void scan()
{
  unsigned char buffer[MAX_EVENT_SIZE];
  int           n_bytes_data, n_bytes_header, n_bytes_trailer, nClosedEvents = 0;
  int           nBad = 0, nSkipped = 0, prioErrors = 0, errors8b10b = 0, errorsRecvChipID = 0;
  TBoardHeader  boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  int nTrains, nRest, nTrigsThisTrain, nTrigsPerTrain = 100;

  nTrains = myNTriggers / nTrigsPerTrain;
  nRest   = myNTriggers % nTrigsPerTrain;

  std::cout << "NTriggers: " << myNTriggers << std::endl;
  std::cout << "NTriggersPerTrain: " << nTrigsPerTrain << std::endl;
  std::cout << "NTrains: " << nTrains << std::endl;
  std::cout << "NRest: " << nRest << std::endl;

  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(fBoards.at(0));

  if (myMOSAIC) {
    myMOSAIC->StartRun();
  }

  for (int itrain = 0; itrain <= nTrains; itrain++) {
    std::cout << "Train: " << itrain << std::endl;
    if (itrain == nTrains) {
      nTrigsThisTrain = nRest;
    }
    else {
      nTrigsThisTrain = nTrigsPerTrain;
    }

    fBoards.at(0)->Trigger(nTrigsThisTrain);

    int itrg   = 0;
    int trials = 0;
    while (itrg < nTrigsThisTrain * fEnabled) {
      if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) <=
          0) { // no event available in buffer yet, wait a bit
        usleep(100);
        trials++;
        if (trials == 3) {
          std::cout << "Reached 3 timeouts, giving up on this event" << std::endl;
          itrg = nTrigsThisTrain * fEnabled;
          ++nSkipped;
          trials = 0;
        }
        continue;
      }
      else {
        // decode DAQboard event
        BoardDecoder::DecodeEvent(fBoards.at(0)->GetConfig()->GetBoardType(), buffer, n_bytes_data,
                                  n_bytes_header, n_bytes_trailer, boardInfo);
        int recv = boardInfo.channel;

        if (boardInfo.decoder10b8bError) ++errors8b10b;
        if (boardInfo.eoeCount) {
          nClosedEvents = boardInfo.eoeCount;
        }
        else {
          nClosedEvents = 1;
        }

        // decode Chip event
        int          n_bytes_chipevent = n_bytes_data - n_bytes_header - n_bytes_trailer;
        int          chipID            = -1;
        unsigned int bunchCounter      = 0;
        bool         Decode            = AlpideDecoder::DecodeEvent(
            buffer + n_bytes_header, n_bytes_chipevent, Hits, 0, boardInfo.channel, prioErrors,
            fConfig->GetScanConfig()->GetParamValue("MAXHITS"), 0x0, &chipID, &bunchCounter);
        ;
        bool RecvMatched = false;
        try {
          int expected_recv = fReceivers.at(chipID);
          if (expected_recv != recv) {
            std::cerr << "Expected Receiver: " << expected_recv << ", actual receiver: " << recv
                      << std::endl;
          }
          else
            RecvMatched = true;
        }
        catch (const std::out_of_range &oor) {
        }
        if (!RecvMatched) {
          ++errorsRecvChipID;
          std::cerr << "Chip ID did not match the receiver, discarding event data" << std::endl;
        }
        if (!Decode) {
          ++nBad;
          std::cout << "Failed to decode event" << std::endl;
          //  printf("Bad Event: ");
          //  for (int i = 0; i < n_bytes_chipevent; i++) {
          //    printf ("%02x ", buffer[n_bytes_header + i]);
          //  }
          //  printf ("\n");
        }
        itrg += nClosedEvents;
        if (Decode && RecvMatched)
          CopyHitData(Hits);
        else
          ++nSkipped;
      }
    }
    // std::cout << "Number of hits: " << Hits->size() << std::endl;
  }
  if (myMOSAIC) {
    myMOSAIC->StopRun();
    std::cout << "Total number of 8b10b decoder errors:               " << errors8b10b << std::endl;
    std::cout << "Total number of receiver / chip ID matching errors: " << errorsRecvChipID
              << std::endl;
  }
  std::cout << "Number of corrupt events:             " << nBad << std::endl;
  std::cout << "Number of skipped points:             " << nSkipped << std::endl;
  std::cout << "Priority encoder errors:              " << prioErrors << std::endl;
  std::cout << std::endl;
  std::cout << fEnabled << " chips were enabled for scan." << std::endl << std::endl;
}

int main(int argc, char **argv)
{

  decodeCommandParameters(argc, argv);
  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  char Suffix[80], fName[200];

  ClearHitData();
  time_t     t   = time(0); // get time now
  struct tm *now = localtime(&t);
  sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday,
          now->tm_hour, now->tm_min, now->tm_sec);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(fBoards.at(0));

  if (fBoards.size() == 1) {

    fBoards.at(0)->SendOpCode(Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode(Alpide::OPCODE_PRST);

    for (unsigned int i = 0; i < fChips.size(); i++) {
      if (fChips.at(i)->GetConfig()->IsEnabled()) {
        fEnabled++;
        configureChip(fChips.at(i));
        fReceivers[fChips.at(i)->GetConfig()->GetChipId()] =
            fChips.at(i)->GetConfig()->GetDataLink();
      }
      else if (fChips.at(i)->GetConfig()->HasEnabledSlave()) {
        AlpideConfig::BaseConfigPLL(fChips.at(i));
      }
    }

    fBoards.at(0)->SendOpCode(Alpide::OPCODE_RORST);

    // put your test here...
    if (fBoards.at(0)->GetConfig()->GetBoardType() == boardMOSAIC) {
      fBoards.at(0)->SetTriggerConfig(true, true, myPulseDelay, myPulseLength * 10);
      fBoards.at(0)->SetTriggerSource(trigInt);
    }
    else if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
      fBoards.at(0)->SetTriggerConfig(true, false, myStrobeDelay, myPulseDelay);
      fBoards.at(0)->SetTriggerSource(trigExt);
    }

    scan();

    sprintf(fName, "Data/NoiseOccupancy_%s.dat", Suffix);
    WriteDataToFile(fName, true);

    sprintf(fName, "Data/ScanConfig_%s.cfg", Suffix);
    WriteScanConfig(fName, fChips.at(0), myDAQBoard);

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
