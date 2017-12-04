#include <string.h>
#include <string>

#include "TNoiseOccupancy.h"
#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardRU.h"

TNoiseOccupancy::TNoiseOccupancy (TScanConfig                   *config, 
                                  std::vector <TAlpide *>        chips, 
                                  std::vector <THic*>            hics, 
                                  std::vector <TReadoutBoard *>  boards, 
                                  std::deque<TScanHisto>        *histoQue, 
                                  std::mutex                    *aMutex) 
  : TDataTaking (config, chips, hics, boards, histoQue, aMutex) 
{
  sprintf(m_name, "Noise Occupancy %.1f V", m_backBias); 
  m_pulse       = false;
  m_pulseLength = 0;
}


//TODO: save number of masked pixels (return value of ApplyMask)
void TNoiseOccupancy::ConfigureChip  (TAlpide *chip)
{
  AlpideConfig::BaseConfig   (chip);
  ConfigureFromu             (chip);
  ConfigureMask              (chip, 0);
  AlpideConfig::ApplyMask    (chip, true);
  AlpideConfig::ConfigureCMU (chip);
}


void TNoiseOccupancy::ConfigureMask (TAlpide *chip, std::vector <TPixHit> *MaskedPixels)
{
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   false);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);
}


THisto TNoiseOccupancy::CreateHisto () 
{
  THisto histo ("HitmapHisto", "HitmapHisto", 1024, 0, 1023, 512, 0, 511);
  return histo;
}


void TNoiseOccupancy::Init ()
{
  TDataTaking::Init();
  m_running = true;
}
