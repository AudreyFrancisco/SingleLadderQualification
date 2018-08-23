#ifndef TRUV1WSUSBIF_H
#define TRUV1WSUSBIF_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1WsUsbIf : public TRUv1WishboneModule {
public:
  static const uint16_t DP23_CMD                     = 0x00;
  static const uint16_t READ_DP23_STS_DP2_WRDS_MSB   = 0x01;
  static const uint16_t READ_DP23_STS_DP2_WRDS_LSB   = 0x02;
  static const uint16_t READ_DP23_STS_DP2_OVFL       = 0x03;
  static const uint16_t READ_DP23_STS_DP3_WRDS_MSB   = 0x04;
  static const uint16_t READ_DP23_STS_DP3_WRDS_LSB   = 0x05;
  static const uint16_t READ_DP23_STS_DP3_OVFL       = 0x06;
  static const uint16_t CFG_DP23_PRODUCER_ADDRESS    = 0x07;
  static const uint16_t READ_DP23_STS_DP2_RDWRDS_MSB = 0x08;
  static const uint16_t READ_DP23_STS_DP2_RDWRDS_LSB = 0x09;
  static const uint16_t READ_DP23_STS_DP2_FULL       = 0x0A;
  static const uint16_t READ_DP23_STS_DP3_RDWRDS_MSB = 0x0B;
  static const uint16_t READ_DP23_STS_DP3_RDWRDS_LSB = 0x0C;
  static const uint16_t READ_DP23_STS_DP3_FULL       = 0x0D;

  TRUv1WsUsbIf(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void DumpConfig();
  void GetDataProducer();
  void SetDataProducer(int trans = 0);
  void LatchCounters();
  void ResetCounters();
};

#endif // TRUV1WSUSBIF_H
