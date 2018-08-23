#ifndef TRUV1DRPBRIDGE_H
#define TRUV1DRPBRIDGE_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1DrpBridge : public TRUv1WishboneModule {
public:
  static const uint16_t DRP_ADDRESS = 1;
  static const uint16_t DRP_DATA    = 2;

  TRUv1DrpBridge(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);


  void     WriteDRP(int trNum = 8, uint16_t address = 0x63, uint16_t data = 0, bool commit = true);
  uint16_t ReadDRP(int trNum = 8, uint16_t address = 0x63, bool commit = true);
  void     DumpConfig();
};

#endif // TRUV1DRPBRIDGE_H
