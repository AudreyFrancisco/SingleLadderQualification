#ifndef TSCURVESCAN_H
#define TSCURVESCAN_H

#include <deque>
#include <mutex>

#include "AlpideDecoder.h"
#include "Common.h"
#include "THisto.h"
#include "TScan.h"

class TSCurveScan : public TMaskScan {
 protected:
  int         m_VPULSEH;
  void ConfigureFromu (TAlpide *chip);
  void ConfigureChip  (TAlpide *chip);
  void ConfigureBoard (TReadoutBoard *board);
  void FillHistos     (std::vector<TPixHit> *Hits, int board);
  //THisto CreateHisto    ();
 public: 
  TSCurveScan  (TScanConfig                   *config,
                std::vector <TAlpide *>        chips,
                std::vector <THic*>            hics,
                std::vector <TReadoutBoard *>  boards,
                std::deque<TScanHisto>        *histoque,
                std::mutex                    *aMutex);
  virtual ~TSCurveScan () {};

  THisto CreateHisto(); //public in TScan, so...
  void Init        ();
  virtual void PrepareStep (int loopIndex) = 0;
  void LoopEnd     (int loopIndex);
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void Execute     ();
  void Terminate   ();

};


class TThresholdScan : public TSCurveScan {
  //Conducts a regular threshold scan
 public:
  TThresholdScan  (TScanConfig                   *config,
                  std::vector <TAlpide *>        chips,
                  std::vector <THic*>            hics,
                  std::vector <TReadoutBoard *>  boards,
                  std::deque<TScanHisto>        *histoque,
                  std::mutex                    *aMutex);  //:
  //TSCurveScan  (config, chips, hics, boards, histoque, aMutex) {
  //  m_step[1] = 1;
  //}
  void PrepareStep (int loopIndex);
  ~TThresholdScan() {};
};

class TtuneVCASNScan : public TSCurveScan {
  //NOTE:  may need new destructor?
  //Conducts a threshold scan changing VCASN
 public:
  TtuneVCASNScan (TScanConfig                   *config,
                  std::vector <TAlpide *>        chips,
                  std::vector <THic*>            hics,
                  std::vector <TReadoutBoard *>  boards,
                  std::deque<TScanHisto>        *histoque,
                  std::mutex                    *aMutex);  //:
  //TSCurveScan  (config, chips, hics, boards, histoque, aMutex) {
  //  m_step[1] = 16; //this will probably never change
  //}
  void PrepareStep (int loopIndex);
  ~TtuneVCASNScan() {};
};

class TtuneITHRScan : public TSCurveScan {
  //Conducts a threshold scan changing ITHR (note:  needs data from VCASNscan first)
 public:
  TtuneITHRScan  (TScanConfig                   *config,
                  std::vector <TAlpide *>        chips,
                  std::vector <THic*>            hics,
                  std::vector <TReadoutBoard *>  boards,
                  std::deque<TScanHisto>        *histoque,
                  std::mutex                    *aMutex);  //:
  //TSCurveScan  (config, chips, hics, boards, histoque, aMutex) {
  //  m_step[1] = 16;
  // }
  void PrepareStep (int loopIndex);
  ~TtuneITHRScan() {};
};

#endif
