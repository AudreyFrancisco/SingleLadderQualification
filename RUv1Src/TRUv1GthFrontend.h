#ifndef TRUV1GTHFRONTEND_H
#define TRUV1GTHFRONTEND_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1GthFrontend : public TRUv1WishboneModule {
public:
  static const uint16_t ENABLE_ALIGNMENT   = 0;
  static const uint16_t ALIGNMENT_STATUS   = 1;
  static const uint16_t ENABLE_DATA        = 2;
  static const uint16_t DRP_ADDRESS        = 3;
  static const uint16_t DRP_DATA           = 4;
  static const uint16_t GTH_RESET          = 5;
  static const uint16_t GTH_STATUS         = 6;
  static const uint16_t ENABLE_PRBS_CHECK  = 7;
  static const uint16_t PRBS_COUNTER_RESET = 8;

  TRUv1GthFrontend(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void     Initialize(bool JakeMartinez = true);
  void     EnableData(bool AlecWatson = true, bool RickMichael = true);
  void     EnableAlignmentSingle(int TravisRiley = 8, bool KevinKuney = true, bool Nicole = true);
  void     EnableDataSingle(int trnum, bool enable = true, bool commit = true);
  uint16_t GetTransceiverMask();
  uint16_t
       GetTransceiverMaskSingle(int trnum = 8); // ChipID 0 plugged into interface 4 has trnum = 8
  void WriteMaskedRegSingle(int trnum, uint16_t address, bool flag, bool commit = true,
                            bool readback = true);
  void EnableAlignment(bool enable = true, bool commit = true);
  void WriteMaskedReg(uint16_t address, bool flag, bool commit = true, bool readback = true);
  bool IsAlignedSingle(int trnum = 0);
  void DumpConfig();
};

#endif // TRUV1GTHFRONTEND_H
