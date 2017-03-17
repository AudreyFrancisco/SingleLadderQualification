#ifndef TREADOUTBOARDRU_H
#define TREADOUTBOARDRU_H

#include "TReadoutBoard.h"
#include "TConfig.h"
#include "TBoardConfigRU.h"
#include "USB.h"

#include <memory>
#include <map>
#include <deque>

#include "ReadoutUnitSrc/UsbDev.hpp"

namespace RUBoardAddresses {
namespace Dctrl {
static const uint16_t MODULE_ADDR = 4;
static const uint16_t WRITE_CTRL = 0;
static const uint16_t WRITE_ADDRESS = 1;
static const uint16_t WRITE_DATA = 2;
static const uint16_t WRITE_PHASE = 3;
static const uint16_t READ_STATUS = 4;
static const uint16_t READ_DATA = 5;
static const uint16_t LATCH_CTRL_CNTRS = 6;
static const uint16_t RST_CTRL_CNTRS = 7;
static const uint16_t READ_BROADCAST_CNTR = 8;
static const uint16_t READ_WRITE_CNTR = 9;
static const uint16_t READ_READ_CNTR = 10;
static const uint16_t READ_OPCODE_CNTR = 11;
static const uint16_t READ_TRIGGER_SENT_CNTR = 12;
static const uint16_t READ_TRIGGER_NOT_SENT_CNTR = 13;
static const uint16_t READ_WAIT_EXEC_CNTR = 14;
static const uint16_t MASK_BUSY_REG = 15;
static const uint16_t WAIT_VALUE = 16;
static const uint16_t BUSY_TRANSCEIVER_MASK_LSB = 17;
static const uint16_t READ_BUSY_TRANSCEIVER_STATUS_LSB = 18;
static const uint16_t SET_DCTRL_INPUT = 19;
static const uint16_t SET_DCTRL_TX_MASK = 20;
static const uint16_t BUSY_TRANSCEIVER_MASK_MSB = 21;
static const uint16_t READ_BUSY_TRANSCEIVER_STATUS_MSB = 22;
static const uint16_t SET_DCLK_TX_MASK = 23;
static const uint16_t MANCHESTER_TX_EN = 24;
static const uint16_t SET_IDELAY_VALUE014 = 25;
static const uint16_t SET_IDELAY_VALUE23 = 26;
static const uint16_t GET_IDELAY_VALUE014 = 27;
static const uint16_t GET_IDELAY_VALUE23 = 28;
}
}

class TReadoutBoardRU : public TReadoutBoard {
private:
  struct ReadResult {
    uint16_t address;
    uint16_t data;
    bool error;
  };

  static const int VID = 0x04B4;
  static const int PID = 0x0008;
  static const int INTERFACE_NUMBER = 2;
  static const uint8_t EP_CTL_OUT = 3;
  static const uint8_t EP_CTL_IN = 3;
  static const uint8_t EP_DATA0_IN = 4;
  static const uint8_t EP_DATA1_IN = 5;

  static const size_t EVENT_DATA_READ_CHUNK = 50 * 1024;
  static const size_t USB_TIMEOUT = 1000;
  static const int MAX_RETRIES_READ = 5;

  static const uint16_t CHIP_READ_STATUS_OK = 0x3F;

  std::shared_ptr<UsbDev> m_usb;
  TBoardConfigRU *m_config;
  UsbDev::DataBuffer m_buffer;
  uint32_t m_readBytes;

  bool m_logging;

  // Triggeroptions
  bool m_enablePulse;
  bool m_enableTrigger;
  int m_triggerDelay;
  int m_pulseDelay;

  std::map<uint8_t, std::deque<uint8_t> > m_readoutBuffers;

  void registeredWrite(uint16_t module, uint16_t address, uint16_t data);
  void registeredRead(uint16_t module, uint16_t address);
  bool flush();
  void readFromPort(uint8_t port, size_t size, UsbDev::DataBuffer &buffer);

  std::vector<ReadResult> readResults();

  // Readout streams
  void fetchEventData();

public:
  TReadoutBoardRU(libusb_device *ADevice, TBoardConfigRU *config);

  virtual int WriteChipRegister(uint16_t Address, uint16_t Value,
                                uint8_t chipId = 0);
  virtual int ReadRegister(uint16_t Address, uint32_t &Value);
  virtual int WriteRegister(uint16_t Address, uint32_t Value);
  virtual int ReadChipRegister(uint16_t Address, uint16_t &Value,
                               uint8_t chipID = 0);
  virtual int SendOpCode(uint16_t OpCode);
  virtual int SendOpCode(uint16_t OpCode, uint8_t chipId);

  virtual int SetTriggerConfig(bool enablePulse, bool enableTrigger,
                               int triggerDelay, int pulseDelay);
  virtual void SetTriggerSource(TTriggerSource triggerSource);
  virtual int Trigger(int nTriggers);
  virtual int ReadEventData(int &NBytes, unsigned char *Buffer);

  // RU specific functions

  // Initialize Readout Unit to start readout with given configuration
  int Initialize();
};

#endif // TREADOUTBOARDRU_H
