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
  m_stop [2] = m_hics.size();

  m_start[1] = 0; 
  m_step [1] = 1;
  m_stop [1] = 1;

  m_start[0] = 0; 
  m_step [0] = 1;
  m_stop [0] = 1;
}
