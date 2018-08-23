#include "TRUv1DrpBridge.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1DrpBridge::TRUv1DrpBridge(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1DrpBridge::WriteDRP(int trNum, uint16_t address, uint16_t data, bool commit)
{
  address &= 0x1ff;
  address |= (trNum << 9);
  data &= 0xffff;

  Write(TRUv1DrpBridge::DRP_ADDRESS, address, false);
  Write(TRUv1DrpBridge::DRP_DATA, data, commit);
}

uint16_t TRUv1DrpBridge::ReadDRP(int trNum, uint16_t address, bool commit)
{
  address &= 0x1ff;
  address |= (trNum << 9);

  Write(TRUv1DrpBridge::DRP_ADDRESS, address, false);
  return Read(TRUv1DrpBridge::DRP_DATA, commit);
}

void TRUv1DrpBridge::DumpConfig()
{
  std::cout << "....TRUV1DRPBRIDGE MODULE CONFIG....\n";
  for (int i = 0; i < 2; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
