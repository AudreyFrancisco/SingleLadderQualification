#include <iostream>
#include <vector>
#include "TFifoAnalysis.h"

TFifoAnalysis::TFifoAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(histoQue, aScan, aScanConfig, aMutex) 
{
}


void TFifoAnalysis::Run() 
{
  std::vector <TChipIndex> chipList;

  while (m_histoQue->size() == 0) {
    sleep(1);
  }

  while ((m_scan->IsRunning() || (m_histoQue->size() > 0))) {
    if (m_histoQue->size() > 0) {
      while (!(m_mutex->try_lock()));
    
      TScanHisto histo = m_histoQue->front();
      if (m_first) {
        histo.GetChipList(chipList);
        m_first = false;
      }

      m_histoQue->pop_front();
      m_mutex   ->unlock();

      for (int ichip = 0; ichip < chipList.size(); ichip++) {
        for (int ireg = 0; ireg < 32; ireg ++) {
          int errors0 = (int) histo (chipList.at(ichip), ireg, 0x0);
          int errors5 = (int) histo (chipList.at(ichip), ireg, 0x5);
          int errorsa = (int) histo (chipList.at(ichip), ireg, 0xa);
          int errorsf = (int) histo (chipList.at(ichip), ireg, 0xf);
        }
      }
    }
    else usleep (300);
  }
}

