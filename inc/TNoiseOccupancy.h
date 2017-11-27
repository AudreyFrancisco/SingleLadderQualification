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
#include "TDataTaking.h"


class TNoiseOccupancy : public TDataTaking {
 private:
  void                  ConfigureChip (TAlpide *chip);
  void                  ConfigureMask (TAlpide *chip, std::vector <TPixHit> *MaskedPixels);
 protected:
  THisto CreateHisto();
 public:
  TNoiseOccupancy       (TScanConfig                   *config,
                         std::vector <TAlpide *>        chips,
                         std::vector <THic*>            hics,
                         std::vector <TReadoutBoard *>  boards,
                         std::deque<TScanHisto>        *histoque,
                         std::mutex                    *aMutex);
  ~TNoiseOccupancy      () {};
  void                  Init         ();
  void                  PrepareStep  (int loopIndex) { (void)(&loopIndex); };
  void                  LoopStart    (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
};



#endif
