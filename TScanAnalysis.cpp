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


