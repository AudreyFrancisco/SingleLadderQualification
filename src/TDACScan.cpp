#include "TDACScan.h"
#include "AlpideConfig.h"

TDACScan::TDACScan (TScanConfig                   *config, 
                    std::vector <TAlpide *>        chips, 
                    std::vector <THic*>            hics, 
                    std::vector <TReadoutBoard *>  boards, 
                    std::deque  <TScanHisto>      *histoQue, 
                    std::mutex                    *aMutex) 
  : TScan (config, chips, hics, boards, histoQue, aMutex) 
{
  strcpy(m_name, "DAC Scan");

  m_start[0]  = m_config->GetParamValue("DACSTART");
  m_stop [0]  = m_config->GetParamValue("DACSTOP");
  m_step [0]  = m_config->GetParamValue("DACSTEP");

  m_start[1]  = Alpide::REG_VRESETP;
  m_step [1]  = 1;
  m_stop [1]  = Alpide::REG_ITHR; 

  m_start[2]  = 0;
  m_step [2]  = 1;
  m_stop [2]  = 1;  // number of chips per hic?

  CreateScanHisto();
}


THisto TDACScan::CreateHisto()
{
  // write currents/voltages for all DAC scans of 1 chip 
  // x-axis: DAC register - 0x601
  // y-axis: DAC setting
  THisto histo ("DAC measurement", "DAC measurement", m_stop[1] - m_start[1]+1 , 0, m_stop[1]-m_start[1],256, 0, 255);
  return histo;
}


void TDACScan::ConfigureChip  (TAlpide *chip)
{
  AlpideConfig::BaseConfig   (chip);
  AlpideConfig::ConfigureCMU (chip);
}


void TDACScan::Init        ()
{
  TScan::Init();
  m_running = true;
  CountEnabledChips();

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    std::cout << "Board " << i << ", found " << m_enabled[i] << " enabled chips" << std::endl;

    m_boards.at(i)->SendOpCode (Alpide::OPCODE_GRST);
    m_boards.at(i)->SendOpCode (Alpide::OPCODE_PRST);
  }

  for (unsigned int i = 0; i < m_chips.size(); i ++) {
    if (! (m_chips.at(i)->GetConfig()->IsEnabled())) continue;
    ConfigureChip (m_chips.at(i));
  }
}


void TDACScan::Execute ()
{
}


void TDACScan::Next (int loopIndex) 
{
}


void TDACScan::LoopEnd (int loopIndex)
{
}


void TDACScan::PrepareStep (int loopIndex)
{
}


void TDACScan::Terminate ()
{
}
