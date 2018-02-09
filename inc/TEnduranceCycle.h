#ifndef TENDURANCECYCLE_H
#define TENDURANCECYCLE_H

#include <map>
#include <mutex>
#include <string>

#include "Common.h"
#include "THIC.h"
#include "THisto.h"
#include "TScan.h"

typedef struct {
  THicType m_hicType;
  bool m_trip;
  float m_iddaClocked;
  float m_idddClocked;
  float m_iddaConfigured;
  float m_idddConfigured;
  float m_tempStart;
  float m_tempEnd;
  int m_nWorkingChips;
} THicCounter;

typedef struct __TCycleParameters : TScanParameters {
  int upTime;
  int downTime;
  int nTriggers;
  int nCycles;
} TCycleParameters;

class TEnduranceCycle : public TScan {
private:
  void CreateMeasurements();
  void ClearCounters();
  THisto CreateHisto() {
    THisto histo;
    return histo;
  };
  void ConfigureBoard(TReadoutBoard *board);
  void ConfigureFromu(TAlpide *chip);
  void ConfigureChip(TAlpide *chip);
  void ConfigureMask(TAlpide *chip);
  void CountWorkingChips();
  std::map<std::string, THicCounter> m_hicCounters;
  std::vector<std::map<std::string, THicCounter>> m_counterVector;

protected:
public:
  TEnduranceCycle(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                  std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                  std::mutex *aMutex);
  ~TEnduranceCycle(){};

  void Init();
  void Execute();
  void Terminate();
  void Next(int loopIndex);
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void LoopEnd(int loopIndex) { (void)loopIndex; };
  void PrepareStep(int loopIndex);
  std::vector<std::map<std::string, THicCounter>> GetCounters() { return m_counterVector; };
};

#endif
