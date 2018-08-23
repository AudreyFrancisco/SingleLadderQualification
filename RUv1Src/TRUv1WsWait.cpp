#include "TRUv1WsWait.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1WsWait::TRUv1WsWait(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1WsWait::ResetControlCounters()
{
  Write(TRUv1WsWait::RST_CTRL_CNTRS, 0x01, false);
  Write(TRUv1WsWait::RST_CTRL_CNTRS, 0, true);
}

uint16_t TRUv1WsWait::GetCounters() { return Read(TRUv1WsWait::READ_WAIT_EXEC_CNTR, true); }

void TRUv1WsWait::SingleWait(uint16_t waitval, bool commit)
{
  if ((waitval | 0xFFFF) != 0xFFFF)
    std::cout << "Your wait value is too big, remember this is in multiples of WB_CLK_PERIOD \n";
  else
    Write(TRUv1WsWait::WAIT_VALUE, waitval, commit);
}

void TRUv1WsWait::DumpConfig()
{
  std::cout << "....WISHBONE WAIT MODULE CONFIG.... \n";
  for (int i = 0; i < 3; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
