#ifndef TSCANALYSIS_H
#define TSCANALYSIS_H

#include <deque>
#include <mutex>
#include <map>
#include <string>
#include <string.h>

#include "THIC.h"
#include "Common.h"

class THisto;
class TScan;
class TScanConfig;
class TScanHisto;

enum THicClassification {CLASS_UNTESTED, CLASS_GREEN, CLASS_RED, CLASS_ORANGE};

typedef enum resultType {status, deadPix, noisyPix, ineffPix, badDcol, thresh, noise, noThreshPix, threshRms, noiseRms, vcasn, ithr, noiseOcc, Err0, Errf, Erra, Err5, ErrBusyOn, ErrBusyOff} TResultVariable;


// base class for classes that contain chip results
// derive class for each analysis
class TScanResultChip {
 public:
  TScanResultChip () {};
  virtual void  WriteToFile (FILE *fp) = 0;
  virtual float GetVariable (TResultVariable var) = 0;
};


class TScanResultHic {
  friend class TScanAnalysis;
 protected: 
  std::map <int, TScanResultChip*> m_chipResults;
  char                             m_resultFile[200];
  THicClassification               m_class;
  const char*                      WriteHicClassification();
 public:
  TScanResultHic () {};
  virtual void       WriteToFile       (FILE *fp) = 0;
  int                AddChipResult     (int aChipId, TScanResultChip *aChipResult);
  void               SetResultFile     (const char *fName) {strcpy(m_resultFile, fName);};
  THicClassification GetClassification ()                  {return m_class;};
  std::map <int, TScanResultChip*> DeleteThisToo() {return m_chipResults;};
  float              GetVariable       (int chip, TResultVariable var);
};


// base class for classes containing complete results
// derive class for each analysis
class TScanResult {
 private:
 protected: 
  std::map <int, TScanResultChip*>        m_chipResults;
  std::map <std::string, TScanResultHic*> m_hicResults;
 public: 
  TScanResult   () {};
  virtual ~TScanResult() {};
  //virtual TScanResult *clone() const=0;
 //TScanResult(const TScanResult &other){m_chipResults=other.m_chipResults;}
 //assignment operation from my base class 
//TScanResult &operator=(const TScanResult &other){if (&other!=this) return *this; m_chipResults=other.m_chipResults; return *this;} 
  int              AddChipResult     (common::TChipIndex idx, 
		                      TScanResultChip *aChipResult);
  int              AddChipResult     (int aIntIndex, TScanResultChip *aChipResult);
  int              AddHicResult      (std::string hicId,   TScanResultHic  *aHicResult);
  int              GetNChips         ()     {return (int) m_chipResults.size();};
  int              GetNHics          ()     {return (int) m_hicResults.size();};
  void             WriteToFile       (const char *fName);
  virtual void     WriteToFileGlobal (FILE *fp)          = 0;
  virtual void     WriteToDB         (const char *hicID) = 0;
  TScanResultChip *GetChipResult     (common::TChipIndex idx);
  TScanResultHic  *GetHicResult      (std::string hic);
  std::map <std::string, TScanResultHic*> GetHicResults () {return m_hicResults;};  
};



class TScanAnalysis {
 protected:
  std::deque <TScanHisto>         *m_histoQue;
  std::vector<common::TChipIndex>  m_chipList;
  std::vector<THic *>              m_hics;
  std::mutex                      *m_mutex;
  std::map <const char *, TResultVariable> m_variableList;
  TScan                       *m_scan;
  TScanConfig                 *m_config;
  TScanResult                 *m_result;
  bool                         m_first;
  bool                         m_started;
  bool                         m_finished;
  virtual TScanResultChip     *GetChipResult     () = 0;
  virtual TScanResultHic      *GetHicResult      () = 0;
  void                         CreateHicResults  ();
  virtual void                 CreateResult      () = 0;
  int                          ReadChipList      ();
  virtual void                 AnalyseHisto      (TScanHisto *histo) = 0;
  virtual void                 InitCounters      () = 0;
 public:
  TScanAnalysis (std::deque<TScanHisto> *histoQue, 
                 TScan                  *aScan, 
                 TScanConfig            *aScanConfig, 
                 std::vector <THic*>     hics,
                 std::mutex             *aMutex);
  virtual ~TScanAnalysis() {};
  virtual void Initialize      () = 0;
  virtual void Run             ();
  virtual void Finalize        () = 0;
  std::map <const char *, TResultVariable> GetVariableList () {return m_variableList;}
  float   GetVariable(std::string aHic, int chip, TResultVariable var);
};


#endif
