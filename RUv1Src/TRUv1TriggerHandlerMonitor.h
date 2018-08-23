#ifndef TRUV1TRIGGERHANDLERMONITOR_H
#define TRUV1TRIGGERHANDLERMONITOR_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1TriggerHandlerMonitor : public TRUv1WishboneModule {
public:
  static const uint16_t CNTR_RST                        = 0x00;
  static const uint16_t CNTR_LATCH                      = 0x01;
  static const uint16_t CNTR_TRIGGER_SENT_LSB           = 0x02;
  static const uint16_t CNTR_TRIGGER_SENT_MSB           = 0x03;
  static const uint16_t CNTR_TRIGGER_NOT_SENT           = 0x04;
  static const uint16_t CNTR_TRG_FIFO_GTH_FULL          = 0x05;
  static const uint16_t CNTR_TRG_FIFO_GTH_OVFL          = 0x06;
  static const uint16_t CNTR_TRG_FIFO_GPIO_FULL         = 0x07;
  static const uint16_t CNTR_TRG_FIFO_GPIO_OVFL         = 0x08;
  static const uint16_t CNTR_RX_FIFO_DATA_AVAILABLE_LSB = 0x09;
  static const uint16_t CNTR_RX_FIFO_DATA_AVAILABLE_MSB = 0x0A;

  /*
  COUNTERS BIT MAPPING:

      TRIGGER_SENT           = 0
      TRIGGER_NOT_SENT       = 1
      TRG_FIFO_GTH_FULL      = 2
      TRG_FIFO_GTH_OVFL      = 3
      TRG_FIFO_GPIO_FULL     = 4
      TRG_FIFO_GPIO_OVFL     = 5
      RX_FIFO_DATA_AVAILABLE = 6

  */
  TRUv1TriggerHandlerMonitor(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
  void LatchCounters();
  void ResetCounters();
};

#endif // TRUV1TRIGGERHANDLERMONITOR_H
