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
  friend class TDigitalAnalysis;
 private: 
  int m_nDead;
  int m_nNoisy;
  int m_nIneff;
  int m_nStuck;
  int m_nBadDcols;
  std::vector <TPixHit> m_stuck;
 public:
  TDigitalResultChip () : TScanResultChip () {};
  void WriteToFile (FILE *fp);
};


class TDigitalResult : public TScanResult {
  friend class TDigitalAnalysis;
 private: 
  int m_nTimeout;
  int m_n8b10b;
  int m_nCorrupt;
 public: 
  TDigitalResult () : TScanResult () {};
  //virtual TDigitalResult* clone() const;
//  TDigitalResult(const TDigitalResult &other):TScanResult(other){/*Body of copy constructor of the TDigitalResult copy constructor*/ }
 // TDigitalResult& operator=(const TDigitalResult& other);
//TDigitalResult& operator=(const TDigitalResult& other){/*handle self assignmet*/ if (&other!=this) return *this;/*handle base class assignemnt*/ TScanResult::operator=(other); return *this;}
  void WriteToFileGlobal (FILE *fp);
  void WriteToDB         (const char *hicID) {};
};


class TDigitalAnalysis : public TScanAnalysis {
 private:
  std::vector <TDigitalCounter> m_counters;
  int                           m_ninj;
  bool HasData          (TScanHisto &histo,  common::TChipIndex idx, int col);
  void InitCounters     ();
  void FillVariableList ();
  void WriteHitData     (TScanHisto histo, int row); 
  void WriteResult      ();
  void WriteStuckPixels ();
 protected:
  TScanResultChip *GetChipResult () {TDigitalResultChip *Result = new TDigitalResultChip; return Result;};
  void             CreateResult  () {};
 public:
  TDigitalAnalysis(std::deque<TScanHisto> *histoQue, 
                   TScan                  *aScan, 
                   TScanConfig            *aScanConfig, 
                   std::vector <THic*>     hics,
                   std::mutex             *aMutex);
  
  void Initialize();
  void Run       ();
  void Finalize  ();
  
  std::vector <TDigitalCounter> GetCounters() {return m_counters;};
};


#endif
