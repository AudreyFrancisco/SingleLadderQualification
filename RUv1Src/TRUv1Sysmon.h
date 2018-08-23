#ifndef TRUV1SYSMON_H
#define TRUV1SYSMON_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1Sysmon : public TRUv1WishboneModule {
public:
  static const uint16_t TMR_MISC = 0x00;
  static const uint16_t DRP_ADDR = 0x01;
  static const uint16_t DRP_DATA = 0x02;

  TRUv1Sysmon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
};

#endif // TRUV1SYSMON_H