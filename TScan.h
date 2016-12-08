#ifndef TSCAN_H
#define TSCAN_H

#include "TAlpide.h"
#include "TReadoutBoard.h"
#include "TScanConfig.h"

const  int  MAXLOOPLEVEL = 3;
const  int  MAXBOARDS    = 2;

extern bool fScanAbort;

class TScan {
 private:
 protected: 
  TScanConfig                  *m_config;
  std::vector <TAlpide *>       m_chips;
  std::vector <TReadoutBoard *> m_boards;
  int m_start   [MAXLOOPLEVEL];
  int m_stop    [MAXLOOPLEVEL];
  int m_step    [MAXLOOPLEVEL];
  int m_value   [MAXLOOPLEVEL];
  int m_enabled [MAXBOARDS];  // number of enabled chips per readout board

  void CountEnabledChips();

 public:
  TScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards);
  ~TScan() {};

  virtual void Init        ()              = 0;
  virtual void Terminate   ()              = 0;
  virtual void LoopStart   (int loopIndex) = 0;
  virtual void LoopEnd     (int loopIndex) = 0;
  virtual void PrepareStep (int loopIndex) = 0;
  virtual void Execute     ()              = 0;
  bool         Loop        (int loopIndex);
  void         Next        (int loopIndex); 
};



class TMaskScan : public TScan {
 private: 
 protected: 
  void ConfigureMaskStage(TAlpide *chip, int istage);
 public: 
  TMaskScan  (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards);
  ~TMaskScan () {};
};

#endif
