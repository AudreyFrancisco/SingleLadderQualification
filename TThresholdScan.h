#ifndef TTHRESHOLDSCAN_H
#define TTHRESHOLDSCAN_H

#include "TScan.h"

class TThresholdScan : public TMaskScan {
 private: 
 protected:
 public: 
  TThresholdScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards);
  ~TThresholdScan () {};
 
};


#endif
