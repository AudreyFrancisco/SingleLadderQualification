#include "TBoardConfigRUv1.h"

#include "TReadoutBoardRUv1.h"

TBoardConfigRUv1::TBoardConfigRUv1(const char *fName, int boardIndex)
    : TBoardConfig(fName, boardIndex)
{
  this->fBoardType = TBoardType::boardRUv1;
  InitParamMap();
}

uint8_t TBoardConfigRUv1::getTransceiverChip(const uint8_t DP, const uint8_t index) const
{
  std::vector<uint8_t> mapping_dp0 = {0, 2, 4, 6, 8, 10, 12, 255};
  std::vector<uint8_t> mapping_dp1 = {1, 3, 5, 7, 9, 11, 13, 255};

  if (index > 7) return 255;
  if (DP == TReadoutBoardRUv1::EP_DATA0_IN)
    return mapping_dp0[index];
  else if (DP == TReadoutBoardRUv1::EP_DATA1_IN)
    return mapping_dp1[index];
  return 255;
}

uint8_t TBoardConfigRUv1::getConnector() const { return 4; }
bool    TBoardConfigRUv1::getInvertPolarity() const { return false; }
bool    TBoardConfigRUv1::enableLogging() const { return true; }
