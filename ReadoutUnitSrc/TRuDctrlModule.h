#ifndef TRUDCTRLMODULE_H
#define TRUDCTRLMODULE_H

#include <cstdint>
#include <iostream>

#include "TRuWishboneModule.h"

class TRuDctrlModule : public TRuWishboneModule {
public:
  static const uint16_t WRITE_CTRL                       = 0;
  static const uint16_t WRITE_ADDRESS                    = 1;
  static const uint16_t WRITE_DATA                       = 2;
  static const uint16_t WRITE_PHASE                      = 3;
  static const uint16_t READ_STATUS                      = 4;
  static const uint16_t READ_DATA                        = 5;
  static const uint16_t LATCH_CTRL_CNTRS                 = 6;
  static const uint16_t RST_CTRL_CNTRS                   = 7;
  static const uint16_t READ_BROADCAST_CNTR              = 8;
  static const uint16_t READ_WRITE_CNTR                  = 9;
  static const uint16_t READ_READ_CNTR                   = 10;
  static const uint16_t READ_OPCODE_CNTR                 = 11;
  static const uint16_t READ_TRIGGER_SENT_CNTR           = 12;
  static const uint16_t READ_TRIGGER_NOT_SENT_CNTR       = 13;
  static const uint16_t READ_WAIT_EXEC_CNTR              = 14;
  static const uint16_t MASK_BUSY_REG                    = 15;
  static const uint16_t WAIT_VALUE                       = 16;
  static const uint16_t BUSY_TRANSCEIVER_MASK_LSB        = 17;
  static const uint16_t READ_BUSY_TRANSCEIVER_STATUS_LSB = 18;
  static const uint16_t SET_DCTRL_INPUT                  = 19;
  static const uint16_t SET_DCTRL_TX_MASK                = 20;
  static const uint16_t BUSY_TRANSCEIVER_MASK_MSB        = 21;
  static const uint16_t READ_BUSY_TRANSCEIVER_STATUS_MSB = 22;
  static const uint16_t SET_DCLK_TX_MASK                 = 23;
  static const uint16_t MANCHESTER_TX_EN                 = 24;
  static const uint16_t SET_IDELAY_VALUE014              = 25;
  static const uint16_t SET_IDELAY_VALUE23               = 26;
  static const uint16_t GET_IDELAY_VALUE014              = 27;
  static const uint16_t GET_IDELAY_VALUE23               = 28;

  static const uint16_t CHIP_READ_STATUS_OK = 0x3F;

  TRuDctrlModule(TReadoutBoardRU &board, uint8_t moduleId, bool logging = false);

  void WriteChipRegister(uint16_t Address, uint16_t Value, uint8_t chipId, bool commit = true);
  int  ReadChipRegister(uint16_t Address, uint16_t &Value, uint8_t chipId);
  void SendOpCode(uint16_t OpCode, bool commit = true);

  int  SetConnector(uint8_t connector, bool commit = true);
  void Wait(uint16_t waittime, bool commit = true);

  // YCM: some function from Python code
  void     SetManchesterEn(bool en = true);
  void     ForcePhase(uint16_t phase);
  void     ReleasePhaseForce();
  bool     PhaseIsForce();
  uint16_t GetPhase();

private:
  uint8_t m_connector;
  bool    m_connectorSet;
};

#endif // TRUDCTRLMODULE_H
