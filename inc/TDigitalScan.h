#ifndef TDIGITALSCAN_H
#define TDIGITALSCAN_H

#include <deque>
#include <mutex>
#include <vector>

#include "AlpideDecoder.h"
#include "Common.h"
#include "THisto.h"
#include "TScan.h"

class TDigitalScan : public TMaskScan {
 private:
  void  ConfigureFromu (TAlpide *chip);
  void  FillHistos     (std::vector<TPixHit> *Hits, int board);
  float m_voltageScale;
 protected:
  void   ConfigureChip  (TAlpide *chip);
  void   ConfigureBoard (TReadoutBoard *board);
  THisto CreateHisto    ();
 public: 
  TDigitalScan   (TScanConfig                   *config, 
                  std::vector <TAlpide *>        chips, 
                  std::vector <THic*>            hics, 
                  std::vector <TReadoutBoard *>  boards, 
                  std::deque<TScanHisto>        *histoque, 
                  std::mutex                    *aMutex);
  virtual ~TDigitalScan  () {};

  virtual void Init        ();
  void         PrepareStep (int loopIndex);
  void         LoopEnd     (int loopIndex);
  void         Next        (int loopIndex);
  void         LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void         Execute     ();
  void         Terminate   ();
  bool         IsNominal   () {return ((m_voltageScale > 0.99) && (m_voltageScale < 1.01));};
  bool         IsLower     () {return (m_voltageScale < 0.9);};
  bool         IsUpper     () {return (m_voltageScale > 1.1);};
};


class TDigitalWhiteFrame : public TDigitalScan {
 public:
    TDigitalWhiteFrame   (TScanConfig                   *config,
                        std::vector <TAlpide *>        chips, 
                        std::vector <THic*>            hics, 
                        std::vector <TReadoutBoard *>  boards, 
                        std::deque<TScanHisto>        *histoque, 
                        std::mutex                    *aMutex);
  virtual ~TDigitalWhiteFrame  () {};
  void ConfigureMaskStage(TAlpide *chip, int istage);
  void Init              ();
};


#endif
