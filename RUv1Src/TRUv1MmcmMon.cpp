#include "TRUv1MmcmMon.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1MmcmMon::TRUv1MmcmMon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1MmcmMon::DumpConfig()
{
  std::cout << "....TRUV1MMCMMON MODULE CONFIG....\n";
  for (int i = 0; i < 13; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
