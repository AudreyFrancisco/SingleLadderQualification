#include <unistd.h>
#include <string.h>
#include "AlpideConfig.h"
#include "TDigitalScan.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardRU.h"

TDigitalScan::TDigitalScan (TScanConfig                   *config, 
                            std::vector <TAlpide *>        chips, 
                            std::vector <THic*>            hics,
                            std::vector <TReadoutBoard *>  boards, 
                            std::deque<TScanHisto>        *histoQue, 
                            std::mutex                    *aMutex) 
  : TMaskScan (config, chips, hics, boards, histoQue, aMutex) 
{
  strcpy(m_name, "Digital Scan");
  m_start[0] = 0;
  m_step [0] = 1;
  m_stop [0] = m_config->GetNMaskStages();

  m_start[1] = 0;
  m_step [1] = 1;
  m_stop [1] = 1;

  m_start[2] = 0;
  m_step [2] = 1;
  m_stop [2] = 1;

  m_nTriggers = m_config->GetParamValue("NINJ");

  CreateScanHisto();
}


void TDigitalScan::ConfigureFromu (TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x0);    // digital pulsing        
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  chip->GetConfig()->GetParamValue("STROBEDURATION"));  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, chip->GetConfig()->GetParamValue("STROBEDELAYCHIP"));   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, chip->GetConfig()->GetParamValue("PULSEDURATION"));   // fromu pulsing 2: pulse length
}


void TDigitalScan::ConfigureChip  (TAlpide *chip)
{
  AlpideConfig::BaseConfig   (chip);
  ConfigureFromu             (chip);
  AlpideConfig::ConfigureCMU (chip);
}


void TDigitalScan::ConfigureBoard (TReadoutBoard *board)
{
  if (board->GetConfig()->GetBoardType() == boardDAQ) {
    // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
    // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
    board->SetTriggerConfig (true, false, 
                             0,
                             2 * board->GetConfig()->GetParamValue("STROBEDELAYBOARD"));
    board->SetTriggerSource (trigExt);
  }
  else {
    board->SetTriggerConfig (true, true, 
                             board->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                             board->GetConfig()->GetParamValue("PULSEDELAY"));
    board->SetTriggerSource (trigInt);
  }
}


void TDigitalScan::FillHistos     (std::vector<TPixHit> *Hits, int board)
{
  common::TChipIndex idx; 
  idx.boardIndex = board;

  for (unsigned int i = 0; i < Hits->size(); i++) {
    if (Hits->at(i).address / 2 != m_row) continue;  // todo: keep track of spurious hits, i.e. hits in non-injected rows
    idx.dataReceiver = Hits->at(i).channel;
    idx.chipId       = Hits->at(i).chipId;

    int col = Hits->at(i).region * 32 + Hits->at(i).dcol * 2;
    int leftRight = ((((Hits->at(i).address % 4) == 1) || ((Hits->at(i).address % 4) == 2))? 1:0); 
    col += leftRight;
    m_histo->Incr(idx, col);
  }
}


THisto TDigitalScan::CreateHisto    ()
{
  THisto histo ("HitmapHisto", "HitmapHisto", 1024, 0, 1023);
  return histo;
}


void TDigitalScan::Init        ()
{
  TScan::Init();
  m_running = true;
  CountEnabledChips();
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    std::cout << "Board " << i << ", found " << m_enabled[i] << " enabled chips" << std::endl;
    ConfigureBoard(m_boards.at(i));

    m_boards.at(i)->SendOpCode (Alpide::OPCODE_GRST);
    m_boards.at(i)->SendOpCode (Alpide::OPCODE_PRST);
  }

  for (unsigned int i = 0; i < m_chips.size(); i ++) {
    if (! (m_chips.at(i)->GetConfig()->IsEnabled())) continue;
    ConfigureChip (m_chips.at(i));
  }
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode (Alpide::OPCODE_RORST);     
    m_boards.at(i)->StartRun   ();
  }
}


void TDigitalScan::PrepareStep (int loopIndex)
{
  switch(loopIndex) {
  case 0:    // innermost loop: mask staging
    std::cout << "mask stage " << m_value[0] << std::endl;
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if (! m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      ConfigureMaskStage(m_chips.at(ichip), m_value[0]);
    }
    break;
  default:
    break;
  }
}


void TDigitalScan::LoopEnd     (int loopIndex)
{

}


void TDigitalScan::Next (int loopIndex) 
{
  if (loopIndex == 0) {
    while (!(m_mutex->try_lock()));
    m_histo   ->SetIndex(m_row);
    std::cout << "SCAN: Writing histo with row " << m_histo->GetIndex() << std::endl;
    m_histoQue->push_back(*m_histo);
    m_mutex   ->unlock();
    m_histo   ->Clear();
  }
  TScan::Next (loopIndex);
}


void TDigitalScan::Execute     ()
{
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
    m_boards.at(iboard)->Trigger(m_nTriggers);
  }

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
		Hits->clear();
    ReadEventData(Hits, iboard);
    FillHistos   (Hits, iboard);
  }
  delete Hits;
}


void TDigitalScan::Terminate   ()
{
  TScan::Terminate();
  // write Data;
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
    TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (m_boards.at(iboard));
    if (myMOSAIC) {
      myMOSAIC->StopRun();
      //delete myMOSAIC;
    }
    TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (m_boards.at(iboard));
    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      //delete myDAQBoard;
    }
  }
  m_running = false;
}


