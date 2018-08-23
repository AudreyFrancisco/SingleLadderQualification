#ifndef TRUV1DCTRLMODULE_H
#define TRUV1DCTRLMODULE_H

#include <cstdint>
#include <iostream>

#include "TRUv1WishboneModule.h"

class TRUv1DctrlModule : public TRUv1WishboneModule {
public:
  /* static const uint16_t WRITE_CTRL                       = 0; */
  /* static const uint16_t WRITE_ADDRESS                    = 1; */
  /* static const uint16_t WRITE_DATA                       = 2; */
  /* static const uint16_t WRITE_PHASE                      = 3; */
  /* static const uint16_t READ_STATUS                      = 4; */
  /* static const uint16_t READ_DATA                        = 5; */
  /* static const uint16_t LATCH_CTRL_CNTRS                 = 6; */
  /* static const uint16_t RST_CTRL_CNTRS                   = 7; */
  /* static const uint16_t READ_BROADCAST_CNTR              = 8; */
  /* static const uint16_t READ_WRITE_CNTR                  = 9; */
  /* static const uint16_t READ_READ_CNTR                   = 10; */
  /* static const uint16_t READ_OPCODE_CNTR                 = 11; */
  /* static const uint16_t READ_TRIGGER_SENT_CNTR           = 12; */
  /* static const uint16_t READ_TRIGGER_NOT_SENT_CNTR       = 13; */
  /* static const uint16_t READ_WAIT_EXEC_CNTR              = 14; */
  /* static const uint16_t MASK_BUSY_REG                    = 15; */
  /* static const uint16_t WAIT_VALUE                       = 16; */
  /* static const uint16_t BUSY_TRANSCEIVER_MASK_LSB        = 17; */
  /* static const uint16_t READ_BUSY_TRANSCEIVER_STATUS_LSB = 18; */
  /* static const uint16_t SET_DCTRL_INPUT                  = 19; */
  /* static const uint16_t SET_DCTRL_TX_MASK                = 20; */
  /* static const uint16_t BUSY_TRANSCEIVER_MASK_MSB        = 21; */
  /* static const uint16_t READ_BUSY_TRANSCEIVER_STATUS_MSB = 22; */
  /* static const uint16_t SET_DCLK_TX_MASK                 = 23; */
  /* static const uint16_t MANCHESTER_TX_EN                 = 24; */
  /* static const uint16_t SET_IDELAY_VALUE014              = 25; */
  /* static const uint16_t SET_IDELAY_VALUE23               = 26; */
  /* static const uint16_t GET_IDELAY_VALUE014              = 27; */
  /* static const uint16_t GET_IDELAY_VALUE23               = 28; */

  static const uint16_t WRITE_CTRL                       = 0x00;
  static const uint16_t WRITE_ADDRESS                    = 0x01;
  static const uint16_t WRITE_DATA                       = 0x02;
  static const uint16_t PHASE_FORCE                      = 0x03;
  static const uint16_t READ_STATUS                      = 0x04;
  static const uint16_t READ_DATA                        = 0x05;
  static const uint16_t LATCH_CTRL_CNTRS                 = 0x06;
  static const uint16_t RST_CTRL_CNTRS                   = 0x07;
  static const uint16_t READ_BROADCAST_CNTR              = 0x08;
  static const uint16_t READ_WRITE_CNTR                  = 0x09;
  static const uint16_t READ_READ_CNTR                   = 0x0A;
  static const uint16_t READ_OPCODE_CNTR                 = 0x0B;
  static const uint16_t READ_TRIGGER_SENT_CNTR           = 0x0C;
  static const uint16_t READ_TRIGGER_NOT_SENT_CNTR       = 0x0D;
  static const uint16_t MASK_BUSY_REG                    = 0x0E;
  static const uint16_t BUSY_TRANSCEIVER_MASK_LSB        = 0x0F;
  static const uint16_t READ_BUSY_TRANSCEIVER_STATUS_LSB = 0x10;
  static const uint16_t SET_DCTRL_INPUT                  = 0x11;
  static const uint16_t SET_DCTRL_TX_MASK                = 0x12;
  static const uint16_t BUSY_TRANSCEIVER_MASK_MSB        = 0x13;
  static const uint16_t READ_BUSY_TRANSCEIVER_STATUS_MSB = 0x14;
  static const uint16_t SET_DCLK_TX_MASK                 = 0x15;
  static const uint16_t MANCHESTER_TX_EN                 = 0x16;
  static const uint16_t SET_IDELAY_VALUE0                = 0x17;
  static const uint16_t SET_IDELAY_VALUE1                = 0x18;
  static const uint16_t SET_IDELAY_VALUE2                = 0x19;
  static const uint16_t SET_IDELAY_VALUE3                = 0x1A;
  static const uint16_t SET_IDELAY_VALUE4                = 0x1B;
  static const uint16_t GET_IDELAY_VALUE0                = 0x1C;
  static const uint16_t GET_IDELAY_VALUE1                = 0x1D;
  static const uint16_t GET_IDELAY_VALUE2                = 0x1E;
  static const uint16_t GET_IDELAY_VALUE3                = 0x1F;
  static const uint16_t GET_IDELAY_VALUE4                = 0x20;
  static const uint16_t AUTO_PHASE_OFFSET                = 0x21;
  static const uint16_t SET_DCLK_PARALLEL_0              = 0x22;
  static const uint16_t SET_DCLK_PARALLEL_1              = 0x23;
  static const uint16_t SET_DCLK_PARALLEL_2              = 0x24;
  static const uint16_t SET_DCLK_PARALLEL_3              = 0x25;
  static const uint16_t SET_DCLK_PARALLEL_4              = 0x26;
  static const uint16_t WAIT_VALUE                       = 0x27;
  static const uint16_t MANCHESTER_RX_DETECTED           = 0x28;
  static const uint16_t MISMATCH                         = 0x29;
  static const uint16_t NUM_REG                          = 0x2A;

  static const uint16_t CHIP_READ_STATUS_OK = 0x3F;

  TRUv1DctrlModule(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

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
  void     ResetCounters();
  void     SetMask(uint16_t mask = 0);
  void     LatchCounters();
  void     CoutAllCounters();

private:
  uint8_t m_connector;
  bool    m_connectorSet;
};

#endif // TRUV1DCTRLMODULE_H
