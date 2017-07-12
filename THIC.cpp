#include "THIC.h"
#include <cstring>

THic::THic (const char *id, /*TPowerBoard *pb, */ int chanVddd, int chanVdda, int chanBias)
{
  strcpy(m_dbId, id);
  //m_powerBoard = pb;
  m_chanVddd = chanVddd;
  m_chanVdda = chanVdda;
  m_chanBias = chanBias;

  m_chips.clear();
}
