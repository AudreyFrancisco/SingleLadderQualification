#ifndef TDIGITALSCAN_H
#define TDIGITALSCAN_H

#include <deque>
#include <mutex>
#include <vector>
#include "TScan.h"
#include "THisto.h"
#include "AlpideDecoder.h"


class TDigitalScan : public TMaskScan {
 private:
  void ConfigureFromu (TAlpide *chip);
  void ConfigureChip  (TAlpide *chip);
  void ConfigureBoard (TReadoutBoard *board);
  void FillHistos     (std::vector<TPixHit> *Hits, int board);
 protected:
  THisto CreateHisto    ();
 public: 
  TDigitalScan   (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoque, std::mutex *aMutex);
  ~TDigitalScan  () {};

  void Init        ();
  void PrepareStep (int loopIndex);
  void LoopEnd     (int loopIndex);
  void Next        (int loopIndex);
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void Execute     ();
  void Terminate   ();
};


#endif
