#include "TDCTRLMeasurement.h"
#include "AlpideConfig.h"
#include <exception>
#include <iostream>
#include <string.h>
#include <string>

TDctrlMeasurement::TDctrlMeasurement(TScanConfig *config, std::vector<TAlpide *> chips,
                                     std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                                     std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  strcpy(m_name, "Dctrl Measurement"); // Display name
  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  // 2nd loop: loop over all chips
  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = m_chips.size();

  // innermost loop: loop over all possible driver settings
  m_start[0] = 0;
  m_step[0]  = 1;
  m_stop[0]  = 15;

  CreateScanHisto();
}


THisto TDctrlMeasurement::CreateHisto()
{
  // save result in a 1-d histogram: x-axis 0..15 corresponds to dctrl setting
  // if second variable needed (rms?) expand to 2-d histo:
  // THisto histo("ErrorHisto", "ErrorHisto", 16, 0, 15, 2, 0, 1);
  THisto histo("ErrorHisto", "ErrorHisto", 16, 0, 15);
  return histo;
}


void TDctrlMeasurement::InitScope()
{
  // put here the initial setup of the scope
}


void TDctrlMeasurement::Init()
{
  TScan::Init();

  InitScope();

  // initial chip configurations, modify if necessary
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }

  for (unsigned int i = 0; i < m_chips.size(); i++) {
    AlpideConfig::ConfigureCMU(m_chips.at(i));
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }
}


int TDctrlMeasurement::GetChipById(std::vector<TAlpide *> chips, int id)
{
  for (unsigned int i = 0; i < chips.size(); i++) {
    if (chips.at(i)->GetConfig()->GetChipId() == id) return i;
  }

  return -1;
}


// prepare step prepares the loop step
// loopIndex 0 (innermost): set the dctrl driver value of the test chip
// loopIndex 1: change the chip under test
void TDctrlMeasurement::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop
    m_testChip->GetConfig()->SetParamValue("DCTRLDRIVER", m_value[0]);
    AlpideConfig::ConfigureBuffers(m_testChip, m_testChip->GetConfig());
    break;
  case 1: // 2nd loop
    m_testChip   = m_chips.at(m_value[1]);
    m_boardIndex = FindBoardIndex(m_testChip);
    sprintf(m_state, "Running %d", m_value[1]);
    break;
  case 2:
    break;
  default:
    break;
  }
}

void TDctrlMeasurement::WriteMem(TAlpide *chip, int ARegion, int AOffset, int AValue)
{
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "WriteMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;

  uint16_t LowVal  = AValue & 0xffff;
  uint16_t HighVal = (AValue >> 16) & 0xff;

  int err           = chip->WriteRegister(LowAdd, LowVal);
  if (err >= 0) err = chip->WriteRegister(HighAdd, HighVal);

  if (err < 0) {
    std::cout << "Cannot write chip register. Exiting ... " << std::endl;
    exit(1);
  }
}

void TDctrlMeasurement::ReadMem(TAlpide *chip, int ARegion, int AOffset, int &AValue,
                                bool &exception)
{
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "ReadMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;

  uint16_t LowVal, HighVal;
  int      err;

  try {
    err = chip->ReadRegister(LowAdd, LowVal);
  }
  catch (std::exception &e) {
    exception = true;
    // std::cout << "Exception " << e.what() << " when reading low value" << std::endl;
    return;
  }
  exception = false;
  if (err >= 0) {
    try {
      err = chip->ReadRegister(HighAdd, HighVal);
    }
    catch (std::exception &e) {
      // std::cout << "Exception " << e.what() << " when reading high value" << std::endl;
      exception = true;
      return;
    }
  }

  if (err < 0) {
    std::cout << "Cannot read chip register. Exiting ... " << std::endl;
    exit(1);
  }

  // Note to self: if you want to shorten the following lines,
  // remember that HighVal is 16 bit and (HighVal << 16) will yield 0
  // :-)
  AValue = (HighVal & 0xff);
  AValue <<= 16;
  AValue |= LowVal;
}


/// Old test measurement from FIFO scan, adapt if necessary
bool TDctrlMeasurement::TestPattern(int pattern, bool &exception)
{
  int readBack;
  WriteMem(m_testChip, m_value[1], m_value[0], pattern);
  ReadMem(m_testChip, m_value[1], m_value[0], readBack, exception);
  if (exception) return false;
  if (readBack != pattern) return false;
  return true;
}

void TDctrlMeasurement::LoopEnd(int loopIndex)
{
  if (loopIndex == 2) {
    while (!(m_mutex->try_lock()))
      ;
    m_histoQue->push_back(*m_histo);
    m_mutex->unlock();
    m_histo->Clear();
  }
}


// Execute does the actual measurement
// this method has to implement the amplitude measurement for one single chip,
// for one driver setting
// results are saved into m_histo as described below
void TDctrlMeasurement::Execute()
{
  common::TChipIndex idx;
  idx.boardIndex   = m_boardIndex;
  idx.chipId       = m_testChip->GetConfig()->GetChipId();
  idx.dataReceiver = m_testChip->GetConfig()->GetParamValue("RECEIVER");

  // skip disabled chips and OB slaves
  if ((m_testChip->GetConfig()->IsEnabled()) &&
      (m_testChip->GetConfig()->GetParamValue("LINKSPEED") != -1)) {

    // Do the measurement here, value has to be saved in the histogram
    // with THisto::Set, idx indicates the chip, e.g.
    // m_histo->Set(idx, m_value[1], measured amplitude)
    // to enter the measured amplitude for the current chip and the current
    // driver setting

    // here only to avoid error "idx set but not used"
    // remove in implementation
    m_histo->Set(idx, m_value[1], 0);
  }
}

void TDctrlMeasurement::Terminate()
{
  TScan::Terminate();

  m_running = false;
}