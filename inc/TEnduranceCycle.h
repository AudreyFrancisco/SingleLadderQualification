#ifndef TENDURANCECYCLE_H
#define TENDURANCECYCLE_H

#include <mutex>
#include <map>
#include <string>

#include "Common.h"
#include "TScan.h"
#include "THisto.h"
#include "THIC.h"

typedef struct {
  THicType hicType;
  bool     trip;
  float    iddaClocked;
  float    idddClocked;
  float    iddaConfigured;
  float    idddConfigured;
  float    tempStart;
  float    tempEnd;
  int      nWorkingChips;
} THicResult;

class TEnduranceCycle : public TScan {
 private:
  int    m_triggers;
  void   CreateMeasurements ();
  void   ClearCounters      ();
  THisto CreateHisto        () {THisto histo; return histo;};
  void   ConfigureBoard     (TReadoutBoard *board);
  void   ConfigureFromu     (TAlpide *chip);
  void   ConfigureChip      (TAlpide *chip);
  void   ConfigureMask      (TAlpide *chip);
  void   CountWorkingChips  ();
  std::map    <std::string, THicResult>            m_hicResults;
  std::vector <std::map <std::string, THicResult>> m_resultVector;
 protected:
 public:
  TEnduranceCycle  (TScanConfig                   *config,
                    std::vector <TAlpide *>        chips,
                    std::vector <THic*>            hics,
                    std::vector <TReadoutBoard *>  boards,
                    std::deque<TScanHisto>        *histoque,
                    std::mutex                    *aMutex);
  ~TEnduranceCycle () {};

  void Init        ();
  void Execute     ();
  void Terminate   ();
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void LoopEnd     (int loopIndex);
  void PrepareStep (int loopIndex) {};
  std::vector <std::map <std::string, THicResult>> GetResults () {return m_resultVector;};
};



#endif
