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
class TH1D;/*H*/
class TH2D;/*H*/

class TThresholdResultChip : public TScanResultChip {
 private:
  
  unsigned int m_boardIndex; 
  unsigned int m_dataReceiver;
  unsigned int m_chipId;
  
  int m_vPulseL;    //?
  int m_vPulseH;    //?
  int m_vPulseStep; //?
  int m_nMask;      //?
  
  int m_counterPixelsNoHits;
  int m_counterPixelsStuck;
  int m_counterPixelsNoThreshold;
  
  double m_thresholdMean;
  double m_thresholdStdDev;
  double m_noiseMean;
  double m_noiseStdDev;
  
  
 public: 
  TThresholdResultChip ();
  ~TThresholdResultChip();
  
  void SetBoardIndex   (unsigned int aBoardIndex); 
  void SetDataReceiver (unsigned int aDataReceiver);
  void SetChipId       (unsigned int aChipId);
  
  void SetVPulseL (int aVPulseL);    
  void SetVPulseH (int aVPulseH);    
  void SetVPulseStep (int aVPulseStep); 
  void SetNMask   (int aNMask);      
  
  void SetCounterPixelsNoHits      (int aCounterPixelsNoHits);
  void SetCounterPixelsStuck       (int aCounterPixelsStuck);
  void SetCounterPixelsNoThreshold (int aCounterPixelsNoThreshold);
  
  void SetThresholdMean   (double aThresholdMean);
  void SetThresholdStdDev (double aThresholdStdDev);
  void SetNoiseMean       (double aNoiseMean);
  void SetNoiseStdDev     (double aNoiseStdDev);
  
  unsigned int GetBoardIndex   (); 
  unsigned int GetDataReceiver ();
  unsigned int GetChipId ();
  
  int GetVPulseL ();
  int GetVPulseH ();
  int GetVPulseStep ();
  int GetNMask   ();
   
  int GetCounterPixelsNoHits      ();
  int GetCounterPixelsStuck       ();
  int GetCounterPixelsNoThreshold ();
  
  double GetThresholdMean   ();
  double GetThresholdStdDev ();
  double GetNoiseMean       ();
  double GetNoiseStdDev     ();
  
};

class TThresholdResult : public TScanResult {
  
 private:
  FILE* m_fileHicResult;
  FILE* m_filePixelByPixelResult;
  FILE* m_fileStuckPixels;
  
 public: 
  TThresholdResult  ();
  ~TThresholdResult ();
  
  void SetFileHicResult          (FILE* aFileName);
  void SetFilePixelByPixelResult (FILE* aFileName);
  void SetFileStuckPixels        (FILE* aFileName);
  
  FILE* GetFileHicResult         ();
  FILE* GetFilePixelByPixelResult();
  FILE* GetFileStuckPixels       ();
  
};

class TThresholdAnalysis : public TScanAnalysis {
  
 private:  
  
  TH1D* hSuperMeanA;/*H*/
  TH1D* hSuperNoiseA;/*H*/
  TH1D* hSuperRedChi2A;/*H*/
  TH1D* hSuperStatusA;/*H*/
  
  TH2D* hSuperDummyA;/*H*/
  TH2D* hSuperNPointsVsStatusA;/*H*/
  TH2D* hSuperNPointsVsChi2A;/*H*/

  TH1D* hSuperMeanB;/*H*/
  TH1D* hSuperNoiseB;/*H*/
  TH1D* hSuperRedChi2B;/*H*/
  TH1D* hSuperStatusB;/*H*/
  
  TH2D* hSuperDummyB;/*H*/
  TH2D* hSuperNPointsVsStatusB;/*H*/
  TH2D* hSuperNPointsVsChi2B;/*H*/

  TH1D* hSuperMeanC;/*H*/
  TH1D* hSuperNoiseC;/*H*/
  TH1D* hSuperRedChi2C;/*H*/
  TH1D* hSuperStatusC;/*H*/
  
  TH2D* hSuperDummyC;/*H*/
  TH2D* hSuperNPointsVsStatusC;/*H*/
  TH2D* hSuperNPointsVsChi2C;/*H*/
  
  TThresholdResult* m_resultThreshold;
   
  const float m_electronPerDac = 10; //[e/DAC]
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
  
  std::map<int,common::TStatVar> m_threshold;
  std::map<int,common::TStatVar> m_noise;
  
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
  TScanResultChip *GetChipResult () {
    TThresholdResultChip *Result= new TThresholdResultChip(); 
    return Result;};
  void            CreateResult  () {};
  
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

//TODO:
//- why fit fails.
//-implement errors.
//-optimize output files.
