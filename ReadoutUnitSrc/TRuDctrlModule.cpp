#include "TRuDctrlModule.h"

#include "../TReadoutBoardRU.h"
#include "../TAlpide.h"

TRuDctrlModule::TRuDctrlModule(TReadoutBoardRU &board, uint8_t moduleId,
                               bool logging)
    : TRuWishboneModule(board, moduleId, logging) {}

void TRuDctrlModule::WriteChipRegister(uint16_t Address, uint16_t Value,
                                       uint8_t chipId, bool commit) {
  uint16_t writeCmd = (Alpide::OPCODE_WROP << 8) | chipId;

  Write(TRuDctrlModule::WRITE_ADDRESS, Address,false);
  Write(TRuDctrlModule::WRITE_DATA, Value,false);
  Write(TRuDctrlModule::WRITE_CTRL, writeCmd, commit); // flush
}

int TRuDctrlModule::ReadChipRegister(uint16_t Address, uint16_t &Value,
                                     uint8_t chipId) {
  uint16_t readCmd = (Alpide::OPCODE_RDOP << 8) | chipId;

  Write(TRuDctrlModule::WRITE_ADDRESS, Address,false);
  Write(TRuDctrlModule::WRITE_CTRL, readCmd,false);
  Read(TRuDctrlModule::READ_STATUS,false);
  Read(TRuDctrlModule::READ_DATA,false);

  m_board.flush();

  auto results = m_board.readResults();
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
  if (status != TRuDctrlModule::CHIP_READ_STATUS_OK) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: ChipRead: Return status NOK, got 0x"
                << std::hex << (int)status << ", expected: " << (int)TRuDctrlModule::CHIP_READ_STATUS_OK  << "\n";
    return -1;
  }
  if (chipid_read != chipId) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: ChipRead: ChipID read NOK, expected "
                << chipId << ", got " << (int)chipid_read << "\n";
    return -2;
  }

  return 0;
}

void TRuDctrlModule::SendOpCode(uint16_t OpCode, bool commit) {
  Write(TRuDctrlModule::WRITE_CTRL, OpCode << 8 | 0, commit);
}

int TRuDctrlModule::SetConnector(uint8_t connector, bool commit) {
  if (connector >= 5)
    return -1;
  Write(TRuDctrlModule::SET_DCTRL_INPUT, connector, commit);
  return 0;
}

void TRuDctrlModule::Wait(uint16_t waittime, bool commit) {
  Write(TRuDctrlModule::WAIT_VALUE, waittime, commit);
}
