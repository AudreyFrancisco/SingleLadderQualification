#include <string.h>
#include "TPowerTest.h"

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
}


void TPowerTest::Init()
{
  TScan::Init();
 
  // - switch off power to all HICs
  // - switch off clock of MOSAIC

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
  // switch on power to m_testHic, wait, measure
  // switch on clock, wait, measure
  // configure, wait, measure
  // Switch on bias at 0V, wait, measure
  // change bias to 3V, wait, measure
  // switch off
}


void TPowerTest::Terminate() 
{
  TScan::Terminate();
  m_running = false;
}
