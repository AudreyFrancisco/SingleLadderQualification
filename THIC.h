#ifndef HIC_H
#define HIC_H

#include <vector>
#include "TAlpide.h"

class THic {
 private:
  std::vector <TAlpide> m_chips;
  // TPowerBoard *m_powerBoard;
  int                   m_chanVdda;
  int                   m_chanVddd;
  int                   m_chanBias;
  char                  m_dbId[50];
 protected:
 public:
  THic (const char *id, /*TPowerBoard pb, */ int chanVddd, int chanVdda, int chanBias);
  float GetIddd   () {return 0;};
  float GetIdda   () {return 0;};
  float GetIBias  () {return 0;};
  char *GetDbId   () {return m_dbId;};
  int   GetNChips () {return m_chips.size();};
  int  AddChip  (TAlpide chip) {m_chips.push_back(chip); return GetNChips();};
};


#endif
