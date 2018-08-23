#include "TRUv1TriggerHandlerMonitor.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1TriggerHandlerMonitor::TRUv1TriggerHandlerMonitor(TReadoutBoardRUv1 &board, uint8_t moduleId,
                                                       bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1TriggerHandlerMonitor::LatchCounters()
{
  Write(1, (1 << 8) - 1, false);
  Write(1, 0);
}

void TRUv1TriggerHandlerMonitor::ResetCounters()
{
  Write(0, (1 << 8) - 1, false);
  Write(0, 0);
}

void TRUv1TriggerHandlerMonitor::DumpConfig()
{
  std::cout << "....TRUV1TRIGGERHANDLERMONITOR MODULE CONFIG....\n";
  for (int i = 0; i < 11; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
