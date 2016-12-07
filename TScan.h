#ifndef TSCAN_H
#define TSCAN_H

#include "TAlpide.h"
#include "TReadoutBoard.h"
#include "TScanConfig.h"

const int MAXLOOPLEVEL = 3;

class TScan {
 private:
  TScanConfig                  *m_config;
  std::vector <TAlpide *>       m_chips;
  std::vector <TReadoutBoard *> m_boards;
 protected: 
 public:
  TScan () {};
  TScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards);
  ~TScan() {};
};



class TMaskScan : public TScan {
 private: 
 protected: 
 public: 
  TMaskScan  (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards);
  ~TMaskScan () {};
};

#endif
