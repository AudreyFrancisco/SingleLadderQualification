#include "TScanConfig.h"

using namespace ScanConfig;

TScanConfig::TScanConfig()
{
  // dummy values for first tests
  m_nInj           = NINJ;
  m_nTrig          = NTRIG;
  m_chargeStart    = CHARGE_START;
  m_chargeStop     = CHARGE_STOP;
  m_chargeStep     = CHARGE_STEP;
  m_nMaskStages    = N_MASK_STAGES;
  m_pixPerRegion   = PIX_PER_REGION;
  m_noiseCutInv    = NOISECUT_INV;
  m_vcasnStart     = VCASN_START;
  m_vcasnStop      = VCASN_STOP;
  m_vcasnStep      = VCASN_STEP;
  m_ithrStart      = ITHR_START;
  m_ithrStop       = ITHR_STOP;
  m_ithrStep       = ITHR_STEP;
  m_scanStep       = SCAN_STEP;
  m_speedy         = SPEEDY;
  m_rawData        = RAW_DATA;
  m_localBusCutRed = LOCALBUSCUTRED;
  m_calVpulsel     = CAL_VPULSEL;

  m_voltageScale   = VOLTAGE_SCALE;

  m_powerCutMinIdda_OB        = POWER_CUT_MINIDDA_OB;
  m_powerCutMinIddd_OB        = POWER_CUT_MINIDDD_OB;
  m_powerCutMinIddaClocked_OB = POWER_CUT_MINIDDA_CLOCKED_OB;
  m_powerCutMinIdddClocked_OB = POWER_CUT_MINIDDD_CLOCKED_OB;
  m_powerCutMaxIddaClocked_OB = POWER_CUT_MAXIDDA_CLOCKED_OB;
  m_powerCutMaxIdddClocked_OB = POWER_CUT_MAXIDDD_CLOCKED_OB;
  m_powerCutMinIdda_IB        = POWER_CUT_MINIDDD_IB;
  m_powerCutMinIddd_IB        = POWER_CUT_MINIDDA_IB;
  m_powerCutMinIddaClocked_IB = POWER_CUT_MINIDDA_CLOCKED_IB;
  m_powerCutMinIdddClocked_IB = POWER_CUT_MINIDDD_CLOCKED_IB;
  m_powerCutMaxIddaClocked_IB = POWER_CUT_MAXIDDA_CLOCKED_IB;
  m_powerCutMaxIdddClocked_IB = POWER_CUT_MAXIDDD_CLOCKED_IB;

  m_fifoCutMaxErr    = FIFO_CUT_MAXERR;
  m_fifoCutMaxFaulty = FIFO_CUT_MAXFAULTY;

  m_digitalMaxBadPerChipOB = DIGITAL_MAXBAD_CHIP_OB;
  m_digitalMaxBadPerChipIB = DIGITAL_MAXBAD_CHIP_IB;
  m_digitalMaxBadPerHicOB  = DIGITAL_MAXBAD_HIC_OB;
  m_digitalMaxBadPerHicIB  = DIGITAL_MAXBAD_HIC_IB;

  m_threshMaxBadPerChipOB = THRESH_MAXBAD_CHIP_OB;
  m_threshMaxBadPerChipIB = THRESH_MAXBAD_CHIP_IB;
  m_threshMaxBadPerHicOB  = THRESH_MAXBAD_HIC_OB;
  m_threshMaxBadPerHicIB  = THRESH_MAXBAD_HIC_IB;
  m_threshMaxNoiseOB      = THRESH_MAXNOISE_OB;
  m_threshMaxNoiseIB      = THRESH_MAXNOISE_IB;

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
  fSettings["VCASN_STEP"]   = &m_vcasnStep;
  fSettings["ITHR_START"]   = &m_ithrStart;
  fSettings["ITHR_STOP"]    = &m_ithrStop;
  fSettings["ITHR_STEP"]    = &m_ithrStep;
  fSettings["SCAN_STEP"]    = &m_scanStep;
  fSettings["SPEEDY"]       = &m_speedy;
  fSettings["RAW_DATA"]     = &m_rawData;

  fSettings["MINIDDA_OB"]         = &m_powerCutMinIdda_OB;
  fSettings["MINIDDD_OB"]         = &m_powerCutMinIddd_OB;
  fSettings["MINIDDA_CLOCKED_OB"] = &m_powerCutMinIddaClocked_OB;
  fSettings["MINIDDD_CLOCKED_OB"] = &m_powerCutMinIdddClocked_OB;
  fSettings["MAXIDDA_CLOCKED_OB"] = &m_powerCutMaxIddaClocked_OB;
  fSettings["MAXIDDD_CLOCKED_OB"] = &m_powerCutMaxIdddClocked_OB;
  fSettings["MINIDDA_IB"]         = &m_powerCutMinIdda_IB;
  fSettings["MINIDDD_IB"]         = &m_powerCutMinIddd_IB;
  fSettings["MINIDDA_CLOCKED_IB"] = &m_powerCutMinIddaClocked_IB;
  fSettings["MINIDDD_CLOCKED_IB"] = &m_powerCutMinIdddClocked_IB;
  fSettings["MAXIDDA_CLOCKED_IB"] = &m_powerCutMaxIddaClocked_IB;
  fSettings["MAXIDDD_CLOCKED_IB"] = &m_powerCutMaxIdddClocked_IB;

  fSettings["FIFO_MAXERR"]        = &m_fifoCutMaxErr;
  fSettings["FIFO_MAXFAULTY"]     = &m_fifoCutMaxFaulty;

  fSettings["DIGITAL_MAXBAD_CHIP_OB"] = &m_digitalMaxBadPerChipOB;
  fSettings["DIGITAL_MAXBAD_CHIP_IB"] = &m_digitalMaxBadPerChipIB;
  fSettings["DIGITAL_MAXBAD_HIC_OB"]  = &m_digitalMaxBadPerHicOB;
  fSettings["DIGITAL_MAXBAD_HIC_IB"]  = &m_digitalMaxBadPerHicIB;

  fSettings["THRESH_MAXBAD_CHIP_OB"] = &m_threshMaxBadPerChipOB;
  fSettings["THRESH_MAXBAD_CHIP_IB"] = &m_threshMaxBadPerChipIB;
  fSettings["THRESH_MAXBAD_HIC_OB"]  = &m_threshMaxBadPerHicOB;
  fSettings["THRESH_MAXBAD_HIC_IB"]  = &m_threshMaxBadPerHicIB;
  fSettings["THRESH_MAXNOISE_OB"]    = &m_threshMaxNoiseOB;
  fSettings["THRESH_MAXNOISE_IB"]    = &m_threshMaxNoiseIB;

  fSettings["CAL_VPULSEL"]        = &m_calVpulsel;
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
