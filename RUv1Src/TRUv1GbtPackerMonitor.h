#ifndef TRUV1GBTPACKERMONITOR_H
#define TRUV1GBTPACKERMONITOR_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1GbtPackerMonitor : public TRUv1WishboneModule {
public:
  static const uint16_t LATCH_COUNTERS   = 0;
  static const uint16_t RESET_COUNTERS   = 1;
  static const uint16_t TRIGGER_RD_LOW   = 2;
  static const uint16_t TRIGGER_RD_HIGH  = 3;
  static const uint16_t SEND_SOP_LOW     = 4;
  static const uint16_t SEND_SOP_HIGH    = 5;
  static const uint16_t SEND_EOP_LOW     = 6;
  static const uint16_t SEND_EOP_HIGH    = 7;
  static const uint16_t PACKET_DONE_LOW  = 8;
  static const uint16_t PACKET_DONE_HIGH = 9;
  static const uint16_t PACKET_TIMEOUT   = 10;
  static const uint16_t STATE_MISMATCH   = 11;
  static const uint16_t FIFO_FULL        = 12;
  static const uint16_t FIFO_OVERFLOW    = 13;
  static const uint16_t EMPTY_PACKET     = 14;


  TRUv1GbtPackerMonitor(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
  void CoutAllCounters();
  void ResetCounters();
  void LatchCounters();
};

#endif // TRUV1GBTPACKERMONITOR_H
