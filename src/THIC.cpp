#include "THIC.h"
#include <cstring>

THic::THic (const char *id, int modId, TPowerBoard *pb, int pbMod)
{
  m_dbId.assign(id);

  m_powerBoard = pb;
  m_pbMod      = pbMod;
  m_moduleId   = modId;

  m_chips.clear();
}


int THic::AddChip (TAlpide *chip) 
{
  m_chips.push_back(chip); 
  return GetNChips();
}


bool THic::ContainsChip (int index)
{
  common::TChipIndex idx;
  idx.boardIndex   = (index >> 8) & 0xf;
  idx.dataReceiver = (index >> 4) & 0xf;
  idx.chipId       = index & 0xf;
  
  return ContainsChip (idx);
}

TAlpide *THic::GetChipById (int chipId)
{
  for (unsigned int i = 0; i < m_chips.size(); i ++) {
    if ( (m_chips.at(i)->GetConfig()->GetChipId() &0xf) == (chipId & 0xf)) return m_chips.at(i);
  }
  return 0;
}


bool THic::IsPowered() 
{
  // TODO: what if partially powered? What about bias?
  if (m_powerBoard && m_powerBoard->IsAnalogChOn(m_pbMod) && m_powerBoard->IsDigitalChOn(m_pbMod)) return true;
  return false;
}


void THic::PowerOn()
{
  if (IsPowered()) return;

  if (m_powerBoard) m_powerBoard->SwitchModule(m_pbMod, true);
}


void THic::PowerOff()
{
  if (!IsPowered()) return;
  
  if (m_powerBoard) m_powerBoard->SwitchModule(m_pbMod, false);
  // Q: do we need to consider case where part of the channels is on?
}


float THic::GetIddd() 
{
  if (m_powerBoard) {
    return m_powerBoard->GetDigitalCurrent(m_pbMod);
  }
  return 0;
}


float THic::GetIdda()
{
  if (m_powerBoard) {
    return m_powerBoard->GetAnalogCurrent(m_pbMod);
  }
  return 0;
}


float THic::GetIBias()
{
  if (m_powerBoard) {
    return m_powerBoard->GetBiasCurrent();
  }
  return 0;
}


// scales all voltages and current limits of the HIC by a given factor
// e.g. aFactor = 1.1 -> +10%
// method takes the value from the config and writes the scaled value to the board
// (config value is left unchanged)
void THic::ScaleVoltage(float aFactor)
{
  if (!m_powerBoard) return;

  TPowerBoardConfig *pbConfig = m_powerBoard->GetConfigurationHandler();
  float              AVSet, AISet, DVSet, DISet;
  bool               BiasOn;

  pbConfig->GetModuleSetUp(m_pbMod, &AVSet, &AISet, &DVSet, &DISet, &BiasOn);

  m_powerBoard->SetModule (m_pbMod, 
                           AVSet *aFactor, 
			   AISet *aFactor, 
                           DVSet *aFactor, 
			   DISet *aFactor, 
			   BiasOn);
}


float THic::GetTemperature() 
{
  float result = 0; 
  int   nChips = 0;

  for (int i = 0; i < (int)m_chips.size(); i++) {
    if (! m_chips.at(i)->GetConfig()->IsEnabled()) continue;
    result += m_chips.at(i)->ReadTemperature();
    nChips ++;
  }
  
  if (nChips > 0) return result / nChips;
  else return 0;
}


THicIB::THicIB (const char *dbId, int modId, TPowerBoard *pb, int pbMod)
  : THic (dbId, modId, pb, pbMod)
{
	m_ctrl=-1; //FIXME: init m_ctrl to avoid not used warning/error (clang)
}


void THicIB::ConfigureInterface (int board, int *rcv, int ctrl) 
{
  m_boardidx = board;
  m_ctrl     = ctrl;
  for (int i = 0; i < 9; i++) {
    m_rcv[i] = rcv[i];
  }
}


common::TChipIndex THicIB::GetChipIndex (int i) 
{
  common::TChipIndex idx;
  if (i > (int)m_chips.size()) {
    std::cout << "Error (THicIB::GetChipIndex): trying to access bad chip" << std::endl;
    return idx;
  }
  TAlpide *chip = m_chips.at(i);

  idx.chipId       = chip->GetConfig()->GetChipId() & 0xf;
  idx.boardIndex   = m_boardidx;
  idx.dataReceiver = m_rcv[idx.chipId];

  return idx;
}


std::vector<int> THicIB::GetBoardIndices () 
{
  std::vector<int> Indices;
  Indices.push_back(m_boardidx);

  return Indices;
}


bool THicIB::ContainsChip (common::TChipIndex idx)
{
  // probably the check on board id is enough...
  if (((int)idx.boardIndex == m_boardidx) &&
      (idx.chipId >= 0) &&
      (idx.chipId < 10)) return true;
  return false;
}


THicOB::THicOB (const char *dbId, int modId, TPowerBoard *pb, int pbMod)
  : THic (dbId, modId, pb, pbMod)
{
}


common::TChipIndex THicOB::GetChipIndex (int i) 
{
  common::TChipIndex idx;
  if (i > (int)m_chips.size()) {
    std::cout << "Error (THicOB::GetChipIndex): trying to access bad chip" << std::endl;
    return idx;
  }

  TAlpide *chip = m_chips.at(i);

  idx.chipId       = chip->GetConfig()->GetChipId() & 0xf;
  if (idx.chipId < 7) {
    idx.boardIndex   = m_boardidx0;
    idx.dataReceiver = m_rcv0;
  }
  else {
    idx.boardIndex   = m_boardidx8;
    idx.dataReceiver = m_rcv8;
  }

  return idx;
}


std::vector<int> THicOB::GetBoardIndices () 
{
  std::vector<int> Indices;
  Indices.push_back(m_boardidx0);
  Indices.push_back(m_boardidx8);

  return Indices;
}


void THicOB::ConfigureMaster (int Master, int board, int rcv, int ctrl) 
{
  if (Master == 0) {
    m_boardidx0 = board;
    m_rcv0      = rcv;
    m_ctrl0     = ctrl;
  }
  else if (Master == 8) {
    m_boardidx8 = board;
    m_rcv8      = rcv;
    m_ctrl8     = ctrl;
  }
  else {
    std::cout << "Warning: bad master id, doing nothing" << std::endl;
  }
}


bool THicOB::ContainsChip (common::TChipIndex idx) 
{
  if (idx.chipId < 7) {
    if (((int)idx.boardIndex   == m_boardidx0) &&
        ((int)idx.dataReceiver == m_rcv0)) 
      return true;
  }
  else {
    if (((int)idx.boardIndex   == m_boardidx8) &&
        ((int)idx.dataReceiver == m_rcv8)) 
      return true;    
  }

  return false;
}
