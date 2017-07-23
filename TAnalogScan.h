#ifndef TANALOGSCAN_H
#define TANALOGSCAN_H

#include <deque>
#include <mutex>

#include "AlpideDecoder.h"
#include "Common.h"
#include "THisto.h"
#include "TScan.h"

class TAnalogScan : public TMaskScan {
 private:
  int         m_VPULSEH;
 protected:
  void ConfigureFromu (TAlpide *chip);
  void ConfigureChip  (TAlpide *chip);
  void ConfigureBoard (TReadoutBoard *board);
  void FillHistos     (std::vector<TPixHit> *Hits, int board);
  THisto CreateHisto    ();
 public: 
  TAnalogScan  (TScanConfig                   *config,
                std::vector <TAlpide *>        chips,
                std::vector <THic*>            hics,
                std::vector <TReadoutBoard *>  boards,
                std::deque<TScanHisto>        *histoque,
                std::mutex                    *aMutex);
  ~TAnalogScan  () {};

  void Init        ();
  void PrepareStep (int loopIndex);
  void LoopEnd     (int loopIndex);
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void Execute     ();
  void Terminate   ();
};


//class TThresholdScan : public TAnalogScan {
  //Conducts a regular threshold scan
// public:
//  TThresholdScan  (TScanConfig                   *config,
//                  std::vector <TAlpide *>        chips,
//                  std::vector <THic*>            hics,
//                  std::vector <TReadoutBoard *>  boards,
//                  std::deque<TScanHisto>        *histoque,
//                  std::mutex                    *aMutex) :
//    TAnalogScan  (config, chips, hics, boards, histoque, aMutex) {
//    m_step[1] = 1;
//  }
//};

class TtuneVCASNScan : public TAnalogScan {
  //NOTE:  may need new destructor?
  //Conducts a threshold scan changing VCASN
 public:
  TtuneVCASNScan (TScanConfig                   *config,
                  std::vector <TAlpide *>        chips,
                  std::vector <THic*>            hics,
                  std::vector <TReadoutBoard *>  boards,
                  std::deque<TScanHisto>        *histoque,
                  std::mutex                    *aMutex) :
    TAnalogScan  (config, chips, hics, boards, histoque, aMutex) {
    m_step[1] = 16;
  }
  void PrepareStep (int loopIndex);
};

class TtuneITHRScan : public TAnalogScan {
  //Conducts a threshold scan changing ITHR (note:  needs data from VCASNscan first)
 public:
  TtuneITHRScan  (TScanConfig                   *config,
                  std::vector <TAlpide *>        chips,
                  std::vector <THic*>            hics,
                  std::vector <TReadoutBoard *>  boards,
                  std::deque<TScanHisto>        *histoque,
                  std::mutex                    *aMutex) :
    TAnalogScan  (config, chips, hics, boards, histoque, aMutex) {
    m_step[1] = 16;
  }
  void PrepareStep (int loopIndex);
};

class TITHRScan : public TAnalogScan {
  //Conducts a regular threshold scan, after first setting ITHR and VCASN per chip based on
  //the results from TtuneVCASN and TtuneITHR
 public:
  TITHRScan      (TScanConfig                   *config,
                  std::vector <TAlpide *>        chips,
                  std::vector <THic*>            hics,
                  std::vector <TReadoutBoard *>  boards,
                  std::deque<TScanHisto>        *histoque,
                  std::mutex                    *aMutex) :
    TAnalogScan  (config, chips, hics, boards, histoque, aMutex) {
    m_step[1] = 1;
  }
};


#endif
