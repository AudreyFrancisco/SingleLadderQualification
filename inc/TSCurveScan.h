#ifndef TSCURVESCAN_H
#define TSCURVESCAN_H

#include <deque>
#include <mutex>

#include "AlpideDecoder.h"
#include "Common.h"
#include "THisto.h"
#include "TScan.h"

typedef struct __TSCurveParameters : TScanParameters {
  bool nominal;
  int  VPULSEH;
  int  VPULSEL;
  int  TARGET;
} TSCurveParameters;

class TSCurveScan : public TMaskScan {
protected:
  //  bool m_nominal;
  // int m_VPULSEH;
  // float m_backBias;
  void ConfigureFromu(TAlpide *chip);
  virtual void ConfigureChip(TAlpide *chip) = 0;
  void ConfigureBoard(TReadoutBoard *board);
  void RestoreNominalSettings();
  void FillHistos(std::vector<TPixHit> *Hits, int board);
  // THisto CreateHisto    ();
  virtual void SetName() = 0;

public:
  TSCurveScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
              std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
              std::mutex *aMutex);
  virtual ~TSCurveScan(){};

  THisto       CreateHisto(); // public in TScan, so...
  void         Init();
  virtual void PrepareStep(int loopIndex) = 0;
  void LoopEnd(int loopIndex);
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void               Execute();
  void               Terminate();
  float              GetBackbias() { return ((TSCurveParameters *)m_parameters)->backBias; };
  bool               GetNominal() { return ((TSCurveParameters *)m_parameters)->nominal; };
  bool SetParameters(TScanParameters *pars);
};

class TThresholdScan : public TSCurveScan {
  // Conducts a regular threshold scan
protected:
  void SetName();

public:
  TThresholdScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                 std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                 std::mutex *aMutex); //:
  void ConfigureChip(TAlpide *chip);
  void PrepareStep(int loopIndex);
  ~TThresholdScan(){};
};

class TtuneVCASNScan : public TSCurveScan {
  // NOTE:  may need new destructor?
  // Conducts a threshold scan changing VCASN
protected:
  void SetName();

public:
  TtuneVCASNScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                 std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                 std::mutex *aMutex); //:
  void ConfigureChip(TAlpide *chip);
  void PrepareStep(int loopIndex);
  ~TtuneVCASNScan(){};
};

class TtuneITHRScan : public TSCurveScan {
  // Conducts a threshold scan changing ITHR (note:  needs data from VCASNscan first)
protected:
  void SetName();

public:
  TtuneITHRScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                std::mutex *aMutex); //:
  void ConfigureChip(TAlpide *chip);
  void PrepareStep(int loopIndex);
  ~TtuneITHRScan(){};
};

#endif
