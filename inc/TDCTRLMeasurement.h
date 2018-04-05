#ifndef TDCTRLMEASUREMENT_H
#define TDCTRLMEASUREMENT_H

#include <mutex>

#include "Common.h"
#include "THisto.h"
#include "TScan.h"
#include "scope_control.h"


class TDctrlMeasurement : public TScan {
private:
  TAlpide *m_testChip;
  int      m_boardIndex;

  int GetChipById(std::vector<TAlpide *> chips, int previousId);
  void ReadMem(TAlpide *chip, int ARegion, int AOffset, int &AValue, bool &exception);
  void WriteMem(TAlpide *chip, int ARegion, int AOffset, int AValue);
  bool TestPattern(int pattern, bool &exception);
  void InitScope();

protected:
  THisto CreateHisto();

public:
  TDctrlMeasurement(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                    std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                    std::mutex *aMutex);
  ~TDctrlMeasurement(){};
  void Init();
  void Execute();
  void Terminate();
  void LoopEnd(int loopIndex);
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void PrepareStep(int loopIndex);
  scope_control    scope;
  static const int peak_p = 0;
  static const int peak_n = 1;
  static const int amp_p  = 2;
  static const int amp_n  = 3;
  static const int rtim_p = 4;
  static const int rtim_n = 5;
  static const int ftim_p = 6;
  static const int ftim_n = 7;
};

#endif
