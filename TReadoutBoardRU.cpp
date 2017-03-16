#include <algorithm>

#include "TReadoutBoardRU.h"

int roundUpToMultiple(int numToRound, int multiple)
{
    if (multiple == 0)
        return numToRound;

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;
}


TReadoutBoardRU::TReadoutBoardRU(libusb_device *ADevice,
                                 TBoardConfigRU *config) : m_buffer(), m_readBytes(0), m_logging(false) {}



void TReadoutBoardRU::registeredWrite(uint16_t module, uint16_t address, uint32_t data) {
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
    size_t written = m_usb->writeData(EP_CTL_OUT, m_buffer,USB_TIMEOUT);

    return toWrite == written;
}

void TReadoutBoardRu::readFromPort(uint8_t port, size_t size, UsbDev::DataBuffer &buffer) {
    buffer.resize(roundUpToMultiple(size,1024));
    m_usb->readData(EP_CTL_IN,buffer,USB_TIMEOUT);
}


std::vector<TReadoutBoardRU::ReadResult> readResults() {
    // Read from Control Port
    UsbDev::DataBuffer buffer_all, buffer_single;
    int retries = 0;
    size_t data_left = m_readBytes;
    while(buffer_all.size() < m_readBytes && retries < MAX_RETRIES_READ) {

        readFromPort(EP_CTL_IN,data_left,buffer_single);
        buffer_all.insert(buffer_all.end(), buffer_single.begin(), buffer_single.end());

        if(buffer_all.size() < m_readBytes) {
            data_left = m_readBytes - buffer_all.size();
            ++retries;
        }
    }
    if(buffer_all < m_readBytes) {
        if(m_logging)
            cout << "TReadoutBoardRU: could not read all data from Control Port. Packet dropped";
    }
    m_readBytes = 0;

    // Process read data
    std::vector<TReadoutBoardRU::ReadResult> results;
    for(int i = 0; i < buffer_all.size(); i+=4) {
        uint16_t data = buffer_all[i] | (buffer_all[i+1] << 8);
        uint8_t address = buffer_all[i+2] | (buffer_all[i+3] << 8);
        bool read_error = (address >> 15) & 1 > 0;
        address &= 0x7FFF;
        if(read_error && m_logging) {
            std::cout << "TReadoutBoardRU: Wishbone error while reading: Address " << address << "\n";
        }
        results.emplace_back(address,data,read_error);
    }
    return results;
}



int TReadoutBoardRU::ReadRegister(uint16_t Address, uint32_t &Value) {
    uint16_t module = Address >> 8 & 0xFF;
    uint16_t sub_address = Address & 0xFF;
    registerRead(module,sub_address);
    flush();
    auto results = readResults();
    if(m_logging && results.size() != 1)
        std::cout << "TReadoutBoardRU: Expected 1 result, got " << results.size() << "\n";
    else
        Value = results[0].data;
}
int TReadoutBoardRU::WriteRegister(uint16_t Address, uint32_t Value) {
    uint16_t module = Address >> 8 & 0xFF;
    uint16_t sub_address = Address & 0xFF;
    uint16_t data = Value & 0xFFFF;
    registerWrite(module,sub_address,data);
    flush();
}

int TReadoutBoardRU::WriteChipRegister(uint16_t Address, uint16_t Value,
                                       uint8_t chipId = 0) {}

int TReadoutBoardRU::ReadChipRegister(uint16_t Address, uint16_t &Value,
                                      uint8_t chipID = 0) {}

int TReadoutBoardRU::SendOpCode(uint16_t OpCode) {}
int TReadoutBoardRU::SendOpCode(uint16_t OpCode, uint8_t chipId) {}
int TReadoutBoardRU::SetTriggerConfig(bool enablePulse, bool enableTrigger,
                                      int triggerDelay, int pulseDelay) {}
void TReadoutBoardRU::SetTriggerSource(TTriggerSource triggerSource) {}
int TReadoutBoardRU::Trigger(int nTriggers) {}
int TReadoutBoardRU::ReadEventData(int &NBytes, unsigned char *Buffer) {}
