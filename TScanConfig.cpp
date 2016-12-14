#include "TScanConfig.h" 

TScanConfig::TScanConfig() 
{
  // dummy values for first tests
  m_chargeStart = 0;
  m_chargeStop  = 50;
  m_chargeStep  = 1;
  m_nMaskStages = 3;

}


void TScanConfig::InitParamMap () 
{
  //fSettings["CHIPID"]           = &fChipId;

}


bool TScanConfig::SetParamValue (const char *Name, const char *Value) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    sscanf (Value, "%d", fSettings.find(Name)->second);
    return true;
  }

  return false;
}


int TScanConfig::GetParamValue (const char *Name) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    return *(fSettings.find(Name)->second);
  }
  return -1;
}


