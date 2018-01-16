#ifndef TLOCALBUSTEST_H
#define TLOCALBUSTEST_H

#include <mutex>
#include "TScan.h"
#include "THisto.h"

class TLocalBusTest : public TScan {
private:
  TAlpide *m_readChip;
  TAlpide *m_writeChip;
  int m_boardIndex;
  std::vector<std::vector<TAlpide *>> m_daisyChains;
  void FindDaisyChains(std::vector<TAlpide *> chips);
  int GetChipById(std::vector<TAlpide *> chips, int previousId);
  bool TestPattern(int pattern);
  bool TestBusy(bool busy);

protected:
  THisto CreateHisto();

public:
  TLocalBusTest(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                std::mutex *aMutex);
  ~TLocalBusTest() {};
  void Init();
  void Execute();
  void Terminate();

  void Next(int loopIndex);
  void LoopStart(int loopIndex) {
    m_value[loopIndex] = m_start[loopIndex];
  };
  void LoopEnd(int loopIndex);
  void PrepareStep(int loopIndex);
};

#endif
