#include <algorithm>

#include "TReadoutBoardRU.h"
#include "TAlpide.h"

namespace AddrDctrl = RUBoardAddresses::Dctrl;

int roundUpToMultiple(int numToRound, int multiple) {
  if (multiple == 0)
    return numToRound;

  int remainder = numToRound % multiple;
  if (remainder == 0)
    return numToRound;

  return numToRound + multiple - remainder;
}

TReadoutBoardRU::TReadoutBoardRU(libusb_device *ADevice, TBoardConfigRU *config)
    : m_buffer(), m_readBytes(0), m_logging(false), m_enablePulse(false),
      m_enableTrigger(true), m_triggerDelay(0), m_pulseDelay(0) {}

void TReadoutBoardRU::registeredWrite(uint16_t module, uint16_t address,
                                      uint32_t data) {
  uint8_t module_byte = module & 0x7F;
  uint8_t address_byte = address = 0xFF;
  uint8_t data_low = data >> 0 & 0xFF;
  uint8_t data_high = data >> 8 & 0xFF;

  module_byte |= 0x80; // Write operation

  m_buffer.push_back(data_low);
  m_buffer.push_back(data_high);
  m_buffer.push_back(address_byte);
  m_buffer.push_back(module_byte);
}

void TReadoutBoardRU::registeredRead(uint16_t module, uint16_t address) {
  uint8_t module_byte = module & 0x7F;
  uint8_t address_byte = address = 0xFF;

  m_buffer.push_back(0);
  m_buffer.push_back(0);
  m_buffer.push_back(address_byte);
  m_buffer.push_back(module_byte);

  m_readBytes += 4;
}

bool TReadoutBoardRU::flush() {
  size_t toWrite = m_buffer.size();
  size_t written = m_usb->writeData(EP_CTL_OUT, m_buffer, USB_TIMEOUT);

  return toWrite == written;
}

void TReadoutBoardRu::readFromPort(uint8_t port, size_t size,
                                   UsbDev::DataBuffer &buffer) {
  buffer.resize(roundUpToMultiple(size, 1024));
  m_usb->readData(EP_CTL_IN, buffer, USB_TIMEOUT);
}

std::vector<TReadoutBoardRU::ReadResult> readResults() {
  // Read from Control Port
  UsbDev::DataBuffer buffer_all, buffer_single;
  int retries = 0;
  size_t data_left = m_readBytes;
  while (buffer_all.size() < m_readBytes && retries < MAX_RETRIES_READ) {

    readFromPort(EP_CTL_IN, data_left, buffer_single);
    buffer_all.insert(buffer_all.end(), buffer_single.begin(),
                      buffer_single.end());

    if (buffer_all.size() < m_readBytes) {
      data_left = m_readBytes - buffer_all.size();
      ++retries;
    }
  }
  if (buffer_all < m_readBytes) {
    if (m_logging)
      cout << "TReadoutBoardRU: could not read all data from Control Port. "
              "Packet dropped";
  }
  m_readBytes = 0;

  // Process read data
  std::vector<TReadoutBoardRU::ReadResult> results;
  for (int i = 0; i < buffer_all.size(); i += 4) {
    uint16_t data = buffer_all[i] | (buffer_all[i + 1] << 8);
    uint8_t address = buffer_all[i + 2] | (buffer_all[i + 3] << 8);
    bool read_error = (address >> 15) & 1 > 0;
    address &= 0x7FFF;
    if (read_error && m_logging) {
      std::cout << "TReadoutBoardRU: Wishbone error while reading: Address "
                << address << "\n";
    }
    results.emplace_back(address, data, read_error);
  }
  return results;
}

int TReadoutBoardRU::ReadRegister(uint16_t Address, uint32_t &Value) {
  uint16_t module = Address >> 8 & 0xFF;
  uint16_t sub_address = Address & 0xFF;
  registeredRead(module, sub_address);
  flush();
  auto results = readResults();
  if (results.size() != 1) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: Expected 1 result, got " << results.size()
                << "\n";
    return -1;
  } else {
    Value = results[0].data;
  }
  return 0;
}
int TReadoutBoardRU::WriteRegister(uint16_t Address, uint32_t Value) {
  uint16_t module = Address >> 8 & 0xFF;
  uint16_t sub_address = Address & 0xFF;
  uint16_t data = Value & 0xFFFF;
  registeredWrite(module, sub_address, data);
  flush();

  return 0;
}

int TReadoutBoardRU::WriteChipRegister(uint16_t Address, uint16_t Value,
                                       uint8_t chipId = 0) {

  uint16_t writeCmd = (Alpide::OPCODE_WROP << 8) | chipId;

  registeredWrite(AddrDctrl::MODULE_ADDR, AddrDctrl::WRITE_ADDRESS, Address);
  registeredWrite(AddrDctrl::MODULE_ADDR, AddrDctrl::WRITE_DATA, Value);
  registeredWrite(AddrDctrl::MODULE_ADDR, AddrDctrl::WRITE_CTRL, writeCmd);
  flush();

  return 0;
}

int TReadoutBoardRU::ReadChipRegister(uint16_t Address, uint16_t &Value,
                                      uint8_t chipID = 0) {

  uint16_t readCmd = (Alpide::OPCODE_RDOP << 8) | chipId;

  registeredWrite(AddrDctrl::MODULE_ADDR, AddrDctrl::WRITE_ADDRESS, Address);
  registeredWrite(AddrDctrl::MODULE_ADDR, AddrDctrl::WRITE_CTRL, writeCmd);
  registeredRead(AddrDctrl::MODULE_ADDR, AddrDctrl::READ_STATUS);
  registeredRead(AddrDctrl::MODULE_ADDR, AddrDctrl::READ_DATA);

  flush();

  auto results = readResults();
  if (results.size() != 2) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: ChipRead: expected 2 results, got "
                << results.size() << "\n";
    return -1;
  }
  Value = results[1].data;

  // Check return status and chip id
  uint16_t status_reg = results[0].data; // State + chipid
  uint8_t chipid_read = status_reg & 0x7F;
  uint8_t status = (status_reg >> 7) & 0x3F;
  if (status != TReadoutBoardRU::CHIP_READ_STATUS_OK) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: ChipRead: Return status NOK, got 0x"
                << std::hex << status << "\n";
    return -1;
  }
  if (chipid_read != chipID) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: ChipRead: ChipID read NOK, expected "
                << chipID << ", got " << chipid_read << "\n";
    return -2;
  }

  return 0;
}

int TReadoutBoardRU::SendOpCode(uint16_t OpCode) {
  registeredWrite(AddrDctrl::MODULE_ADDR, AddrDctrl::WRITE_CTRL,
                  opcode << 8 | 0);
  flush();
}

int TReadoutBoardRU::SendOpCode(uint16_t OpCode, uint8_t chipId) {
  SendOpcode(OpCode);
}

int TReadoutBoardRU::SetTriggerConfig(bool enablePulse, bool enableTrigger,
                                      int triggerDelay, int pulseDelay) {
  m_enablePulse = enablePulse;
  m_enableTrigger = enableTrigger;
  m_triggerDelay = triggerDelay;
  m_pulseDelay = pulseDelay;
}

void TReadoutBoardRU::SetTriggerSource(TTriggerSource triggerSource) {}

int TReadoutBoardRU::Trigger(int nTriggers) {
  for (int i = 0; i < nTriggers; ++i) {
    if (enablePulse)
      registeredWrite(AddrDctrl::MODULE_ADDR, AddrDctrl::WRITE_CTRL,
                      Alpide::OPCODE_PULSE);
    if (m_pulseDelay > 0)
      registeredWrite(AddrDctrl::MODULE_ADDR, ADdrDctrl::WAIT_VALUE,
                      m_pulseDelay);
    if (enableTrigger)
      registeredWrite(AddrDctrl::MODULE_ADDR, AddrDctrl::WRITE_CTRL,
                      Alpide::OPCODE_TRIGGER1);
    if (m_triggerDelay > 0)
      registeredWrite(AddrDctrl::MODULE_ADDR, AddrDctrl::WAIT_VALUE,
                      m_triggerDelay);
  }
  flush();
}

void TReadoutBoardRU::fetchEventData() {
  static UsbDev::DataBuffer buffer;
  for (auto port : { EP_DATA0_IN, EP_DATA1_IN }) {
    // read chunk from USB
    readFromPort(port, TReadoutBoardRU::EVENT_DATA_READ_CHUNK, buffer);

    // Filter, remove status, remove padded bytes, split to dataport
    for (int i = 0; i < buffer.size(); i += 4) {
      uint8_t status = buffer[i + 3];
      uint8_t index = status >> 5;
      uint8_t chip = m_config->getTransceiverChip(port, index);

      for (int j = 0; j < 3; j++) {
        bool isPadded = (status >> (2 + j)) & 1 == 1;
        if (!isPadded) {
          m_readoutBuffers[chip].push_back(buffer[i + j]);
        }
      }
    }
  }
}

int TReadoutBoardRU::ReadEventData(int &NBytes, unsigned char *Buffer) {
  // Todo
  return -1;
}


int TReadoutBoardRU::Initialize() {

}
