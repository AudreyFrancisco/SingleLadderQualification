#ifndef TRUV1WSWAIT_H
#define TRUV1WSWAIT_H

#include "TRUv1WishboneModule.h"

class TRUv1WsWait : public TRUv1WishboneModule {
public:
  static const uint16_t WAIT_VALUE          = 0;
  static const uint16_t RST_CTRL_CNTRS      = 1;
  static const uint16_t READ_WAIT_EXEC_CNTR = 2;

  TRUv1WsWait(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void     ResetControlCounters();
  uint16_t GetCounters();
  void     SingleWait(uint16_t waitval, bool commit = true);
  void     Wait(uint16_t waitval, bool commit = true);
  void     DumpConfig();
};

#endif // TRUV1WSWAIT_H
