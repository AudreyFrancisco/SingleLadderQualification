#ifndef TFIFOTEST_H
#define TFIFOTEST_H

#include <mutex>

#include "Common.h"
#include "TScan.h"
#include "THisto.h"

class TFifoTest : public TScan {
 private:
  TAlpide *m_testChip;
  int      m_boardIndex;

  int      GetChipById    (std::vector <TAlpide *> chips, int previousId);
  void     ReadMem        (TAlpide *chip, int ARegion, int AOffset, int &AValue, bool &exception);
  void     WriteMem       (TAlpide *chip, int ARegion, int AOffset, int AValue);
  bool     TestPattern    (int pattern, bool &exception);
  float    m_voltageScale;
  int      m_mlvdsStrength;
 protected: 
  THisto   CreateHisto();
 public:
  TFifoTest   (TScanConfig                   *config, 
               std::vector <TAlpide *>        chips, 
               std::vector <THic*>            hics,
               std::vector <TReadoutBoard *>  boards, 
               std::deque<TScanHisto>        *histoque, 
               std::mutex                    *aMutex);
  ~TFifoTest       () {};
  void Init        ();
  void Execute     ();
  void Terminate   ();
  void LoopEnd     (int loopIndex);
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void PrepareStep (int loopIndex);
  bool IsNominal   () {return ((m_voltageScale > 0.99) && (m_voltageScale < 1.01));};
  bool IsLower     () {return (m_voltageScale < 0.9);};
  bool IsUpper     () {return (m_voltageScale > 1.1);};
};

#endif
