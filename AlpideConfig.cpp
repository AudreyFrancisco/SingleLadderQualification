#include "AlpideConfig.h"
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
