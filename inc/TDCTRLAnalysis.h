#ifndef TDCTRLANALYSIS_H
#define TDCTRLANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"


class TDctrlResultChip : public TScanResultChip {
  friend class TDctrlAnalysis;

private:
public:
  TDctrlResultChip() : TScanResultChip(){};
  void WriteToFile(FILE *fp);
  float GetVariable(TResultVariable var);
};

class TDctrlResultHic : public TScanResultHic {
  friend class TDctrlAnalysis;

private:
public:
  TDctrlResultHic() : TScanResultHic(){};
  void WriteToFile(FILE *fp);
  void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TDctrlResult : public TScanResult {
  friend class TDctrlAnalysis;

public:
  TDctrlResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; };
};

class TDctrlAnalysis : public TScanAnalysis {
private:
  void               WriteResult();
  void               FillVariableList();
  THicClassification GetClassification(TDctrlResultHic *result);

protected:
  TScanResultChip *GetChipResult()
  {
    TDctrlResultChip *Result = new TDctrlResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TDctrlResultHic *Result = new TDctrlResultHic();
    return Result;
  };
  void CreateResult(){};
  void AnalyseHisto(TScanHisto *histo);
  string GetPreviousTestType();
  void   InitCounters();

public:
  TDctrlAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                 std::vector<THic *> hics, std::mutex *aMutex, TDctrlResult *aResult = 0);
  void Initialize();
  void Finalize();
};

#endif
