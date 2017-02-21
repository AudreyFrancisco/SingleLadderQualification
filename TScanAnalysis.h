#ifndef TSCANALYSIS_H
#define TSCANALYSIS_H

#include <deque>
#include "THisto.h"

class TScanAnalysis {
 protected:
  std::deque<TScanHisto> *m_histoQue;
 public:
  TScanAnalysis (std::deque<TScanHisto> *histoQue);
  void Run();
};




#endif
