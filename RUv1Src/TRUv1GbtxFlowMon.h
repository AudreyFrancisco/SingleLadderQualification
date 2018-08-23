#ifndef TRUV1GBTXFLOWMON_H
#define TRUV1GBTXFLOWMON_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1GbtxFlowMon : public TRUv1WishboneModule {
public:
  static const uint16_t LATCH_COUNTERS                 = 0x00;
  static const uint16_t RESET_COUNTERS                 = 0x01;
  static const uint16_t READ_COUNTER_SWT_DOWNLINK      = 0x02;
  static const uint16_t READ_COUNTER_TRG_DOWNLINK      = 0x03;
  static const uint16_t READ_COUNTER_SWT_UPLINK        = 0x04;
  static const uint16_t READ_COUNTER_SOP_UPLINK        = 0x05;
  static const uint16_t READ_COUNTER_EOP_UPLINK        = 0x06;
  static const uint16_t READ_COUNTER_OVERFLOW_DOWNLINK = 0x07;
  static const uint16_t READ_COUNTER_OVERFLOW_UPLINK   = 0x08;

  TRUv1GbtxFlowMon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
  void ResetCounters();
  void LatchCounters();
};

#endif // TRUV1GBTXFLOWMON_H
