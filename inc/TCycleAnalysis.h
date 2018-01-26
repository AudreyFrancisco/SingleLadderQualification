#ifndef TCYCLEANALYSIS_H
#define TCYCLEANALYSIS_H

#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TCycleResultChip : public TScanResultChip {
public:
  TCycleResultChip() : TScanResultChip(){};
  void WriteToFile(FILE *fp) { (void)fp; };
  float GetVariable(TResultVariable var);
};

class TCycleResultHic : public TScanResultHic {
  friend class TCycleAnalysis;

private:
  int m_nTrips;
  int m_minWorkingChips;
  int m_nChipFailures;
  float m_avDeltaT;
  float m_maxDeltaT;
  float m_avIdda;
  float m_maxIdda;
  float m_minIdda;
  float m_avIddd;
  float m_maxIddd;
  float m_minIddd;
  char m_cycleFile[200];
  void SetCycleFile(const char *fName) { strcpy(m_cycleFile, fName); };

protected:
public:
  TCycleResultHic() : TScanResultHic(){};
  void WriteToFile(FILE *fp);
};

class TCycleResult : public TScanResult {
  friend class TCycleAnalysis;

private:
  int m_nCycles;

protected:
public:
  TCycleResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp);
};

class TCycleAnalysis : public TScanAnalysis {
private:
protected:
  TScanResultChip *GetChipResult() {
    TCycleResultChip *Result = new TCycleResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult() {
    TCycleResultHic *Result = new TCycleResultHic();
    return Result;
  };
  void CreateResult(){};
  void InitCounters();
  void WriteResult();
  void AnalyseHisto(TScanHisto *histo) { (void)histo; };
  string GetPreviousTestType() { return string(""); }; // done only once
public:
  TCycleAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                 std::vector<THic *> hics, std::mutex *aMutex, TCycleResult *aResult = 0);
  void Initialize() { CreateHicResults(); };
  void Run(){};
  void Finalize();
};

#endif
