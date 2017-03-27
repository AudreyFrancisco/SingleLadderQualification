#ifndef TLOCALBUSTEST_H
#define TLOCALBUSTEST_H

#include "TScan.h"
#include "THisto.h"

class TLocalBusTest : public TScan {
 private:
  TAlpide *m_readChip;
  TAlpide *m_writeChip;
  std::vector<std::vector <TAlpide*>> m_daisyChains;
  int      FindDaisyChains(std::vector <TAlpide *> chips);
  int      GetChipById    (std::vector <TAlpide *> chips, int previousId);
 protected: 
  THisto CreateHisto() {THisto histo; return histo;};
 public:
  TLocalBusTest   (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoque);
  ~TLocalBusTest  () {};
  void Init () {};
  void Execute ();
  void Terminate () {};
  void LoopStart (int loopIndex) {};
  void LoopEnd   (int loopIndex) {};
  void PrepareStep(int loopIndex) {};

};



#endif
