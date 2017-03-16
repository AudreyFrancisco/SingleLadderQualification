#ifndef TREADOUTBOARDRU_H
#define TREADOUTBOARDRU_H

#include "TReadoutBoard.h"
#include "TConfig.h"
#include "TBoardConfigRU.h"
#include "USB.h"

class TReadoutBoardRU : public TReadoutBoard {
private:
  static const int VID = 0x04B4;
  static const int PID = 0x0008;
  static const int INTERFACE_NUMBER = 2;
  static const uint8_t EP_CTL_OUT = 3;
  static const uint8_t EP_CTL_IN = 3;
  static const uint8_t EP_DATA0_IN = 4;
  static const uint8_t EP_DATA1_IN = 5;

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
};

#endif // TREADOUTBOARDRU_H
