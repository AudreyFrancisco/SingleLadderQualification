#ifndef TSCAN_H
#define TSCAN_H

#include <deque>
#include <mutex>
#include <vector>

#include "TAlpide.h"
#include "AlpideDecoder.h"
#include "THisto.h"
#include "TReadoutBoard.h"
#include "TScanConfig.h"

const  int  MAXLOOPLEVEL = 3;
const  int  MAXBOARDS    = 2;

extern bool fScanAbort;

typedef struct {
  int n8b10b;
  int nCorruptEvent;
  int nPrioEncoder;
  int nTimeout;
} TErrorCounter;

class TScan {
 private:
 protected: 
  TScanConfig                  *m_config;
  char                          m_name[40];
  std::vector <TAlpide *>       m_chips;
  std::vector <TReadoutBoard *> m_boards;
  TScanHisto                   *m_histo;
  std::deque <TScanHisto>      *m_histoQue;
  std::mutex                   *m_mutex;
  bool                          m_running;
  int m_start   [MAXLOOPLEVEL];
  int m_stop    [MAXLOOPLEVEL];
  int m_step    [MAXLOOPLEVEL];
  int m_value   [MAXLOOPLEVEL];
  int m_enabled [MAXBOARDS];  // number of enabled chips per readout board

  void           CountEnabledChips  ();
  virtual THisto CreateHisto        () = 0;

 public:
  TScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue, std::mutex *aMutex);
  ~TScan() {};

  virtual void Init            ()              = 0;
  virtual void Terminate       ()              = 0;
  virtual void LoopStart       (int loopIndex) = 0;
  virtual void LoopEnd         (int loopIndex) = 0;
  virtual void PrepareStep     (int loopIndex) = 0;
  virtual void Execute         ()              = 0;
  bool         Loop            (int loopIndex);
  virtual void Next            (int loopIndex); 
  void         CreateScanHisto ();
  bool         IsRunning       () {return m_running;};
  TScanHisto   GetTScanHisto   () {return *m_histo;};
  const char  *GetName         () {return m_name;};  
};

class TMaskScan : public TScan {
 private: 
 protected: 
  int                      m_pixPerStage;
  int                      m_nTriggers;
  int                      m_row;
  std::vector <TPixHit>    m_stuck;
  TErrorCounter            m_errorCount;
  void ConfigureMaskStage(TAlpide *chip, int istage);
  void ReadEventData     (std::vector <TPixHit> *Hits, int iboard);
 public: 
  TMaskScan  (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue, std::mutex *aMutex);
  ~TMaskScan () {};
  std::vector <TPixHit> GetStuckPixels () {return m_stuck;};
  TErrorCounter         GetErrorCount  () {return m_errorCount;};
};

#endif
