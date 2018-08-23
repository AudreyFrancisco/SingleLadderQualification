#include "TRUv1GbtxCont.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1GbtxCont::TRUv1GbtxCont(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1GbtxCont::EnableController(bool commit)
{
  Write(TRUv1GbtxCont::GBTX_CONTROLLER_ENABLE, 1, commit);
}

void TRUv1GbtxCont::ResetGbtxAsic(bool commit)
{
  Write(TRUv1GbtxCont::GBTX_RESET, 0, commit);
  Write(TRUv1GbtxCont::GBTX_RESET, 1, commit);
}

void TRUv1GbtxCont::LoadIDelay(bool commit) { Write(TRUv1GbtxCont::IDELAY_LOAD, 1, commit); }

void TRUv1GbtxCont::SetIDelayValue(uint8_t groupnum, uint16_t idelayval, bool commit)
{
  Write(TRUv1GbtxCont::SET_IDELAY_VALUE0 + groupnum, idelayval, commit);
}

uint16_t *TRUv1GbtxCont::GetIDelay()
{
  static uint16_t idelvals[10];

  for (int i = 0; i < 10; i++) {
    idelvals[i] = Read(TRUv1GbtxCont::GET_IDELAY_VALUE0 + i, true);
  }
  return idelvals;
}

void TRUv1GbtxCont::SetIDelayAll(uint16_t idelvals[10], bool commit)
{
  for (int i = 0; i < 10; i++) {
    Write(TRUv1GbtxCont::SET_IDELAY_VALUE0 + i, idelvals[i], commit);
  }
  LoadIDelay();
}

void TRUv1GbtxCont::LoadBitslipRx(bool commit)
{
  Write(TRUv1GbtxCont::BITSLIP_RX_LOAD, 1, false);
  Write(TRUv1GbtxCont::BITSLIP_RX_LOAD, 0, commit);
}

void TRUv1GbtxCont::SetBitslipRx(uint16_t val, bool commit)
{
  Write(TRUv1GbtxCont::BITSLIP_RX_CONTROL, val, false);
  LoadBitslipRx(commit);
}

uint16_t TRUv1GbtxCont::GetBitslipRx() { return Read(TRUv1GbtxCont::BITSLIP_RX_CONTROL, true); }

void TRUv1GbtxCont::SetTxPattern(uint16_t val, bool commit)
{
  Write(TRUv1GbtxCont::TX_PATTERN_SELECTION, val, commit);
}

uint16_t TRUv1GbtxCont::GetTxPattern() { return Read(TRUv1GbtxCont::TX_PATTERN_SELECTION, true); }

void TRUv1GbtxCont::SetTx1Pattern(uint16_t val, bool commit)
{
  Write(TRUv1GbtxCont::TX1_PATTERN_SELECTION, val, commit);
}

uint16_t TRUv1GbtxCont::GetTx1Pattern() { return Read(TRUv1GbtxCont::TX1_PATTERN_SELECTION, true); }

uint16_t TRUv1GbtxCont::GetLossLockFlag() { return Read(TRUv1GbtxCont::RXRDY_FLAG, true); }

void TRUv1GbtxCont::DumpConfig()
{
  std::cout << "....GBTX CONTROLLER MODULE CONFIG....\n";
  for (int i = 0; i < 30; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
