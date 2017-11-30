#ifndef TREADOUTANALYSIS_H
#define TREADOUTANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>
#include <map>

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"
#include "Common.h"
#include "AlpideDecoder.h"


class TReadoutResultChip : public TScanResultChip {
friend class TReadoutAnalysis;
 private: 
 public:
  TReadoutResultChip () : TScanResultChip () {};
};



class TReadoutResultHic : public TScanResultHic {
friend class TReadoutAnalysis;
 private: 
  TErrorCounter m_errorCounter;
 public:
  TReadoutResultHic () : TScanResultHic () {};
};


class TReadoutResult : public TScanResult {
friend class TReadoutAnalysis;
 private: 
 public:
  TReadoutResult () : TScanResult () {};
  void WriteToFileGlobal (FILE *fp) { (void)fp; };
};


class TReadoutAnalysis : public TScanAnalysis {
 private:
  int  m_nTrig; 
  int  m_occ;
  void FillVariableList () {};
 protected: 
 public:
  TReadoutAnalysis(std::deque<TScanHisto> *histoQue,
                   TScan                  *aScan,
                   TScanConfig            *aScanConfig,
                   std::vector <THic*>     hics,
                   std::mutex             *aMutex,
                   TReadoutResult         *aResult = 0);
  void Initialize ();
  void Finalize   ();
};



#endif
