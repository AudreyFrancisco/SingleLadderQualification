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
  
  FILE* m_fileSummary;
  FILE* m_filePixelNoHits;
  FILE* m_filePixelStuck;
  FILE* m_filePixelNoThreshold; // To do, based on chi2 cut?
  FILE* m_filePixelFitResult;
  FILE* m_fileRawData; 
  
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
  
  void SetFileSummary          (FILE* aFileName); 
  void SetFilePixelNoHits      (FILE* aFileName);
  void SetFilePixelStuck       (FILE* aFileName);
  void SetFilePixelNoThreshold (FILE* aFileName); 
  void SetFilePixelFitResult   (FILE* aFileName);
  void SetFileRawData          (FILE* aFileName); 
  
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
  
  FILE* GetFileSummary          ();
  FILE* GetFilePixelNoHits      ();
  FILE* GetFilePixelStuck       ();
  FILE* GetFilePixelNoThreshold (); 
  FILE* GetFilePixelFitResult   ();
  FILE* GetFileRawData          (); 
 
  void WriteToFile (FILE *fp) {}; 
};

class TThresholdResult : public TScanResult { 
protected:
 // std::map <int, TScanResultChip*> abs_chipResults;
 public: 
  TThresholdResult  ();
  ~TThresholdResult ();
 // virtual TThresholdResult *clone() const;
  //TThresholdResult (const TThresholdResult &_tresult):TScanResult(_tresult){}; 
//  TThresholdResult  &operator=(const TThresholdResult &_tresult){m_chipResults=_tresult.m_chipResults; return *this;} 
   //TThresholdResult(const TScanResult &_result):TScanResult(_result){}
 // TThresholdResult *TThresholdResult:: clone() const {return new TThresholdResult(*this);} 
  void WriteToFileGlobal   (FILE *fp) {};
  void WriteToDB     (const char *hicID) {};
};

class TThresholdAnalysis : public TScanAnalysis {
  
 private:  
  static constexpr float m_electronPerDac = 10; //[e/DAC]
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

  int m_resultFactor; //basically determines scan type; 10 default if regular, 1 if vcasn, -1 if ithr
  
  double m_cutChi2; // Or float ?
  
  std::map<int,TThresholdResultChip> m_resultChip;
  std::map<int,common::TStatVar> m_threshold;
  std::map<int,common::TStatVar> m_noise;
  
  std::string GetFileName(common::TChipIndex aChipIndex,
			  std::string fileType);
  bool                      CheckPixelNoHits(TGraph* aGraph);
  bool                      CheckPixelStuck (TGraph* aGraph);
  common::TErrFuncFitResult DoFit           (TGraph* aGraph);
  
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
