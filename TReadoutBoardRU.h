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
#include "ReadoutUnitSrc/TRuDctrlModule.h"
#include "ReadoutUnitSrc/TRuTransceiverModule.h"


class TReadoutBoardRU : public TReadoutBoard {
public:
  struct ReadResult {
    uint16_t address;
    uint16_t data;
    bool error;
    ReadResult(uint16_t address, uint16_t data, bool error) : address(address),data(data),error(error){}
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

  static const uint8_t MODULE_MASTER = 0;
  static const uint8_t MODULE_STATUS = 1;
  static const uint8_t MODULE_VOLTAGE = 2;
  static const uint8_t MODULE_DCTRL = 3;
  static const uint8_t MODULE_DATA0 = 4;
private:
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

  // Readout streams
  void fetchEventData();

public:
  // Modules
  std::shared_ptr<TRuDctrlModule> dctrl;
  std::map<uint8_t, std::shared_ptr<TRuTransceiverModule> > transceiver_array;
public:
  TReadoutBoardRU(TBoardConfigRU *config);

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

  void registeredWrite(uint16_t module, uint16_t address, uint16_t data);
  void registeredRead(uint16_t module, uint16_t address);
  bool flush();
  void readFromPort(uint8_t port, size_t size, UsbDev::DataBuffer &buffer);
  std::vector<ReadResult> readResults();

  void checkGitHash();
};

#endif // TREADOUTBOARDRU_H
