#include "TRUv1WsMaster.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1WsMaster::TRUv1WsMaster(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

uint16_t *TRUv1WsMaster::WbmGetErrCntr(bool verbose)
{
  static uint16_t slowerReadResults[3];
  slowerReadResults[0] = Read(TRUv1WsMaster::READ_WBM_WRERRCNTR, true);
  slowerReadResults[1] = Read(TRUv1WsMaster::READ_WBM_RDERRCNTR, true);
  slowerReadResults[2] = Read(TRUv1WsMaster::READ_WBM_SEEERRCNTR, true);
  if (verbose) {
    std::cout << "WRERRCNTR = " << slowerReadResults[0] << "\n";
    std::cout << "RDERRCNTR = " << slowerReadResults[1] << "\n";
    std::cout << "SEEERRCNTR = " << slowerReadResults[2] << "\n";
  }

  return slowerReadResults;
}

uint16_t TRUv1WsMaster::GetRdErrCntr(bool commit)
{
  return Read(TRUv1WsMaster::READ_WBM_RDERRCNTR, commit);
}

uint16_t TRUv1WsMaster::GetWrErrCntr(bool commit)
{
  return Read(TRUv1WsMaster::READ_WBM_WRERRCNTR, commit);
}

void TRUv1WsMaster::WbmRstErrCntr(bool commit) { Write(TRUv1WsMaster::RST_WBM_CNTR, 7, commit); }


uint16_t TRUv1WsMaster::GetMismatch() { return Read(TRUv1WsMaster::MISMATCH, true); }

void TRUv1WsMaster::DumpConfig()
{
  std::cout << "....WISHBONE MASTER MODULE CONFIG.... \n";
  for (int i = 0; i < 5; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
