#ifndef TDIGITALANALYSIS_H
#define TDIGITALANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "Common.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"
#include "THisto.h"

typedef struct {
  int boardIndex;
  int receiver;
  int chipId;
  int nCorrect;
  int nIneff;
  int nNoisy;
} TDigitalCounter;


class TDigitalResultChip : public TScanResultChip {
 public: 
  TDigitalResultChip () : TScanResultChip () {};
};


class TDigitalResult : public TScanResult {
 public: 
  TDigitalResult () : TScanResult () {};
};


class TDigitalAnalysis : public TScanAnalysis {
 private:
  std::vector <TDigitalCounter> m_counters;
  int                           m_ninj;
  bool HasData          (TScanHisto &histo,  common::TChipIndex idx, int col);
  void InitCounters     ();
  void WriteHitData     (TScanHisto histo, int row); 
  void WriteResult      ();
  void WriteStuckPixels ();
 protected:
  TScanResultChip GetChipResult () {TDigitalResultChip Result; return Result;};
  void            CreateResult  () {};
 public:
  TDigitalAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  
  void Initialize() {};
  void Run       ();
  void Finalize  ();
  
  std::vector <TDigitalCounter> GetCounters() {return m_counters;};
};

#endif
