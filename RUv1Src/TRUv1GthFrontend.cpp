#include "TRUv1GthFrontend.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1GthFrontend::TRUv1GthFrontend(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}


void TRUv1GthFrontend::Initialize(bool commit)
{
  EnableData(false);
  EnableAlignment(false);
  Write(TRUv1GthFrontend::GTH_RESET, 1 << 15, false);
  m_board.wait_module->SingleWait(15, false);
  Write(TRUv1GthFrontend::GTH_RESET, 0, commit);

  bool isReset    = false;
  int  numRetries = 0;
  while (!isReset || (numRetries == 5)) {
    isReset = (Read(TRUv1GthFrontend::GTH_STATUS) & (1 << 15)) > 0;
    m_board.wait_module->SingleWait(200, true);
    numRetries++;
  }
  if ((numRetries == 5) && !isReset) std::cout << "GTH RESET FAILED \n";
}

void TRUv1GthFrontend::EnableAlignment(bool enable, bool commit)
{
  WriteMaskedReg(TRUv1GthFrontend::ENABLE_ALIGNMENT, enable, commit);
}

void TRUv1GthFrontend::EnableData(bool enable, bool commit)
{
  WriteMaskedReg(TRUv1GthFrontend::ENABLE_DATA, enable, commit);
}

void TRUv1GthFrontend::EnableAlignmentSingle(int trnum, bool enable, bool commit)
{
  WriteMaskedRegSingle(trnum, TRUv1GthFrontend::ENABLE_ALIGNMENT, enable, commit);
}

void TRUv1GthFrontend::EnableDataSingle(int trnum, bool enable, bool commit)
{
  WriteMaskedRegSingle(trnum, TRUv1GthFrontend::ENABLE_DATA, enable, commit);
}

void TRUv1GthFrontend::WriteMaskedReg(uint16_t address, bool flag, bool commit, bool readback)
{
  uint16_t reg  = 0;
  uint16_t mask = GetTransceiverMask();
  if (flag)
    reg |= mask;
  else
    reg &= ~mask;
  Write(address, reg, commit);
}

void TRUv1GthFrontend::WriteMaskedRegSingle(int trnum, uint16_t address, bool flag, bool commit,
                                            bool readback)
{
  uint16_t reg  = 0;
  uint16_t mask = GetTransceiverMaskSingle(trnum);
  if (flag)
    reg |= mask;
  else
    reg &= ~mask;
  Write(address, reg, commit);
}

uint16_t TRUv1GthFrontend::GetTransceiverMask()
{
  uint16_t mask = 0;
  for (int i = 0; i < 9; i++) {
    mask |= 1 << (i);
  }
  return mask;
}
uint16_t TRUv1GthFrontend::GetTransceiverMaskSingle(int trnum)
{
  uint16_t mask = Read(TRUv1GthFrontend::ENABLE_ALIGNMENT);
  mask |= 1 << trnum;
  return mask;
}

void TRUv1GthFrontend::DumpConfig()
{
  std::cout << "....TRUV1GTHFRONTEND MODULE CONFIG....\n";
  for (int i = 0; i < 9; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
