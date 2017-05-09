#include <iostream>
#include "TFifoTest.h"
#include "AlpideConfig.h"

TFifoTest::TFifoTest (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue, std::mutex *aMutex) 
  : TScan (config, chips, boards, histoQue, aMutex) 
{
  m_start[2] = 0;
  m_step [2] = 1;
  m_stop [2] = m_chips.size();

  m_start[1] = 0; 
  m_step [1] = 1;
  m_stop [1] = 32;

  m_start[0] = 0; 
  m_step [0] = 1;
  m_stop [0] = 128;
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


void TFifoTest::PrepareStep(int loopIndex) 
{

  switch (loopIndex) {
  case 0:    // innermost loop
    break;
  case 1:    // 2nd loop
    break;
  case 2:    // outermost loop: change chip
    m_testChip = m_chips.at(m_value[2]);
    break;
  default: 
    break;
  }
}


bool TFifoTest::TestPattern (int pattern) {

}


void TFifoTest::Execute() 
{
  TestPattern (0xffff);
  TestPattern (0x0);
  TestPattern (0xaaaa);
  TestPattern (0x5555);

}
