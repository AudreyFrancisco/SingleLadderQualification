#include "THicConfig.h"

THicConfig::THicConfig(TConfig *config, int modId)
{
  fConfig  = config;
  fModId   = (modId & 0x7);
  fEnabled = true;
  InitParamMap();
}

void THicConfig::InitParamMap()
{
  fSettings["MODID"] = &fModId;
  fSettings["ENHIC"] = &fEnabled;
}

bool THicConfig::SetParamValue(std::string Name, std::string Value)
{
  if (fSettings.find(Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = std::stoi(Value);
    return true;
  }

  return false;
}

bool THicConfig::SetParamValue(std::string Name, int Value)
{
  if (fSettings.find(Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = Value;
    return true;
  }

  return false;
}

int THicConfig::GetParamValue(std::string Name)
{
  if (fSettings.find(Name) != fSettings.end()) {
    return *(fSettings.find(Name)->second);
  }
  return -1;
}

THicConfigOB::THicConfigOB(TConfig *config, int modId) : THicConfig(config, modId)
{
  fEnabledA8    = true;
  fEnabledB0    = true;
  fHSPosById    = GetModId();
  fPoweredCombo = false;

  InitParamMap();
}

void THicConfigOB::InitParamMap()
{
  fSettings["ENSIDEA8"]   = &fEnabledA8;
  fSettings["ENSIDEB0"]   = &fEnabledB0;
  fSettings["HSPOSBYID"]  = &fModId; // Use modId as position in HS by default
  fSettings["POWERCOMBO"] = &fPoweredCombo;

  THicConfig::InitParamMap();
}
