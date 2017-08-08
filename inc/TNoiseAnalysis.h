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
  friend class TNoiseAnalysis;
  friend class TApplyMask;
 private: 
  std::vector <TPixHit> m_noisyPixels;
  double                m_occ;
 public: 
  TNoiseResultChip () : TScanResultChip () {};
  void AddNoisyPixel (TPixHit pixel) {m_noisyPixels.push_back(pixel);};  
  void SetOccupancy  (double occ)    {m_occ = occ;};
  void WriteToFile   (FILE *fp);
  std::vector <TPixHit> GetNoisyPixels() {return m_noisyPixels;};
};


class TNoiseResultHic : public TScanResultHic {
  friend class TNoiseAnalysis;
  friend class TApplyMask;
 private:
  double m_occ;
  int    m_nNoisy;
  char   m_noisyFile[200];
 public: 
  TNoiseResultHic () : TScanResultHic () {};
  void SetNoisyFile (const char *fName) {strcpy(m_noisyFile, fName);};
  void WriteToFile  (FILE *fp);
};


class TNoiseResult : public TScanResult {
  friend class TNoiseAnalysis;
  friend class TApplyMask;
 private: 
  std::vector <TPixHit> m_noisyPixels;
 public: 
  TNoiseResult () : TScanResult () {};
  void WriteToFileGlobal (FILE *fp)          {};
  void WriteToDB         (const char *hicID) {};
};


class TNoiseAnalysis : public TScanAnalysis {
 private: 
  int          m_nTrig;
  float        m_noiseCut;
  void         WriteResult      ();
  void         FillVariableList ();
  void         WriteNoisyPixels (THic *hic);
 protected: 
  TScanResultChip *GetChipResult () {TNoiseResultChip *Result = new TNoiseResultChip(); return Result;};  
  TScanResultHic  *GetHicResult  () {TNoiseResultHic  *Result = new TNoiseResultHic (); return Result;};
  void             CreateResult  () {};
  void             AnalyseHisto  (TScanHisto *histo);
  void             InitCounters  (){};
 public: 
  TNoiseAnalysis(std::deque<TScanHisto> *histoQue, 
                 TScan                  *aScan, 
                 TScanConfig            *aScanConfig, 
                 std::vector <THic*>     hics,
                 std::mutex             *aMutex, 
                 TNoiseResult           *aResult = 0);
  void Initialize ();
  void Finalize   ();
};



#endif
