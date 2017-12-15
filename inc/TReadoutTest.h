#ifndef TREADOUTTEST_H
#define TREADOUTTEST_H

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


class TReadoutTest : public TDataTaking {
 private:
  int  m_row;
  int  m_linkSpeed;
  int  m_occupancy;
  int  m_driverStrength;
  int  m_preemp;
  void ConfigureChip (TAlpide *chip);
  void ConfigureMask (TAlpide *chip, std::vector <TPixHit> *MaskedPixels);
 protected:
  THisto CreateHisto();
 public:
  TReadoutTest     (TScanConfig                   *config,
                    std::vector <TAlpide *>        chips,
                    std::vector <THic*>            hics,
                    std::vector <TReadoutBoard *>  boards,
                    std::deque<TScanHisto>        *histoque,
                    std::mutex                    *aMutex);
  ~TReadoutTest    () {};
  int  GetRow      () {return m_row;};
  int  GetDriver   () {return m_driverStrength;};
  int  GetLinkSpeed() {return m_linkSpeed;};
  int  GetPreemp   () {return m_preemp;};
  void Init        ();
  void PrepareStep (int loopIndex) { (void)(&loopIndex); };
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void Terminate   ();
};



#endif
