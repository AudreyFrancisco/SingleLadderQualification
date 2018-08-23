#include "TRUv1WsUsbIf.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1WsUsbIf::TRUv1WsUsbIf(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1WsUsbIf::LatchCounters()
{
  uint16_t data = 1 << 15 | 1 << 14 | 1 << 11 | 1 << 10 | 1 << 7 | 1 << 6 | 1 << 3 | 1 << 2;
  Write(TRUv1WsUsbIf::DP23_CMD, data);
}
void TRUv1WsUsbIf::GetDataProducer()
{
  uint16_t data = Read(TRUv1WsUsbIf::CFG_DP23_PRODUCER_ADDRESS);
  uint16_t dp2  = data & 0xF;
  uint16_t dp3  = data >> 8 & 0xF;
  std::cout << "DATA PORT 2 PRODUCER: " << dp2 << std::endl;
  std::cout << "DATA PORT 3 PRDUCER: " << dp3 << std::endl;
}

void TRUv1WsUsbIf::SetDataProducer(int trans)
{
  uint16_t data = (trans & 0xF) << 8 | (trans & 0xF) << 0;
  Write(TRUv1WsUsbIf::CFG_DP23_PRODUCER_ADDRESS, data);
}

void TRUv1WsUsbIf::ResetCounters()
{
  uint16_t data = 1 << 13 | 1 << 12 | 1 << 9 | 1 << 8 | 1 << 5 | 1 << 4 | 1 << 1 | 1 << 0;
  Write(TRUv1WsUsbIf::DP23_CMD, data);
}
void TRUv1WsUsbIf::DumpConfig()
{
  std::cout << "....TRUV1WSUSBIF MODULE CONFIG....\n";
  for (int i = 0; i < 14; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i) << "\n";
  }
}
