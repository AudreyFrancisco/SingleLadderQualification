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

unsigned int mySampleDist = 1;
unsigned int mySampleRepetition = 1;


int configureChip(TAlpide *chip) {
  // put all chip configurations before the start of the test here
  chip->WriteRegister (Alpide::REG_MODECONTROL,   0x20);
  chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x60);
  AlpideConfig::ConfigureCMU (chip);
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
  case Alpide::REG_ADC_DAC_INPUT:
    VDAC = 9;
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


void scanCurrentDac(TAlpide *chip, Alpide::TRegister ADac, const char *Name, unsigned int sampleDist = 1, unsigned int sampleRepetition = 1, string suffix = "") {
  char     fName[50];
  float    Current;
  uint16_t old;

  sprintf (fName, "Data/IDAC_%s_Chip%d_%d_%s.dat", Name, chip->GetConfig()->GetChipId(), chip->GetConfig()->GetCtrInt(), suffix.c_str() );
  FILE *fp = fopen (fName, "w");

  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  std::cout << "ChipID = " << chip->GetConfig()->GetChipId() << "    Scanning DAC " << Name << std::endl;

  chip->ReadRegister (ADac, old);
  if (!myDAQBoard) { // MOSAIC board internal ADC read
    for (unsigned int value = 0; value < 256; value += sampleDist) {
      for (unsigned int repetition = 0; repetition < sampleRepetition; ++repetition) {
        chip->WriteRegister (ADac, value);
        Current = chip->ReadDACCurrent(ADac);
        fprintf (fp, "%d %.3f\n", value, Current);
      }
    }
  } else { // DAQ board : external ADC read
    SetDACMon (chip, ADac);
    usleep(100000);
    for (unsigned int value = 0; value < 256; value += sampleDist) {
      for (unsigned int repetition = 0; repetition < sampleRepetition; ++repetition) {
        chip->WriteRegister (ADac, value);
        Current = myDAQBoard->ReadMonI();
        fprintf (fp, "%d %.3f\n", value, Current);
      }
    }
  }
  chip->WriteRegister (ADac, old);
  fclose (fp);
}


void scanVoltageDac(TAlpide *chip, Alpide::TRegister ADac, const char *Name, unsigned int sampleDist = 1, unsigned int sampleRepetition = 1, string suffix = "") {
  char     fName[50];
  float    Voltage;
  uint16_t old;
  sprintf (fName, "Data/VDAC_%s_Chip%d_%d_%s.dat", Name, chip->GetConfig()->GetChipId(), chip->GetConfig()->GetCtrInt(), suffix.c_str());
  FILE *fp = fopen (fName, "w");

  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  std::cout << "ChipID = " << chip->GetConfig()->GetChipId() << "    Scanning DAC " << Name << std::endl;

  chip->ReadRegister (ADac, old);
  if (!myDAQBoard) { // MOSAIC board internal ADC read
    for (unsigned int value = 0; value < 256; value += sampleDist) {
      for (unsigned int repetition = 0; repetition < sampleRepetition; ++repetition) {
        chip->WriteRegister (ADac, value);
        Voltage = chip->ReadDACVoltage(ADac);
        fprintf (fp, "%d %.3f\n", value, Voltage);
      }
    }
  } else { // DAQ board : external ADC read
    SetDACMon (chip, ADac);
    usleep(100000);
    for (unsigned int value = 0; value < 256; value += sampleDist) {
      for (unsigned int repetition = 0; repetition < sampleRepetition; ++repetition) {
        chip->WriteRegister (ADac, value);
        Voltage = myDAQBoard->ReadMonV();
        fprintf (fp, "%d %.3f\n", value, Voltage);
      }
    }
  }
  chip->WriteRegister (ADac, old);
  fclose (fp);
}

void scanADCDac(TAlpide *chip, unsigned int sampleDist = 1, unsigned int sampleRepetition = 1, string suffix = "") {
  Alpide::TRegister ADac = Alpide::REG_ADC_DAC_INPUT;
  char     fName[50];
  float    Voltage;
  uint16_t old;
  sprintf (fName, "Data/ADC_DAC_Chip%d_%d_%s.dat", chip->GetConfig()->GetChipId(), chip->GetConfig()->GetCtrInt(), suffix.c_str());
  FILE *fp = fopen (fName, "w");

  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  std::cout << "ChipID = " << chip->GetConfig()->GetChipId() << "    Scanning ADC DAC" << std::endl;

  chip->ReadRegister (ADac, old);
  if (!myDAQBoard) { // MOSAIC board internal ADC read
    std::cerr << "The ADC DAC can only be measured using the DAQ Board!" << std::endl;
    fputs("# The ADC DAC can only be measured using the DAQ Board!",fp );
  } else { // DAQ board : external ADC read
    SetDACMon (chip, ADac);
    chip->SetTheADCCtrlRegister(Alpide::MODE_SUPERMANUAL, Alpide::INP_AVSS, Alpide::COMP_296uA, Alpide::RAMP_1us);

    usleep(100000);
    for (unsigned int value = 0; value < 256; value += sampleDist) {
      for (unsigned int repetition = 0; repetition < sampleRepetition; ++repetition) {
        chip->WriteRegister (ADac, value);
        Voltage = myDAQBoard->ReadMonV();
        fprintf (fp, "%d %.3f\n", value, Voltage);
      }
    }
  }
  chip->WriteRegister (ADac, old);
  fclose (fp);
}



int main(int argc, char** argv) {
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  char Suffix[14];
  snprintf(Suffix, 14, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

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

      scanVoltageDac (fChips.at(i), Alpide::REG_VRESETP, "VRESETP", mySampleDist, mySampleRepetition, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VRESETD, "VRESETD", mySampleDist, mySampleRepetition, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VCASP,   "VCASP",   mySampleDist, mySampleRepetition, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VCASN,   "VCASN",   mySampleDist, mySampleRepetition, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VPULSEH, "VPULSEH", mySampleDist, mySampleRepetition, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VPULSEL, "VPULSEL", mySampleDist, mySampleRepetition, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VCASN2,  "VCASN2",  mySampleDist, mySampleRepetition, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VCLIP,   "VCLIP",   mySampleDist, mySampleRepetition, Suffix);
      scanVoltageDac (fChips.at(i), Alpide::REG_VTEMP,   "VTEMP",   mySampleDist, mySampleRepetition, Suffix);

      scanCurrentDac (fChips.at(i), Alpide::REG_IAUX2,   "IAUX2",   mySampleDist, mySampleRepetition, Suffix);
      scanCurrentDac (fChips.at(i), Alpide::REG_IRESET,  "IRESET",  mySampleDist, mySampleRepetition, Suffix);
      scanCurrentDac (fChips.at(i), Alpide::REG_IDB,     "IDB",     mySampleDist, mySampleRepetition, Suffix);
      scanCurrentDac (fChips.at(i), Alpide::REG_IBIAS,   "IBIAS",   mySampleDist, mySampleRepetition, Suffix);
      scanCurrentDac (fChips.at(i), Alpide::REG_ITHR,    "ITHR",    mySampleDist, mySampleRepetition, Suffix);

      scanADCDac (fChips.at(i), mySampleDist, mySampleRepetition, Suffix);

      if (!(myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0)))) { // measurement OP code let's DAQ board get stuck
        // AVDD
        char     fName[50];
        snprintf (fName, 50, "Data/AVDD_Chip%d_%d_%s.dat", fChips.at(i)->GetConfig()->GetChipId(), fChips.at(i)->GetConfig()->GetCtrInt(), Suffix);
        FILE *fp = fopen (fName, "w");

        uint16_t theResult = 0;
        float    theValue  = 0.;
        fChips.at(i)->SetTheDacMonitor(Alpide::REG_ANALOGMON);
        usleep(5000);
        fChips.at(i)->SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_AVDD, Alpide::COMP_296uA, Alpide::RAMP_1us);
        usleep(5000);

        for (unsigned int repetition = 0; repetition < mySampleRepetition; ++repetition) {
          fBoards.at(0)->SendCommand ( Alpide::COMMAND_ADCMEASURE, fChips.at(i));
          usleep(5000);
          fChips.at(i)->ReadRegister(Alpide::REG_ADC_AVSS, theResult);
          theValue = 2. * ((float)theResult - (float)(fChips.at(i)->GetADCOffset())) * 0.823e-3; // first approximation
          fprintf (fp, "%d %.3f\n", repetition, theValue);
        }
        fclose (fp);

        // Temperature
        snprintf (fName, 50, "Data/TEMP_Chip%d_%d_%s.dat", fChips.at(i)->GetConfig()->GetChipId(), fChips.at(i)->GetConfig()->GetCtrInt(), Suffix);
        fp = fopen (fName, "w");

        theResult = 0;
        theValue  = 0.;
        fChips.at(i)->SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_Temperature, Alpide::COMP_296uA, Alpide::RAMP_1us);

        for (unsigned int repetition = 0; repetition < mySampleRepetition; ++repetition) {
          fprintf (fp, "%d %.3f\n", repetition, fChips.at(i)->ReadTemperature());
        }
        fclose (fp);
      }
    }

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
