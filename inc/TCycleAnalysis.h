#ifndef TCYCLEANALYSIS_H
#define TCYCLEANALYSIS_H

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"


class TCycleResultChip : public TScanResultChip {
 public:
  TCycleResultChip () : TScanResultChip () {};
  void  WriteToFile (FILE *fp)             {(void)fp;};
  float GetVariable (TResultVariable var)  {return 0;};  
};


class TCycleResultHic : public TScanResultHic {
  friend class TCycleAnalysis;
 private:
  int   m_nTrips;
  int   m_minWorkingChips;
  int   m_nChipFailures;
  float m_avDeltaT;
  float m_maxDeltaT;
  float m_avIdda;
  float m_maxIdda;
  float m_minIdda;
  float m_avIddd;
  float m_maxIddd;
  float m_minIddd;
 protected:
 public:
  TCycleResultHic () : TScanResultHic () {};
  void WriteToFile (FILE *fp)            {(void) fp;};   // TODO: implement
};


class TCycleResult : public TScanResult {
  friend class TCycleAnalysis;
 private:
  int m_nCycles;
 protected:
 public:
  TCycleResult () : TScanResult() {};
  void WriteToFileGlobal (FILE *fp) {(void)fp;};
};


class TCycleAnalysis : public TScanAnalysis {
 private:
 protected:
  TScanResultChip *GetChipResult () {TCycleResultChip *Result = new TCycleResultChip(); return Result;};
  TScanResultHic  *GetHicResult  () {TCycleResultHic  *Result = new TCycleResultHic (); return Result;};
  void             CreateResult () {};
  void             InitCounters ();
 public:
  TCycleAnalysis(std::deque<TScanHisto> *histoQue,
                 TScan                  *aScan,
                 TScanConfig            *aScanConfig,
                 std::vector <THic*>     hics,
                 std::mutex             *aMutex,
                 TCycleResult           *aResult = 0);
  void Initialize () {CreateHicResults();};
  void Run        () {};
  void Finalize   ();
};



#endif
