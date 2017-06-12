#ifndef TNOISEANALYSIS_H
#define TNOISEANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"


class TNoiseAnalysis : public TScanAnalysis {
 private: 
  int   m_nTrig;
  float m_noiseCut;
  void  WriteResult();
 public: 
  TNoiseAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  void Initialize () {};
  void Run        ();
  void Finalize   () { WriteResult(); };  
};



#endif
