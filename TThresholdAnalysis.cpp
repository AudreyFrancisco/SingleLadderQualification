#include <iostream>
#include <vector>
#include "TThresholdAnalysis.h"

TThresholdAnalysis::TThresholdAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(histoQue, aScan, aScanConfig, aMutex) 
{
}


void TThresholdAnalysis::Run() 
{

  std::vector <TChipIndex> chipList;

  while (m_histoQue->size() == 0) {
    sleep(1);
  }

  while (m_scan->IsRunning()) {
    if (m_histoQue->size() > 0) {
      while (!(m_mutex->try_lock()));
    
      TScanHisto histo = m_histoQue->front();
      histo.GetChipList(chipList);

      m_histoQue->pop_front();
      m_mutex   ->unlock();

      for (int ichip = 0; ichip < chipList.size(); ichip++) {
        for (int i = 0; i < 50; i ++) {
          std::cout << histo(chipList.at(ichip), 0, i) << " ";
        }
	std::cout << std::endl;
      }
     
      std::cout << "In Scan analysis <<=========================================" << std::endl;
      std::cout << std::endl;
    }
  }

}
