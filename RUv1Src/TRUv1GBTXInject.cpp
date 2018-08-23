#include "TRUv1GBTXInject.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1GBTXInject::TRUv1GBTXInject(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1GBTXInject::StartTriggeredMode(bool enable)
{

  if (enable)
    SendTrigger(1 << 7);
  else
    SendTrigger(1 << 8);
}

void TRUv1GBTXInject::SendTrigger(uint32_t trigger_type, uint16_t bc, uint32_t orbit, bool commit)
{

  Write(0, (trigger_type & 0xFFFF), false);
  Write(1, ((trigger_type >> 16) & 0xFFFF), false);
  Write(2, (bc & 0xFFF), false);
  Write(3, (orbit & 0xFFFF), false);
  Write(4, ((orbit >> 16) & 0xFFFF), false);
  Write(5, 1, false);
  Write(6, 1, commit);
}

void TRUv1GBTXInject::DumpConfig()
{
  std::cout << "....TRUV1GBTXINJECT MODULE CONFIG....\n";
  for (int i = 0; i < 0; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}

void TRUv1GBTXInject::TakeControl(bool enable)
{
  if (enable)
    Write(7, 1);
  else
    Write(7, 0);
}
