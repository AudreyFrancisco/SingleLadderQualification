#include "TScanConfig.h" 

using namespace ScanConfig;

TScanConfig::TScanConfig() 
{
  // dummy values for first tests
  m_nInj         = NINJ;
  m_nTrig        = NTRIG;
  m_chargeStart  = CHARGE_START;
  m_chargeStop   = CHARGE_STOP;
  m_chargeStep   = CHARGE_STEP;
  m_nMaskStages  = N_MASK_STAGES;
  m_pixPerRegion = PIX_PER_REGION;
  InitParamMap();
}


void TScanConfig::InitParamMap () 
{
  fSettings["NINJ"]         = &m_nInj;
  fSettings["NTRIG"]        = &m_nTrig;
  fSettings["CHARGESTART"]  = &m_chargeStart;
  fSettings["CHARGESTOP"]   = &m_chargeStop;
  fSettings["CHARGESTEP"]   = &m_chargeStep;
  fSettings["NMASKSTAGES"]  = &m_nMaskStages;
  fSettings["PIXPERREGION"] = &m_pixPerRegion;
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


