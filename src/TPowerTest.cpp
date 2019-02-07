#include "TPowerTest.h"
#include "AlpideConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include <string.h>
#include <string>
#include <thread>

TPowerTest::TPowerTest(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                       std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue,
                       std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias = 0;
  m_ivcurve              = (m_config->GetParamValue("IVCURVE") != 0);

  strcpy(m_name, "Power Test");
  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = 1;

  m_start[0] = 0;
  m_step[0]  = 1;
  m_stop[0]  = m_hics.size();

  CreateMeasurements();
  m_histo = 0;
}

void TPowerTest::CreateMeasurements()
{
  // create map with measurement structure for each HIC
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    THicCurrents hicCurrents;
    hicCurrents.hicType = m_hics.at(i)->GetHicType();
    hicCurrents.maxBias = ((float)(m_config->GetParamValue("IVPOINTS")) - 1) / 10;
    m_hicCurrents.insert(
        std::pair<std::string, THicCurrents>(m_hics.at(i)->GetDbId(), hicCurrents));
  }
}

void TPowerTest::Init()
{
  InitBase(true);
  // switch power off here or hic-wise in execute?
}

void TPowerTest::PrepareStep(int loopIndex)
{
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

void TPowerTest::DoIVCurve(THicCurrents &result)
{
  for (int i = 0; i < m_config->GetParamValue("IVPOINTS"); i++) {
    float voltage = -((float)i) / 10.;
    m_testHic->GetPowerBoard()->SetBiasVoltage(voltage);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    result.ibias[i] = m_testHic->GetIBias() * 1000; // convert in mA
    // overcurrent protection; will be counted as trip; last voltage is saved as max back bias
    // voltage
    if (result.ibias[i] > m_config->GetParamValue("MAXIBIAS")) {
      printf("MAXIBIAS exceeded at %g: %g > %d, switching off Vbias\n", voltage, result.ibias[i],
             m_config->GetParamValue("MAXIBIAS"));
      m_hicCurrents.find(m_testHic->GetDbId())->second.maxBias = voltage - .1;
      m_testHic->GetPowerBoard()->SetBiasVoltage(0.0);
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
      break;
    }
  }
}

void TPowerTest::Execute()
{
  std::vector<int>       boardIndices = m_testHic->GetBoardIndices();
  std::vector<TAlpide *> chips        = m_testHic->GetChips();

  std::map<std::string, THicCurrents>::iterator currentIt =
      m_hicCurrents.find(m_testHic->GetDbId());

  // power off and disable control interface
  m_testHic->PowerOff();
  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC *)m_boards.at(boardIndices.at(i));
    board->enableControlInterfaces(false);
  }

  // power on and correct voltage drop
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  m_testHic->GetPowerBoard()->SwitchAnalogOn(m_testHic->GetPbMod());
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  m_testHic->GetPowerBoard()->SwitchDigitalOn(m_testHic->GetPbMod());

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  m_testHic->GetPowerBoard()->CorrectVoltageDrop(m_testHic->GetPbMod());

  // measure -> switchon, no clock
  currentIt->second.idddSwitchon = m_testHic->GetIddd();
  currentIt->second.iddaSwitchon = m_testHic->GetIdda();

  // enable control interface and send GRST
  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC *)m_boards.at(boardIndices.at(i));
    board->enableControlInterfaces(true);
    board->SendOpCode(Alpide::OPCODE_GRST);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  m_testHic->GetPowerBoard()->CorrectVoltageDrop(m_testHic->GetPbMod());

  // measure -> Clocked
  currentIt->second.idddClocked = m_testHic->GetIddd();
  currentIt->second.iddaClocked = m_testHic->GetIdda();


  // configure chips
  for (unsigned int i = 0; i < chips.size(); i++) {
    if (!(chips.at(i)->GetConfig()->IsEnabled())) continue;
    if (m_testHic != chips.at(i)->GetHic()) continue;
    AlpideConfig::BaseConfig(chips.at(i));
    AlpideConfig::ConfigureCMU(chips.at(i));
  }


  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC *)m_boards.at(boardIndices.at(i));
    board->SendOpCode(Alpide::OPCODE_RORST);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  m_testHic->GetPowerBoard()->CorrectVoltageDrop(m_testHic->GetPbMod());

  // measure -> Configured
  currentIt->second.idddConfigured = m_testHic->GetIddd();
  currentIt->second.iddaConfigured = m_testHic->GetIdda();

  // check if supply tripped
  if (!(m_testHic->IsPowered())) {
    currentIt->second.trip = true;
  }
  else {
    currentIt->second.trip = false;
  }

  // switch on back bias only for module under test
  for (int i = 0; i < 8; i++) {
    if (i == m_testHic->GetBbChannel()) {
      m_testHic->GetPowerBoard()->SetBiasOn(i, true);
    }
    else {
      m_testHic->GetPowerBoard()->SetBiasOff(i);
    }
  }

  currentIt->second.ibias0 = m_testHic->GetIBias() * 1000;

  // measure IV curve or only bias current at 3 V
  if (m_ivcurve) {
    DoIVCurve(currentIt->second);
    currentIt->second.ibias3 = currentIt->second.ibias[30];
  }
  else {
    m_testHic->GetPowerBoard()->SetBiasVoltage(-3.0);
    sleep(1);
    currentIt->second.ibias3 = m_testHic->GetIBias() * 1000;
  }

  // check if supply tripped during BB IV-curve
  if ((!currentIt->second.trip) && (!m_testHic->IsPowered())) {
    std::cout << "Back bias tripped low voltage" << std::endl;
    currentIt->second.tripBB = true;
  }
  else {
    currentIt->second.tripBB = false;
  }

  // check if back bias tripped
  if ((m_testHic->GetPowerBoard()->GetBiasVoltage() > -1.0)) {
    std::cout << "reading bias voltage of " << m_testHic->GetPowerBoard()->GetBiasVoltage()
              << std::endl;
    currentIt->second.tripBB = true;
  }

  m_testHic->GetPowerBoard()->SetBiasVoltage(0.0);
  m_testHic->GetPowerBoard()->SetBiasOff(m_testHic->GetBbChannel());
  THicOB *obHic = dynamic_cast<THicOB *>(m_testHic);
  if (obHic && obHic->IsPowerCombo()) {
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  std::cout << "starting resistance measurement" << std::endl;

  /*m_testHic->GetPowerBoard()->SwitchAnalogOn(m_testHic->GetPbMod());
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  m_testHic->GetPowerBoard()->SwitchDigitalOn(m_testHic->GetPbMod());
  */
  float dVDig, dVAna, dIDig, dIAna;

  DigitalCurrentStep(dVDig, dVAna, dIDig, dIAna);

  float RGND;

  if (std::abs(dIAna) < 0.02) {
    std::cout << "Calculating ground resistance" << std::endl;
    RGND = dVAna / dIDig;
  }
  std::cout << "The ground resistance is " << RGND << std::endl;

  // lower digital voltage

  m_testHic->GetPowerBoard()->SetDigitalVoltage(m_testHic->GetPbMod(), 1.62);

  std::cout << "the digital voltage on the chips is" << m_testHic->GetDigitalVoltage() << std::endl;

  float RGND2;
  DigitalCurrentStep(dVDig, dVAna, dIDig, dIAna);

  RGND2 = dVAna / dIDig;

  std::cout << "the second ground resistance is" << RGND2 << std::endl;

  if (RGND != RGND2) std::cout << "There is a difference in the GND resistances" << endl;

  float RDig;

  RDig = (dVDig / dIDig) - RGND;

  std::cout << "the digital resistance is " << RDig << std::endl;

  // going back to nomial voltage
  float RAna;

  m_testHic->GetPowerBoard()->SetDigitalVoltage(m_testHic->GetPbMod(), 1.82);

  AnalogCurrentStep(dVAna, dIAna);

  RAna = (dVAna / dIAna) - RGND;

  std::cout << "the analogue resistance is " << RAna << std::endl;
}

void TPowerTest::Terminate()
{
  TScan::Terminate();
  m_running = false;
}

void TPowerTest::DigitalCurrentStep(float &dVDig, float &dVAna, float &dIDig, float &dIAna)
{

  std::vector<int>       boardIndices = m_testHic->GetBoardIndices();
  std::vector<TAlpide *> chips        = m_testHic->GetChips();

  // send GRST
  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC *)m_boards.at(boardIndices.at(i));
    board->enableControlInterfaces(true);
    board->SendOpCode(Alpide::OPCODE_GRST);
  }
  // measure voltages and currents

  float averagedvchipbefore = 0;
  float averageavchipbefore = 0;
  float ianaloguepbbefore   = 0;
  float idigitalpbbefore    = 0;

  float averagedvchipafter = 0;
  float averageavchipafter = 0;
  float ianaloguepbafter   = 0;
  float idigitalpbafter    = 0;

  averageavchipbefore = m_testHic->GetAnalogueVoltage();
  averagedvchipbefore = m_testHic->GetDigitalVoltage();
  ianaloguepbbefore   = m_testHic->GetIdda();
  idigitalpbbefore    = m_testHic->GetIddd();

  // configure chips
  for (unsigned int i = 0; i < chips.size(); i++) {
    if (!(chips.at(i)->GetConfig()->IsEnabled())) continue;
    if (m_testHic != chips.at(i)->GetHic()) continue;
    AlpideConfig::BaseConfig(chips.at(i));
    AlpideConfig::ConfigureCMU(chips.at(i));
  }


  averageavchipafter = m_testHic->GetAnalogueVoltage();
  averagedvchipafter = m_testHic->GetDigitalVoltage();
  ianaloguepbafter   = m_testHic->GetIdda();
  idigitalpbafter    = m_testHic->GetIddd();

  dVDig = averagedvchipafter - averagedvchipbefore;
  dVAna = averageavchipafter - averageavchipbefore;
  dIDig = idigitalpbbefore - idigitalpbafter;
  dIAna = ianaloguepbbefore - ianaloguepbafter;
}

void TPowerTest::AnalogCurrentStep(float &dVAna, float &dIAna)
{

  std::vector<int>       boardIndices = m_testHic->GetBoardIndices();
  std::vector<TAlpide *> chips        = m_testHic->GetChips();

  // send GRST
  for (unsigned int i = 0; i < boardIndices.size(); i++) {
    TReadoutBoardMOSAIC *board = (TReadoutBoardMOSAIC *)m_boards.at(boardIndices.at(i));
    board->SendOpCode(Alpide::OPCODE_GRST);
  }
  // measure voltages and currents

  float averageavchipbefore = 0;
  float ianaloguepbbefore   = 0;

  float averageavchipafter = 0;
  float ianaloguepbafter   = 0;

  averageavchipbefore = m_testHic->GetAnalogueVoltage();
  ianaloguepbbefore   = m_testHic->GetIdda();

  // set IBIAS 0
  for (unsigned int i = 0; i < chips.size(); i++) {
    if (!(chips.at(i)->GetConfig()->IsEnabled())) continue;
    if (m_testHic != chips.at(i)->GetHic()) continue;
    chips.at(i)->WriteRegister(Alpide::REG_IBIAS, 0);
  }

  averageavchipafter = m_testHic->GetAnalogueVoltage();
  ianaloguepbafter   = m_testHic->GetIdda();

  dVAna = averageavchipafter - averageavchipbefore;
  std::cout << "the dVAna is" << dVAna << std::endl;
  dIAna = ianaloguepbbefore - ianaloguepbafter;
}
