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
#include "scope_control.h"
#include <unistd.h>

TBoardType                   fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *>       fChips;
TConfig *                    fConfig;

bool Verbose = false;
int  fErrCount0;
int  fErrCount5;
int  fErrCountf;

int fEnabled = 0; // variable to count number of enabled chips; leave at 0

int fTotalErr;

int configureChip(TAlpide *chip)
{
  // put all chip configurations before the start of the test here
  chip->WriteRegister(Alpide::REG_MODECONTROL, 0x20);
  if (fConfig->GetDeviceType() == TYPE_CHIP) chip->WriteRegister(Alpide::REG_CMUDMU_CONFIG, 0x60);

  return 0;
}

void WriteMem(TAlpide *chip, int ARegion, int AOffset, int AValue)
{
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "WriteMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;

  uint16_t LowVal  = AValue & 0xffff;
  uint16_t HighVal = (AValue >> 16) & 0xff;

  int err           = chip->WriteRegister(LowAdd, LowVal);
  if (err >= 0) err = chip->WriteRegister(HighAdd, HighVal);

  if (err < 0) {
    std::cout << "Cannot write chip register. Exiting ... " << std::endl;
    exit(1);
  }
}

void ReadMem(TAlpide *chip, int ARegion, int AOffset, int &AValue)
{
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "ReadMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;

  uint16_t LowVal, HighVal;

  int err           = chip->ReadRegister(LowAdd, LowVal);
  if (err >= 0) err = chip->ReadRegister(HighAdd, HighVal);

  if (err < 0) {
    std::cout << "Cannot read chip register. Exiting ... " << std::endl;
    exit(1);
  }

  // Note to self: if you want to shorten the following lines,
  // remember that HighVal is 16 bit and (HighVal << 16) will yield 0
  // :-)
  AValue = (HighVal & 0xff);
  AValue <<= 16;
  AValue |= LowVal;
}

bool MemReadback(TAlpide *chip, int ARegion, int AOffset, int AValue)
{
  int Value;

  WriteMem(chip, ARegion, AOffset, AValue);
  ReadMem(chip, ARegion, AOffset, Value);

  if (Value != AValue) {
    if (Verbose) {
      std::cout << "Error in mem " << ARegion << "/0x" << std::hex << AOffset << ": wrote "
                << AValue << ", read: " << Value << std::dec << std::endl;
    }
    return false;
  }
  return true;
}

void MemTest(TAlpide *chip, int ARegion, int AOffset)
{
  if (!MemReadback(chip, ARegion, AOffset, 0x0)) fErrCount0++;
  if (!MemReadback(chip, ARegion, AOffset, 0x555555)) fErrCount5++;
  if (!MemReadback(chip, ARegion, AOffset, 0xffffff)) fErrCountf++;
}

int main(int argc, char **argv)
{

  decodeCommandParameters(argc, argv);

  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

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

    fTotalErr = 0;

    scope_control scope;
    // scope.debug_en = true;
    if (!scope.open_auto()) {
      std::cout << "Scope not detected" << std::endl;
      return 1;
    }
    scope.get_errors();

    for (int i = 1; i <= 4; i++) {
      scope.enable_ch(i);
      scope.set_vscale_ch(i, 200e-3);
      scope.set_dc_coupling_ch(i, false);
    }
    scope.set_timescale(5e-9);

    scope.set_trigger_ext();
    scope.set_trigger_slope_rising(false);
    scope.set_trigger_position(1.1e-6);
    scope.set_ext_trigger_level(-0.5);
    scope.setup_measure();

    for (const auto &rChip : fChips) {
      if (!rChip->GetConfig()->IsEnabled()) continue;
      if (rChip->GetConfig()->GetCtrInt() != 1) continue;
      std::cout << std::endl
                << "Doing scope test on ControlInterface " << rChip->GetConfig()->GetCtrInt()
                << "  chip ID " << rChip->GetConfig()->GetChipId() << std::endl;

      scope.single_capture();
      usleep(100000);
      MemTest(rChip, 0, 0);
      scope.wait_for_trigger();
      scope.en_measure_ch(1);
      scope.get_meas();
      scope.en_measure_ch(2);
      scope.get_meas();
      break;
    }

    for (const auto &rChip : fChips) {
      if (!rChip->GetConfig()->IsEnabled()) continue;
      if (rChip->GetConfig()->GetCtrInt() != 0) continue;
      std::cout << std::endl
                << "Doing scope test on ControlInterface " << rChip->GetConfig()->GetCtrInt()
                << "  chip ID " << rChip->GetConfig()->GetChipId() << std::endl;

      scope.single_capture();
      usleep(100000);
      MemTest(rChip, 0, 0);
      scope.wait_for_trigger();
      scope.en_measure_ch(3);
      scope.get_meas();
      scope.en_measure_ch(4);
      scope.get_meas();
      break;
    }

    printf("CH1 Peak-to-peak : %e Amplitude : %e Risetime : %e Falltime : %e\n", scope.ch1.peak,
           scope.ch1.amp, scope.ch1.rtim, scope.ch1.ftim);
    printf("CH2 Peak-to-peak : %e Amplitude : %e Risetime : %e Falltime : %e\n", scope.ch2.peak,
           scope.ch2.amp, scope.ch2.rtim, scope.ch2.ftim);
    printf("CH3 Peak-to-peak : %e Amplitude : %e Risetime : %e Falltime : %e\n", scope.ch3.peak,
           scope.ch3.amp, scope.ch3.rtim, scope.ch3.ftim);
    printf("CH4 Peak-to-peak : %e Amplitude : %e Risetime : %e Falltime : %e\n", scope.ch4.peak,
           scope.ch4.amp, scope.ch4.rtim, scope.ch4.ftim);

    for (const auto &rBoard : fBoards) {
      TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(rBoard);
      if (myDAQBoard) {
        myDAQBoard->PowerOff();
        delete myDAQBoard;
      }
    }
  }

  return 0;
}
