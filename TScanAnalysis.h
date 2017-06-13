#ifndef TSCANALYSIS_H
#define TSCANALYSIS_H

#include <deque>
#include <mutex>
#include <map>

#include "Common.h"

class THisto;
class TScan;
class TScanConfig;
class TScanHisto;

// base class for classes that contain chip results
// derive class for each analysis
class TScanResultChip {
 public:
  TScanResultChip () {};
};


// base class for classes containing complete results
// derive class for each analysis
class TScanResult {
 private:
 protected: 
  std::map <int, TScanResultChip> m_chipResults;
 public: 
  TScanResult   () {};
  int AddChipResult (common::TChipIndex idx, 
		     TScanResultChip aChipResult);
  int AddChipResult (int aIntIndex, TScanResultChip aChipResult);
  
};


class TScanAnalysis {
 protected:
  std::deque <TScanHisto>         *m_histoQue;
  std::vector<common::TChipIndex>  m_chipList;
  std::mutex                      *m_mutex;
  TScanResult                     *m_result;
  
  TScan                       *m_scan;
  TScanConfig                 *m_config;
  bool                         m_first;
  virtual TScanResultChip      GetChipResult     () = 0;
  void                         CreateChipResults ();
  virtual void                 CreateResult      () = 0;
  int                          ReadChipList      ();
 public:
  TScanAnalysis (std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  virtual void Initialize() = 0; 
  virtual void Run       () = 0;
  virtual void Finalize  () = 0; 
  TScanResult  GetScanResult () {return *m_result;};  
};


#endif
