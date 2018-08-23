#include "TRUv1GbtPacker.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1GbtPacker::TRUv1GbtPacker(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1GbtPacker::EnableDataForward(uint16_t enable_data_forward, bool commit)
{
  uint16_t data = (enable_data_forward & 1);
  Write(TRUv1GbtPacker::ADD_GBT_PACKER_SETTINGS, data, commit);
}

void TRUv1GbtPacker::UnmaskLane(uint16_t receiverNum)
{
  uint32_t initVal =
      (Read(TRUv1GbtPacker::ADD_MASK_DATALANE_H) << 8) | Read(TRUv1GbtPacker::ADD_MASK_DATALANE_L);
  uint32_t finVal     = (initVal & (~(1 << receiverNum) & 0xffffffff));
  uint16_t writeDataL = finVal & 0xffff;
  uint16_t writeDataH = (finVal >> 8) & 0xffff;


  Write(TRUv1GbtPacker::ADD_MASK_DATALANE_L, writeDataL);
  if (Read(TRUv1GbtPacker::ADD_MASK_DATALANE_L) != writeDataL)
    std::cout << "WARNING: GBT LANE MASK UNSUCCESSFUL, VALUE IS "
              << Read(TRUv1GbtPacker::ADD_MASK_DATALANE_L) << std::endl;
  Write(TRUv1GbtPacker::ADD_MASK_DATALANE_H, writeDataH);
  if (Read(TRUv1GbtPacker::ADD_MASK_DATALANE_H) != writeDataH)
    std::cout << "WARNING: GBT LANE MASK UNSUCCESSFUL, VALUE IS "
              << Read(TRUv1GbtPacker::ADD_MASK_DATALANE_H) << std::endl;
}

void TRUv1GbtPacker::MaskAllLanes()
{
  Write(TRUv1GbtPacker::ADD_MASK_DATALANE_L, 0xffff);
  if (Read(TRUv1GbtPacker::ADD_MASK_DATALANE_L) != 0xffff)
    std::cout << "WARNING: GBT TOTAL LANE MASK UNSUCCESSFUL, VALUE IS "
              << Read(TRUv1GbtPacker::ADD_MASK_DATALANE_L) << std::endl;
  Write(TRUv1GbtPacker::ADD_MASK_DATALANE_H, 0xffff);
  if (Read(TRUv1GbtPacker::ADD_MASK_DATALANE_H) != 0xffff)
    std::cout << "WARNING: GBT TOTAL LANE MASK UNSUCCESSFUL, VALUE IS "
              << Read(TRUv1GbtPacker::ADD_MASK_DATALANE_H) << std::endl;
}
void TRUv1GbtPacker::DumpConfig()
{
  std::cout << "....TRUV1GBTPACKER MODULE CONFIG....\n";
  for (int i = 0; i < 10; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
