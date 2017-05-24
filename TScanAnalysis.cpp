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
  m_first    = true;
  m_chipList.clear ();
}


void TScanAnalysis::CreateChipResults () 
{
  if (m_chipList.size() == 0) {
    std::cout  << "Warning (TScanAnalysis::CreateResult): chip list is empty, doing nothing" << std::endl;
    return;
  }

  for (int i = 0; i < m_chipList.size(); i ++) {
    TScanResultChip result = GetChipResult();
    TChipIndex      idx    = m_chipList.at(i);
    int             id     = (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf);
    m_chipResults.insert(std::pair<int, TScanResultChip> (id, result));
  }
  
}
