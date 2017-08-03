#include "THicConfig.h"

THicConfig::THicConfig(TConfig* config, int modId)
{
	fConfig  = config;
	fModId   = (modId & 0x7);
	fEnabled = true;
	InitParamMap();
}


void THicConfig::InitParamMap () 
{
	fSettings["MODID"] = &fModId; 
  fSettings["ENHIC"] = &fEnabled;
}


bool THicConfig::SetParamValue (const char *Name, const char *Value) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    sscanf (Value, "%d", fSettings.find(Name)->second);
    return true;
  }

  return false;
}


bool THicConfig::SetParamValue (const char *Name, int Value) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = Value;
    return true;
  }

  return false;
}

int THicConfig::GetParamValue (const char *Name) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    return *(fSettings.find(Name)->second);
  }
  return -1;
}


THicConfigOB::THicConfigOB(TConfig* config, int modId):
	THicConfig(config, modId)
{
	fEnabledA8 = true;
	fEnabledB0 = true;
	fHSPosById = GetModId();
  
	InitParamMap();
}


void THicConfigOB::InitParamMap()
{
	fSettings["ENSIDEA8"]  = &fEnabledA8;
	fSettings["ENSIDEB0"]  = &fEnabledB0;
	fSettings["HSPOSBYID"] = &fModId;  //Use modId as position in HS by default

	THicConfig::InitParamMap();	
}
