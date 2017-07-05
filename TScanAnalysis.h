#ifndef TSCANALYSIS_H
#define TSCANALYSIS_H

#include <deque>
#include <mutex>
#include <map>

#include "Common.h"

class THisto;
class TScan;
class TScanConfig;
class TScanHisto;

// base class for classes that contain chip results
// derive class for each analysis
class TScanResultChip {
 public:
  TScanResultChip () {};
};


// base class for classes containing complete results
// derive class for each analysis
class TScanResult {
 private:
 protected: 
  std::map <int, TScanResultChip*> m_chipResults;
 public: 
  TScanResult   () {};
  //virtual TScanResult *clone() const=0;
 //TScanResult(const TScanResult &other){m_chipResults=other.m_chipResults;}
 //assignment operation from my base class 
//TScanResult &operator=(const TScanResult &other){if (&other!=this) return *this; m_chipResults=other.m_chipResults; return *this;} 
  int              AddChipResult (common::TChipIndex idx, 
		                  TScanResultChip *aChipResult);
  int              AddChipResult (int aIntIndex, TScanResultChip *aChipResult);
  int              GetNChips     ()     {return m_chipResults.size();};
  virtual void     WriteToFile   (const char *fName) = 0;
  virtual void     WriteToDB     (const char *hicID) = 0;
  TScanResultChip *GetChipResult (common::TChipIndex idx);
};


typedef enum resultType {status, deadPix, noisyPix, ineffPix, badDcol, thresh, noise, threshRms, noiseRms, noiseOcc, fifoErr0, fifoErrf, fifoErra, fifoErr5} TResultVariable;


class TScanAnalysis {
 protected:
  std::deque <TScanHisto>         *m_histoQue;
  std::vector<common::TChipIndex>  m_chipList;
  std::mutex                      *m_mutex;
  std::map <const char *, TResultVariable> m_variableList;
  TScan                       *m_scan;
  TScanConfig                 *m_config;
  TScanResult                 *m_result;
  bool                         m_first;
  virtual TScanResultChip     *GetChipResult     () = 0;
  void                         CreateChipResults ();
  virtual void                 CreateResult      () = 0;
  int                          ReadChipList      ();
 public:
  TScanAnalysis (std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  virtual void Initialize      () = 0; 
  virtual void Run             () = 0;
  virtual void Finalize        () = 0; 
  std::map <const char *, TResultVariable> GetVariableList () {return m_variableList;}
};


#endif
