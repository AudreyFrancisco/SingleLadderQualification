#ifndef TNOISEANALYSIS_H
#define TNOISEANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>
#include <map>

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"
#include "Common.h"
#include "AlpideDecoder.h"

class TNoiseResultChip : public TScanResultChip {
 private: 
  std::vector <TPixHit> m_noisyPixels;
  float                 m_occ;
 public: 
  TNoiseResultChip () : TScanResultChip () {};
  void AddNoisyPixel (TPixHit pixel) {m_noisyPixels.push_back(pixel);};
  void SetOccupancy  (float occ)     {m_occ = occ;};
};


class TNoiseResult : public TScanResult {
 private: 
  std::map <common::TChipIndex, TNoiseResultChip> m_chipResult;
 public: 
  TNoiseResult () : TScanResult () {};
  TNoiseResultChip *GetChipResult (common::TChipIndex chip) {};
};


class TNoiseAnalysis : public TScanAnalysis {
 private: 
  int          m_nTrig;
  float        m_noiseCut;
  TNoiseResult m_result;
  void         WriteResult();
 protected: 
  TScanResultChip GetChipResult () {TNoiseResultChip Result; return Result;};  
  void            CreateResult  () {};
 public: 
  TNoiseAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  void Initialize ();
  void Run        ();
  void Finalize   () { WriteResult(); };  
};



#endif
