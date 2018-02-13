#ifndef TSCANALYSIS_H
#define TSCANALYSIS_H

#include <deque>
#include <map>
#include <mutex>
#include <string.h>
#include <string>

//#include "THIC.h"
#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include "Common.h"

class THisto;
class TScan;
class TScanConfig;
class TScanHisto;
class THic;
struct TScanParameters__;
typedef TScanParameters__ TScanParameters;

// Warning: has to stay ordered such, that worse test result = higher entry
typedef enum hicClassification {
  CLASS_UNTESTED,
  CLASS_GREEN,
  CLASS_ORANGE,
  CLASS_PARTIAL,
  CLASS_RED
} THicClassification;

typedef enum {
  status,
  deadPix,
  noisyPix,
  ineffPix,
  stuckPix,
  unmaskablePix,
  badDcol,
  thresh,
  noise,
  noThreshPix,
  hotPix,
  threshRms,
  noiseRms,
  vcasn,
  ithr,
  noiseOcc,
  Err0,
  Errf,
  Erra,
  Err5,
  ErrBusyOn,
  ErrBusyOff
} TResultVariable;

// base class for classes that contain chip results
// derive class for each analysis
class TScanResultChip {
protected:
  string m_outputPath;

public:
  TScanResultChip(){};
  virtual void WriteToFile(FILE *fp)             = 0;
  virtual float GetVariable(TResultVariable var) = 0;
  void SetOutputPath(string path) { m_outputPath = path; };
  string                    GetOutputPath() { return m_outputPath; };
};

class TScanResultHic {
  friend class TScanAnalysis;

protected:
  std::map<int, TScanResultChip *> m_chipResults;
  char               m_resultFile[200];
  THicClassification m_class;
  const char *       WriteHicClassification();
  string             m_outputPath;
  TScanParameters *  m_scanParameters;

public:
  TScanResultHic(){};
  virtual void WriteToFile(FILE *fp) = 0;
  virtual void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
  int AddChipResult(int aChipId, TScanResultChip *aChipResult);
  void SetResultFile(const char *fName) { strcpy(m_resultFile, fName); };
  THicClassification             GetClassification() { return m_class; };
  std::map<int, TScanResultChip *> DeleteThisToo() { return m_chipResults; };
  float GetVariable(int chip, TResultVariable var);
  string GetOutputPath() { return m_outputPath; };
};

// base class for classes containing complete results
// derive class for each analysis
class TScanResult {
private:
protected:
  std::map<int, TScanResultChip *>        m_chipResults;
  std::map<std::string, TScanResultHic *> m_hicResults;

public:
  TScanResult(){};
  virtual ~TScanResult(){};
  // virtual TScanResult *clone() const=0;
  // TScanResult(const TScanResult &other){m_chipResults=other.m_chipResults;}
  // assignment operation from my base class
  // TScanResult &operator=(const TScanResult &other){if (&other!=this) return *this;
  // m_chipResults=other.m_chipResults; return *this;}
  int AddChipResult(common::TChipIndex idx, TScanResultChip *aChipResult);
  int AddChipResult(int aIntIndex, TScanResultChip *aChipResult);
  int AddHicResult(std::string hicId, TScanResultHic *aHicResult);
  int  GetNChips() { return (int)m_chipResults.size(); };
  int  GetNHics() { return (int)m_hicResults.size(); };
  void WriteToFile(const char *fName);
  virtual void WriteToFileGlobal(FILE *fp) = 0;
  void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
  TScanResultChip *GetChipResult(common::TChipIndex idx);
  TScanResultHic *GetHicResult(std::string hic);
  std::map<std::string, TScanResultHic *> GetHicResults() { return m_hicResults; };
};

class TScanAnalysis {
protected:
  std::deque<TScanHisto> *        m_histoQue;
  std::vector<common::TChipIndex> m_chipList;
  std::vector<THic *>             m_hics;
  std::mutex *                    m_mutex;
  std::map<const char *, TResultVariable> m_variableList;
  TScan *                  m_scan;
  TScanConfig *            m_config;
  TScanResult *            m_result;
  bool                     m_first;
  bool                     m_started;
  bool                     m_finished;
  virtual TScanResultChip *GetChipResult() = 0;
  virtual TScanResultHic * GetHicResult()  = 0;
  void                     CreateHicResults();
  virtual void             CreateResult() = 0;
  int                      ReadChipList();
  virtual void AnalyseHisto(TScanHisto *histo) = 0;
  virtual void       InitCounters()            = 0;
  int                GetPreviousActivityType();
  virtual string     GetPreviousTestType() = 0;
  THicClassification DoCut(THicClassification oldClass, THicClassification failClass, float value,
                           float cut, string cutName, bool greaterThan = false);

public:
  TScanAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                std::vector<THic *> hics, std::mutex *aMutex);
  virtual ~TScanAnalysis(){};
  virtual void Initialize() = 0;
  virtual void Run();
  virtual void Finalize() = 0;
  std::map<const char *, TResultVariable> GetVariableList() { return m_variableList; }
  float GetVariable(std::string aHic, int chip, TResultVariable var);
  static const char *WriteHicClassification(THicClassification hicClass);
  THicClassification GetClassification();
};

#endif
