#ifndef TEYEANALYSIS_H
#define TEYEANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

typedef struct {
  int verLow;
  int verHigh;
  int horLow;
  int horHigh;
} TEyeCounter;

class TVirtualPad;
class TH2;

class TEyeResultChip : public TScanResultChip {
  friend class TEyeAnalysis;

private:
  int m_verLow;
  int m_verHigh;
  int m_horLow;
  int m_horHigh;

public:
  TEyeResultChip() : TScanResultChip(), m_verLow(0), m_verHigh(0), m_horLow(0), m_horHigh(0){};
  void  WriteToFile(FILE *fp);
  float GetVariable(TResultVariable var)
  {
    (void)var;
    return 0;
  }; // TODO
};


class TEyeResultHic : public TScanResultHic {
  friend class TEyeAnalysis;

private:
  int m_nTimeout;
  int m_n8b10b;
  int m_nOversize;
  int m_nCorrupt;

public:
  TEyeResultHic() : TScanResultHic(){};
  TScanParameters *GetScanParameters() const { return m_scanParameters; }
  void             WriteToFile(FILE *fp);
  void             WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
  {
    (void)db;
    (void)activity;
  }; // TODO
};


class TEyeResult : public TScanResult {
  friend class TEyeAnalysis;

public:
  TEyeResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; }; // TODO
};


class TEyeAnalysis : public TScanAnalysis {
private:
  std::vector<TEyeCounter> m_counters;
  void                     FillVariableList(){};
  THicClassification       GetClassification(TEyeResultHic *result);
  void                     WriteResult();

protected:
  TScanResultChip *GetChipResult()
  {
    TEyeResultChip *Result = new TEyeResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TEyeResultHic *Result = new TEyeResultHic();
    return Result;
  };
  void   CreateResult(){};
  void   AnalyseHisto(TScanHisto *histo);
  string GetPreviousTestType() { return string(""); }; // TODO
  void   InitCounters();
  void   CalculatePrediction(std::string hicName) { (void)hicName; };

public:
  TEyeAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
               std::vector<THic *> hics, std::mutex *aMutex, TEyeResult *aResult = 0);
  void Initialize(); // TODO
  void Finalize();

  static void PlotHisto(TVirtualPad &p, TH2 &h, const std::string &filename = "");
};


#endif
