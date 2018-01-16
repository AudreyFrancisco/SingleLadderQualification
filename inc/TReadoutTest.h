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

typedef struct __TReadoutParameters : TScanParameters 
{
  int   row;
  int   triggers;
  int   linkSpeed;
  int   occupancy;
  int   driverStrength;
  int   preemp;
  int   pllStages;
  float voltageScale;
} TReadoutParameters;


class TReadoutTest : public TDataTaking {
 private:
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
  int  GetRow      () {return ((TReadoutParameters*)m_parameters)->row;};
  int  GetDriver   () {return ((TReadoutParameters*)m_parameters)->driverStrength;};
  int  GetLinkSpeed() {return ((TReadoutParameters*)m_parameters)->linkSpeed;};
  int  GetPreemp   () {return ((TReadoutParameters*)m_parameters)->preemp;};
  void Init        ();
  void PrepareStep (int loopIndex) { (void)(&loopIndex); };
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void Terminate   ();
};



#endif
