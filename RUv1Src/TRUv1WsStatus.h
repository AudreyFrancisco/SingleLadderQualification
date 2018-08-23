#ifndef TRUV1WSSTATUS_H
#define TRUV1WSSTATUS_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1WsStatus : public TRUv1WishboneModule {
public:
  static const uint16_t GITHASH_LSB = 0x00;
  static const uint16_t GITHASH_MSB = 0x01;
  static const uint16_t DATE_LSB    = 0x02;
  static const uint16_t DATE_CSB    = 0x03;
  static const uint16_t DATE_MSB    = 0x04;
  static const uint16_t OS_LSB      = 0x05;

  TRUv1WsStatus(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
};

#endif // TRUV1WSSTATUS_H