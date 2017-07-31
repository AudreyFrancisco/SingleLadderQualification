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


bool THic::IsPowered() 
{
  // TODO: what if partially powered? What about bias?
  if (m_powerBoard && m_powerBoard->IsAnalogChOn(m_pbMod) && m_powerBoard->IsDigitalChOn(m_pbMod)) return true;
  return false;
}


void THic::PowerOn()
{
  if (IsPowered()) return;

  // otherwise: power on
}


void THic::PowerOff()
{
  if (!IsPowered()) return;
  
  //otherwise: power off 
  // Q: do we need to consider case where part of the channels is on?
}


float THic::GetTemperature() 
{
  float result = 0; 
  int   nChips = 0;

  for (int i = 0; i < m_chips.size(); i++) {
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
}


common::TChipIndex THicIB::GetChipIndex (int i) 
{
  common::TChipIndex idx;
  if (i > m_chips.size()) {
    std::cout << "Error (THicIB::GetChipIndex): trying to access bad chip" << std::endl;
    return idx;
  }
  TAlpide *chip = m_chips.at(i);

  idx.chipId       = chip->GetConfig()->GetChipId() & 0xf;
  idx.boardIndex   = m_boardidx;
  idx.dataReceiver = m_rcv[idx.chipId];

  return idx;
}


bool THicIB::ContainsChip (common::TChipIndex idx)
{
  // probably the check on board id is enough...
  if ((idx.boardIndex == m_boardidx) &&
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
  if (i > m_chips.size()) {
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
    if ((idx.boardIndex   == m_boardidx0) &&
        (idx.dataReceiver == m_rcv0)) 
      return true;
  }
  else {
    if ((idx.boardIndex   == m_boardidx8) &&
        (idx.dataReceiver == m_rcv8)) 
      return true;    
  }

  return false;
}
