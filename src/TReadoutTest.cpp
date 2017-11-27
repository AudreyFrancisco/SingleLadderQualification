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
  // variables needed: occupancy, DTU settings, trigger frequency
}


//TODO: save number of masked pixels (return value of ApplyMask)
void TReadoutTest::ConfigureChip  (TAlpide *chip)
{
  AlpideConfig::BaseConfig   (chip);
  ConfigureFromu             (chip);
  ConfigureMask              (chip, 0);
  AlpideConfig::ApplyMask    (chip, true);
  AlpideConfig::ConfigureCMU (chip);
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
