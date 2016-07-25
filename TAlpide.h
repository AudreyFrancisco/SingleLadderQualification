#ifndef ALPIDE_H
#define ALPIDE_H


#include "TReadoutBoard.h"
#include "TConfig.h"

namespace Alpide {
  typedef enum {
    REG_COMMAND         = 0x0,
    REG_MODECONTROL     = 0x1,
    REG_REGDISABLE_LOW  = 0x2,
    REG_REGDISABLE_HIGH = 0x3, 
    REG_IBIAS           = 0x60d
  } TRegister;

  typedef enum {
    OPCODE_TRIGGER1 = 0xb1,
    OPCODE_TRIGGER2 = 0x55,
    OPCODE_TRIGGER3 = 0xc9,
    OPCODE_TRIGGER4 = 0x2d,
    OPCODE_GRST     = 0xd2,
    OPCODE_PRST     = 0xe4,
    OPCODE_PULSE    = 0x78,
    OPCODE_BCRST    = 0x36,
    OPCODE_DEBUG    = 0xaa,
    OPCODE_RORST    = 0x63,
    OPCODE_WROP     = 0x9c,
    OPCODE_RDOP     = 0x4e
  } TOpCode;

};

class TAlpide {
 private:
  int            fChipId;
  TReadoutBoard *fReadoutBoard;
 protected: 
 public:
  TAlpide (TConfig *config, int chipId);
  TAlpide (TConfig *config, int chipId, TReadoutBoard *readoutBoard);
  void SetReadoutBoard (TReadoutBoard *readoutBoard) {fReadoutBoard = readoutBoard;};
  
  int ReadRegister       (Alpide::TRegister address, uint16_t &value);
  int WriteRegister      (Alpide::TRegister address, uint16_t value, bool verify = false);
  int ModifyRegisterBits (Alpide::TRegister address, uint8_t lowBit, uint8_t nBits, uint16_t value, bool verify = false);

  //int SendOpCode         (Alpide::TOpCode opcode);

  //int SendCommandSequence (vector <> sequence);
};


#endif  /* ALPIDE_H */
