#include "TDigitalScan.h"
#include "AlpideConfig.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardRU.h"
#include <string.h>
#include <string>
#include <unistd.h>

TDigitalScan::TDigitalScan(TScanConfig *config, std::vector<TAlpide *> chips,
                           std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                           std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TMaskScan(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias                             = m_config->GetBackBias();
  ((TDigitalParameters *)m_parameters)->voltageScale = config->GetVoltageScale();

  SetName();

  m_start[0] = 380;
  m_step[0]  = 1;
  m_stop[0]  = 390;

  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = 1;

  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  m_nTriggers = m_config->GetParamValue("NINJ");
}


void TDigitalScan::SetName()
{
  if (IsNominal()) {
    sprintf(m_name, "Digital Scan BB %d", (int)((TDigitalParameters *)m_parameters)->backBias);
  }
  else if (IsUpper() && (((TDigitalParameters *)m_parameters)->voltageScale < 1.2)) {
    sprintf(m_name, "Digital Scan BB %d, V +10%%",
            (int)((TDigitalParameters *)m_parameters)->backBias);
  }
  else if (((TDigitalParameters *)m_parameters)->voltageScale > 0.8 && IsLower()) {
    sprintf(m_name, "Digital Scan BB %d, V -10%%",
            (int)((TDigitalParameters *)m_parameters)->backBias);
  }
  else {
    std::cout << "Warning: unforeseen voltage scale, using 1" << std::endl;
    ((TDigitalParameters *)m_parameters)->voltageScale = 1.0;
    sprintf(m_name, "Digital Scan BB %d", (int)((TDigitalParameters *)m_parameters)->backBias);
  }
}


bool TDigitalScan::SetParameters(TScanParameters *pars)
{
  TDigitalParameters *dPars = dynamic_cast<TDigitalParameters *>(pars);
  if (dPars) {
    std::cout << "TDigitalScan: Updating parameters" << std::endl;
    ((TDigitalParameters *)m_parameters)->voltageScale = dPars->voltageScale;
    ((TDigitalParameters *)m_parameters)->backBias     = dPars->backBias;
    SetName();
    return true;
  }
  else {
    std::cout << "TDigitalScan::SetParameters: Error, bad parameter type, doing nothing"
              << std::endl;
    return false;
  }
}


void TDigitalScan::ConfigureFromu(TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1, 0x0); // digital pulsing
  chip->WriteRegister(
      Alpide::REG_FROMU_CONFIG2,
      chip->GetConfig()->GetParamValue("STROBEDURATION")); // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1,
                      chip->GetConfig()->GetParamValue("STROBEDELAYCHIP")); // fromu pulsing 1:
                                                                            // delay pulse - strobe
                                                                            // (not used here, since
                                                                            // using external
                                                                            // strobe)
  chip->WriteRegister(
      Alpide::REG_FROMU_PULSING2,
      chip->GetConfig()->GetParamValue("PULSEDURATION")); // fromu pulsing 2: pulse length
}

void TDigitalScan::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);
  ConfigureFromu(chip);
  AlpideConfig::ConfigureCMU(chip);
}

void TDigitalScan::ConfigureBoard(TReadoutBoard *board)
{
  if (board->GetConfig()->GetBoardType() == boardDAQ) {
    // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe
    // delay
    // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
    board->SetTriggerConfig(true, false, 0,
                            2 * board->GetConfig()->GetParamValue("STROBEDELAYBOARD"));
    board->SetTriggerSource(trigExt);
  }
  else {
    board->SetTriggerConfig(true, true, board->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                            board->GetConfig()->GetParamValue("PULSEDELAY"));
    board->SetTriggerSource(trigInt);
  }
}

void TDigitalScan::FillHistos(std::vector<TPixHit> *Hits, int board)
{
  common::TChipIndex idx;
  idx.boardIndex = board;

  for (unsigned int i = 0; i < Hits->size(); i++) {
    if (Hits->at(i).address / 2 != m_row)
      continue; // todo: keep track of spurious hits, i.e. hits in non-injected rows
    idx.dataReceiver = Hits->at(i).channel;
    idx.chipId       = Hits->at(i).chipId;

    int col = Hits->at(i).region * 32 + Hits->at(i).dcol * 2;
    int leftRight =
        ((((Hits->at(i).address % 4) == 1) || ((Hits->at(i).address % 4) == 2)) ? 1 : 0);
    col += leftRight;
    m_histo->Incr(idx, col);
  }
}

THisto TDigitalScan::CreateHisto()
{
  THisto histo("HitmapHisto", "HitmapHisto", 1024, 0, 1023);
  return histo;
}

void TDigitalScan::Init()
{
  TScan::Init();
  m_running = true;
  SetBackBias();
  CreateScanHisto();
  CountEnabledChips();

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (((TDigitalParameters *)m_parameters)->voltageScale != 1.) {
      m_hics.at(ihic)->ScaleVoltage(((TDigitalParameters *)m_parameters)->voltageScale);
    }
  }

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    std::cout << "Board " << i << ", found " << m_enabled[i] << " enabled chips" << std::endl;
    ConfigureBoard(m_boards.at(i));

    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_PRST);
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }

  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!(m_chips.at(i)->GetConfig()->IsEnabled())) continue;
    ConfigureChip(m_chips.at(i));
  }

  //  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
  //  m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  //}

  // char dummy[10];
  // std::cout << "after configure chip, press enter to proceed" << std::endl;
  // std::cin >> dummy;

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_RORST);
    m_boards.at(i)->StartRun();
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }
}

void TDigitalScan::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop: mask staging
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      ConfigureMaskStage(m_chips.at(ichip), m_value[0]);
    }
    sprintf(m_state, "Running %d", m_value[0]);
    break;
  default:
    break;
  }
}

void TDigitalScan::LoopEnd(int loopIndex) {}

void TDigitalScan::Next(int loopIndex)
{
  if (loopIndex == 0) {
    while (!(m_mutex->try_lock()))
      ;
    m_histo->SetIndex(m_row);
    // std::cout << "SCAN: Writing histo with row " << m_histo->GetIndex() << std::endl;
    m_histoQue->push_back(*m_histo);
    m_mutex->unlock();
    m_histo->Clear();
  }
  TScan::Next(loopIndex);
}

void TDigitalScan::Execute()
{
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    m_boards.at(iboard)->Trigger(m_nTriggers);
  }

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    Hits->clear();
    ReadEventData(Hits, iboard);
    FillHistos(Hits, iboard);
  }
  delete Hits;
}

void TDigitalScan::Terminate()
{
  TScan::Terminate();

  // restore old voltage
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (((TDigitalParameters *)m_parameters)->voltageScale != 1.) {
      m_hics.at(ihic)->ScaleVoltage(1.);
    }
  }

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(m_boards.at(iboard));
    if (myMOSAIC) {
      myMOSAIC->StopRun();
      // delete myMOSAIC;
    }
    TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(m_boards.at(iboard));
    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      // delete myDAQBoard;
    }
  }

  SwitchOffBackbias();

  m_running = false;
}

TDigitalWhiteFrame::TDigitalWhiteFrame(TScanConfig *config, std::vector<TAlpide *> chips,
                                       std::vector<THic *>          hics,
                                       std::vector<TReadoutBoard *> boards,
                                       std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TDigitalScan(config, chips, hics, boards, histoQue, aMutex)
{
  SetName();
}

void TDigitalWhiteFrame::ConfigureMaskStage(TAlpide *chip, int istage)
{
  m_row = AlpideConfig::ConfigureMaskStage(chip, m_pixPerStage, istage, false, true);
}

void TDigitalWhiteFrame::Init()
{
  TScan::Init();

  SetBackBias();
  CreateScanHisto();
  m_running = true;
  CountEnabledChips();

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    std::cout << "Board " << i << ", found " << m_enabled[i] << " enabled chips" << std::endl;
    ConfigureBoard(m_boards.at(i));

    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_PRST);
  }

  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!(m_chips.at(i)->GetConfig()->IsEnabled())) continue;
    ConfigureChip(m_chips.at(i));
    AlpideConfig::WritePixRegAll(m_chips.at(i), Alpide::PIXREG_MASK, true);
  }
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_RORST);
    m_boards.at(i)->StartRun();
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }
}


void TDigitalWhiteFrame::SetName()
{
  sprintf(m_name, "Digital White Frame BB %d", (int)((TDigitalParameters *)m_parameters)->backBias);
}
