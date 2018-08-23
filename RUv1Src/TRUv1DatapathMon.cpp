#include "TRUv1DatapathMon.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1DatapathMon::TRUv1DatapathMon(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1DatapathMon::DumpConfig()
{
  std::cout << "....TRUV1DATAPATHMON MODULE CONFIG....\n";
  for (int i = 0; i < 18; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}

void TRUv1DatapathMon::ResetCounters() { Write(1, 0xFFFF, true); }
void TRUv1DatapathMon::LatchCounters() { Write(0, 0xFFFF, true); }

void TRUv1DatapathMon::CoutAllCounters()
{


  std::cout << ".....READING ALL COUNTERS ACROSS ALL LANES ON DATAPATHMON MODULE.....\n";

  for (int i = 0; i < 9; i++) {

    std::cout << dec;

    std::cout << "Lane " << i
              << " EVENT_COUNT_LOW = " << Read(17 * i + TRUv1DatapathMon::EVENT_COUNT_LOW + 2)
              << std::endl;
    std::cout << "Lane " << i
              << " EVENT_COUNT_HIGH = " << Read(17 * i + TRUv1DatapathMon::EVENT_COUNT_HIGH + 2)
              << std::endl;
    std::cout << "Lane " << i << " DECODE_ERROR_COUNT  = "
              << Read(17 * i + TRUv1DatapathMon::DECODE_ERROR_COUNT + 2) << std::endl;
    std::cout << "Lane " << i
              << " EVENT_ERROR_COUNT = " << Read(17 * i + TRUv1DatapathMon::EVENT_ERROR_COUNT + 2)
              << std::endl;
    std::cout << "Lane " << i
              << " BUSY_COUNT = " << Read(17 * i + TRUv1DatapathMon::EMPTY_REGION_COUNT + 2)
              << std::endl;
    std::cout << "Lane " << i << " DOUBLE_BUSY_ON_COUNT = " << Read(17 * i + 5 + 2) << std::endl;
    std::cout << "Lane " << i << " IDLE_WORD_COUNT = " << Read(17 * i + 6 + 2) << std::endl;
    std::cout << "Lane " << i << " CPLL_LOCK_LOSS_COUNT = " << Read(17 * i + 7 + 2) << std::endl;
    std::cout << "Lane " << i << " CDR_LOCK_LOSS_COUNT = " << Read(17 * i + 8 + 2) << std::endl;
    std::cout << "Lane " << i << " ALIGNED_LOSS_COUNT = " << Read(17 * i + 9 + 2) << std::endl;
    std::cout << "Lane " << i << " ELASTIC_BUF_OVERFLOW_COUNT = " << Read(17 * i + 10 + 2)
              << std::endl;
    std::cout << "Lane " << i << " LANE_FIFO_FULL_COUNT = " << Read(17 * i + 11 + 2) << std::endl;
    std::cout << "Lane " << i << " LANE_PACKAGER_LANE_TIMEOUT_COUNT = " << Read(17 * i + 12 + 2)
              << std::endl;
    std::cout << "Lane " << i << " GBT_PACKER_LANE_STOPS_COUNT = " << Read(17 * i + 13 + 2)
              << std::endl;
    std::cout << "Lane " << i << " LANE_PACKAGER_STOP_COUNT = " << Read(17 * i + 14 + 2)
              << std::endl;
    std::cout << "Lane " << i << " LANE_PACKAGER_START_COUNT  = " << Read(17 * i + 15 + 2)
              << std::endl;
    std::cout << "Lane " << i
              << " GBT_PACKER_LANE_START_VIOLATION_COUNT = " << Read(17 * i + 16 + 2) << std::endl;
  }

  std::cout << ".....ALL RELEVANT COUNTERS READ ON DATAPATHMON MODULE..... \n";
}
