#ifndef TRUV1WSRADMON_H
#define TRUV1WSRADMON_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1WsRadMon : public TRUv1WishboneModule {
public:
  static const uint16_t MODULE_0_FIRST     = 0;
  static const uint16_t MODULE_0_LAST      = 10;
  static const uint16_t MODULE_1_FIRST     = 11;
  static const uint16_t MODULE_1_LAST      = 15;
  static const uint16_t MODULE_2_FIRST     = 16;
  static const uint16_t MODULE_2_LAST      = 20;
  static const uint16_t MODULE_3_FIRST     = 21;
  static const uint16_t MODULE_3_LAST      = 27;
  static const uint16_t MODULE_4_FIRST     = 28;
  static const uint16_t MODULE_4_LAST      = 31;
  static const uint16_t MODULE_5_FIRST     = 32;
  static const uint16_t MODULE_5_LAST      = 32;
  static const uint16_t MODULE_6_FIRST     = 33;
  static const uint16_t MODULE_6_LAST      = 34;
  static const uint16_t MODULE_DUMMY_FIRST = 35;
  static const uint16_t MODULE_DUMMY_LAST  = 35;

  TRUv1WsRadMon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
};

#endif // TRUV1WSRADMON_H