#ifndef TCYCLEANALYSIS_H
#define TCYCLEANALYSIS_H

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"

class TCycleResultHic : public TScanResultHic {
  friend class TCycleAnalysis;
 private:
 protected:
 public:
  TCycleResultHic () : TScanResultHic () {};
};


class TCycleResult : public TScanResult {
  friend class TCycleAnalysis;
 private:
 protected:
 public:
  TCycleResult () : TScanResult() {};
  void WriteToFileGlobal (FILE *fp) {(void)fp;};
};


class TCycleAnalysis : public TScanAnalysis {
 private:
 protected:
  void             CreateResult () {};
  void             InitCounters () {};
 public:
  TCycleAnalysis(std::deque<TScanHisto> *histoQue,
                 TScan                  *aScan,
                 TScanConfig            *aScanConfig,
                 std::vector <THic*>     hics,
                 std::mutex             *aMutex,
                 TCycleResult           *aResult = 0);
  void Initialize () {CreateHicResults();};
  void Run        () {};
  void Finalize   ();
};



#endif
