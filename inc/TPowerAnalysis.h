#ifndef TPOWERANALYSIS_H
#define TPOWERANALYSIS_H

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"


class TPowerResultChip : public TScanResultChip {
 public: 
  TPowerResultChip () : TScanResultChip () {};
  void  WriteToFile (FILE *fp)            {};
  float GetVariable (TResultVariable var) {return 0;};
};


class TPowerResultHic : public TScanResultHic {
  friend class TPowerAnalysis;
 private:
  float    iddaSwitchon;
  float    idddSwitchon;
  float    iddaClocked;
  float    idddClocked;
  float    iddaConfigured;
  float    idddConfigured;
  float    ibias0;
  float    ibias3;
 protected:
 public:
  TPowerResultHic () : TScanResultHic () {};
  void WriteToFile (FILE *fp);
};


class TPowerResult : public TScanResult {
  friend class TPowerAnalysis;
 private:
 protected:
 public:
  TPowerResult () : TScanResult() {};
  void WriteToFileGlobal (FILE *fp) {};
  void WriteToDB         (const char *hicID) {};
};


class TPowerAnalysis : public TScanAnalysis {
 private:
  THicClassification GetClassification   (THicCurrents currents);
  THicClassification GetClassificationIB (THicCurrents currents);
  THicClassification GetClassificationOB (THicCurrents currents);
 protected:
  TScanResultChip *GetChipResult() {TPowerResultChip *result = new TPowerResultChip(); return result;};
  TScanResultHic  *GetHicResult () {TPowerResultHic  *result = new TPowerResultHic();  return result;};
  void             CreateResult () {};
  void             InitCounters () {};
  void             WriteResult  ();
  void             AnalyseHisto (TScanHisto *histo) {};
 public:
  TPowerAnalysis(std::deque<TScanHisto> *histoQue, 
                 TScan                  *aScan, 
                 TScanConfig            *aScanConfig, 
                 std::vector <THic*>     hics,
                 std::mutex             *aMutex, 
                 TPowerResult           *aResult = 0);
  void Initialize () {CreateHicResults();};
  void Run        () {};
  void Finalize   ();
};

#endif
