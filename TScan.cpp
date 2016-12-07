#include "TScan.h"

bool fScanAbort;

TScan::TScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards) 
{
  m_config = config;
  m_chips  = chips; 
  m_boards = boards;

  fScanAbort = false;
}


bool TScan::Loop (int loopIndex) 
{
  if (fScanAbort) return false;     // check for abort flag first

  if ((m_step[loopIndex] > 0) && (m_value[loopIndex] < m_stop[loopIndex])) return true;  // limit check for positive steps 
  if ((m_step[loopIndex] < 0) && (m_value[loopIndex] > m_stop[loopIndex])) return true;  // same for negative steps

  return false;

}


void TScan::Next (int loopIndex) 
{
  m_value[loopIndex] += m_step[loopIndex];
}


TMaskScan::TMaskScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards) 
: TScan(config, chips, boards)
{
}


