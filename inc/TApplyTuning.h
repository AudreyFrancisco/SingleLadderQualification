#ifndef TAPPLYTUNING_H
#define TAPPLYTUNING_H

#include <deque>
#include <vector>
#include <mutex>

#include "Common.h"
#include "TScanAnalysis.h"
#include "TThresholdAnalysis.h"
#include "TSCurveAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"
#include "THisto.h"


class TApplyTuning : public TScanAnalysis {
 private:
 protected: 
  TScanResultChip    *GetChipResult () {return 0;};
  TScanResultHic     *GetHicResult  () {return 0;};
  void                CreateResult  () {};
  void                AnalyseHisto  (TScanHisto *histo) {};
  void                InitCounters  () {};
  virtual const char* GetDACName    () = 0;
 public:
  TApplyTuning (std::deque<TScanHisto> *histoQue, 
                TScan                  *aScan,
                TScanConfig            *aScanConfig, 
                std::vector<THic*>      hics,
                std::mutex             *aMutex,
                TSCurveResult          *aResult);
  void Initialize() {};
  void Finalize  () {};
  void Run       ();
};


class TApplyVCASNTuning : public TApplyTuning {
 protected:
  const char *GetDACName () {return "VCASN";};
 public:
  TApplyVCASNTuning (std::deque<TScanHisto> *histoQue, 
                    TScan                   *aScan,
                    TScanConfig             *aScanConfig, 
                    std::vector<THic*>       hics,
                    std::mutex              *aMutex,
                    TSCurveResult           *aResult)
    : TApplyTuning (histoQue, aScan, aScanConfig, hics, aMutex, aResult) {};
};


class TApplyITHRTuning : public TApplyTuning {
 protected:
  const char *GetDACName () {return "ITHR";};
 public:
  TApplyITHRTuning (std::deque<TScanHisto> *histoQue, 
                    TScan                   *aScan,
                    TScanConfig             *aScanConfig, 
                    std::vector<THic*>       hics,
                    std::mutex              *aMutex,
                    TSCurveResult           *aResult)
    : TApplyTuning (histoQue, aScan, aScanConfig, hics, aMutex, aResult) {};
};

#endif
