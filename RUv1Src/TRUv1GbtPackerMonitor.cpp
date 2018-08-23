#include "TRUv1GbtPackerMonitor.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1GbtPackerMonitor::TRUv1GbtPackerMonitor(TReadoutBoardRUv1 &board, uint8_t moduleId,
                                             bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1GbtPackerMonitor::DumpConfig()
{
  std::cout << "....TRUV1GBTPACKERMONITOR MODULE CONFIG....\n";
  for (int i = 0; i < 0; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
void TRUv1GbtPackerMonitor::LatchCounters() { Write(0, 0xFFFF); }
void TRUv1GbtPackerMonitor::ResetCounters() { Write(1, 0xFFFF); }

void TRUv1GbtPackerMonitor::CoutAllCounters()
{

  std::cout << ".....READING ALL COUNTERS ACROSS ALL LANES ON GBT PACK MON  MODULE.....\n";


  std::cout << dec;

  std::cout << " TRIGGER_RD_LOW = " << Read(2) << std::endl;
  std::cout << " TRIGGER_RD_HIGH = " << Read(3) << std::endl;
  std::cout << " SEND_SOP_LOW  = " << Read(4) << std::endl;
  std::cout << " SEND_SOP_HIGH = " << Read(5) << std::endl;
  std::cout << " SEND_EOP_LOW = " << Read(6) << std::endl;
  std::cout << " SEND_EOP_HIGH = " << Read(7) << std::endl;
  std::cout << " PACKET_DONE_LOW = " << Read(8) << std::endl;
  std::cout << " PACKET_DONE_HIGH = " << Read(9) << std::endl;
  std::cout << " FIFO_FULL = " << Read(12) << std::endl;
  std::cout << " FIFO_OVERFLOW = " << Read(13) << std::endl;
  std::cout << " EMPTY_PACKET  = " << Read(14) << std::endl;


  std::cout << ".....ALL RELEVANT COUNTERS READ ON GBT PACK MON MODULE..... \n";
}
