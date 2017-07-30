#include "TBoardConfig.h"
#include <stdio.h>

using namespace BoardConfig;

TBoardConfig::TBoardConfig(const char *fName, int boardIndex)
{
  fTriggerDelay = STROBEDELAY;
  fPulseDelay   = PULSEDELAY;
}


void TBoardConfig::InitParamMap() 
{
  fSettings["STROBEDELAYBOARD"] = &fTriggerDelay;
  fSettings["PULSEDELAY"]       = &fPulseDelay;
}


bool TBoardConfig::SetParamValue (const char *Name, const char *Value) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    sscanf (Value, "%d", fSettings.find(Name)->second);
    return true;
  }

  return false;
}


int TBoardConfig::GetParamValue (const char *Name) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    return *(fSettings.find(Name)->second);
  }
  return -1;
}

