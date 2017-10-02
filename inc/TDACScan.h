#ifndef TDACSCAN_H
#define TDACSCAN_H

#include <mutex>
#include "TScan.h"
#include "THisto.h"
#include "TAlpide.h"

class TDACScan : public TScan {
 private:
  void ConfigureChip  (TAlpide *chip);
 protected: 
  THisto   CreateHisto();
 public:
  TDACScan        (TScanConfig                   *config, 
                   std::vector <TAlpide *>        chips, 
                   std::vector <THic*>            hics, 
                   std::vector <TReadoutBoard *>  boards, 
                   std::deque<TScanHisto>        *histoque, 
                   std::mutex                    *aMutex);
  ~TDACScan      () {};
  void Init      ();
  void Execute   ();
  void Terminate ();

  void Next        (int loopIndex);
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void LoopEnd     (int loopIndex);
  void PrepareStep (int loopIndex);
};


#endif
