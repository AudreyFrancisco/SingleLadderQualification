#ifndef TRUV1GPIOFRONTEND_H
#define TRUV1GPIOFRONTEND_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1GpioFrontend : public TRUv1WishboneModule {
public:
  static const uint16_t ENABLE_ALIGNMENT_L   = 0;
  static const uint16_t ENABLE_ALIGNMENT_H   = 1;
  static const uint16_t ALIGNMENT_STATUS_L   = 2;
  static const uint16_t ALIGNMENT_STATUS_H   = 3;
  static const uint16_t ENABLE_REALIGN_L     = 4;
  static const uint16_t ENABLE_REALIGN_H     = 5;
  static const uint16_t ENABLE_DATA_L        = 6;
  static const uint16_t ENABLE_DATA_H        = 7;
  static const uint16_t IDELAY_VALUE         = 8;
  static const uint16_t IDELAY_LOAD_L        = 9;
  static const uint16_t IDELAY_LOAD_H        = 10;
  static const uint16_t ENABLE_PRBS_CHECK_L  = 11;
  static const uint16_t ENABLE_PRBS_CHECK_H  = 12;
  static const uint16_t PRBS_RESET_COUNTER_L = 13;
  static const uint16_t PRBS_RESET_COUNTER_H = 14;
  static const uint16_t PRBS_COUNTER_LANE_0  = 15;
  static const uint16_t LANE_CHIP_MASK_0     = 47;

  TRUv1GpioFrontend(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void     Initialize(bool commit = true);
  void     EnableData(bool enable = true, bool commit = true);
  void     EnableAlignmentSingle(int trnum = 8, bool enable = true, bool commit = true);
  void     EnableDataSingle(int trnum, bool enable = true, bool commit = true);
  uint16_t GetTransceiverMask();
  uint16_t
       GetTransceiverMaskSingle(int trnum = 8); // ChipID 0 plugged into interface 4 has trnum = 8
  void WriteMaskedRegSingle(int trnum, uint16_t address_h, uint16_t address_l, bool flag,
                            bool commit = true, bool readback = true);
  void EnableAlignment(bool enable = true, bool commit = true);
  void WriteMaskedReg(uint16_t address_h, uint16_t address_l, bool flag, bool commit = true,
                      bool readback = true);
  bool IsAlignedSingle(int trnum = 0);
  void DumpConfig();
};

#endif // TRUV1GPIOFRONTEND_H
