#ifndef TTHRESHOLDSCAN_H
#define TTHRESHOLDSCAN_H

#include <deque>
#include "TScan.h"
#include "THisto.h"
#include "AlpideDecoder.h"


class TThresholdScan : public TMaskScan {
 private:
  int         m_VPULSEH; 
  int         m_nTriggers;

  void ConfigureFromu (TAlpide *chip);
  void ConfigureChip  (TAlpide *chip);
  void ConfigureBoard (TReadoutBoard *board);
  void FillHistos     (std::vector<TPixHit> *Hits, int board);
 protected:
  THisto CreateHisto    ();
 public: 
  TThresholdScan   (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoque);
  ~TThresholdScan  () {};

  void Init        ();
  void PrepareStep (int loopIndex);
  void LoopEnd     (int loopIndex);
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void Execute     ();
  void Terminate   ();
};


#endif
