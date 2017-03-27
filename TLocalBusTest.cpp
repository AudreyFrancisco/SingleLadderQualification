#include <iostream>
#include "TLocalBusTest.h"
#include "AlpideDebug.h"
#include "AlpideConfig.h"

TLocalBusTest::TLocalBusTest (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue) 
  : TScan (config, chips, boards, histoQue) 
{
  FindDaisyChains(chips);
  m_start[2] = 0;
  m_stop [2] = m_daisyChains.size();
  m_step [2] = 1;

  // stop values here have to be set on the fly as they depend on the number of enabled chips
  m_start[1] = 0; 
  m_step [1] = 1;

  m_start[0] = 0; 
  m_step [0] = 1;
}



void TLocalBusTest::Init() 
{
  for (int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }
  for (int i = 0; i < m_chips.size(); i++) {
    m_chips.at(i)->GetConfig()->SetInitialToken(false);
    m_chips.at(i)->GetConfig()->SetEnableDdr   (false);
    AlpideConfig::ConfigureCMU(m_chips.at(i));
  }
}


int TLocalBusTest::GetChipById (std::vector <TAlpide*> chips, int id) 
{
  for (int i = 0; i < chips.size(); i++) {
    if (chips.at(i)->GetConfig()->GetChipId() == id) return i;
  }

  return -1;
}


int TLocalBusTest::FindDaisyChains(std::vector <TAlpide *> chips) 
{
  int totalChips = 0;
  int iChip      = 0;
  int maxChip    = -1;
  std::vector <TAlpide *> daisyChain;

  while ((totalChips < chips.size()) && (maxChip < (int)chips.size()-1))
  {
    // find next enabled chip and put into new daisy chain vector
    for (iChip = maxChip + 1; (iChip < chips.size()) && (!(chips.at(iChip)->GetConfig()->IsEnabled())); iChip++) {
      totalChips ++;
    }

    daisyChain.push_back(chips.at(iChip));
    totalChips ++;

    // go back through daisy chain until next chip would be iChip
    int chipId = chips.at(iChip)->GetConfig()->GetChipId();
    int iiChip = iChip;

    while (chips.at(iiChip)->GetConfig()->GetPreviousId() != chipId) {
      int previousId = chips.at(iiChip)->GetConfig()->GetPreviousId();

      iiChip         = GetChipById (chips, previousId);
      if (iiChip < 0) {
	std::cout << "Something went wrong, Did not find chip Id" << previousId << std::endl;
        exit(1);
      }
      if (iiChip > maxChip) maxChip = iiChip;
      totalChips ++;
      daisyChain.push_back(chips.at(iiChip));
    }

    m_daisyChains.push_back(daisyChain);
    daisyChain.clear();    
  }

  for (int i = 0; i < m_daisyChains.size(); i++) {
    std::cout << "Daisy chain no " << i << ": " << std::endl;
    for (int ii = 0; ii < m_daisyChains.at(i).size(); ii ++) {
      std::cout << "  chip id " << m_daisyChains.at(i).at(ii)->GetConfig()->GetChipId() << std::endl;
    }
  }
}


void TLocalBusTest::PrepareStep(int loopIndex) 
{

  switch (loopIndex) {
  case 0:    // innermost loop: change read chip
    m_readChip = m_daisyChains.at(m_value[2]).at(m_value[0]);
    break;
  case 1:    // 2nd loop: change write chip
    m_writeChip = m_daisyChains.at(m_value[2]).at(m_value[1]);
    // give token to write chip, take away in loop end

    m_writeChip->ModifyRegisterBits (Alpide::REG_CMUDMU_CONFIG, 4, 1, 1);
    break;
  case 2:    // outermost loop: change daisy chain
    m_stop[0] = m_daisyChains.at(m_value[2]).size();
    m_stop[1] = m_daisyChains.at(m_value[2]).size();
    break;
  default: 
    break;
  }
}


void TLocalBusTest::LoopEnd (int loopIndex) {


}


void TLocalBusTest::Next (int loopIndex) {
  if (loopIndex == 1) {
    m_writeChip->ModifyRegisterBits (Alpide::REG_CMUDMU_CONFIG, 4, 1, 0);
  }
  TScan::Next (loopIndex);
}


void TLocalBusTest::Execute() 
{
  TDMUDebugStream debugStream;
  TBMUDebugStream bmuDebugStream;
  m_writeChip->WriteRegister(REG_TEST_CONTROL, 0x201);

  uint16_t Value;
  m_writeChip->ReadRegister (Alpide::REG_CMUDMU_CONFIG, Value);
  //std::cout << "writeChip CMUDMU config reg = 0x" << std::hex << Value << std::dec << std::endl; 
  for (int iboard = 0; iboard < m_boards.size(); iboard++) {
    m_boards.at(iboard)->SendOpCode(Alpide::OPCODE_DEBUG);
  }
  if (!AlpideDebug::GetDMUDebugStream(m_readChip, debugStream)) {
    std::cout << "Problem reading debug stream" << std::endl;
  }
  if (!AlpideDebug::GetBMUDebugStream(m_readChip, bmuDebugStream)) {
    std::cout << "Problem reading debug stream" << std::endl;
  }
  int busy = (int) bmuDebugStream.BusyInState;
  int readId  = m_readChip->GetConfig()->GetChipId();
  int writeId = m_writeChip->GetConfig()->GetChipId();
   
  m_writeChip->WriteRegister(REG_TEST_CONTROL, 0x0);
  for (int iboard = 0; iboard < m_boards.size(); iboard++) {
    m_boards.at(iboard)->SendOpCode(Alpide::OPCODE_DEBUG);
  }
  if (!AlpideDebug::GetBMUDebugStream(m_readChip, bmuDebugStream)) {
    std::cout << "Problem reading debug stream" << std::endl;
  }
  std::cout << "Write " << writeId << ", read " << readId << ", Value 0x" << std::hex << debugStream.LocalBusValue << ", busy " << busy << ", no busy " << (int) bmuDebugStream.BusyInState << std::dec << std::endl;
}
