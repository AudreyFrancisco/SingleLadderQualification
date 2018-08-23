#ifndef TRUV1GBTPACKER_H
#define TRUV1GBTPACKER_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1GbtPacker : public TRUv1WishboneModule {
public:
  static const uint16_t ADD_FEE_ID              = 0;
  static const uint16_t ADD_GBT_PRIORITY        = 1;
  static const uint16_t ADD_MAX_WORD_COUNT      = 2;
  static const uint16_t ADD_MAX_PACKET_COUNT    = 3;
  static const uint16_t ADD_WAIT_DATA_TIMEOUT   = 4;
  static const uint16_t ADD_SEND_DATA_TIMEOUT   = 5;
  static const uint16_t ADD_LANE_TIMEOUT        = 6;
  static const uint16_t ADD_MASK_DATALANE_L     = 7;
  static const uint16_t ADD_MASK_DATALANE_H     = 8;
  static const uint16_t ADD_GBT_PACKER_SETTINGS = 9;

  TRUv1GbtPacker(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void EnableDataForward(uint16_t enable_data_forward = 0, bool commit = true);
  void DumpConfig();
  void UnmaskLane(uint16_t recNum = 8);
  void MaskAllLanes();
};

#endif // TRUV1GBTPACKER_H
