#ifndef TPOWERANALYSIS_H
#define TPOWERANALYSIS_H

#include "TPowerTest.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TPowerResultChip : public TScanResultChip {
public:
  TPowerResultChip() : TScanResultChip(){};
  void WriteToFile(FILE *fp) { (void)fp; };
  float GetVariable(TResultVariable var) {
    (void)(&var);
    return 0;
  };
};

class TPowerResultHic : public TScanResultHic {
  friend class TPowerAnalysis;

private:
  bool trip;
  float iddaSwitchon;
  float idddSwitchon;
  float iddaClocked;
  float idddClocked;
  float iddaConfigured;
  float idddConfigured;
  float ibias0;
  float ibias3;
  float ibias[50];
  char m_ivFile[200];

protected:
public:
  TPowerResultHic() : TScanResultHic(){};
  void SetIVFile(const char *fName) { strcpy(m_ivFile, fName); };
  void WriteToFile(FILE *fp);
  void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TPowerResult : public TScanResult {
  friend class TPowerAnalysis;

private:
protected:
public:
  TPowerResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; };
};

class TPowerAnalysis : public TScanAnalysis {
private:
  void WriteIVCurve(THic *hic);
  THicClassification GetClassification(THicCurrents currents);
  THicClassification GetClassificationIB(THicCurrents currents);
  THicClassification GetClassificationOB(THicCurrents currents);

protected:
  TScanResultChip *GetChipResult() {
    TPowerResultChip *result = new TPowerResultChip();
    return result;
  };
  TScanResultHic *GetHicResult() {
    TPowerResultHic *result = new TPowerResultHic();
    return result;
  };
  void CreateResult(){};
  void InitCounters(){};
  void WriteResult();
  void AnalyseHisto(TScanHisto *histo) { (void)&histo; };
  string GetPreviousTestType();

public:
  TPowerAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                 std::vector<THic *> hics, std::mutex *aMutex, TPowerResult *aResult = 0);
  void Initialize() { CreateHicResults(); };
  void Run(){};
  void Finalize();
};

#endif
