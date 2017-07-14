#ifndef HIC_H
#define HIC_H

#include <vector>
#include "TAlpide.h"

class TAlpide;

class THic {
 private:
  std::vector <TAlpide*> m_chips;
  // TPowerBoard *m_powerBoard;
  int                   m_moduleId;
  int                   m_chanVdda;
  int                   m_chanVddd;
  int                   m_chanBias;
  char                  m_dbId[50];
 protected:
 public:
  THic (const char *dbId, int modId,/*TPowerBoard pb, */ int chanVddd, int chanVdda, int chanBias);
  bool  IsPowered      ();
  void  PowerOn        ();
  void  PowerOff       ();
  float GetIddd        () {return 0;};
  float GetIdda        () {return 0;};
  float GetIBias       () {return 0;};
  float GetTemperature ();
  char *GetDbId        () {return m_dbId;};
  int   GetModId       () {return m_moduleId;};
  int   GetNChips      () {return m_chips.size();};
  int   AddChip        (TAlpide *chip);
};


#endif
