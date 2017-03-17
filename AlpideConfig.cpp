#include "AlpideConfig.h"
#include "SetupHelpers.h"
#include <iostream>


void AlpideConfig::Init (TAlpide *chip) {
  
  ClearPixSelectBits (chip, true);
  
}


// clear all column and row select bits
// if clearPulseGating is set also the pulse gating registers will be reset 
// (possibly useful at startup, but not in-between setting of mask patterns)
void AlpideConfig::ClearPixSelectBits (TAlpide *chip, bool clearPulseGating) {
  if (clearPulseGating) 
    chip->WriteRegister(0x48f, 0);
  else 
    chip->WriteRegister(0x487, 0);
}


void AlpideConfig::WritePixConfReg (TAlpide *chip, Alpide::TPixReg reg, bool data) {
  uint16_t pixconfig = (int) reg & 0x1;
  pixconfig         |= (data?1:0) << 1;

  chip->WriteRegister (Alpide::REG_PIXELCONFIG, pixconfig);
}



// This method writes data to the selected pixel register in the whole matrix simultaneously
// To be checked whether this works or whether a loop over rows has to be implemented
void AlpideConfig::WritePixRegAll (TAlpide *chip, Alpide::TPixReg reg, bool data) {
  WritePixConfReg (chip, reg, data);

  // set all colsel and all rowsel to 1
  chip->WriteRegister (0x487,0xffff);

  ClearPixSelectBits (chip, false);
}


// Writes data to complete row. This assumes that select bits have been cleared before
void AlpideConfig::WritePixRegRow (TAlpide *chip, Alpide::TPixReg reg, bool data, int row) {
  WritePixConfReg (chip, reg, data);

  // set all colsel to 1 and leave all rowsel at 0
  chip->WriteRegister (0x483,0xffff);

  // for correct region set one rowsel to 1
  int region = row / 16;
  int bit    = row % 16;

  int address = 0x404 | (region << 11);
  int value   = 1 << bit;

  chip->WriteRegister (address, value);

  ClearPixSelectBits (chip, false);
}


void AlpideConfig::WritePixRegSingle (TAlpide *chip, Alpide::TPixReg reg, bool data, int row, int col) {
  WritePixConfReg (chip, reg, data);

  // set correct colsel bit
  int region  = col / 32;            // region that contains the corresponding col select
  int bit     = col % 16;            // bit that has to be written (0 - 15)
  int highlow = (col % 32) / 16;     // decide between col <15:0> (0) and col <31:16> (1)

  int address = 0x400 | (region << 11) | (1 << highlow);
  int value   = 1 << bit;

  chip->WriteRegister (address, value);

  // set correct rowsel bit 
  region = row / 16;
  bit    = row % 16;

  address = 0x404 | (region << 11);
  value   = 1 << bit;

  chip->WriteRegister (address, value);

  ClearPixSelectBits (chip, false);
}


// Alpide 3 settings, to be confirmed
void AlpideConfig::ApplyStandardDACSettings (TAlpide *chip, float backBias) {
  if (backBias == 0) {
    chip->WriteRegister(Alpide::REG_VCASN,    60);
    chip->WriteRegister(Alpide::REG_VCASN2,   62);
    chip->WriteRegister(Alpide::REG_VRESETD, 147);
    chip->WriteRegister(Alpide::REG_IDB,      29);
  }
  else if (backBias == 3) {
    chip->WriteRegister(Alpide::REG_VCASN,   105);
    chip->WriteRegister(Alpide::REG_VCASN2,  117);
    chip->WriteRegister(Alpide::REG_VCLIP,    60);
    chip->WriteRegister(Alpide::REG_VRESETD, 147);
    chip->WriteRegister(Alpide::REG_IDB,      29);
  }
  else if (backBias == 6) {
    chip->WriteRegister(Alpide::REG_VCASN,   135);
    chip->WriteRegister(Alpide::REG_VCASN2,  147);
    chip->WriteRegister(Alpide::REG_VCLIP,   100);
    chip->WriteRegister(Alpide::REG_VRESETD, 170);
    chip->WriteRegister(Alpide::REG_IDB,      29);
  }
  else {
    std::cout << "Settings not defined for back bias " << backBias << " V. Please set manually." << std::endl;
  }

}


// should it be allowed to pass a config or should always the chip config be used?
void AlpideConfig::ConfigureFromu (TAlpide *chip, Alpide::TPulseType pulseType, bool testStrobe, TChipConfig *config ) {
  // for the time being use these hard coded values; if needed move to configuration
  int  mebmask          = 0;
  bool rotatePulseLines = false;
  bool internalStrobe   = false;    // strobe sequencer for continuous mode
  bool busyMonitoring   = true;


  if (!config) config = chip->GetConfig();

  uint16_t fromuconfig = 0;
  
  fromuconfig |= mebmask;
  fromuconfig |= (internalStrobe   ? 1:0)          << 3;
  fromuconfig |= (busyMonitoring   ? 1:0)          << 4;
  fromuconfig |= ((int) pulseType)                 << 5;
  fromuconfig |= (testStrobe       ? 1:0)          << 6;
  fromuconfig |= (rotatePulseLines ? 1:0)          << 7;
  fromuconfig |= (config->GetTriggerDelay() & 0x7) << 8;
  
  chip->WriteRegister (Alpide::REG_FROMU_CONFIG1,  fromuconfig);
  chip->WriteRegister (Alpide::REG_FROMU_CONFIG2,  config->GetStrobeDuration());
  chip->WriteRegister (Alpide::REG_FROMU_PULSING1, config->GetStrobeDelay());
  chip->WriteRegister (Alpide::REG_FROMU_PULSING2, config->GetPulseDuration());
  
}


void AlpideConfig::ConfigureBuffers (TAlpide *chip, TChipConfig *config) {
  if (!config) config = chip->GetConfig();
  
  uint16_t clocks = 0, ctrl = 0;

  clocks |= (config->GetDclkReceiver () & 0xf);
  clocks |= (config->GetDclkDriver   () & 0xf) << 4;
  clocks |= (config->GetMclkReceiver () & 0xf) << 8;
  
  ctrl   |= (config->GetDctrlReceiver() & 0xf);
  ctrl   |= (config->GetDctrlDriver  () & 0xf) << 4;

  chip->WriteRegister (Alpide::REG_CLKIO_DACS, clocks);
  chip->WriteRegister (Alpide::REG_CMUIO_DACS, ctrl);
}


void AlpideConfig::ConfigureCMU (TAlpide *chip, TChipConfig *config) {
  if (!config) config = chip->GetConfig();

  uint16_t cmuconfig = 0;

  cmuconfig |= (config->GetPreviousId() & 0xf);
  cmuconfig |= (config->GetInitialToken     () ? 1:0) << 4;
  cmuconfig |= (config->GetDisableManchester() ? 1:0) << 5;
  cmuconfig |= (config->GetEnableDdr        () ? 1:0) << 6;
  
  chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, cmuconfig);
}


// return value: active row (needed for threshold scan histogramming)
int AlpideConfig::ConfigureMaskStage (TAlpide *chip, int nPix, int iStage) {
  // check that nPix is one of (1, 2, 4, 8, 16, 32)
  if ((nPix <= 0) || (nPix & (nPix - 1)) || (nPix > 32)) {      
    std::cout << "Warning: bad number of pixels for mask stage (" << nPix << ", using 1 instead" << std::endl;
    nPix = 1;
  }
  WritePixRegAll (chip, Alpide::PIXREG_MASK,   true);
  WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);

  // complete row
  if (nPix == 32) {
    WritePixRegRow(chip, Alpide::PIXREG_MASK,   false, iStage);
    WritePixRegRow(chip, Alpide::PIXREG_SELECT, true, iStage);
    return iStage;
  }
  else {
    int colStep = 32 / nPix;
    for (int icol = 0; icol < 1024; icol += colStep) {
      WritePixRegSingle (chip, Alpide::PIXREG_MASK,   false, iStage % 512, icol + iStage / 512);
      WritePixRegSingle (chip, Alpide::PIXREG_SELECT, true,  iStage % 512, icol + iStage / 512);   
    }
    return (iStage % 512);
  }
}


void AlpideConfig::WriteControlReg (TAlpide *chip, Alpide::TChipMode chipMode, TChipConfig *config) {
  if (!config) config = chip->GetConfig();

  uint16_t controlreg = 0;
  
  controlreg |= (uint16_t) chipMode;
  
  controlreg |= (config->GetEnableClustering    () ? 1:0) << 2;
  controlreg |= (config->GetMatrixReadoutSpeed  () & 0x1) << 3;
  controlreg |= (config->GetSerialLinkSpeed     () & 0x3) << 4;
  controlreg |= (config->GetEnableSkewingGlobal () ? 1:0) << 6;
  controlreg |= (config->GetEnableSkewingStartRO() ? 1:0) << 7;
  controlreg |= (config->GetEnableClockGating   () ? 1:0) << 8;
  controlreg |= (config->GetEnableCMUReadout    () ? 1:0) << 9;
  
  chip->WriteRegister (Alpide::REG_MODECONTROL, controlreg);
  
}


void AlpideConfig::BaseConfigPLL (TAlpide *chip) 
{
  TChipConfig *config = chip->GetConfig();
  if (config->GetParamValue("LINKSPEED") == -1) return; // high-speed link deactivated


  uint16_t Phase      = config->GetParamValue("PLLPHASE");  // 4bit Value, default 8
  uint16_t Stages     = config->GetParamValue("PLLSTAGES"); // 0 = 3 stages, 1 = 4,  3 = 5 (typical 4)
  uint16_t ChargePump = config->GetParamValue("CHARGEPUMP");
  uint16_t Driver     = config->GetParamValue("DTUDRIVER");
  uint16_t Preemp     = config->GetParamValue("DTUPREEMP");
  uint16_t Value;

  Value = (Stages & 0x3) | 0x4 | 0x8 | ((Phase & 0xf) << 4);   // 0x4: narrow bandwidth, 0x8: PLL off

  chip->WriteRegister (Alpide::REG_DTU_CONFIG, Value);

  Value = (ChargePump & 0xf) | ((Driver & 0xf) << 4) | ((Preemp & 0xf) << 8);

  chip->WriteRegister (Alpide::REG_DTU_DACS, Value);

  // Clear PLL off signal
  Value = (Stages & 0x3) | 0x4 | ((Phase & 0xf) << 4);   // 0x4: narrow bandwidth, 0x8: PLL off
  chip->WriteRegister (Alpide::REG_DTU_CONFIG, Value);
  // Force PLL reset
  Value = (Stages & 0x3) | 0x4 | 0x100 |((Phase & 0xf) << 4);   // 0x4: narrow bandwidth, 0x100: Reset
  chip->WriteRegister (Alpide::REG_DTU_CONFIG, Value);
  Value = (Stages & 0x3) | 0x4 |((Phase & 0xf) << 4);           // Reset off
  chip->WriteRegister (Alpide::REG_DTU_CONFIG, Value);


}


void AlpideConfig::BaseConfigMask (TAlpide *chip)
{
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   true);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);
}


void AlpideConfig::BaseConfigFromu (TAlpide *chip)
{
}


void AlpideConfig::BaseConfigDACs (TAlpide *chip)
{
  chip->WriteRegister (Alpide::REG_VPULSEH, chip->GetConfig()->GetParamValue("VPULSEH"));
  chip->WriteRegister (Alpide::REG_VPULSEL, chip->GetConfig()->GetParamValue("VPULSEL"));
  chip->WriteRegister (Alpide::REG_VRESETD, chip->GetConfig()->GetParamValue("VRESETD"));
  chip->WriteRegister (Alpide::REG_VCASN,   chip->GetConfig()->GetParamValue("VCASN"));
  chip->WriteRegister (Alpide::REG_VCASN2,  chip->GetConfig()->GetParamValue("VCASN2"));
  chip->WriteRegister (Alpide::REG_VCLIP,   chip->GetConfig()->GetParamValue("VCLIP"));
  chip->WriteRegister (Alpide::REG_ITHR,    chip->GetConfig()->GetParamValue("ITHR"));
  chip->WriteRegister (Alpide::REG_IDB,     chip->GetConfig()->GetParamValue("IDB"));
  chip->WriteRegister (Alpide::REG_IBIAS,   chip->GetConfig()->GetParamValue("IBIAS"));
  chip->WriteRegister (Alpide::REG_VCASP,   chip->GetConfig()->GetParamValue("VCASP"));
  // not used DACs..
  chip->WriteRegister (Alpide::REG_VTEMP,   chip->GetConfig()->GetParamValue("VTEMP"));
  chip->WriteRegister (Alpide::REG_VRESETP, chip->GetConfig()->GetParamValue("VRESETP"));
  chip->WriteRegister (Alpide::REG_IRESET,  chip->GetConfig()->GetParamValue("IRESET"));
  chip->WriteRegister (Alpide::REG_IAUX2,   chip->GetConfig()->GetParamValue("IAUX2"));
}


void AlpideConfig::BaseConfig (TAlpide *chip)
{
  // put all chip configurations before the start of the test here

  chip->WriteRegister (Alpide::REG_MODECONTROL, 0x20); // set chip to config mode
  //TODO: use chip config here, the config should be written accordingly at this point!


  // CMU/DMU config: turn manchester encoding off or on etc, initial token=1, disable DDR
  int cmudmu_config = 0x10 | ((chip->GetConfig()->GetDisableManchester()) ? 0x20 : 0x00);

  BaseConfigFromu(chip);
  BaseConfigDACs (chip);
  BaseConfigMask (chip);
  BaseConfigPLL  (chip);

  uint16_t value;

  switch (chip->GetConfig()->GetParamValue("LINKSPEED")) {
  case -1: // DTU not activated
	value = 0x21;
	break;
  case 400: 
    value = 0x01;
    break;
  case 600: 
    value = 0x11;
    break;
  case 1200: 
    value = 0x21;
    break;
  default: 
    std::cout << "Warning: invalid link speed, using 1200" << std::endl;
    value = 0x21;
    break;
  }

  chip->WriteRegister (Alpide::REG_MODECONTROL, value); // strobed readout mode
}


void AlpideConfig::PrintDebugStream (TAlpide *chip) 
{
  uint16_t Value;

  std::cout << "Debug Stream chip id " << chip->GetConfig()->GetChipId() << ": " << std::endl;

  for (int i = 0; i < 2; i++) {
    chip->ReadRegister(Alpide::REG_BMU_DEBUG, Value);
    std::cout << "  BMU Debug reg word " << i << ": " << std::hex << Value << std::dec << std::endl;  
  }
  for (int i = 0; i < 4; i++) {
    chip->ReadRegister(Alpide::REG_DMU_DEBUG, Value);
    std::cout << "  DMU Debug reg word " << i << ": " << std::hex << Value << std::dec << std::endl;  
  }
  for (int i = 0; i < 9; i++) {
    chip->ReadRegister(Alpide::REG_FROMU_DEBUG, Value);
    std::cout << "  FROMU Debug reg word " << i << ": " << std::hex << Value << std::dec << std::endl;  
  }


}
