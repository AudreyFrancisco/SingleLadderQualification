#include "TAlpide.h"
#include <iostream>

using namespace Alpide;

TAlpide::TAlpide (TChipConfig *config) {
  fConfig = config;
  fChipId = config->GetChipId();
}


void TAlpide::SetEnable (bool Enable) {
  fReadoutBoard->SetChipEnable (fChipId, Enable);
  fConfig      ->SetEnable     (Enable);
}


int TAlpide::ReadRegister (TRegister address, uint16_t &value) {
  return ReadRegister ((uint16_t) address, value);
}


int TAlpide::ReadRegister (uint16_t address, uint16_t &value) {
  int err = fReadoutBoard->ReadChipRegister(address, value, fChipId);
  if (err < 0) return err;  // readout board should have thrown an exception before

  return err;

}


int TAlpide::WriteRegister (TRegister address, uint16_t value, bool verify) {
  return WriteRegister ((uint16_t) address, value, verify);
}


int TAlpide::WriteRegister (uint16_t address, uint16_t value, bool verify) {
  int result = fReadoutBoard->WriteChipRegister(address, value, fChipId);
  if ((!verify) || (result < 0)) return result;

  uint16_t check;
  result = ReadRegister (address, check);
  if (result < 0) return result;
  if (check != value) return -1;      // raise exception (warning) readback != write value;
  return 0;  
}


int TAlpide::ModifyRegisterBits (TRegister address, uint8_t lowBit, uint8_t nBits, uint16_t value, bool verify) {
  if ((lowBit < 0) || (lowBit > 15) || (lowBit + nBits > 15)) {
    return -1;    // raise exception illegal limits
  }
  uint16_t registerValue, mask = 0xffff; 
  ReadRegister(address, registerValue); 
  
  for (int i = lowBit; i < lowBit + nBits; i++) {
    mask -= 1 << i;
  }

  registerValue &= mask;                  // set all bits that are to be overwritten to 0
  value         &= (1 << nBits) -1;       // make sure value fits into nBits
  registerValue |= value << nBits;        // or value into the foreseen spot

  return WriteRegister (address, registerValue, verify);

}
