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
                            std::mutex                    *aMutex) 
  : TDataTaking (config, chips, hics, boards, histoQue, aMutex) 
{
  sprintf(m_name, "ReadoutTest"); 
  // variables needed: readout speed, occupancy, DTU settings, trigger frequency

  m_linkSpeed      = config->GetParamValue ("READOUTSPEED");
  m_occupancy      = config->GetParamValue ("READOUTOCC");
  m_driverStrength = config->GetParamValue ("READOUTDRIVER");
  m_preemp         = config->GetParamValue ("READOUTPREEMP");
}


//TODO: save number of masked pixels (return value of ApplyMask)
void TReadoutTest::ConfigureChip  (TAlpide *chip)
{
  // store driver settings and set temporary ones, used in this scan 
  // (used by BaseConfig -> BaseConfigPLL
  int backupDriver = chip->GetConfig()->GetParamValue("DTUDRIVER");
  int backupPreemp = chip->GetConfig()->GetParamValue("DTUPREEMP");
  chip->GetConfig()->SetParamValue("DTUDRIVER", m_driverStrength);
  chip->GetConfig()->SetParamValue("DTUPREEMP", m_preemp);
  AlpideConfig::BaseConfig   (chip);
  ConfigureFromu             (chip);
  ConfigureMask              (chip, 0);
  AlpideConfig::ApplyMask    (chip, true);
  AlpideConfig::ConfigureCMU (chip);
  // restore previous settings
  chip->GetConfig()->SetParamValue("DTUDRIVER", backupDriver);
  chip->GetConfig()->SetParamValue("DTUPREEMP", backupPreemp);
}


// TODO: Add masking / selecting of given occupancy
void TReadoutTest::ConfigureMask (TAlpide *chip, std::vector <TPixHit> *MaskedPixels)
{
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   true);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);

}


THisto TReadoutTest::CreateHisto () 
{
  THisto histo ("HitmapHisto", "HitmapHisto", 1024, 0, 1023, 512, 0, 511);
  return histo;
}


void TReadoutTest::Init ()
{
  TDataTaking::Init();
  m_running = true;
}
