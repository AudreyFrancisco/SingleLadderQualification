#ifndef TRUV1GBTXINJECT_H
#define TRUV1GBTXINJECT_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1GBTXInject : public TRUv1WishboneModule {
public:
  TRUv1GBTXInject(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);
  void StartTriggeredMode(bool enable = true);
  void SendTrigger(uint32_t trigger_type, uint16_t bc = 0xdead, uint32_t orbit = 0xfaceb00c,
                   bool commit = true);
  void DumpConfig();
  void TakeControl(bool enable = true);
};

#endif // TRUV1GBTXINJECT_H
