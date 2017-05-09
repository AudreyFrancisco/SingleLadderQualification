#ifndef TFIFOTEST_H
#define TFIFOTEST_H

#include <mutex>
#include "TScan.h"
#include "THisto.h"

class TFifoTest : public TScan {
 private:
  TAlpide *m_testChip;

  int      GetChipById    (std::vector <TAlpide *> chips, int previousId);
  bool     TestPattern    (int pattern);

 protected: 
  THisto CreateHisto() {THisto histo; return histo;};
 public:
  TFifoTest   (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoque, std::mutex *aMutex);
  ~TFifoTest  () {};
  void Init ();
  void Execute ();
  void Terminate () {};

  void LoopStart  (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void PrepareStep(int loopIndex);

};

#endif
