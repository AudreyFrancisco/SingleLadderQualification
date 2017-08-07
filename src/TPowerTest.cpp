#include <string.h>
#include "TPowerTest.h"
#include "AlpideConfig.h"

TPowerTest::TPowerTest (TScanConfig                   *config, 
                        std::vector <TAlpide *>        chips, 
                        std::vector <THic*>            hics, 
                        std::vector <TReadoutBoard *>  boards, 
                        std::deque<TScanHisto>        *histoQue, 
                        std::mutex                    *aMutex) 
  : TScan (config, chips, hics, boards, histoQue, aMutex) 
{
  strcpy(m_name, "Power Test");
  m_start[2] = 0;
  m_step [2] = 1;
  m_stop [2] = 1;

  m_start[1] = 0; 
  m_step [1] = 1;
  m_stop [1] = 1;

  m_start[0] = 0; 
  m_step [0] = 1;
  m_stop [0] = m_hics.size();
  
  CreateMeasurements();
}


void TPowerTest::CreateMeasurements()
{
  // create map with measurement structure for each HIC
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    THicCurrents hicCurrents;
    m_hicCurrents.insert (std::pair<std::string, THicCurrents> (m_hics.at(i)->GetDbId(), hicCurrents));
  }
}


void TPowerTest::Init()
{
  TScan::Init();

  // switch power off here or hic-wise in execute?

}


void TPowerTest::PrepareStep(int loopIndex) 
{
  switch (loopIndex) {
  case 0:  // innermost loop: change HIC
    m_testHic = m_hics.at (m_value[0]);
    break;
  case 1:  // 2nd loop
    break;
  case 2:  // outermost loop
    break;
  }
}


void TPowerTest::Execute()
{
  std::vector<int>       boardIndices = m_testHic->GetBoardIndices();
  std::vector<TAlpide*>  chips        = m_testHic->GetChips();
  
  std::map<std::string, THicCurrents>::iterator currentIt = m_hicCurrents.find(m_testHic->GetDbId());

  m_testHic->PowerOff();

  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC*)m_boards.at(i);
    board->enableControlInterfaces(false);
  }

  m_testHic->PowerOn();
  sleep(1);

  // measure -> switchon, no clock
  currentIt->second.idddSwitchon = m_testHic->GetIddd();
  currentIt->second.iddaSwitchon = m_testHic->GetIdda();

  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC*)m_boards.at(i);
    board->enableControlInterfaces(true);
    board->SendOpCode (Alpide::OPCODE_GRST);
  }  

  sleep(1);

  // measure -> Clocked
  currentIt->second.idddClocked = m_testHic->GetIddd();
  currentIt->second.iddaClocked = m_testHic->GetIdda();

  for (unsigned int i = 0; i < chips.size(); i++) {
    if (!(chips.at(i)->GetConfig()->IsEnabled())) continue;
    AlpideConfig::BaseConfig  (chips.at(i));
    AlpideConfig::ConfigureCMU(chips.at(i));
  }
  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC*)m_boards.at(i);
    board->SendOpCode (Alpide::OPCODE_RORST);
  }  
  sleep(1);

  // measure -> Configured
  currentIt->second.idddConfigured = m_testHic->GetIddd();
  currentIt->second.iddaConfigured = m_testHic->GetIdda();

  currentIt->second.ibias0 = m_testHic->GetIBias();

  // TODO: change bias to 3V, wait, measure

  // change bias back to 0 V
  // switch off?
}


void TPowerTest::Terminate() 
{
  TScan::Terminate();
  m_running = false;
}
