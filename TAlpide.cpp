#include "TAlpide.h"

using namespace Alpide;

TAlpide::TAlpide (TConfig *config, int chipId) {
  fChipId = chipId;
}


int TAlpide::ReadRegister (TRegister address, uint16_t &value) {
  return fReadoutBoard->ReadChipRegister((uint16_t) address, value, fChipId);
}


int TAlpide::WriteRegister (TRegister address, uint16_t value, bool verify) {
  int result = fReadoutBoard->WriteChipRegister((uint16_t) address, value, fChipId);
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
