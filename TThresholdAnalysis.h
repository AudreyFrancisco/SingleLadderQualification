#ifndef TTHRESHOLDANALYSIS_H
#define TTHRESHOLDANALYSIS_H

#include <deque>
#include <mutex>
#include <string>
#include <time.h>
#include <vector>

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"

class TF1;
class TGraph;

class TThresholdResultChip : public TScanResultChip {
 public: 
  TThresholdResultChip () : TScanResultChip () {};
};

class TThresholdAnalysis : public TScanAnalysis {
  
 private:
  
  const float m_electronPerDac = 10;
  const std::string m_analisysName = "TThresholdAnalysis";
  
  int m_startPulseAmplitude;
  int m_stopPulseAmplitude;
  int m_stepPulseAmplitude;
  int m_nPulseInj;
  
  // MB-TODO: Rename "NoHits" as dead ?
  int m_counterPixelsNoHits;
  int m_counterPixelsNoThreshold;
  int m_counterPixelsStuck;
  
  bool m_fDoFit;
  bool m_fDoDumpRawData;
  
  int m_sumGoodThresholds;
  int m_counterGoodThresholds;
  
  double m_cutChi2; // Or float ?
  
  struct tm* m_time;
  
  FILE* m_filePixelNoHits;
  FILE* m_filePixelStuck;
  FILE* m_filePixelNoThreshold;
  FILE* m_fileRawData;
  FILE* m_fileFitResults;
  
  // TODO
  //std::vector<FILE*> m_filePixelNoHits;
  //std::vector<FILE*> m_filePixelStuck;
  //std::vector<FILE*> m_filePixelNoThreshold;
  //std::vector<FILE*> m_fileRawData;
  
  std::string GetFileName(TChipIndex aChipIndex,std::string fileType);
  bool   CheckPixelNoHits(TGraph* aGraph);
  bool   CheckPixelStuck (TGraph* aGraph);
  std::pair<double,double>   DoFit           (TGraph* aGraph);
  
  bool HasData(TScanHisto &scanHisto, 
	       TChipIndex idx, 
	       int col);
  
 protected:
  TScanResultChip GetChipResult () {TThresholdResultChip Result; return Result;};
 public:
  TThresholdAnalysis(std::deque<TScanHisto> *scanHistoQue, 
		     TScan *aScan, 
		     TScanConfig *aScanConfig, 
		     std::mutex *aMutex);
  
  void Initialize();
  void Run       ();
  void Finalize  ();
  
};

#endif

// STUFF TO IMPLEMENT FROM MARKUS' TALK :
// Raw data;
// Fit data;
// # pixels with no hits;
// Addr. of pixels with no hits, dump to file;
// # pixels with no threshold;
// Addr. of pixels with no threshold, dump to file;
// Threshold avg. + rms per chip;
// Noise avg. + rms per chip;
// # stuck pixels, dump to file; 
// Addr. of stuck pixels, dump to file;

// STUFF TO IMPLEMENT FROM MARKUS' EMAIL:
// Retrieve charge min, max, nInj;
// Flag to dump raw data in file;
// Flat to fit;
// Cut for scan success;
// Return boolean;
