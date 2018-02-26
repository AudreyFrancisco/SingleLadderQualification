#ifndef HIC_H
#define HIC_H

#include "Common.h"
#include "TAlpide.h"
#include "TPowerBoard.h"
#include "TScanAnalysis.h"
#include <string>
#include <vector>

class TAlpide;

typedef enum { HIC_IB, HIC_OB } THicType;

class THic {
private:
protected:
  std::vector<TAlpide *> m_chips;
  TPowerBoard *          m_powerBoard;
  int                    m_moduleId; // module ID as used in chip IDs
  int                    m_pbMod;    // module number inside power board
  // unique identifiers
  int                m_hicNumber; // TODO: find out name and format ...
  std::string        m_dbId;      // ... in db: int? string?
  THicClassification m_class;

public:
  THic(const char *dbId, int modId, TPowerBoard *pb, int pbMod);
  virtual ~THic(){};
  int          GetNumber() { return m_hicNumber; };
  bool         IsPowered();
  bool         IsPoweredAnalog();
  bool         IsPoweredDigital();
  bool         IsEnabled();
  void         Disable();
  unsigned int GetNEnabledChips();
  virtual void PowerOn() = 0;
  void         PowerOff();
  float        GetIddd();
  float        GetIdda();
  float        GetIBias();
  float        GetTemperature();
  void ScaleVoltage(float aFactor);
  std::string  GetDbId() { return m_dbId; };
  int          GetModId() { return m_moduleId; };
  unsigned int GetNChips() { return m_chips.size(); };
  int AddChip(TAlpide *chip);
  virtual bool ContainsChip(common::TChipIndex idx) = 0;
  bool ContainsChip(int index);
  virtual bool ContainsReceiver(int boardIndex, int rcv) = 0;
  virtual int GetReceiver(int boardIndex, int chipId)    = 0;
  virtual common::TChipIndex GetChipIndex(int i) = 0;
  virtual std::vector<int> GetBoardIndices()     = 0;
  virtual THicType         GetHicType()          = 0;
  std::vector<TAlpide *>   GetChips() { return m_chips; };
  TAlpide *GetChipById(int chipId);
  TPowerBoard *GetPowerBoard() { return m_powerBoard; };
  int          GetPbMod() { return m_pbMod; };
  void SwitchBias(bool on);
  float GetAnalogueVoltage();
  void AddClassification(THicClassification aClass);
  THicClassification GetClassification();
};

class THicOB : public THic {
private:
  int m_boardidx0; // readout board index for master 0
  int m_boardidx8; // readout board index for master 0
  int m_rcv0;      // receiver for master 0
  int m_rcv8;      // receiver for master 8
  int m_ctrl0;     // control interface for master 0
  int m_ctrl8;     // control interface for master 8
protected:
public:
  THicOB(const char *dbId, int modId, TPowerBoard *pb, int pbMod);
  virtual ~THicOB(){};
  common::TChipIndex GetChipIndex(int i);
  THicType         GetHicType() { return HIC_OB; };
  std::vector<int> GetBoardIndices();
  bool ContainsChip(common::TChipIndex idx);
  bool ContainsReceiver(int boardIndex, int rcv);
  virtual int GetReceiver(int boardIndex, int chipId);
  void ConfigureMaster(int Master, int board, int rcv, int ctrl);
  void PowerOn();
};

class THicIB : public THic {
private:
  int m_boardidx;
  int m_rcv[9];
  int m_ctrl; // control interface
protected:
public:
  THicIB(const char *dbId, int modId, TPowerBoard *pb, int pbMod);
  virtual ~THicIB(){};
  common::TChipIndex GetChipIndex(int i);
  THicType         GetHicType() { return HIC_IB; };
  std::vector<int> GetBoardIndices();
  bool ContainsChip(common::TChipIndex idx);
  bool ContainsReceiver(int boardIndex, int rcv);
  virtual int GetReceiver(int boardIndex, int chipId);
  void ConfigureInterface(int board, int *rcv, int ctrl);
  void PowerOn();
};

#endif
