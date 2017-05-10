#ifndef TFIFOANALYSIS_H
#define TFIFOANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"


class TFifoAnalysis : public TScanAnalysis {
 private:
 protected:
 public:
  TFifoAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  void Run();
 };

#endif
