#ifndef TTHRESHOLDSCAN_H
#define TTHRESHOLDSCAN_H

#include "TScan.h"

class TThresholdScan : public TMaskScan {
 private:
  int m_VPULSEH; 
  int m_nTriggers;
  void ConfigureFromu (TAlpide *chip);
  void ConfigureChip  (TAlpide *chip);
  void ConfigureBoard (TReadoutBoard *board);
 protected:
 public: 
  TThresholdScan   (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards);
  ~TThresholdScan  () {};
  void Init        ();
  void PrepareStep (int loopIndex);
  void LoopEnd     (int loopIndex) {};
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void Execute     ();
  void Terminate   ();
};


#endif
