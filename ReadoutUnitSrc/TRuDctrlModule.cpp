#include "TRuDctrlModule.h"

#include "TReadoutBoardRU.h"
#include "TAlpide.h"

TRuDctrlModule::TRuDctrlModule(TReadoutBoardRU &board, uint8_t moduleId,
                               bool logging)
  : TRuWishboneModule(board, moduleId, logging),m_connector(0),m_connectorSet(false) {}

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
                << std::hex << (int)status << ", expected: " << (int)TRuDctrlModule::CHIP_READ_STATUS_OK  << std::dec << "\n";
    return -1;
  }
  if (chipid_read != chipId) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: ChipRead: ChipID read NOK, expected "
                << (int)chipId << ", got " << (int)chipid_read << "\n";
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
  if(m_connectorSet == false || connector != m_connector) {
    Write(TRuDctrlModule::SET_DCTRL_INPUT, connector, commit);
    m_connector = connector;
    m_connectorSet=true;
  }

  return 0;
}

void TRuDctrlModule::Wait(uint16_t waittime, bool commit) {
  Write(TRuDctrlModule::WAIT_VALUE, waittime, commit);
}

	//YCM: some function from Python code
void TRuDctrlModule::SetManchesterEn(bool en)
{
	Write(TRuDctrlModule::MANCHESTER_TX_EN, (en & 0x1), true);
}

void TRuDctrlModule::ForcePhase(uint16_t phase)
{
	if (phase > 0x7) {
		std::cout << "Warning: PHASE value > 7, nothing done" << std::endl;
		return;
	}
	Write(TRuDctrlModule::WRITE_PHASE,phase,true);
}

void TRuDctrlModule::ReleasePhaseForce()
{
	uint16_t phase = Read(TRuDctrlModule::WRITE_PHASE,true);
	Write(TRuDctrlModule::WRITE_PHASE,(phase&0x3),true);
	return;
}
	
bool TRuDctrlModule::PhaseIsForce()
{
	uint16_t ret = Read(TRuDctrlModule::WRITE_PHASE,true);
	return (ret >> 2) & 0x1;
}

uint16_t TRuDctrlModule::GetPhase()
{
	uint16_t ret = Read(TRuDctrlModule::WRITE_PHASE,true);
	return (ret & 0x3);
}

