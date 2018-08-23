#include "TRUv1WsStatus.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1WsStatus::TRUv1WsStatus(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1WsStatus::DumpConfig()
{
  std::cout << "....TRUV1WSSTATUS MODULE CONFIG....\n";
  for (int i = 0; i < 6; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
