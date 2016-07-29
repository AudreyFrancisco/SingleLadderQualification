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
    REG_FROMU_CONFIG1   = 0x4,
    REG_FROMU_CONFIG2   = 0x5,
    REG_FROMU_CONFIG3   = 0x6,
    REG_FROMU_PULSING1  = 0x7,
    REG_FROMU_PULSING2  = 0x8,
    REG_FROMU_STATUS1   = 0x9,
    REG_FROMU_STATUS2   = 0xa,
    REG_FROMU_STATUS3   = 0xb,
    REG_FROMU_STATUS4   = 0xc,
    REG_FROMU_STATUS5   = 0xd,
    REG_CLKIO_DACS      = 0xe,
    REG_CMUIO_DACS      = 0xf,
    REG_CMUDMU_CONFIG   = 0x10,       
    REG_CMUDMU_STATUS   = 0x11,
    REG_CMUFIFO_LOW     = 0x12,
    REG_CMUFIFO_HIGH    = 0x13,
    REG_DTU_CONFIG      = 0x14,
    REG_DTU_DACS        = 0x15,
    REG_PLL_LOCK1       = 0x16,    
    REG_PLL_LOCK2       = 0x17,    
    REG_DTU_TEST1       = 0x18,
    REG_DTU_TEST2       = 0x19,
    REG_DTU_TEST3       = 0x1a,
    REG_BUSY_MINWIDTH   = 0x1b,
    REG_PIXELCONFIG     = 0x500,
    REG_VRESETP         = 0x601, 
    REG_VRESETD         = 0x602, 
    REG_VCASP           = 0x603,
    REG_VCASN           = 0x604,
    REG_VPULSEH         = 0x605,
    REG_VPULSEL         = 0x606,
    REG_VCASN2          = 0x607,
    REG_VCLIP           = 0x608,
    REG_VTEMP           = 0x609,
    REG_IAUX2           = 0x60a,
    REG_IRESET          = 0x60b,
    REG_IDB             = 0x60c,
    REG_IBIAS           = 0x60d,
    REG_ITHR            = 0x60e
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

  typedef enum {
    PIXREG_MASK   = 0x0,
    PIXREG_SELECT = 0x1
  } TPixReg;
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
  int ReadRegister       (uint16_t address, uint16_t &value);
  int WriteRegister      (uint16_t address, uint16_t value, bool verify = false);
  int ModifyRegisterBits (Alpide::TRegister address, uint8_t lowBit, uint8_t nBits, uint16_t value, bool verify = false);

  //int SendOpCode         (Alpide::TOpCode opcode);

  //int SendCommandSequence (vector <> sequence);
};


#endif  /* ALPIDE_H */
