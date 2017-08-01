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
  m_noiseCutInv  = NOISECUT_INV;
  m_vcasnStart   = VCASN_START;  //there's probably no need to change these four
  m_vcasnStop    = VCASN_STOP;
  m_ithrStart    = ITHR_START;
  m_ithrStop     = ITHR_STOP;
  m_speedy       = SPEEDY;
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
  fSettings["NOISECUT_INV"] = &m_noiseCutInv;

  fSettings["VCASN_START"]  = &m_vcasnStart;
  fSettings["VCASN_STOP"]   = &m_vcasnStop;
  fSettings["ITHR_START"]   = &m_ithrStart;
  fSettings["ITHR_STOP"]    = &m_ithrStop;
  fSettings["SPEEDY"]       = &m_speedy;
  //m_ithr = NULL;  //not always used
  //m_vcasn = NULL;
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


/*void TScanConfig::SetVcasnArr (int hics, float *vcasn) { //copy vcasn array to m_vcasn
  m_vcasn = new int[hics];
  for(int i=0; i<hics; i++) {
    m_vcasn[i] = (int)(vcasn[i]+.5); //rounding matters
  }
}
void TScanConfig::SetIthrArr (int hics, float *ithr) {
  m_ithr = new int[hics];
  for(int i=0; i<hics; i++) {
    m_ithr[i] = (int)(ithr[i]+.5); //rounding matters
  }
}*/


