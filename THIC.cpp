#include "THIC.h"
#include <cstring>

THic::THic (const char *id, int modId, /*TPowerBoard *pb, */ int chanVddd, int chanVdda, int chanBias)
{
  strcpy(m_dbId, id);
  //m_powerBoard = pb;
  m_chanVddd = chanVddd;
  m_chanVdda = chanVdda;
  m_chanBias = chanBias;

  m_moduleId = modId;
  m_chips.clear();
}


int THic::AddChip (TAlpide *chip) 
{
  m_chips.push_back(chip); 
  return GetNChips();
}


bool THic::IsPowered() 
{
  // return PowerBoard channels for Vdda, vddd and back bias are on
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


THicIB::THicIB (const char *dbId, int modId,/*TPowerBoard pb, */ int chanVddd, int chanVdda, int chanBias)
  : THic (dbId, modId, chanVddd, chanVdda, chanBias)
{
}


THicOB::THicOB (const char *dbId, int modId,/*TPowerBoard pb, */ int chanVddd, int chanVdda, int chanBias)
  : THic (dbId, modId, chanVddd, chanVdda, chanBias)
{
}
