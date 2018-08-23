#include "TRUv1WsRadMon.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1WsRadMon::TRUv1WsRadMon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1WsRadMon::DumpConfig()
{
  std::cout << "....TRUV1WSRADMON MODULE CONFIG....\n";
  for (int i = 0; i < 16; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
