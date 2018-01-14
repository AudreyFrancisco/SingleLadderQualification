#include <string.h>
#include <string>

#include "TReadoutTest.h"
#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardRU.h"

TReadoutTest::TReadoutTest (TScanConfig                   *config, 
                            std::vector <TAlpide *>        chips, 
                            std::vector <THic*>            hics, 
                            std::vector <TReadoutBoard *>  boards, 
                            std::deque<TScanHisto>        *histoQue, 
                            std::mutex                    *aMutex,
			    int                            pllStages) 
  : TDataTaking (config, chips, hics, boards, histoQue, aMutex) 
{
  // trigger frequency, number of triggers have to be set in scan config
  // before creating readout test object

  m_pulse          = true;
  m_pulseLength    = chips.at(0)->GetConfig()->GetParamValue("PULSEDURATION");
  m_row            = config->GetParamValue ("READOUTROW");
  m_linkSpeed      = config->GetParamValue ("READOUTSPEED");
  m_occupancy      = config->GetParamValue ("READOUTOCC");
  m_driverStrength = config->GetParamValue ("READOUTDRIVER");
  m_preemp         = config->GetParamValue ("READOUTPREEMP");
  m_pllStages      = pllStages;
  m_voltageScale   = config->GetVoltageScale();

  sprintf(m_name, "ReadoutTest %d %d %d", m_linkSpeed, m_driverStrength, m_preemp); 
}


//TODO: save number of masked pixels (return value of ApplyMask)
void TReadoutTest::ConfigureChip  (TAlpide *chip)
{
  // store driver settings and set temporary ones, used in this scan 
  // (used by BaseConfig -> BaseConfigPLL
  int backupDriver = chip->GetConfig()->GetParamValue("DTUDRIVER");
  int backupPreemp = chip->GetConfig()->GetParamValue("DTUPREEMP");
  int backupSpeed  = chip->GetConfig()->GetParamValue("LINKSPEED");
  int backupStages = chip->GetConfig()->GetParamValue("PLLSTAGES");
  chip->GetConfig()->SetParamValue("DTUDRIVER", m_driverStrength);
  chip->GetConfig()->SetParamValue("DTUPREEMP", m_preemp);
  chip->GetConfig()->SetParamValue("LINKSPEED", m_linkSpeed);
  if (m_pllStages >= 0) {
    chip->GetConfig()->SetParamValue("PLLSTAGES", m_linkSpeed);    
  }
  AlpideConfig::BaseConfig   (chip);
  ConfigureFromu             (chip);
  ConfigureMask              (chip, 0);
  AlpideConfig::ApplyMask    (chip, true);
  AlpideConfig::ConfigureCMU (chip);
  // restore previous settings
  chip->GetConfig()->SetParamValue("DTUDRIVER", backupDriver);
  chip->GetConfig()->SetParamValue("DTUPREEMP", backupPreemp);
  chip->GetConfig()->SetParamValue("LINKSPEED", backupSpeed);
  if (m_pllStages >= 0) {
    chip->GetConfig()->SetParamValue("PLLSTAGES", backupStages);    
  }
}


// TODO: Add masking / selecting of given occupancy
void TReadoutTest::ConfigureMask (TAlpide *chip, std::vector <TPixHit> *MaskedPixels)
{
  AlpideConfig::ConfigureMaskStage (chip, m_occupancy, m_row, true, true);
  //  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   true);
  //AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);
}


THisto TReadoutTest::CreateHisto () 
{
  THisto histo ("HitmapHisto", "HitmapHisto", 1024, 0, 1023, 512, 0, 511);
  return histo;
}


void TReadoutTest::Init ()
{
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    TReadoutBoardMOSAIC *mosaic = (TReadoutBoardMOSAIC *)m_boards.at(i);
    if (mosaic) {
      if      (m_linkSpeed == 400)  mosaic-> setSpeedMode (Mosaic::RCV_RATE_400);
      else if (m_linkSpeed == 600)  mosaic-> setSpeedMode (Mosaic::RCV_RATE_600);
      else if (m_linkSpeed == 1200) mosaic-> setSpeedMode (Mosaic::RCV_RATE_1200);
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (m_voltageScale != 1.) {
      m_hics.at(ihic)->ScaleVoltage(m_voltageScale);
    }      
  }
    
  TDataTaking::Init();
  m_running = true;
}


void TReadoutTest::Terminate() 
{
  TDataTaking::Terminate();

  // restore old voltage
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (m_voltageScale != 1.) {
      m_hics.at(ihic)->ScaleVoltage(1.);
    }      
  }
  
  // reset link speed in mosaic; chip configs have been reset already in ConfigureChip()
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    TReadoutBoardMOSAIC *mosaic = (TReadoutBoardMOSAIC *)m_boards.at(i);
    if (mosaic) {
      int linkSpeed = m_chips.at(0)->GetConfig()->GetParamValue("LINKSPEED");
      if      (linkSpeed == 400)  mosaic-> setSpeedMode (Mosaic::RCV_RATE_400);
      else if (linkSpeed == 600)  mosaic-> setSpeedMode (Mosaic::RCV_RATE_600);
      else if (linkSpeed == 1200) mosaic-> setSpeedMode (Mosaic::RCV_RATE_1200);
    }
  }
}
