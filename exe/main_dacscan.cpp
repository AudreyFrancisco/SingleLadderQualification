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



TConfig* config;
std::vector <TReadoutBoard *> fBoards;
TBoardType boardType;
std::vector <TAlpide *> fChips;
TReadoutBoardDAQ *myDAQBoard;

int mySampleDist = 1;


int configureChip(TAlpide *chip) {
  // put all chip configurations before the start of the test here
  chip->WriteRegister (Alpide::REG_MODECONTROL,   0x20);
  chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x60);
  return 0;
}


// this is ugly, but there is no simple relation between the DAC addresses and the
// corresponding value in the monitoring register -> TODO: define map
void SetDACMon (TAlpide *chip, Alpide::TRegister ADac, int IRef = 2) {
  int VDAC, IDAC;
  uint16_t Value;
  switch (ADac) {
  case Alpide::REG_VRESETP:
    VDAC = 4;
    IDAC = 0;
    break;
  case Alpide::REG_VRESETD:
    VDAC = 5;
    IDAC = 0;
    break;
  case Alpide::REG_VCASP:
    VDAC = 1;
    IDAC = 0;
    break;
  case Alpide::REG_VCASN:
    VDAC = 0;
    IDAC = 0;
    break;
  case Alpide::REG_VPULSEH:
    VDAC = 2;
    IDAC = 0;
    break;
  case Alpide::REG_VPULSEL:
    VDAC = 3;
    IDAC = 0;
    break;
  case Alpide::REG_VCASN2:
    VDAC = 6;
    IDAC = 0;
    break;
  case Alpide::REG_VCLIP:
    VDAC = 7;
    IDAC = 0;
    break;
  case Alpide::REG_VTEMP:
    VDAC = 8;
    IDAC = 0;
    break;
  case Alpide::REG_IAUX2:
    IDAC = 1;
    VDAC = 0;
    break;
  case Alpide::REG_IRESET:
    IDAC = 0;
    VDAC = 0;
    break;
  case Alpide::REG_IDB:
    IDAC = 3;
    VDAC = 0;
    break;
  case Alpide::REG_IBIAS:
    IDAC = 2;
    VDAC = 0;
    break;
  case Alpide::REG_ITHR:
    IDAC = 5;
    VDAC = 0;
    break;
  default:
    VDAC = 0;
    IDAC = 0;
    break;
  }


  Value = VDAC & 0xf;
  Value |= (IDAC & 0x7) << 4;
  Value |= (IRef & 0x3) << 9;

  chip->WriteRegister (Alpide::REG_ANALOGMON, Value);

}


void scanCurrentDac(TAlpide *chip, Alpide::TRegister ADac, const char *Name, int sampleDist = 1, string suffix = "") {
  char     fName[50];
  float    Current;
  uint16_t old;

  sprintf (fName, "Data/IDAC_%s_Chip%d_%d_%s.dat", Name, chip->GetConfig()->GetChipId(), chip->GetConfig()->GetCtrInt(), suffix.c_str() );
  FILE *fp = fopen (fName, "w");

  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  std::cout << "ChipID = " << chip->GetConfig()->GetChipId() << "    Scanning DAC " << Name << std::endl;

  chip->ReadRegister (ADac, old);
  if (!myDAQBoard) { // MOSAIC board internal ADC read
    for (int i = 0; i < 256; i += sampleDist) {
      chip->WriteRegister (ADac, i);
      Current = chip->ReadDACCurrent(ADac);
      fprintf (fp, "%d %.3f\n", i, Current);
    }
  } else { // DAQ board : external ADC read
    SetDACMon (chip, ADac);
    usleep(100000);
    for (int i = 0; i < 256; i += sampleDist) {
      chip->WriteRegister (ADac, i);
      Current = myDAQBoard->ReadMonI();
      fprintf (fp, "%d %.3f\n", i, Current);
    }
  }
  chip->WriteRegister (ADac, old);
  fclose (fp);
}


void scanVoltageDac(TAlpide *chip, Alpide::TRegister ADac, const char *Name, int sampleDist = 1, string suffix = "") {
  char     fName[50];
  float    Voltage;
  uint16_t old;
  sprintf (fName, "Data/VDAC_%s_Chip%d_%d_%s.dat", Name, chip->GetConfig()->GetChipId(), chip->GetConfig()->GetCtrInt(), suffix.c_str());
  FILE *fp = fopen (fName, "w");

  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  std::cout << "ChipID = " << chip->GetConfig()->GetChipId() << "    Scanning DAC " << Name << std::endl;

  chip->ReadRegister (ADac, old);
  if (!myDAQBoard) { // MOSAIC board internal ADC read
    for (int i = 0; i < 256; i += sampleDist) {
      chip->WriteRegister (ADac, i);
      Voltage = chip->ReadDACVoltage(ADac);
      fprintf (fp, "%d %.3f\n", i, Voltage);
    }
  } else { // DAQ board : external ADC read
    SetDACMon (chip, ADac);
    usleep(100000);
    for (int i = 0; i < 256; i += sampleDist) {
      chip->WriteRegister (ADac, i);
      Voltage = myDAQBoard->ReadMonV();
      fprintf (fp, "%d %.3f\n", i, Voltage);
    }
  }
  chip->WriteRegister (ADac, old);
  fclose (fp);
}


int main(int argc, char** argv) {
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  char Suffix[13];
  snprintf(Suffix, 13, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  decodeCommandParameters(argc, argv);
  initSetup(config, &fBoards, &boardType, &fChips);

  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  if (fBoards.size() == 1) {

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    if ((myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0)))) {
      for (unsigned int i = 0; i < fChips.size(); i ++) {
        configureChip (fChips.at(i));
      }
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);

    for (unsigned int i = 0; i < fChips.size(); i ++) {

      scanVoltageDac (fChips.at(i), Alpide::REG_VRESETP, "VRESETP", mySampleDist, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VRESETD, "VRESETD", mySampleDist, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VCASP,   "VCASP",   mySampleDist, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VCASN,   "VCASN",   mySampleDist, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VPULSEH, "VPULSEH", mySampleDist, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VPULSEL, "VPULSEL", mySampleDist, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VCASN2,  "VCASN2",  mySampleDist, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VCLIP,   "VCLIP",   mySampleDist, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VTEMP,   "VTEMP",   mySampleDist, Suffix);

      scanCurrentDac (fChips.at(i), Alpide::REG_IAUX2,   "IAUX2",   mySampleDist, Suffix);
      scanCurrentDac (fChips.at(i), Alpide::REG_IRESET,  "IRESET",  mySampleDist, Suffix);
      scanCurrentDac (fChips.at(i), Alpide::REG_IDB,     "IDB",     mySampleDist, Suffix);
      scanCurrentDac (fChips.at(i), Alpide::REG_IBIAS,   "IBIAS",   mySampleDist, Suffix);
      scanCurrentDac (fChips.at(i), Alpide::REG_ITHR,    "ITHR",    mySampleDist, Suffix);
    }

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
