#include <unistd.h>
#include <string.h>
#include <string>
#include "TSCurveScan.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardDAQ.h"
#include "AlpideConfig.h"


TSCurveScan::TSCurveScan       (TScanConfig                   *config,
                                std::vector <TAlpide *>        chips,
                                std::vector <THic*>            hics,
                                std::vector <TReadoutBoard *>  boards,
                                std::deque<TScanHisto>        *histoQue,
                                std::mutex                    *aMutex)
  : TMaskScan (config, chips, hics, boards, histoQue, aMutex) 
{
  m_backBias  = m_config->GetBackBias  ();
  m_nominal   = (m_config->GetParamValue("NOMINAL") == 1);
}


TThresholdScan::TThresholdScan (TScanConfig                   *config,
                                std::vector <TAlpide *>        chips,
                                std::vector <THic*>            hics,
                                std::vector <TReadoutBoard *>  boards,
                                std::deque<TScanHisto>        *histoQue,
                                std::mutex                    *aMutex)
  : TSCurveScan (config, chips, hics, boards, histoQue, aMutex)
{
  m_start[0]  = m_config->GetChargeStart();
  m_stop [0]  = m_config->GetChargeStop ();
  m_step [0]  = m_config->GetChargeStep ();

  m_start[1]  = 0;
  m_step [1]  = 1;
  m_stop [1]  = m_config->GetNMaskStages();

  m_start[2]  = 0;
  m_step [2]  = 1;
  m_stop [2]  = 1;

  m_VPULSEH   = 170;
  m_nTriggers = m_config->GetParamValue("NINJ");

  sprintf(m_name, "Threshold Scan %.1f V", m_backBias); 

  CreateScanHisto();
}

TtuneVCASNScan::TtuneVCASNScan (TScanConfig                   *config,
                                std::vector <TAlpide *>        chips,
                                std::vector <THic*>            hics,
                                std::vector <TReadoutBoard *>  boards,
                                std::deque<TScanHisto>        *histoQue,
                                std::mutex                    *aMutex)
  : TSCurveScan (config, chips, hics, boards, histoQue, aMutex)
{
  strcpy(m_name, "Tune VCASN Scan");
  m_start[0]  = m_config->GetVcasnStart();
  m_stop [0]  = m_config->GetVcasnStop ();
  m_step [0]  = m_config->GetVcasnStep ();

  m_start[1]  = 0;
  m_step [1]  = m_config->GetScanStep   ();
  m_stop [1]  = m_config->GetParamValue ("TUNINGMAXROW");

  m_start[2]  = 0;
  m_step [2]  = 1;
  m_stop [2]  = 1;

  m_VPULSEH   = 170;
  m_TARGET    = m_config->GetParamValue("TARGETTHRESH");
  if (m_TARGET % 10) {
    m_TARGET -= (m_TARGET %10);
    m_TARGET += 10;
    std::cout << "Warning: threshold target not multiple of 10, rounding up to " << m_TARGET; 
  }
  m_VPULSEL = m_VPULSEH - m_TARGET / 10;
  m_nTriggers = m_config->GetParamValue("NINJ");
  CreateScanHisto();
}

TtuneITHRScan::TtuneITHRScan   (TScanConfig                   *config,
                                std::vector <TAlpide *>        chips,
                                std::vector <THic*>            hics,
                                std::vector <TReadoutBoard *>  boards,
                                std::deque<TScanHisto>        *histoQue,
                                std::mutex                    *aMutex)
  : TSCurveScan (config, chips, hics, boards, histoQue, aMutex)
{
  strcpy(m_name, "Tune ITHR Scan");
  m_start[0]  = m_config->GetIthrStart();
  m_stop [0]  = m_config->GetIthrStop ();
  m_step [0]  = m_config->GetIthrStep ();

  m_start[1]  = 0;
  m_step [1]  = m_config->GetScanStep   ();
  m_stop [1]  = m_config->GetParamValue ("TUNINGMAXROW");

  m_start[2]  = 0;
  m_step [2]  = 1;
  m_stop [2]  = 1;

  m_VPULSEH   = 170;
  m_TARGET    = m_config->GetParamValue("TARGETTHRESH");
  if (m_TARGET % 10) {
    m_TARGET -= (m_TARGET %10);
    m_TARGET += 10;
    std::cout << "Warning: threshold target not multiple of 10, rounding up to " << m_TARGET; 
  }
  m_VPULSEL = m_VPULSEH - m_TARGET / 10;
  m_nTriggers = m_config->GetParamValue("NINJ");
  CreateScanHisto();
}



void TSCurveScan::RestoreNominalSettings()
{
  if (m_backBias == 0.0) {
    for (unsigned int i = 0; i < m_chips.size(); i++) {
      m_chips.at(i)->GetConfig()->SetParamValue("ITHR",   50);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN",  50);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN2", 57);
      m_chips.at(i)->GetConfig()->SetParamValue("VCLIP",  0);
    }
  }
  else if ((m_backBias > 2.99) && (m_backBias < 3.01)) {
    for (unsigned int i = 0; i < m_chips.size(); i++) {
      m_chips.at(i)->GetConfig()->SetParamValue("ITHR",   50);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN",  105);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN2", 117);
      m_chips.at(i)->GetConfig()->SetParamValue("VCLIP",  60);
    }
  }
}


void TSCurveScan::ConfigureBoard(TReadoutBoard *board)
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


void TSCurveScan::ConfigureFromu(TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x20);      // analogue pulsing
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  chip->GetConfig()->GetParamValue("STROBEDURATION"));  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, chip->GetConfig()->GetParamValue("STROBEDELAYCHIP"));   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, chip->GetConfig()->GetParamValue("PULSEDURATION"));   // fromu pulsing 2: pulse length
}

void TThresholdScan::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  ConfigureFromu(chip);

  AlpideConfig::ConfigureCMU (chip);
}

void TtuneVCASNScan::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  ConfigureFromu(chip);

  AlpideConfig::ConfigureCMU (chip);

  for(unsigned int i = 0; i < m_chips.size(); i++) {
    m_chips.at(i)->WriteRegister(Alpide::REG_VPULSEL, m_VPULSEL);
  }
}

void TtuneITHRScan::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  ConfigureFromu(chip);

  AlpideConfig::ConfigureCMU (chip);

  for(unsigned int i = 0; i < m_chips.size(); i++) {
    m_chips.at(i)->WriteRegister(Alpide::REG_VPULSEL, m_VPULSEL);
  }
}


THisto TSCurveScan::CreateHisto() {
  THisto histo ("ThresholdHisto", "ThresholdHisto", 1024, 0, 1023, 1+(m_stop[0] - m_start[0]) / m_step[0], m_start[0], m_stop[0]);
  std::cout << "CREATING: " << (m_stop[0]-m_start[0])/m_step[0] << ", " << m_start[0] << ", " << m_stop[0] << std::endl;
  return histo;
}


void TSCurveScan::Init() {
  TScan::Init();

  if (m_nominal) RestoreNominalSettings();

  m_running = true;

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TPowerBoard *pb = m_hics.at(ihic)->GetPowerBoard();
    if (!pb) continue;
    if (m_backBias == 0) {
      m_hics.at(ihic)->SwitchBias (false);
      pb             ->SetBiasVoltage(0);
    }
    else {
      m_hics.at(ihic)->SwitchBias   (true);
      pb             ->SetBiasVoltage( (-1.)* m_backBias);
    }
  }

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
    TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (m_boards.at(i));

    if (myMOSAIC) {
     myMOSAIC->StartRun();
    }
  }
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if(!m_hics.at(ihic)->GetPowerBoard()) continue;
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }
}



void TThresholdScan::PrepareStep (int loopIndex)
{
  switch (loopIndex) {
  case 0:    // innermost loop: change VPULSEL
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if (! m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->WriteRegister(Alpide::REG_VPULSEL, m_VPULSEH - m_value[0]);
    }
    break;
  case 1:    // 2nd loop: mask staging
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if (! m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      ConfigureMaskStage(m_chips.at(ichip), m_value[1]);
    }
    sprintf(m_state, "Running %d", m_value[1]);
    break;
  default:
    break;
  }
}


//Need different registers for different classes...
void TtuneVCASNScan::PrepareStep (int loopIndex)
{
  switch (loopIndex) {
  case 0:    // innermost loop: change VCASN
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if (! m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->WriteRegister(Alpide::REG_VCASN, m_value[0]);
      m_chips.at(ichip)->WriteRegister(Alpide::REG_VCASN2, m_value[0] + 12);
    }
    break;
  case 1:    // 2nd loop: mask staging
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if (! m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      ConfigureMaskStage(m_chips.at(ichip), m_value[1]);
    }
    break;
  default:
    break;
  }
}

void TtuneITHRScan::PrepareStep (int loopIndex)
{
  switch (loopIndex) {
  case 0:    // innermost loop: change ITHR
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if (! m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->WriteRegister(Alpide::REG_ITHR, m_value[0]);
    }
    break;
  case 1:    // 2nd loop: mask staging
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if (! m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      ConfigureMaskStage(m_chips.at(ichip), m_value[1]);
    }
    break;
  default:
    break;
  }
}



void TSCurveScan::Execute()
{
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
    m_boards.at(iboard)->Trigger(m_nTriggers);
  }

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
		Hits->clear();
    usleep(1000);
    ReadEventData (Hits, iboard);
    FillHistos    (Hits, iboard);
  }
  delete Hits;
}


void TSCurveScan::FillHistos (std::vector<TPixHit> *Hits, int board)
{
  common::TChipIndex idx;
  idx.boardIndex = board;
  for (unsigned int i = 0; i < Hits->size(); i++) {
    if (Hits->at(i).address / 2 != m_row) continue;  // todo: keep track of spurious hits, i.e. hits in non-injected rows
    // !! This will not work when allowing several chips with the same Id
    idx.dataReceiver = Hits->at(i).channel;
    idx.chipId       = Hits->at(i).chipId;

    int col = Hits->at(i).region * 32 + Hits->at(i).dcol * 2;
    int leftRight = ((((Hits->at(i).address % 4) == 1) || ((Hits->at(i).address % 4) == 2))? 1:0);
    col += leftRight;
    // TODO: Catch this case earlier (do not fill hit vector for corrupt events
    try {
      m_histo->Incr(idx, col, m_value[0] - m_start[0]); //m_value is too large (>20) often!!
    }
    catch (...) {
      std::cout << "Caught exception in TSCurveScan::FillHistos, trying to fill histo for chipID " << idx.chipId << ", receiver " << idx.dataReceiver << std::endl;
    }
  }


}


void TSCurveScan::LoopEnd(int loopIndex)
{
  if (loopIndex == 0) {
    while (!(m_mutex->try_lock()));
    m_histo   ->SetIndex(m_row);
    std::cout << "SCAN: Writing histo with row " << m_histo->GetIndex() << std::endl;
    m_histoQue->push_back(*m_histo);
    m_mutex   ->unlock();
    m_histo   ->Clear();
  }
}


void TSCurveScan::Terminate()
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

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (m_backBias != 0) {
      m_hics.at(ihic)->SwitchBias (false);
      m_hics.at(ihic)->GetPowerBoard()->SetBiasVoltage(0);
    }
  }

  m_running = false;
  //YCM: Print error summary
  std::cout << "Total number of 8b10b decoder errors: " << m_errorCount.n8b10b        << std::endl;
  std::cout << "Number of corrupt events:             " << m_errorCount.nCorruptEvent << std::endl;
  std::cout << "Number of skipped points:             " << m_errorCount.nTimeout      << std::endl;
  std::cout << "Priority encoder errors:              " << m_errorCount.nPrioEncoder  << std::endl;
  std::cout << std::endl;
}
