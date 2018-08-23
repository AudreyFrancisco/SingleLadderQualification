#include "TRUv1DctrlModule.h"

#include "TAlpide.h"
#include "TReadoutBoardRUv1.h"

int k = 0;

TRUv1DctrlModule::TRUv1DctrlModule(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging), m_connector(0), m_connectorSet(false)
{
}

void TRUv1DctrlModule::WriteChipRegister(uint16_t Address, uint16_t Value, uint8_t chipId,
                                         bool commit)
{
  uint16_t writeCmd = (Alpide::OPCODE_WROP << 8) | chipId;

  if (Address == 0x60c) {
    k += 1;
    if (k == 3) {
      // TAlpide * fuck = 0;
      // fuck->DumpRegisters();
    }
  }
  Write(TRUv1DctrlModule::WRITE_ADDRESS, Address, false);
  Write(TRUv1DctrlModule::WRITE_DATA, Value, false);
  Write(TRUv1DctrlModule::WRITE_CTRL, writeCmd, commit); // flush
}

int TRUv1DctrlModule::ReadChipRegister(uint16_t Address, uint16_t &Value, uint8_t chipId)
{
  uint16_t readCmd = (Alpide::OPCODE_RDOP << 8) | chipId;

  Write(TRUv1DctrlModule::WRITE_ADDRESS, Address, false);
  Write(TRUv1DctrlModule::WRITE_CTRL, readCmd, false);
  Read(TRUv1DctrlModule::READ_STATUS, false);
  Read(TRUv1DctrlModule::READ_DATA, false);

  m_board.flush();

  auto results = m_board.readResults();
  if (results.size() != 2) {
    if (m_logging)
      std::cout << "TReadoutBoardRUv1: ChipRead: expected 2 results, got " << results.size()
                << "\n";
    return -1;
  }
  Value = results[1].data;


  // Check return status and chip id
  uint16_t status_reg  = results[0].data; // State + chipid
  uint8_t  chipid_read = status_reg & 0x7F;
  uint8_t  status      = (status_reg >> 7) & 0x3F;
  if (status != TRUv1DctrlModule::CHIP_READ_STATUS_OK) {
    if (m_logging)
      std::cout << "TReadoutBoardRUv1: ChipRead: Return status NOK, got 0x" << std::hex
                << (int)status << ", expected: " << (int)TRUv1DctrlModule::CHIP_READ_STATUS_OK
                << std::dec << "\n";
    return -1;
  }
  if (chipid_read != chipId) {
    if (m_logging)
      std::cout << "TReadoutBoardRUv1: ChipRead: ChipID read NOK, expected " << (int)chipId
                << ", got " << (int)chipid_read << "\n";
    return -2;
  }

  return 0;
}

void TRUv1DctrlModule::SendOpCode(uint16_t OpCode, bool commit)
{
  Write(TRUv1DctrlModule::WRITE_CTRL, OpCode << 8 | 0, commit);
}

int TRUv1DctrlModule::SetConnector(uint8_t connector, bool commit)
{
  if (connector >= 5) return -1;

  if (m_connectorSet == false || connector != m_connector) {
    Write(TRUv1DctrlModule::SET_DCTRL_INPUT, connector, commit);
    m_connector    = connector;
    m_connectorSet = true;
  }

  return 0;
}

void TRUv1DctrlModule::Wait(uint16_t waittime, bool commit)
{
  Write(TRUv1DctrlModule::WAIT_VALUE, waittime, commit);
}

// YCM: some function from Python code
void TRUv1DctrlModule::SetManchesterEn(bool en)
{
  Write(TRUv1DctrlModule::MANCHESTER_TX_EN, (en & 0x1), true);
}

void TRUv1DctrlModule::ForcePhase(uint16_t phase)
{
  if (phase > 0x7) {
    std::cout << "Warning: PHASE value > 7, nothing done" << std::endl;
    return;
  }
  Write(TRUv1DctrlModule::PHASE_FORCE, phase, true);
}

void TRUv1DctrlModule::ReleasePhaseForce()
{
  uint16_t phase = Read(TRUv1DctrlModule::PHASE_FORCE, true);
  Write(TRUv1DctrlModule::PHASE_FORCE, (phase & 0x3), true);
  return;
}

bool TRUv1DctrlModule::PhaseIsForce()
{
  uint16_t ret = Read(TRUv1DctrlModule::PHASE_FORCE, true);
  return (ret >> 2) & 0x1;
}

uint16_t TRUv1DctrlModule::GetPhase()
{
  uint16_t ret = Read(TRUv1DctrlModule::PHASE_FORCE, true);
  return (ret & 0x3);
}

void TRUv1DctrlModule::ResetCounters()
{
  Write(TRUv1DctrlModule::RST_CTRL_CNTRS, 0x7F, false);
  Write(TRUv1DctrlModule::RST_CTRL_CNTRS, 0, true);
}

void TRUv1DctrlModule::SetMask(uint16_t mask)
{
  Write(TRUv1DctrlModule::SET_DCTRL_TX_MASK, mask);
  usleep(100);
  if (Read(TRUv1DctrlModule::SET_DCTRL_TX_MASK) != mask) std::cout << "DCTRL MASK UNSUCCESSFUL \n";
}


void TRUv1DctrlModule::LatchCounters() { Write(TRUv1DctrlModule::LATCH_CTRL_CNTRS, 0x7F, true); }
void TRUv1DctrlModule::CoutAllCounters()
{


  std::cout << dec;
  std::cout << ".....READING ALL COUNTERS ON DCTRL MODULE.....\n";
  std::cout << "READ_BROADCAST_CNTR = " << Read(TRUv1DctrlModule::READ_BROADCAST_CNTR) << std::endl;
  std::cout << "READ_WRITE_CNTR = " << Read(TRUv1DctrlModule::READ_WRITE_CNTR) << std::endl;
  std::cout << "READ_READ_CNTR = " << Read(TRUv1DctrlModule::READ_READ_CNTR) << std::endl;
  std::cout << "READ_OPCODE_CNTR = " << Read(TRUv1DctrlModule::READ_OPCODE_CNTR) << std::endl;
  std::cout << "READ_TRIGGER_SENT_CNTR = " << Read(TRUv1DctrlModule::READ_TRIGGER_SENT_CNTR)
            << std::endl;
  std::cout << "READ_TRIGGER_NOT_SENT_CNTR = " << Read(TRUv1DctrlModule::READ_TRIGGER_NOT_SENT_CNTR)
            << std::endl;
  std::cout << ".....ALL COUNTERS READ ON DCTRL MODULE..... \n";
}
