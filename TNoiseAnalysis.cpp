#include <iostream>
#include <vector>
#include "TNoiseAnalysis.h"

TNoiseAnalysis::TNoiseAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(histoQue, aScan, aScanConfig, aMutex) 
{
  m_nTrig    = m_config->GetParamValue("NTRIG");
  m_noiseCut = m_nTrig / m_config->GetParamValue("NOISECUT_INV");
}


void TNoiseAnalysis::WriteResult() 
{
}


void TNoiseAnalysis::Run() 
{
  while (m_histoQue->size() == 0) {
    sleep(1);
  }

  while ((m_scan->IsRunning() || (m_histoQue->size() > 0))) {
    if (m_histoQue->size() > 0) {
      while (!(m_mutex->try_lock()));
    
      TScanHisto histo = m_histoQue->front();
      histo.GetChipList (m_chipList);

      m_histoQue->pop_front();
      m_mutex   ->unlock   ();

      for (int ichip = 0; ichip < m_chipList.size(); ichip ++) {
      }
    }
   
    else usleep(300);
  }
}
