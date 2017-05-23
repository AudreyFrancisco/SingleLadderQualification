#ifndef TSCANALYSIS_H
#define TSCANALYSIS_H

#include <deque>
#include <mutex>
#include <map>
#include "THisto.h"
#include "TScan.h"
#include "TScanConfig.h"


class TScanResult {
 public:
  TScanResult () {};
};


class TScanAnalysis {
 protected:
  std::deque <TScanHisto>     *m_histoQue;
  std::vector<TChipIndex>      m_chipList;
  std::mutex                  *m_mutex;
  std::map <int, TScanResult>  m_result;

  TScan                       *m_scan;
  TScanConfig                 *m_config;
  bool                         m_first;
  virtual TScanResult          GetResultObject () = 0;
  void                         CreateResult    ();
 public:
  TScanAnalysis (std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  virtual void Initialize() = 0; 
  virtual void Run       () = 0;
  virtual void Finalize  () = 0; 
  
};




#endif
