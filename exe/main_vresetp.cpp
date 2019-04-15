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

static TConfig *                    config;
static std::vector<TReadoutBoard *> fBoards;
static TBoardType                   boardType;
static std::vector<TAlpide *>       fChips;

static unsigned int mySampleDist       = 25;
static unsigned int mySampleRepetition = 1;

int configureChip(TAlpide *chip)
{
  // put all chip configurations before the start of the test here
  chip->WriteRegister(Alpide::REG_MODECONTROL, 0x20);
  chip->WriteRegister(Alpide::REG_CMUDMU_CONFIG, 0x60);
  AlpideConfig::ConfigureCMU(chip);
  return 0;
}

void SetDACMon(TAlpide *chip, Alpide::TRegister vdac, Alpide::TRegister idac, int IRef = 2)
{
  int      VDAC, IDAC;
  uint16_t Value;

  switch (vdac) {
  case Alpide::REG_VRESETP:
    VDAC = 4;
    break;
  case Alpide::REG_VRESETD:
    VDAC = 5;
    break;
  case Alpide::REG_VCASP:
    VDAC = 1;
    break;
  case Alpide::REG_VCASN:
    VDAC = 0;
    break;
  case Alpide::REG_VPULSEH:
    VDAC = 2;
    break;
  case Alpide::REG_VPULSEL:
    VDAC = 3;
    break;
  case Alpide::REG_VCASN2:
    VDAC = 6;
    break;
  case Alpide::REG_VCLIP:
    VDAC = 7;
    break;
  case Alpide::REG_VTEMP:
    VDAC = 8;
    break;
  case Alpide::REG_ADC_DAC_INPUT:
    VDAC = 9;
    break;
  default:
    VDAC = 0;
  }

  switch (idac) {
  case Alpide::REG_IAUX2:
    IDAC = 1;
    break;
  case Alpide::REG_IRESET:
    IDAC = 0;
    break;
  case Alpide::REG_IDB:
    IDAC = 3;
    break;
  case Alpide::REG_IBIAS:
    IDAC = 2;
    break;
  case Alpide::REG_ITHR:
    IDAC = 5;
    break;
  default:
    IDAC = 0;
    break;
  }

  Value = VDAC & 0xf;
  Value |= (IDAC & 0x7) << 4;
  Value |= (IRef & 0x3) << 9;

  chip->WriteRegister(Alpide::REG_ANALOGMON, Value);
  printf("configuring monitoring outputs: 0x%04x\n", Value);
}

bool isVoltageDac(Alpide::TRegister dac) { return (dac < Alpide::REG_VTEMP); }

void scanDac(TAlpide *chip, Alpide::TRegister dac_scn, Alpide::TRegister dac_mon, std::string name,
             unsigned int step = 1, unsigned int rep = 1, std::string suffix = "")
{
  std::cout << "ChipID = " << chip->GetConfig()->GetChipId() << "    Scan " << name << std::endl;

  uint16_t dac_old;
  chip->ReadRegister(dac_scn, dac_old);

  TReadoutBoardDAQ *daqBoard = dynamic_cast<TReadoutBoardDAQ *>(chip->GetReadoutBoard());
  if (daqBoard) {
    SetDACMon(chip, dac_mon, dac_mon);
    usleep(10000);
  }

  float first = -1.;
  float last  = -1.;
  for (unsigned int value = 0; value < 256; value += step) {
    for (unsigned int repetition = 0; repetition < rep; ++repetition) {
      chip->WriteRegister(dac_scn, value);
      float adc_scn =
          isVoltageDac(dac_scn) ? chip->ReadDACVoltage(dac_scn) : chip->ReadDACCurrent(dac_scn);
      float adc_mon =
          isVoltageDac(dac_mon) ? chip->ReadDACVoltage(dac_mon) : chip->ReadDACCurrent(dac_mon);
      float adc_ext =
          daqBoard ? isVoltageDac(dac_mon) ? daqBoard->ReadMonV() : daqBoard->ReadMonI() : 0.;
      if (first < 0.) first = adc_mon;
      last = adc_mon;
      printf("%d %.3f %.3f %.3f\n", value, adc_scn, adc_mon, adc_ext);
    }
  }
  if (std::abs(last - first) > 0.005) printf("WARNING\n");
  printf("last - first = %.3f\n", last - first);

  chip->WriteRegister(dac_scn, dac_old);
}

int main(int argc, char **argv)
{
  decodeCommandParameters(argc, argv);
  initSetup(config, &fBoards, &boardType, &fChips);

  if (!fBoards.empty()) {

    for (auto rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_GRST);
      rBoard->SendOpCode(Alpide::OPCODE_PRST);
    }

    for (auto rChip : fChips) {
      if (!rChip->GetConfig()->IsEnabled()) continue;
      configureChip(rChip);
    }

    for (auto rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_RORST);
    }

    for (auto chip : fChips) {
      if (!chip->GetConfig()->IsEnabled()) continue;

      chip->WriteRegister(Alpide::REG_BYPASS_BUFFER, 0x08);
      usleep(2000);

      scanDac(chip, Alpide::REG_VRESETP, Alpide::REG_VCLIP, "VRESETP_VCLIP", mySampleDist,
              mySampleRepetition);

      scanDac(chip, Alpide::REG_VCLIP, Alpide::REG_VRESETP, "VCLIP_VRESETP", mySampleDist,
              mySampleRepetition);

      scanDac(chip, Alpide::REG_VRESETP, Alpide::REG_ITHR, "VRESETP_ITHR", mySampleDist,
              mySampleRepetition);

      scanDac(chip, Alpide::REG_VRESETP, Alpide::REG_IBIAS, "VRESETP_IBIAS", mySampleDist,
              mySampleRepetition);
    }

    for (auto rBoard : fBoards) {
      TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(rBoard);
      if (myDAQBoard) {
        myDAQBoard->PowerOff();
      }
    }
  }

  return 0;
}
