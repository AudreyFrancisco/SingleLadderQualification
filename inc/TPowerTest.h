#ifndef TPOWERTEST_H
#define TPOWERTEST_H

#include <map>
#include <mutex>
#include <string>

#include "Common.h"
#include "THIC.h"
#include "THisto.h"
#include "TScan.h"

typedef struct {
  THicType hicType;
  bool     trip;
  bool     tripBB;
  float    iddaSwitchon;
  float    idddSwitchon;
  float    iddaClocked;
  float    idddClocked;
  float    iddaConfigured;
  float    idddConfigured;
  float    ibias0;
  float    ibias3;
  float    maxBias;
  float    ibias[61];
} THicCurrents;

class TPowerTest : public TScan {
private:
  THic * m_testHic;
  void   CreateMeasurements();
  THisto CreateHisto()
  {
    THisto histo;
    return histo;
  };
  void                                DoIVCurve(THicCurrents &result);
  std::map<std::string, THicCurrents> m_hicCurrents;
  bool                                m_ivcurve;

protected:
  void CreateScanParameters() { m_parameters = new TScanParameters; };

public:
  TPowerTest(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
             std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
             std::mutex *aMutex);
  ~TPowerTest(){};

  void Init();
  void Execute();
  void Terminate();
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void LoopEnd(int loopIndex) { (void)(&loopIndex); };
  void PrepareStep(int loopIndex);
  std::map<std::string, THicCurrents> GetCurrents() { return m_hicCurrents; };
};

#endif
