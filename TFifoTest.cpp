#include <iostream>
#include <string.h>
#include "TFifoTest.h"
#include "AlpideConfig.h"

TFifoTest::TFifoTest (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue, std::mutex *aMutex) 
  : TScan (config, chips, boards, histoQue, aMutex) 
{
  strcpy(m_name, "Fifo Scan");
  m_start[2] = 0;
  m_step [2] = 1;
  m_stop [2] = m_chips.size();

  m_start[1] = 0; 
  m_step [1] = 1;
  m_stop [1] = 32;

  m_start[0] = 0; 
  m_step [0] = 1;
  m_stop [0] = 128;

  CreateScanHisto ();
}


THisto TFifoTest::CreateHisto () 
{
  // count errors in bins corresponding to pattern, 
  // e.g. error in pattern 0xaaaa in region 16 -> Incr(16, 10)
  THisto histo ("ErrorHisto", "ErrorHisto", 32, 0 , 31, 16, 0, 15);
  return histo;
}


void TFifoTest::Init() 
{
  for (int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }
  for (int i = 0; i < m_chips.size(); i++) {
    AlpideConfig::ConfigureCMU(m_chips.at(i));
  }
}


int TFifoTest::GetChipById (std::vector <TAlpide*> chips, int id) 
{
  for (int i = 0; i < chips.size(); i++) {
    if (chips.at(i)->GetConfig()->GetChipId() == id) return i;
  }

  return -1;
}


// seems the board index is not accessible anywhere.
// for the time being do like this...
int  TFifoTest::FindBoardIndex (TAlpide *chip) 
{
  for (int i = 0; i < m_boards.size(); i++) {
    if (m_boards.at(i) == chip->GetReadoutBoard()) return i;
  }
  return -1;
}


void TFifoTest::PrepareStep(int loopIndex) 
{

  switch (loopIndex) {
  case 0:    // innermost loop
    break;
  case 1:    // 2nd loop
    break;
  case 2:    // outermost loop: change chip
    m_testChip   = m_chips.at     (m_value[2]);
    m_boardIndex = FindBoardIndex (m_testChip);
    break;
  default: 
    break;
  }
}

void TFifoTest::WriteMem (TAlpide *chip, int ARegion, int AOffset, int AValue) {
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "WriteMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;
  
  uint16_t LowVal  = AValue & 0xffff;
  uint16_t HighVal = (AValue >> 16) & 0xff;

  int err = chip->WriteRegister (LowAdd,  LowVal);
  if (err >= 0) err = chip->WriteRegister (HighAdd, HighVal);
 
  if (err < 0) {
    std::cout << "Cannot write chip register. Exiting ... " << std::endl;
    exit (1);
  }
 
}


void TFifoTest::ReadMem (TAlpide *chip, int ARegion, int AOffset, int &AValue) {
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "ReadMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;
 
  uint16_t LowVal, HighVal;

  int err = chip->ReadRegister (LowAdd, LowVal);
  if (err >= 0) err = chip->ReadRegister (HighAdd, HighVal);

  if (err < 0) {
    std::cout << "Cannot read chip register. Exiting ... " << std::endl;
    exit (1);
  }

  // Note to self: if you want to shorten the following lines, 
  // remember that HighVal is 16 bit and (HighVal << 16) will yield 0 
  // :-)
  AValue = (HighVal & 0xff);
  AValue <<= 16;
  AValue |= LowVal;
}


bool TFifoTest::TestPattern (int pattern) {
  int readBack;
  WriteMem (m_testChip, m_value[1], m_value[0], pattern); 
  ReadMem  (m_testChip, m_value[1], m_value[0], readBack);
  if (readBack != pattern) return false;
  return true;

}


void TFifoTest::LoopEnd(int loopIndex) 
{
  if (loopIndex == 2) {
    while (!(m_mutex->try_lock()));
    m_histoQue->push_back(*m_histo);
    m_mutex   ->unlock();
    m_histo   ->Clear();
  }
}


void TFifoTest::Execute() 
{
  common::TChipIndex idx;
  idx.boardIndex   = m_boardIndex;
  idx.chipId       = m_testChip->GetConfig()->GetChipId    ();
  idx.dataReceiver = m_testChip->GetConfig()->GetParamValue("RECEIVER");

  if (m_testChip->GetConfig()->IsEnabled()) {
    if (!TestPattern (0xffff)) m_histo->Incr(idx, m_value[1], 0xf);
    if (!TestPattern (0x0))    m_histo->Incr(idx, m_value[1], 0x0);;
    if (!TestPattern (0xaaaa)) m_histo->Incr(idx, m_value[1], 0xa);;
    if (!TestPattern (0x5555)) m_histo->Incr(idx, m_value[1], 0x5);;
  }
}
