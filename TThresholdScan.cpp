#include "TThresholdScan.h"

TThresholdScan::TThresholdScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards) 
  : TMaskScan (config, chips, boards) 
{
  m_start[0] = m_config->GetChargeStart();
  m_stop [0] = m_config->GetChargeStop ();
  m_step [0] = m_config->GetChargeStep ();
}
