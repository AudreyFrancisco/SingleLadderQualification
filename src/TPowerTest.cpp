#include <string>
#include <string.h>
#include "TPowerTest.h"
#include "TReadoutBoardMOSAIC.h"
#include "AlpideConfig.h"

TPowerTest::TPowerTest(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                       std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue,
                       std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex) {
  strcpy(m_name, "Power Test");
  m_start[2] = 0;
  m_step[2] = 1;
  m_stop[2] = 1;

  m_start[1] = 0;
  m_step[1] = 1;
  m_stop[1] = 1;

  m_start[0] = 0;
  m_step[0] = 1;
  m_stop[0] = m_hics.size();

  CreateMeasurements();
  m_histo = 0;
}

void TPowerTest::CreateMeasurements() {
  // create map with measurement structure for each HIC
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    THicCurrents hicCurrents;
    hicCurrents.hicType = m_hics.at(i)->GetHicType();
    m_hicCurrents.insert(
        std::pair<std::string, THicCurrents>(m_hics.at(i)->GetDbId(), hicCurrents));
  }
}

void TPowerTest::Init() {
  TScan::Init();
  // switch power off here or hic-wise in execute?
}

void TPowerTest::PrepareStep(int loopIndex) {
  switch (loopIndex) {
  case 0: // innermost loop: change HIC
    m_testHic = m_hics.at(m_value[0]);
    break;
  case 1: // 2nd loop
    break;
  case 2: // outermost loop
    break;
  }
}

void TPowerTest::DoIVCurve(THicCurrents &result) {
  for (int i = 0; i < m_config->GetParamValue("IVPOINTS"); i++) {
    float voltage = -i / 10;
    m_testHic->GetPowerBoard()->SetBiasVoltage(voltage);
    sleep(1);
    result.ibias[i] = m_testHic->GetIBias() * 1000; // convert in mA
    // overcurrent protection; will be counted as trip
    if (result.ibias[i] > m_config->GetParamValue("MAXIBIAS")) {
      m_testHic->GetPowerBoard()->SetBiasVoltage(0.0);
      sleep(1);
      break;
    }
  }
}

void TPowerTest::Execute() {
  std::vector<int> boardIndices = m_testHic->GetBoardIndices();
  std::vector<TAlpide *> chips = m_testHic->GetChips();

  std::map<std::string, THicCurrents>::iterator currentIt =
      m_hicCurrents.find(m_testHic->GetDbId());
  m_testHic->PowerOff();

  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC *)m_boards.at(boardIndices.at(i));
    board->enableControlInterfaces(false);
  }

  m_testHic->PowerOn();
  m_testHic->GetPowerBoard()->CorrectVoltageDrop(m_testHic->GetPbMod());

  // measure -> switchon, no clock
  currentIt->second.idddSwitchon = m_testHic->GetIddd();
  currentIt->second.iddaSwitchon = m_testHic->GetIdda();

  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC *)m_boards.at(boardIndices.at(i));
    board->enableControlInterfaces(true);
    board->SendOpCode(Alpide::OPCODE_GRST);
  }

  m_testHic->GetPowerBoard()->CorrectVoltageDrop(m_testHic->GetPbMod());

  // measure -> Clocked
  currentIt->second.idddClocked = m_testHic->GetIddd();
  currentIt->second.iddaClocked = m_testHic->GetIdda();

  for (unsigned int i = 0; i < chips.size(); i++) {
    if (!(chips.at(i)->GetConfig()->IsEnabled()))
      continue;
    AlpideConfig::BaseConfig(chips.at(i));
    AlpideConfig::ConfigureCMU(chips.at(i));
  }
  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC *)m_boards.at(boardIndices.at(i));
    board->SendOpCode(Alpide::OPCODE_RORST);
  }

  m_testHic->GetPowerBoard()->CorrectVoltageDrop(m_testHic->GetPbMod());

  // measure -> Configured
  currentIt->second.idddConfigured = m_testHic->GetIddd();
  currentIt->second.iddaConfigured = m_testHic->GetIdda();

  // switch on back bias only for module under test
  for (int i = 0; i < 8; i++) {
    if (i == m_testHic->GetPbMod()) {
      m_testHic->GetPowerBoard()->SetBiasOn(i);
    } else {
      m_testHic->GetPowerBoard()->SetBiasOff(i);
    }
  }

  currentIt->second.ibias0 = m_testHic->GetIBias() * 1000;

  // measure IV curve or only bias current at 3 V
  if (m_config->GetParamValue("IVCURVE")) {
    DoIVCurve(currentIt->second);
    currentIt->second.ibias3 = currentIt->second.ibias[30];
  } else {
    m_testHic->GetPowerBoard()->SetBiasVoltage(3.0);
    sleep(1);
    currentIt->second.ibias3 = m_testHic->GetIBias() * 1000;
  }

  // check if tripped
  if ((m_testHic->GetPowerBoard()->GetBiasVoltage() > -1.0) || (!(m_testHic->IsPowered()))) {
    std::cout << "reading bias voltage of " << m_testHic->GetPowerBoard()->GetBiasVoltage()
              << std::endl;
    currentIt->second.trip = true;
  } else {
    currentIt->second.trip = false;
  }
  m_testHic->GetPowerBoard()->SetBiasVoltage(0.0);
}

void TPowerTest::Terminate() {
  TScan::Terminate();
  m_running = false;
}
