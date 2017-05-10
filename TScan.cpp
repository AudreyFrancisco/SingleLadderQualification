#include <iostream>
#include "TScan.h"
#include "AlpideConfig.h"

bool fScanAbort;

TScan::TScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue, std::mutex *aMutex) 
{
  m_config = config;
  m_chips  = chips; 
  m_boards = boards;

  m_histoQue = histoQue;
  m_mutex    = aMutex;

  fScanAbort = false;

  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  sprintf(config->GetfNameSuffix(), "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
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

  //std::cout << "in count enabled chips, boards_size = " << m_boards.size() << ", chips_size = " << m_chips.size() << std::endl;
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


void TScan::CreateScanHisto () 
{
  TChipIndex id; 
  m_histo = new TScanHisto();

  THisto histo = CreateHisto ();

  for (int iboard = 0; iboard < m_boards.size(); iboard ++) {
    for (int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if ((m_chips.at(ichip)->GetConfig()->IsEnabled()) && (m_chips.at(ichip)->GetReadoutBoard() == m_boards.at(iboard))) {
        id.boardIndex       = iboard;
        id.dataReceiver     = m_chips.at(ichip)->GetConfig()->GetParamValue("RECEIVER"); 
        id.chipId           = m_chips.at(ichip)->GetConfig()->GetChipId();

        m_histo->AddHisto (id, histo);        
      }
    }
  }  
  std::cout << "CreateHisto: generated map with " << m_histo->GetSize() << " elements" << std::endl;
}



TMaskScan::TMaskScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue, std::mutex *aMutex) 
  : TScan(config, chips, boards, histoQue, aMutex)
{
  m_pixPerStage = m_config->GetParamValue("PIXPERREGION");
  m_stuck.clear  ();
}


void TMaskScan::ConfigureMaskStage(TAlpide *chip, int istage) {
  m_row = AlpideConfig::ConfigureMaskStage (chip, m_pixPerStage, istage);
}


