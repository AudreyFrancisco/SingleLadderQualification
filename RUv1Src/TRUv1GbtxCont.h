#ifndef TRUV1GBTXCONT_H
#define TRUV1GBTXCONT_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1GbtxCont : public TRUv1WishboneModule {
public:
  static const uint16_t GBTX_CONTROLLER_ENABLE = 0;
  static const uint16_t GBTX_RESET             = 1;
  static const uint16_t SET_IDELAY_VALUE0      = 2;
  static const uint16_t SET_IDELAY_VALUE1      = 3;
  static const uint16_t SET_IDELAY_VALUE2      = 4;
  static const uint16_t SET_IDELAY_VALUE3      = 5;
  static const uint16_t SET_IDELAY_VALUE4      = 6;
  static const uint16_t SET_IDELAY_VALUE5      = 7;
  static const uint16_t SET_IDELAY_VALUE6      = 8;
  static const uint16_t SET_IDELAY_VALUE7      = 9;
  static const uint16_t SET_IDELAY_VALUE8      = 10;
  static const uint16_t SET_IDELAY_VALUE9      = 11;
  static const uint16_t GET_IDELAY_VALUE0      = 12;
  static const uint16_t GET_IDELAY_VALUE1      = 13;
  static const uint16_t GET_IDELAY_VALUE2      = 14;
  static const uint16_t GET_IDELAY_VALUE3      = 15;
  static const uint16_t GET_IDELAY_VALUE4      = 16;
  static const uint16_t GET_IDELAY_VALUE5      = 17;
  static const uint16_t GET_IDELAY_VALUE6      = 18;
  static const uint16_t GET_IDELAY_VALUE7      = 19;
  static const uint16_t GET_IDELAY_VALUE8      = 20;
  static const uint16_t GET_IDELAY_VALUE9      = 21;
  static const uint16_t IDELAY_LOAD            = 22;
  static const uint16_t BITSLIP_RX_CONTROL     = 23;
  static const uint16_t BITSLIP_RX_LOAD        = 24;
  static const uint16_t TX_PATTERN_SELECTION   = 25;
  static const uint16_t TX1_PATTERN_SELECTION  = 26;
  static const uint16_t MISMATCH               = 27;
  static const uint16_t RXRDY_FLAG             = 28;
  static const uint16_t NUM_REGS               = 29;

  TRUv1GbtxCont(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void      EnableController(bool commit = true);
  void      ResetGbtxAsic(bool commit = true);
  void      LoadIDelay(bool commit = true);
  void      SetIDelayAll(uint16_t idelvals[10], bool commit = true);
  void      SetIDelayValue(uint8_t groupnum, uint16_t idelayval, bool commit = true);
  uint16_t *GetIDelay();
  void      LoadBitslipRx(bool commit = true);
  void      SetBitslipRx(uint16_t val, bool commit = true);
  uint16_t  GetBitslipRx();
  void      SetTxPattern(uint16_t val, bool commit = true);
  uint16_t  GetTxPattern();
  void      SetTx1Pattern(uint16_t val, bool commit = true);
  uint16_t  GetTx1Pattern();
  uint16_t  GetLossLockFlag();
  void      DumpConfig();
};

#endif // TRUV1GBTXCONT_H
