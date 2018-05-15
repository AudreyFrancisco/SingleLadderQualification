#ifndef TSCURVEANALYSIS_H
#define TSCURVEANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "Common.h"
#include "DBHelpers.h"
#include "THisto.h"
#include "TSCurveScan.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

#include <TF1.h>
#include <TGraph.h>

class TSCurveAnalysis;

class TSCurveResultChip : public TScanResultChip {
  friend class TSCurveAnalysis;

private:
  TSCurveAnalysis *m_analysis;
  float            m_thresholdAv;
  float            m_thresholdRms;
  float            m_threshRelativeRms;
  float            m_noiseAv;
  float            m_noiseRms;
  float            m_deviation;
  int              m_nEntries;
  float            m_noiseSq;
  float            m_threshSq;
  int              m_nNoThresh;
  int              m_nDead;
  int              m_nHot;
  char             m_rawFile[200];
  char             m_fitFile[200];
  FILE *           m_rawFP;
  FILE *           m_fitFP;

public:
  TSCurveResultChip(TSCurveAnalysis *aAnalysis) : TScanResultChip() { m_analysis = aAnalysis; };
  void SetRawFile(const char *fName) { strcpy(m_rawFile, fName); };
  void SetFitFile(const char *fName) { strcpy(m_fitFile, fName); };
  void WriteToFile(FILE *fp);
  float GetVariable(TResultVariable var);
  void  CalculateAverages();
  float GetThresholdMean() { return m_thresholdAv; };
};

class TSCurveResultHic : public TScanResultHic {
  friend class TSCurveAnalysis;
  friend class TApplyTuning;

private:
  float         m_noiseAv;
  float         m_noiseRms;
  int           m_nEntries;
  float         m_noiseSq;
  float         m_minChipAv;
  float         m_maxChipAv;
  float         m_maxChipNoise;
  float         m_maxRelativeRms;
  float         m_maxDeviation;
  int           m_nDead;
  int           m_nNoThresh;
  int           m_nNoThreshWorstChip;
  int           m_nDeadWorstChip;
  int           m_nHot;
  float         m_backBias;
  bool          m_nominal;
  bool          m_VCASNTuning;
  bool          m_ITHRTuning;
  bool          m_thresholdScan;
  char          m_stuckFile[200];
  TErrorCounter m_errorCounter;
  void GetParameterSuffix(std::string &suffix, std::string &file_suffix);

public:
  TSCurveResultHic() : TScanResultHic(){};
  void CalculateAverages();
  void SetStuckFile(const char *fName) { strcpy(m_stuckFile, fName); };
  void WriteToFile(FILE *fp);
  void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TSCurveResult : public TScanResult {
  friend class TSCurveAnalysis;

private:
  int m_nTimeout;
  int m_n8b10b;
  int m_nCorrupt;

public:
  TSCurveResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp);
};

class TSCurveAnalysis : public TScanAnalysis {
  friend class TSCurveResultChip;

private:
  static constexpr float m_electronPerDac = 10;
  float                  m_resultFactor;
  float                  m_targetThreshold;
  int                    m_nPulseInj;
  int                    m_startPulseAmplitude;
  int                    m_stopPulseAmplitude;
  int                    m_stepPulseAmplitude;

  bool m_fDoFit;
  bool m_speedy;
  bool m_writeRawData;
  bool m_writeNoHitPixels;
  bool m_writeNoThreshPixels;
  bool m_writeStuckPixels;
  bool m_writeFitResults;
  bool m_nominal;

  void InitCounters();
  void FillVariableList();
  void WriteResult();
  void PrepareFiles();
  void CloseFiles();
  bool IsVCASNTuning() { return (fabs(m_resultFactor - 1) < 0.01); };
  bool IsThresholdScan() { return (m_resultFactor > 1); };
  bool IsITHRTuning() { return (m_resultFactor < 0); };
  bool CheckPixelNoHits(TGraph *aGraph);
  bool CheckPixelHot(TGraph *aGraph);
  double meanGraph(TGraph *resultGraph);
  double rmsGraph(TGraph *resultGraph);
  void ddxGraph(TGraph *aGraph, TGraph *resultGraph);
  //  double ErrorFunc                 (double* x, double* par);
  float FindStartStandard(TGraph *aGraph, int nInj);
  float FindStartInverse(TGraph *aGraph, int nInj);
  float FindStart(TGraph *aGraph, int resultFactor, int nInj);

  void FillGraph(TGraph *aGraph);
  common::TErrFuncFitResult DoFit(TGraph *aGraph, bool speedy = false);
  common::TErrFuncFitResult DoSpeedyFit(TGraph *aGraph);
  common::TErrFuncFitResult DoRootFit(TGraph *aGraph);
  THicClassification GetClassificationOB(TSCurveResultHic *result, THic *hic);
  THicClassification GetClassificationIB(TSCurveResultHic *result, THic *hic);

protected:
  TScanResultChip *GetChipResult()
  {
    TSCurveResultChip *Result = new TSCurveResultChip(this);
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TSCurveResultHic *Result = new TSCurveResultHic();
    return Result;
  };
  void CreateResult(){};
  void AnalyseHisto(TScanHisto *histo);
  virtual string GetPreviousTestType();
  void CalculatePrediction(std::string hicName) { (void)hicName; };

public:
  // constructor: result factor determines type of analysis
  // default: threshold scan
  // 1      : VCASN scan
  // -1     : ITHR scan
  TSCurveAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                  std::vector<THic *> hics, std::mutex *aMutex, TSCurveResult *aResult = 0,
                  float resultFactor = m_electronPerDac);

  void Initialize();
  void Finalize();
};

#endif
