#ifndef TFIFOTEST_H
#define TFIFOTEST_H

#include <mutex>
#include "TScan.h"
#include "THisto.h"

class TFifoTest : public TScan {
 private:
  TAlpide *m_testChip;
  int      m_boardIndex;

  int      GetChipById    (std::vector <TAlpide *> chips, int previousId);
  void     ReadMem        (TAlpide *chip, int ARegion, int AOffset, int &AValue);
  void     WriteMem       (TAlpide *chip, int ARegion, int AOffset, int AValue);
  bool     TestPattern    (int pattern);
  int      FindBoardIndex (TAlpide *chip);
 protected: 
  THisto   CreateHisto();
 public:
  TFifoTest   (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoque, std::mutex *aMutex);
  ~TFifoTest       () {};
  void Init        ();
  void Execute     ();
  void Terminate   () {};
  void LoopEnd     (int loopIndex);
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void PrepareStep (int loopIndex);

};

#endif
