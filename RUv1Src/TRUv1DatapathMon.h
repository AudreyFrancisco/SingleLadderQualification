#ifndef TRUV1DATAPATHMON_H
#define TRUV1DATAPATHMON_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1DatapathMon : public TRUv1WishboneModule {
public:
  static const uint16_t EVENT_COUNT_LOW             = 0;
  static const uint16_t EVENT_COUNT_HIGH            = 1;
  static const uint16_t DECODE_ERROR_COUNT          = 2;
  static const uint16_t EVENT_ERROR_COUNT           = 3;
  static const uint16_t EMPTY_REGION_COUNT          = 4;
  static const uint16_t BUSY_COUNT                  = 5;
  static const uint16_t BUSY_VIOLATION_COUNT        = 6;
  static const uint16_t DOUBLE_BUSY_ON_COUNT        = 7;
  static const uint16_t DOUBLE_BUSY_OFF_COUNT       = 8;
  static const uint16_t IDLE_WORD_COUNT             = 9;
  static const uint16_t LANE_FIFO_FULL_COUNT        = 10;
  static const uint16_t LANE_FIFO_OVERFLOW_COUNT    = 11;
  static const uint16_t CPLL_LOCK_LOSS_COUNT        = 12;
  static const uint16_t CDR_LOCK_LOSS_COUNT         = 13;
  static const uint16_t ALIGNED_LOSS_COUNT          = 14;
  static const uint16_t REALIGN_COUNT               = 15;
  static const uint16_t ELASTIC_BUF_OVERFLOW_COUNT  = 16;
  static const uint16_t ELASTIC_BUF_UNDERFLOW_COUNT = 17;

  TRUv1DatapathMon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
  void ResetCounters();
  void CoutAllCounters();
  void LatchCounters();
};

#endif // TRUV1DATAPATHMON_H
