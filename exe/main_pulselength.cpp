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
#include <unistd.h>

TBoardType                   fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *>       fChips;
TConfig *                    fConfig;

// test setttings ----------------------------------------------------------------------

int myStrobeLength = 10;   // strobe length in units of 25 ns
int myStrobeDelay  = 0;    // not needed right now as strobe generated on DAQ board
int myPulseLength  = 2000; // 2000 = 50 us

int myNTriggers = 50;

// Pixel to test
int myRow = 5;
// int myCol = 13;
int myCol = 8; // Col within region, 0:15

// charge range
int myChargeStart = 1;   // 1 default
int myChargeStop  = 160; // 160 default
// int myChargeStop   = 4; // 160 default
int myChargeStep = 2;

// delay between pulse and strobe seems to be 50 ns + 12.5 ns * PulseDelay + 25 ns * StrobeDelay
int myPulseDelayStart = 36; // 36 = 500 ns
// int myPulseDelayStop    = 1600;  // 1000 = 12550 ns
int myPulseDelayStop = 1200; // 1200 = 15050 ns
int myPulseDelayStep = 16;   // 200 ns

// -------------------------------------------------------------------------------------

int AddressToColumn(int ARegion, int ADoubleCol, int AAddress)
{
  int Column    = ARegion * 32 + ADoubleCol * 2; // Double columns before ADoubleCol
  int LeftRight = ((((AAddress % 4) == 1) || ((AAddress % 4) == 2))
                       ? 1
                       : 0); // Left or right column within the double column

  Column += LeftRight;

  return Column;
}

int AddressToRow(int ARegion, int ADoubleCol, int AAddress)
{
  int Row = AAddress / 2;
  return Row;
}

void WriteDataToFile(const char *fName, std::vector<TPixHit> *Hits, int charge, int delay,
                     bool Recreate)
{
  FILE *fp;
  int   nHits = 0;
  int   col, row;
  if (Recreate)
    fp = fopen(fName, "w");
  else
    fp = fopen(fName, "a");

  for (int ireg = 0; ireg < 32; ireg++) {
    nHits = 0;
    // std::cout << ireg << std::endl;
    for (unsigned int i = 0; i < Hits->size(); i++) {
      col = AddressToColumn(Hits->at(i).region, Hits->at(i).dcol, Hits->at(i).address);
      row = AddressToRow(Hits->at(i).region, Hits->at(i).dcol, Hits->at(i).address);

      if ((col == ireg * 32 + myCol) && (row == myRow)) {
        nHits++;
      }
    }
    if (nHits) {
      // printf("       %d %d %d %d %d\n", ireg*32+myCol, row, charge, delay, nHits);
      fprintf(fp, "%d %d %d %d %d\n", ireg * 32 + myCol, row, charge, delay, nHits);
    }
  }
  Hits->clear();

  fclose(fp);
}

void WriteScanConfig(const char *fName, TAlpide *chip, TReadoutBoardDAQ *daqBoard)
{
  char  Config[1000];
  FILE *fp = fopen(fName, "w");

  chip->DumpConfig("", false, Config);
  // std::cout << Config << std::endl;
  fprintf(fp, "%s\n", Config);
  daqBoard->DumpConfig("", false, Config);
  fprintf(fp, "%s\n", Config);
  // std::cout << Config << std::endl;

  fprintf(fp, "\n");

  fprintf(fp, "NTRIGGERS %i\n", myNTriggers);
  fprintf(fp, "ROW %i\n", myRow);
  fprintf(fp, "COL %i\n", myCol);

  fprintf(fp, "CHARGESTART %i\n", myChargeStart);
  fprintf(fp, "CHARGESTOP %i\n", myChargeStop);
  fprintf(fp, "CHARGESTEP %i\n", myChargeStep);

  fprintf(fp, "PULSEDELAYSTART %i\n", myPulseDelayStart);
  fprintf(fp, "PULSEDELAYSTOP %i\n", myPulseDelayStop);
  fprintf(fp, "PULSEDELAYSTEP %i\n", myPulseDelayStep);

  fclose(fp);
}

// initialisation of Fromu
int configureFromu(TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,
                      0x20); // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2, myStrobeLength); // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, myStrobeDelay); // fromu pulsing 1: delay pulse -
                                                                  // strobe (not used here, since
                                                                  // using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength); // fromu pulsing 2: pulse length
  return 0;
}

// initialisation of fixed mask
int configureMask(TAlpide *chip)
{
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_MASK, true);
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_SELECT, false);

  for (int ireg = 0; ireg < 32; ireg++) {
    AlpideConfig::WritePixRegSingle(chip, Alpide::PIXREG_MASK, false, myRow, ireg * 32 + myCol);
    AlpideConfig::WritePixRegSingle(chip, Alpide::PIXREG_SELECT, true, myRow, ireg * 32 + myCol);
  }
  return 0;
}

int configureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  configureFromu(chip);
  configureMask(chip);

  chip->WriteRegister(Alpide::REG_MODECONTROL, 0x21); // strobed readout mode

  AlpideConfig::ConfigureCMU(chip);

  return 0;
}

void scan(const char *fName)
{
  unsigned char         buffer[1024 * 4000];
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
  int                   nSkipped = 0, prioErrors = 0;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  // for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
  for (int icharge = myChargeStart; icharge < myChargeStop; icharge += myChargeStep) {
    std::cout << "Charge = " << icharge << std::endl;
    fChips.at(0)->WriteRegister(Alpide::REG_VPULSEL, 170 - icharge);

    // for (int idelay = myPulseDelayStart; idelay < myPulseDelayStop; idelay ++) {
    for (int idelay = myPulseDelayStart; idelay < myPulseDelayStop; idelay += myPulseDelayStep) {
      // std::cout << "    Delay = " << idelay << std::endl;
      fBoards.at(0)->SetTriggerConfig(true, false, myStrobeDelay, idelay);
      fBoards.at(0)->Trigger(myNTriggers);

      int itrg   = 0;
      int trials = 0;
      while (itrg < myNTriggers) {
        if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) <=
            0) {       // no event available in buffer yet, wait a bit
          usleep(1000); // Increment from 100us
          trials++;
          if (trials == 10) {
            std::cout << "Reached 10 timeouts, giving up on this event" << std::endl;
            itrg = myNTriggers;
            nSkipped++;
            trials = 0;
          }
          continue;
        }
        else {
          // decode DAQboard event
          BoardDecoder::DecodeEvent(boardDAQ, buffer, n_bytes_data, n_bytes_header, n_bytes_trailer,
                                    boardInfo);
          // decode Chip event
          int n_bytes_chipevent = n_bytes_data - n_bytes_header - n_bytes_trailer;
          AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits, 0,
                                     boardInfo.channel, prioErrors,
                                     fConfig->GetScanConfig()->GetParamValue("MAXHITS"));

          itrg++;
        }
      }
      if ((idelay == myPulseDelayStart) && (icharge == myChargeStart)) {
        WriteDataToFile(fName, Hits, icharge, idelay, true);
        // std::cout << "Wrote data to new file" << std::endl;
      }
      else {
        WriteDataToFile(fName, Hits, icharge, idelay, false);
        // std::cout << "Wrote data to file" << std::endl;
      }
    }
  }
}

int main(int argc, char **argv)
{

  decodeCommandParameters(argc, argv);

  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  char Suffix[80], fName[200];

  time_t     t   = time(0); // get time now
  struct tm *now = localtime(&t);
  sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday,
          now->tm_hour, now->tm_min, now->tm_sec);
  sprintf(fName, "Data/PulselengthScan_%s.dat", Suffix);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(fBoards.at(0));

  if (fBoards.size() == 1) {

    fBoards.at(0)->SendOpCode(Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode(Alpide::OPCODE_PRST);

    for (unsigned int i = 0; i < fChips.size(); i++) {
      configureChip(fChips.at(i));
    }

    fBoards.at(0)->SendOpCode(Alpide::OPCODE_RORST);

    // put your test here...
    fBoards.at(0)->SetTriggerSource(trigExt);

    scan(fName);

    sprintf(fName, "Data/ScanConfig_%s.cfg", Suffix);
    WriteScanConfig(fName, fChips.at(0), myDAQBoard);

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
