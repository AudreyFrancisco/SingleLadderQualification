#ifndef TPOWERTEST_H
#define TPOWERTEST_H

#include <mutex>

#include "Common.h"
#include "TScan.h"
#include "THisto.h"


class TPowerTest : public TScan {
 private: 
 protected:
 public:
  TPowerTest  (TScanConfig                   *config, 
               std::vector <TAlpide *>        chips, 
               std::vector <THic*>            hics,
               std::vector <TReadoutBoard *>  boards, 
               std::deque<TScanHisto>        *histoque, 
               std::mutex                    *aMutex);
  ~TPowerTest () {};

  void Init        () {};
  void Execute     () {};
  void Terminate   () {};
  void LoopStart   (int loopIndex) {};
  void LoopEnd     (int loopIndex) {};
  void PrepareStep (int loopIndex) {};
};



#endif
