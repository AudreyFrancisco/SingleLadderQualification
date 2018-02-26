#include <string>
#include <string.h>
#include "TEnduranceCycle.h"
#include "TReadoutBoardMOSAIC.h"
#include "AlpideConfig.h"

TEnduranceCycle::TEnduranceCycle(TScanConfig *config, std::vector<TAlpide *> chips,
                                 std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                                 std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex) {
  strcpy(m_name, "Endurance Cycle");
  m_start[2] = 0;
  m_step[2] = 1;
  m_stop[2] = 1;

  m_start[1] = 0;
  m_step[1] = 1;
  m_stop[1] = 1;

  m_start[0] = 0;
  m_step[0] = 1;
  m_stop[0] = config->GetParamValue("ENDURANCECYCLES"); // loop over hics?

  m_triggers = config->GetParamValue("ENDURANCETRIGGERS");

  CreateMeasurements();
  m_histo = 0;
}

void TEnduranceCycle::CreateMeasurements() {
  // create map with measurement structure for each HIC
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    THicCounter hicCounter;
    hicCounter.m_hicType = m_hics.at(i)->GetHicType();
    m_hicCounters.insert(std::pair<std::string, THicCounter>(m_hics.at(i)->GetDbId(), hicCounter));
  }
}

void TEnduranceCycle::ClearCounters() {
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_nWorkingChips = 0;
  }
}

void TEnduranceCycle::Init() {
  TScan::Init();
  // configure readout boards
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    ConfigureBoard(m_boards.at(i));
  }
  // disable all receivers
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!(m_chips.at(i)->GetConfig()->IsEnabled()))
      continue;
    m_chips.at(i)->GetReadoutBoard()->SetChipEnable(m_chips.at(i), false);
  }
}

// Try to communicate with all chips, disable chips that are not answering
void TEnduranceCycle::CountWorkingChips() {
  uint16_t WriteValue = 10;
  uint16_t Value;

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }

  for (unsigned int i = 0; i < m_chips.size(); i++) {
    m_chips.at(i)->WriteRegister(0x60d, WriteValue);
    try {
      m_chips.at(i)->ReadRegister(0x60d, Value);
      if (WriteValue == Value) {
        THic *hic = m_chips.at(i)->GetHic();
        m_hicCounters.at(hic->GetDbId()).m_nWorkingChips++;
        m_chips.at(i)->SetEnable(true);
      } else {
        m_chips.at(i)->SetEnable(false);
      }
    }
    catch (exception &e) {
      m_chips.at(i)->SetEnable(false);
    }
  }
}

void TEnduranceCycle::ConfigureBoard(TReadoutBoard *board) {
  if (board->GetConfig()->GetBoardType() == boardDAQ) {
    // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe
    // delay
    // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
    board->SetTriggerConfig(true, false, 0,
                            2 * board->GetConfig()->GetParamValue("STROBEDELAYBOARD"));
    board->SetTriggerSource(trigExt);
  } else {
    board->SetTriggerConfig(false, true, board->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                            board->GetConfig()->GetParamValue("PULSEDELAY"));
    board->SetTriggerSource(trigInt);
  }
}

void TEnduranceCycle::ConfigureFromu(TAlpide *chip) {
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
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, 0); // fromu pulsing 2: pulse length
}

void TEnduranceCycle::ConfigureChip(TAlpide *chip) {
  AlpideConfig::BaseConfig(chip);
  ConfigureFromu(chip);
  ConfigureMask(chip);
  AlpideConfig::ConfigureCMU(chip);
}

void TEnduranceCycle::ConfigureMask(TAlpide *chip) {
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_MASK, true);
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_SELECT, false);

  // TODO: decide on correct masking; for the time being: enable row 0
  AlpideConfig::WritePixRegRow(chip, Alpide::PIXREG_MASK, false, 0);
  AlpideConfig::WritePixRegRow(chip, Alpide::PIXREG_SELECT, true, 0);
}

void TEnduranceCycle::Execute() {
  // 1) Power on all HICs, check for trips, measure currents
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->PowerOn();
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_idddClocked = m_hics.at(ihic)->GetIddd();
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_iddaClocked = m_hics.at(ihic)->GetIdda();
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_trip = !(m_hics.at(ihic)->IsPowered());
  }

  // 2) enable all chips, check control interfaces -> number of working chips
  //    measure initial temperature (here and not earlier to avoid non-working chips
  CountWorkingChips();
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_tempStart = m_hics.at(ihic)->GetTemperature();
  }

  // 3) configure chips, measure currents
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_PRST);
  }

  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!(m_chips.at(i)->GetConfig()->IsEnabled()))
      continue;
    ConfigureChip(m_chips.at(i));
  }
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_RORST);
    m_boards.at(i)->StartRun();
  }
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_idddConfigured = m_hics.at(ihic)->GetIddd();
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_iddaConfigured = m_hics.at(ihic)->GetIdda();
  }

  // 4) trigger
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    m_boards.at(iboard)->Trigger(m_triggers);
  }

  // 5) measure final temperature & power off
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_tempEnd = m_hics.at(ihic)->GetTemperature();
    m_hics.at(ihic)->PowerOff();
  }
}

void TEnduranceCycle::LoopEnd(int loopIndex) {
  if (loopIndex == 0) {
    m_counterVector.push_back(m_hicCounters);
    ClearCounters();
  }
}

void TEnduranceCycle::Terminate() {
  TScan::Terminate();
  m_running = false;
  // re-enable receivers in readout board for all enabled chips
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    m_chips.at(i)->GetReadoutBoard()->SetChipEnable(m_chips.at(i),
                                                    m_chips.at(i)->GetConfig()->IsEnabled());
  }
}
