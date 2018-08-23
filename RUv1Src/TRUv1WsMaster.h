#ifndef TRUV1WSMASTER_H
#define TRUV1WSMASTER_H

#include "TRUv1WishboneModule.h"

class TRUv1WsMaster : public TRUv1WishboneModule {
public:
  static const uint16_t READ_WBM_WRERRCNTR  = 0;
  static const uint16_t READ_WBM_RDERRCNTR  = 1;
  static const uint16_t READ_WBM_SEEERRCNTR = 2;
  static const uint16_t RST_WBM_CNTR        = 3;
  static const uint16_t MISMATCH            = 4;


  TRUv1WsMaster(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  uint16_t *WbmGetErrCntr(bool verbose = true);
  uint16_t  GetRdErrCntr(bool commit = true);
  uint16_t  GetWrErrCntr(bool commit = true);
  void      WbmRstErrCntr(bool commit = true);
  uint16_t  GetMismatch();
  void      DumpConfig();
};

#endif // TRUV1WSMASTER_H
