#ifndef TDIGITALANALYSIS_H
#define TDIGITALANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "Common.h"
#include "THisto.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

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
  int                  m_nDead;
  int                  m_nNoisy;
  int                  m_nIneff;
  int                  m_nStuck;
  int                  m_nBadDcols;
  std::vector<TPixHit> m_stuck;

public:
  TDigitalResultChip() : TScanResultChip(){};
  void WriteToFile(FILE *fp);
  float GetVariable(TResultVariable var);
};

class TDigitalResultHic : public TScanResultHic {
  friend class TDigitalAnalysis;

private:
  int           m_nDead;
  int           m_nBad;
  int           m_nStuck;
  int           m_nBadDcols;
  char          m_stuckFile[200];
  bool          m_lower;
  bool          m_upper;
  bool          m_nominal;
  TErrorCounter m_errorCounter;
  void GetParameterSuffix(std::string &suffix, std::string &file_suffix);

public:
  TDigitalResultHic() : TScanResultHic(){};
  void SetStuckFile(const char *fName) { strcpy(m_stuckFile, fName); };
  void WriteToFile(FILE *fp);
  void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TDigitalResult : public TScanResult {
  friend class TDigitalAnalysis;

private:
  int m_nTimeout;
  int m_n8b10b;
  int m_nCorrupt;

public:
  TDigitalResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp);
};

class TDigitalAnalysis : public TScanAnalysis {
private:
  std::vector<TDigitalCounter> m_counters;
  int                          m_ninj;
  bool HasData(TScanHisto &histo, common::TChipIndex idx, int col);
  void InitCounters();
  void FillVariableList();
  void WriteHitData(TScanHisto *histo, int row);
  void WriteResult();
  void WriteStuckPixels(THic *hic);
  THicClassification GetClassificationOB(TDigitalResultHic *result);
  THicClassification GetClassificationIB(TDigitalResultHic *result);

protected:
  TScanResultChip *GetChipResult()
  {
    TDigitalResultChip *Result = new TDigitalResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TDigitalResultHic *Result = new TDigitalResultHic();
    return Result;
  };
  void CreateResult(){};
  void AnalyseHisto(TScanHisto *histo);
  string GetPreviousTestType();

public:
  TDigitalAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                   std::vector<THic *> hics, std::mutex *aMutex, TDigitalResult *aResult = 0);

  void Initialize();
  void Finalize();

  std::vector<TDigitalCounter> GetCounters() { return m_counters; };
};

#endif
