#include "TRUv1GpioFrontend.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1GpioFrontend::TRUv1GpioFrontend(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1GpioFrontend::Initialize(bool commit)
{
  EnableData(false);
  EnableAlignment(false);
}

void TRUv1GpioFrontend::EnableAlignment(bool enable, bool commit)
{
  WriteMaskedReg(TRUv1GpioFrontend::ENABLE_ALIGNMENT_H, TRUv1GpioFrontend::ENABLE_ALIGNMENT_L,
                 enable, commit);
}

void TRUv1GpioFrontend::EnableData(bool enable, bool commit)
{
  WriteMaskedReg(TRUv1GpioFrontend::ENABLE_DATA_H, TRUv1GpioFrontend::ENABLE_DATA_L, enable,
                 commit);
}

void TRUv1GpioFrontend::EnableAlignmentSingle(int trnum, bool enable, bool commit)
{
  WriteMaskedRegSingle(trnum, TRUv1GpioFrontend::ENABLE_ALIGNMENT_H,
                       TRUv1GpioFrontend::ENABLE_ALIGNMENT_L, enable, commit);
}

void TRUv1GpioFrontend::EnableDataSingle(int trnum, bool enable, bool commit)
{
  WriteMaskedRegSingle(trnum, TRUv1GpioFrontend::ENABLE_DATA_H, TRUv1GpioFrontend::ENABLE_DATA_L,
                       enable, commit);
}

void TRUv1GpioFrontend::WriteMaskedReg(uint16_t address_h, uint16_t address_l, bool flag,
                                       bool commit, bool readback)
{
  uint16_t reg  = 0;
  uint16_t mask = GetTransceiverMask();
  if (flag)
    reg |= mask;
  else
    reg &= ~mask;
  uint16_t reg_l = reg & 0xffff;
  uint16_t reg_h = (reg >> 16) & 0xffff;
  Write(address_l, reg_l, false);
  usleep(100);
  Write(address_h, reg_h, commit);
}

void TRUv1GpioFrontend::WriteMaskedRegSingle(int trnum, uint16_t address_h, uint16_t address_l,
                                             bool flag, bool commit, bool readback)
{
  uint16_t reg  = 0;
  uint16_t mask = GetTransceiverMaskSingle(trnum);
  if (flag)
    reg |= mask;
  else
    reg &= ~mask;
  uint16_t reg_l = reg & 0xffff;
  uint16_t reg_h = (reg >> 16) & 0xffff;
  Write(address_l, reg_l, false);
  usleep(100);
  Write(address_h, reg_h, commit);
}

uint16_t TRUv1GpioFrontend::GetTransceiverMask()
{
  uint16_t mask = 0;
  for (int i = 0; i < 28; i++) {
    mask |= 1 << (i);
  }
  return mask;
}
uint16_t TRUv1GpioFrontend::GetTransceiverMaskSingle(int trnum)
{
  uint16_t mask = 0;
  mask |= 1 << trnum;
  return mask;
}

void TRUv1GpioFrontend::DumpConfig()
{
  std::cout << "....TRUV1GPIOFRONTEND MODULE CONFIG....\n";
  for (int i = 0; i < 17; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
