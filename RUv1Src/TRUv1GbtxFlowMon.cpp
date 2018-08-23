#include "TRUv1GbtxFlowMon.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1GbtxFlowMon::TRUv1GbtxFlowMon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1GbtxFlowMon::ResetCounters()
{
  Write(0, (1 << 7) - 1, false);
  Write(0, 0);
}

void TRUv1GbtxFlowMon::DumpConfig()
{
  std::cout << "....TRUV1GBTXFLOWMON MODULE CONFIG....\n";
  for (int i = 0; i < 9; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}

void TRUv1GbtxFlowMon::LatchCounters()
{
  Write(1, (1 << 7) - 1, false);
  Write(1, 0);
}
