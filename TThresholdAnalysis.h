#ifndef TTHRESHOLDANALYSIS_H
#define TTHRESHOLDANALYSIS_H

#include <deque>
#include <map>
#include <mutex>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>

#include "Common.h"

#include "TScanAnalysis.h"

class THisto;
class TScan;
class TScanConfig;
class TScanHisto;

class TF1;
class TGraph;

class TThresholdResultChip : public TScanResultChip {
  friend class TThresholdAnalysis;
 private:
  
  unsigned int m_boardIndex; 
  unsigned int m_dataReceiver;
  unsigned int m_chipId;
  
  int m_counterPixelsNoHits;
  int m_counterPixelsStuck;
  int m_counterPixelsNoThreshold;
  int m_counterPixelsYesThreshold;
  
  double m_thresholdMean;
  double m_thresholdStdDev;
  double m_noiseMean;
  double m_noiseStdDev;
  
  common::TStatVar m_threshold;
  common::TStatVar m_noise;
  
 public: 
  TThresholdResultChip (): TScanResultChip(){};
  ~TThresholdResultChip(){};
  
  void WriteToFile (FILE *fp){};
  
};

class TThresholdResult : public TScanResult {
  
 private:
  FILE* m_fileSummary;
  FILE* m_filePixelByPixel;
  
 public: 
  TThresholdResult  (): TScanResult () {};
  ~TThresholdResult (){};
  
  // virtual TThresholdResult *clone() const;
  //TThresholdResult (const TThresholdResult &_tresult):TScanResult(_tresult){}; 
  //  TThresholdResult  &operator=(const TThresholdResult &_tresult){m_chipResults=_tresult.m_chipResults; return *this;} 
  //TThresholdResult(const TScanResult &_result):TScanResult(_result){}
  // TThresholdResult *TThresholdResult:: clone() const {return new TThresholdResult(*this);} 
  void WriteToFileGlobal (FILE *fp) {};
  void WriteToDB         (const char *hicID) {};
  
  void SetFileSummary(FILE* aFileName)
  {m_fileSummary=aFileName;};
  void SetFilePixelByPixel(FILE* aFileName)
  {m_filePixelByPixel=aFileName;};
  
  FILE* GetFileSummary()
  {return m_fileSummary;};
  FILE* GetFilePixelByPixel()
  {return m_filePixelByPixel;};
  
};

class TThresholdAnalysis : public TScanAnalysis {
  
 private:  
  static constexpr float m_electronPerDac = 10; //[e/DAC]
  const char* m_analisysName = "ThresholdAnalysis";
  
  int m_startPulseAmplitude;
  int m_stopPulseAmplitude;
  int m_stepPulseAmplitude;
  int m_nPulseInj;
  
  bool m_fDoFit;
  
  int m_resultFactor; //basically determines scan type; 10 default if regular, 1 if vcasn, -1 if ithr 
  
  double m_cutChi2;
  
  std::string GetFileName(common::TChipIndex aChipIndex,
			  std::string fileType);
  bool CheckPixelNoHits(TGraph* aGraph);
  bool CheckPixelStuck (TGraph* aGraph);
  double FindStart(TGraph* aGraph);
  common::TErrFuncFitResult DoFit (TGraph* aGraph);
  
  bool HasData(TScanHisto &scanHisto, 
	       common::TChipIndex idx, 
	       int col);
  
 protected:
  TScanResultChip *GetChipResult () {TThresholdResultChip *Result = new TThresholdResultChip(); return Result;};
  void            CreateResult  () {};
  
 public:
  TThresholdAnalysis(std::deque<TScanHisto> *scanHistoQue, 
		     TScan *aScan, 
		     TScanConfig *aScanConfig,
                     std::vector <THic*> hics,
		     std::mutex *aMutex,
                     int resultFactor = m_electronPerDac); //MUST BE 1 for a vcasn scan, and *-1* for an ithr scan!!!  Else use default.
  
  void Initialize();
  void Run       ();
  void Finalize  ();

  float GetResultThreshold(int chip); //new; returns mean threshold of ith chip
  
};

#endif

//TODO:
//-implement errors.
//-optimize output files.
