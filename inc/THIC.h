#ifndef HIC_H
#define HIC_H

#include <vector>
#include <string>
#include "TAlpide.h"
#include "TPowerBoard.h"
#include "Common.h"

class TAlpide;


class THic {
 private:
 protected:
  std::vector <TAlpide*> m_chips;
  TPowerBoard          *m_powerBoard;
  int                   m_moduleId;  // module ID as used in chip IDs
  int                   m_pbMod;     // module number inside power board
  // unique identifiers
  int                   m_hicNumber; // TODO: find out name and format ...
  std::string           m_dbId;      // ... in db: int? string?
 public:
  THic (const char *dbId, int modId, TPowerBoard *pb, int pbMod);
  int                  GetNumber      () {return m_hicNumber;};
  bool                 IsPowered      ();
  void                 PowerOn        ();
  void                 PowerOff       ();
  float                GetIddd        () {return 0;};
  float                GetIdda        () {return 0;};
  float                GetIBias       () {return 0;};
  float                GetTemperature ();
  std::string          GetDbId        () {return m_dbId;};
  int                  GetModId       () {return m_moduleId;};
  int                  GetNChips      () {return m_chips.size();};
  int                  AddChip        (TAlpide *chip);
  virtual bool         ContainsChip   (common::TChipIndex idx) = 0;
  bool                 ContainsChip   (int index);
  virtual common::TChipIndex GetChipIndex   (int i) = 0;
};


class THicOB : public THic {
 private:
  int                   m_boardidx0;  // readout board index for master 0
  int                   m_boardidx8;  // readout board index for master 0
  int                   m_rcv0;       // receiver for master 0
  int                   m_rcv8;       // receiver for master 8
  int                   m_ctrl0;      // control interface for master 0
  int                   m_ctrl8;      // control interface for master 8
 protected: 
 public:
  THicOB (const char *dbId, int modId, TPowerBoard *pb, int pbMod);
  common::TChipIndex GetChipIndex    (int i);
  bool               ContainsChip    (common::TChipIndex idx);
  void               ConfigureMaster (int Master, int board, int rcv, int ctrl);
};


class THicIB : public THic {
 private: 
  int                  m_boardidx;
  int                  m_rcv[9];
  int                  m_ctrl;       // control interface
 protected:
 public:
  THicIB (const char *dbId, int modId, TPowerBoard *pb, int pbMod);
  common::TChipIndex GetChipIndex (int i);
  bool               ContainsChip (common::TChipIndex idx);
};


#endif