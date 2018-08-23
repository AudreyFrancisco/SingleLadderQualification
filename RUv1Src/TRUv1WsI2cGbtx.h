#ifndef TRUV1WSI2CGBTX_H
#define TRUV1WSI2CGBTX_H

#include <cstdint>
#include <iostream>
#include <string>

#include "TRUv1WishboneModule.h"

class TRUv1WsI2cGbtx : public TRUv1WishboneModule {
public:
  static const uint16_t ADDRESS_GBTX0 = 0;
  static const uint16_t ADDRESS_GBTX1 = 1;
  static const uint16_t ADDRESS_GBTX2 = 2;
  static const uint16_t DATA          = 3;

  TRUv1WsI2cGbtx(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);
  void LoadConfig(int gbtx = 0, const char *filename = "facebook");
  void DumpConfig();
  void WriteGBTXAddress(int gbtx_num = 0, uint16_t addr = 0, uint16_t val = 0, bool commit = true);
  uint16_t ReadGBTXAddress(int gbtx_num = 0, uint16_t addr = 0);
  bool     isConfigLoaded(int gbtx = 0, const char *filename = "facebook");
};

#endif // TRUV1WSI2CGBTX_H
