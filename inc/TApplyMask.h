#ifndef TAPPLYMASK_H
#define TAPPLYMASK_H

#include <deque>
#include <vector>
#include <mutex>

#include "Common.h"
#include "TScanAnalysis.h"
#include "TNoiseAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"
#include "THisto.h"

class TApplyMask : public TScanAnalysis {
private:
protected:
  TScanResultChip *GetChipResult() {
    return 0;
  };
  TScanResultHic *GetHicResult() {
    return 0;
  };
  void CreateResult() {};
  void AnalyseHisto(TScanHisto *histo) {
    (void)histo;
  };
  void InitCounters() {};

public:
  TApplyMask(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
             std::vector<THic *> hics, std::mutex *aMutex, TNoiseResult *aResult);
  void Initialize() {};
  void Finalize() {};
  void Run();
};

#endif
