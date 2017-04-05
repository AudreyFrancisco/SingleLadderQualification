#ifndef TSCANALYSIS_H
#define TSCANALYSIS_H

#include <deque>
#include <mutex>
#include "THisto.h"
#include "TScan.h"
#include "TScanConfig.h"

class TScanAnalysis {
 protected:
  std::deque<TScanHisto> *m_histoQue;
  std::mutex             *m_mutex;
  TScan                  *m_scan;
  TScanConfig            *m_config;
 public:
  TScanAnalysis (std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  virtual void Run() = 0;
};




#endif
