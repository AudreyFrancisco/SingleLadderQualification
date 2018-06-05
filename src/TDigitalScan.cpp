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
  printf("Breaks here 1\n");
  float voltageScale = config->GetVoltageScale();
  m_parameters       = new TDigitalParameters;

  ((TDigitalParameters *)m_parameters)->voltageScale = voltageScale;

  SetName();

  m_start[0] = 0;
  m_step[0]  = 1;
  m_stop[0]  = m_config->GetNMaskStages();

  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = 1;

  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  m_nTriggers = m_config->GetParamValue("NINJ");
  printf("Number of triggers sent per pixel = %d \n", m_nTriggers);

  CreateScanHisto();
  printf("Breaks here 2\n");
}


void TDigitalScan::SetName()
{
  printf("Breaks here 3\n");

  std::cout << "voltageScale = " << ((TDigitalParameters *)m_parameters)->voltageScale << std::endl;
  if (IsNominal()) {
    strcpy(m_name, "Digital Scan");
  }
  else if (IsUpper() && (((TDigitalParameters *)m_parameters)->voltageScale < 1.2)) {
    strcpy(m_name, "Digital Scan, V +10%");
  }
  else if (((TDigitalParameters *)m_parameters)->voltageScale > 0.8 && IsLower()) {
    strcpy(m_name, "Digital Scan, V -10%");
  }
  else {
    std::cout << "Warning: unforeseen voltage scale, using 1" << std::endl;
    ((TDigitalParameters *)m_parameters)->voltageScale = 1.0;
    strcpy(m_name, "Digital Scan");
  }
  printf("Breaks here 4\n");
}


bool TDigitalScan::SetParameters(TScanParameters *pars)
{
  printf("Breaks here 5\n");
  TDigitalParameters *dPars = dynamic_cast<TDigitalParameters *>(pars);
  if (dPars) {
    std::cout << "TDigitalScan: Updating parameters" << std::endl;
    ((TDigitalParameters *)m_parameters)->voltageScale = dPars->voltageScale;
    SetName();
    return true;
  }
  else {
    std::cout << "TDigitalScan::SetParameters: Error, bad parameter type, doing nothing"
              << std::endl;
    return false;
  }
  printf("Breaks here 6\n");
}


void TDigitalScan::ConfigureFromu(TAlpide *chip)
{
  printf("Breaks here 7\n");
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
  printf("Breaks here 8\n");
}

void TDigitalScan::ConfigureChip(TAlpide *chip)
{
  printf("Breaks here 9\n");

  AlpideConfig::BaseConfig(chip);
  ConfigureFromu(chip);
  AlpideConfig::ConfigureCMU(chip);

  printf("Breaks here 10\n");
}

void TDigitalScan::ConfigureBoard(TReadoutBoard *board)
{
  printf("Breaks here 11\n");
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
  printf("Breaks here 12\n");
}

void TDigitalScan::FillHistos(std::vector<TPixHit> *Hits, int board)
{
  printf("Breaks here 13\n");
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
  printf("Breaks here 14\n");
}

THisto TDigitalScan::CreateHisto()
{
  THisto histo("HitmapHisto", "HitmapHisto", 1024, 0, 1023);
  return histo;
}

void TDigitalScan::Init()
{
  printf("Breaks here 15\n");
  TScan::Init();
  m_running = true;
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

  /*for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }*/

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

  /*for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }*/
  printf("Breaks here 16\n");
}

void TDigitalScan::PrepareStep(int loopIndex)
{
  printf("Breaks here 17\n");
  switch (loopIndex) {
    printf("Breakpoint 17.1\n");
  case 0: // innermost loop: mask staging
    std::cout << "mask stage " << m_value[0] << std::endl;
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      printf("Breakpoint 17.2\n");
      printf("Trying to apply mask on CHIP %u\n", ichip);
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      printf("Breakpoint 17.3\n");
      ConfigureMaskStage(m_chips.at(ichip), m_value[0]);
    }
    sprintf(m_state, "Running %d", m_value[0]);
    break;
  default:
    break;
  }
  printf("Breaks here 18\n");
}

void TDigitalScan::LoopEnd(int loopIndex) {}

void TDigitalScan::Next(int loopIndex)
{
  printf("Breaks here 19\n");
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
  printf("Breaks here 20\n");
}

void TDigitalScan::Execute()
{
  printf("Breaks here 21\n");
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;
  printf("Breaks here 21.1\n");

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    printf("Breaks here 21.2\n");
    m_boards.at(iboard)->Trigger(m_nTriggers);
  }
  printf("Breaks here 21.3\n");

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    printf("Breaks here 21.4\n");

    Hits->clear();
    printf("Breaks here 21.5\n");

    ReadEventData(Hits, iboard);
    printf("Breaks here 21.6\n");

    FillHistos(Hits, iboard);
    printf("Breaks here 21.7\n");
  }
  delete Hits;
  printf("Breaks here 22\n");
}

void TDigitalScan::Terminate()
{
  printf("Breaks here 23\n");
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
  m_running = false;
  printf("Breaks here 24\n");
}

TDigitalWhiteFrame::TDigitalWhiteFrame(TScanConfig *config, std::vector<TAlpide *> chips,
                                       std::vector<THic *>          hics,
                                       std::vector<TReadoutBoard *> boards,
                                       std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TDigitalScan(config, chips, hics, boards, histoQue, aMutex)
{
  strcpy(m_name, "Digital White Frame");
  printf("Breaks here 25\n");
}

void TDigitalWhiteFrame::ConfigureMaskStage(TAlpide *chip, int istage)
{
  printf("Breaks here 26\n");

  m_row = AlpideConfig::ConfigureMaskStage(chip, m_pixPerStage, istage, false, true);
}

void TDigitalWhiteFrame::Init()
{
  printf("Breaks here 27\n");
  TScan::Init();
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
  printf("Breaks here 28\n");
  /*for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }*/
}
