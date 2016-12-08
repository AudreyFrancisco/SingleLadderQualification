#include "TScan.h"
#include "AlpideConfig.h"

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


void TScan::CountEnabledChips() 
{
  for (int i = 0; i < MAXBOARDS; i++) {
    m_enabled[i] = 0;
  }
  for (int iboard = 0; iboard < m_boards.size(); iboard ++) {
    for (int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if ((m_chips.at(ichip)->GetConfig()->IsEnabled()) && (m_chips.at(ichip)->GetReadoutBoard() == m_boards.at(iboard))) {
        m_enabled[iboard] ++;
      }
    }
  }

}




TMaskScan::TMaskScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards) 
: TScan(config, chips, boards)
{
}


void TMaskScan::ConfigureMaskStage(TAlpide *chip, int istage) {
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   true);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);

  AlpideConfig::WritePixRegRow(chip, Alpide::PIXREG_MASK,   false, istage);
  AlpideConfig::WritePixRegRow(chip, Alpide::PIXREG_SELECT, true, istage);
  //for (int icol = 0; icol < 1024; icol += 8) {
  //  AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_MASK,   false, istage % 512, icol + istage / 512);
  //  AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_SELECT, true,  istage % 512, icol + istage / 512);   
  //}

}


