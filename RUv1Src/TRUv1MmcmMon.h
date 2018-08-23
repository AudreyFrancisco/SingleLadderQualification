#ifndef TRUV1MMCMMON_H
#define TRUV1MMCMMON_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1MmcmMon : public TRUv1WishboneModule {
public:
  static const uint16_t GET_LOCK_COUNTER        = 0;
  static const uint16_t MISMATCH_LSB            = 1;
  static const uint16_t MISMATCH_MSB            = 2;
  static const uint16_t MISMATCH_2ND_LSB        = 3;
  static const uint16_t MISMATCH_2ND_MSB        = 4;
  static const uint16_t MISMATCH_COUNTER        = 5;
  static const uint16_t MISMATCH_2ND_COUNTER    = 6;
  static const uint16_t LATCH_COUNTERS          = 7;
  static const uint16_t RESET_COUNTERS          = 8;
  static const uint16_t GET_COUNTER_CLK_0_FIRST = 9;
  static const uint16_t GET_COUNTER_CLK_0_LAST  = 12;
  static const uint16_t GET_COUNTER_CLK_N_FIRST = 27;
  static const uint16_t GET_COUNTER_CLK_N_LAST  = 29;

  TRUv1MmcmMon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
};

#endif // TRUV1MMCMMON_H