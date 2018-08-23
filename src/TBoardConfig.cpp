#include "TBoardConfig.h"
#include <stdio.h>

using namespace BoardConfig;

TBoardConfig::TBoardConfig(const char *fName, int boardIndex)
{
  fTriggerDelay = STROBEDELAY;
  fPulseDelay   = PULSEDELAY;
  fNChips       = NCHIPS;
  fDipswitch    = DIPSWITCH;
}

void TBoardConfig::InitParamMap()
{
  fSettings["STROBEDELAYBOARD"]   = &fTriggerDelay;
  fSettings["PULSEDELAY"]         = &fPulseDelay;
  fSettings["NCHIPS"]             = &fNChips;
  fSettings["READOUTCRUEMULATOR"] = &fReadoutCRUEmulator;
  fSettings["DIPSWITCH"]          = &fDipswitch;
}

bool TBoardConfig::SetParamValue(std::string Name, std::string Value)
{
  if (fSettings.find(Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = std::stoi(Value);
    return true;
  }

  return false;
}

int TBoardConfig::GetParamValue(std::string Name)
{
  if (fSettings.find(Name) != fSettings.end()) {
    return *(fSettings.find(Name)->second);
  }
  return -1;
}
