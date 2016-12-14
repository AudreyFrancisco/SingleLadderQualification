#include "TScanConfig.h" 

using namespace ScanConfig;

TScanConfig::TScanConfig() 
{
  // dummy values for first tests
  m_chargeStart = CHARGE_START;
  m_chargeStop  = CHARGE_STOP;
  m_chargeStep  = CHARGE_STEP;
  m_nMaskStages = N_MASK_STAGES;

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


