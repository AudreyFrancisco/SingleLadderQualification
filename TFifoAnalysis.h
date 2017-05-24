#ifndef TFIFOANALYSIS_H
#define TFIFOANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"


typedef struct {
  int boardIndex;
  int receiver;
  int chipId;
  int err0;
  int err5;
  int erra;
  int errf;
} TFifoCounter;


class TFifoResultChip : public TScanResultChip {
 public: 
  TFifoResultChip () : TScanResultChip () {};
};


class TFifoResult : public TScanResult {
 public: 
  TFifoResult () : TScanResult () {};
};


class TFifoAnalysis : public TScanAnalysis {
 private:
  std::vector <TFifoCounter> m_counters; 
  void InitCounters ();
  void WriteResult  ();
 protected:
  TScanResultChip GetChipResult () {TFifoResultChip Result; return Result;};
  void            CreateResult  () {};
 public:
  TFifoAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  void Initialize () {};
  void Run        ();
  void Finalize   ();
 };

#endif
