#ifndef TNOISEOCCUPANCY_H
#define TNOISEOCCUPANCY_H

#include <deque>
#include <mutex>
#include <vector>
#include <map>
#include <string>

#include "AlpideDecoder.h"
#include "Common.h"
#include "THisto.h"
#include "TScan.h"

const int kTrigPerTrain = 100;

class TNoiseOccupancy : public TScan { 
 private:
  int                                   m_nTrains;
  int                                   m_nLast;
  float                                 m_backBias;
  std::vector <TPixHit>                 m_stuck;
  TErrorCounter                         m_errorCount;
  void ConfigureFromu  (TAlpide *chip);
  void ConfigureChip   (TAlpide *chip);
  void ConfigureBoard  (TReadoutBoard *board);
  void ConfigureMask   (TAlpide *chip, std::vector <TPixHit> *MaskedPixels);
  void FillHistos      (std::vector<TPixHit> *Hits, int board);
  void ReadEventData   (std::vector <TPixHit> *Hits, int iboard, int nTriggers);
  void FindTimeoutHics (int iboard, int *triggerCounts, int nTriggers);
 protected:
  THisto CreateHisto();
 public:
  TNoiseOccupancy   (TScanConfig                   *config, 
                     std::vector <TAlpide *>        chips, 
                     std::vector <THic*>            hics, 
                     std::vector <TReadoutBoard *>  boards, 
                     std::deque<TScanHisto>        *histoque, 
                     std::mutex                    *aMutex);
  ~TNoiseOccupancy  () {};
  void Init         ();
  void PrepareStep  (int loopIndex) {};
  void LoopStart    (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void LoopEnd      (int loopIndex);
  void Execute      ();
  void Terminate    ();
};



#endif
