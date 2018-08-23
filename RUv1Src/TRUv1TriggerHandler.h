#ifndef TRUV1TRIGGERHANDLER_H
#define TRUV1TRIGGERHANDLER_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1TriggerHandler : public TRUv1WishboneModule {
public:
  static const uint16_t ENABLE               = 0x00;
  static const uint16_t TRIGGER_PERIOD       = 0x01;
  static const uint16_t PULSE_nTRIGGER       = 0x02;
  static const uint16_t TRIGGER_MIN_DISTANCE = 0x03;
  static const uint16_t MODE                 = 0x04;
  static const uint16_t MISMATCH_WB          = 0x05;

  TRUv1TriggerHandler(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
};

#endif // TRUV1TRIGGERHANDLER_H