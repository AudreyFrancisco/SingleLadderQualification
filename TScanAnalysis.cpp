#include <iostream>
#include <unistd.h>
#include "TScanAnalysis.h"
#include "THisto.h"

TScanAnalysis::TScanAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aConfig, std::mutex *aMutex) 
{
  m_histoQue = histoQue;
  m_mutex    = aMutex;
  m_scan     = aScan;
  m_config   = aConfig;
}


void TScanAnalysis::Run() 
{

  while (m_histoQue->size() == 0) {
    sleep(1);
  }

  TChipIndex idx = {0, 5, 1};
  while (m_scan->IsRunning()) {
    if (m_histoQue->size() > 0) {
      while (!(m_mutex->try_lock()));
    
      TScanHisto histo = m_histoQue->front();
      m_histoQue->pop_front();
      m_mutex   ->unlock();

      for (int i = 0; i < 50; i ++) {
        std::cout << histo(idx, 0, i) << " ";
      }
      std::cout << "In Scan analysis <<=========================================" << std::endl;
      std::cout << std::endl;
    }
  }

}



