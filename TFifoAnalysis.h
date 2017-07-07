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
  friend class TFifoAnalysis;
 private:
  int m_err0;
  int m_err5;
  int m_erra;
  int m_errf;
 public: 
  TFifoResultChip () : TScanResultChip () {};
};


class TFifoResult : public TScanResult {
  friend class TFifoAnalysis;
 public: 
  TFifoResult () : TScanResult () {};
  void WriteToFile   (const char *fName) {};
  void WriteToDB     (const char *hicID) {};
};


class TFifoAnalysis : public TScanAnalysis {
 private:
  std::vector <TFifoCounter> m_counters; 
  void InitCounters     ();
  void WriteResult      ();
  void FillVariableList ();
 protected:
  TScanResultChip *GetChipResult () {TFifoResultChip *Result = new TFifoResultChip(); return Result;};
  void            CreateResult  () {};
 public:
  TFifoAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  void Initialize ();
  void Run        ();
  void Finalize   ();
 };

#endif
