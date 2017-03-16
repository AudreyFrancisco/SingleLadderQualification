#include "TReadoutBoardRU.h"

TReadoutBoardRU::TReadoutBoardRU(libusb_device *ADevice,
                                 TBoardConfigRU *config) : TUSBBoard(ADevice) {}
int TReadoutBoardRU::WriteChipRegister(uint16_t Address, uint16_t Value,
                                       uint8_t chipId = 0) {}
int TReadoutBoardRU::ReadRegister(uint16_t Address, uint32_t &Value) {}
int TReadoutBoardRU::WriteRegister(uint16_t Address, uint32_t Value) {}
int TReadoutBoardRU::ReadChipRegister(uint16_t Address, uint16_t &Value,
                                      uint8_t chipID = 0) {}
int TReadoutBoardRU::SendOpCode(uint16_t OpCode) {}
int TReadoutBoardRU::SendOpCode(uint16_t OpCode, uint8_t chipId) {}
int TReadoutBoardRU::SetTriggerConfig(bool enablePulse, bool enableTrigger,
                                      int triggerDelay, int pulseDelay) {}
void TReadoutBoardRU::SetTriggerSource(TTriggerSource triggerSource) {}
int TReadoutBoardRU::Trigger(int nTriggers) {}
int TReadoutBoardRU::ReadEventData(int &NBytes, unsigned char *Buffer) {}
