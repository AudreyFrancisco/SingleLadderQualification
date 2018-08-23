#include "TRUv1Sysmon.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1Sysmon::TRUv1Sysmon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1Sysmon::DumpConfig()
{
  std::cout << "....TRUV1SYSMON MODULE CONFIG....\n";
  for (int i = 0; i < 3; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
