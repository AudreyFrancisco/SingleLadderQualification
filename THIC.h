#ifndef HIC_H
#define HIC_H

#include <vector>
#include "TAlpide.h"

class TAlpide;

class THic {
 private:
 protected:
  std::vector <TAlpide*> m_chips;
  // TPowerBoard *m_powerBoard;
  int                   m_moduleId;
  int                   m_chanVdda;
  int                   m_chanVddd;
  int                   m_chanBias;
  char                  m_dbId[50];
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


class THicOB : public THic {
 private:
  int                   m_boardidx0;  // readout board index for master 0
  int                   m_boardidx8;  // readout board index for master 0
  int                   m_rcv0;       // receiver for master 0
  int                   m_rcv8;       // receiver for master 8
 protected: 
 public:
  THicOB (const char *dbId, int modId,/*TPowerBoard pb, */ int chanVddd, int chanVdda, int chanBias);
};


class THicIB : public THic {
 private: 
  int                  m_boardidx;
  int                  m_rcv[9];
 protected:
 public:
  THicIB (const char *dbId, int modId,/*TPowerBoard pb, */ int chanVddd, int chanVdda, int chanBias);
};


#endif
