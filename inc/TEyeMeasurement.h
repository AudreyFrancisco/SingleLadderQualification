#ifndef TEYEMEASUREMENT_H
#define TEYEMEASUREMENT_H

#include <mutex>

#include "Common.h"
#include "THisto.h"
#include "TScan.h"

class TEyeMeasurement : public TScan {
private:
  TAlpide *m_testChip;
  int      m_boardIndex;

protected:
  THisto CreateHisto();

public:
  TEyeMeasurement(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                  std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                  std::mutex *aMutex);
  ~TEyeMeasurement(){};

  void Init();
  void Execute();
  void Terminate();
  void LoopEnd(int loopIndex);
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void PrepareStep(int loopIndex);
};


#endif
