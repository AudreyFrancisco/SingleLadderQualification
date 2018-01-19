#include "TScanConfig.h"
#include "TChipConfig.h"
#include <string>

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
  m_dacStart       = DAC_START;
  m_dacStop        = DAC_STOP;
  m_dacStep        = DAC_STEP;
  m_nDacSamples    = NDACSAMPLES;
  m_scanStep       = SCAN_STEP;
  m_tuningMaxrow   = TUNING_MAXROW;
  m_speedy         = SPEEDY;
  m_rawData        = RAWDATA;
  m_ivCurve        = IVCURVE;
  m_ivPoints       = IVPOINTS;
  m_maxIbias       = MAXIBIAS;
  m_localBusCutRed = LOCALBUSCUTRED;
  m_calVpulsel     = CAL_VPULSEL;
  m_targetThresh   = TARGET_THRESHOLD;
  m_voltageScale   = VOLTAGE_SCALE;
  m_backBias       = BACKBIAS;
  m_nominal        = NOMINAL;
  m_isMasked       = false;
  m_mlvdsStrength  = ChipConfig::DCTRL_DRIVER;

  m_readoutSpeed     = READOUTSPEED;
  m_readoutOcc       = READOUTOCC;
  m_readoutDriver    = READOUTDRIVER;
  m_readoutPreemp    = READOUTPREEMP;
  m_readoutRow       = READOUTROW;
  m_readoutPllStages = READOUTPLLSTAGES;


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
  m_powerCutMaxBias3V_IB      = POWER_CUT_MAXBIAS_3V_IB;
  m_powerCutMaxBias3V_OB      = POWER_CUT_MAXBIAS_3V_OB;
  m_powerMaxFactor4V_IB       = POWER_MAXFACTOR_4V_IB;
  m_powerMaxFactor4V_OB       = POWER_MAXFACTOR_4V_OB;

  m_fifoCutMaxErr    = FIFO_CUT_MAXERR;
  m_fifoCutMaxFaulty = FIFO_CUT_MAXFAULTY;

  m_digitalMaxBadPerChipOB      = DIGITAL_MAXBAD_CHIP_OB;
  m_digitalMaxBadPerChipIB      = DIGITAL_MAXBAD_CHIP_IB;
  m_digitalMaxBadPerHicOB       = DIGITAL_MAXBAD_HIC_OB;
  m_digitalMaxBadPerHicIB       = DIGITAL_MAXBAD_HIC_IB;
  m_digitalMaxDeadPerChipGreen  = DIGITAL_MAXDEAD_CHIP_GREEN;
  m_digitalMaxDeadPerChipOrange = DIGITAL_MAXDEAD_CHIP_ORANGE;

  m_digitalMaxNoMaskHicIB      = DIGITAL_MAXNOMASK_HIC_IB;
  m_digitalMaxNoMaskHicOB      = DIGITAL_MAXNOMASK_HIC_OB;
  m_digitalMaxNoMaskStuckHicIB = DIGITAL_MAXNOMASKSTUCK_HIC_IB;
  m_digitalMaxNoMaskStuckHicOB = DIGITAL_MAXNOMASKSTUCK_HIC_OB;

  m_threshMaxBadPerChipOB = THRESH_MAXBAD_CHIP_OB;
  m_threshMaxBadPerChipIB = THRESH_MAXBAD_CHIP_IB;
  m_threshMaxBadPerHicOB  = THRESH_MAXBAD_HIC_OB;
  m_threshMaxBadPerHicIB  = THRESH_MAXBAD_HIC_IB;
  m_threshMaxNoiseOB      = THRESH_MAXNOISE_OB;
  m_threshMaxNoiseIB      = THRESH_MAXNOISE_IB;

  m_enduranceCycles   = ENDURANCE_CYCLES;
  m_enduranceTriggers = ENDURANCE_TRIGGERS;
  
  m_useDataPath = false;

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
  fSettings["DACSTART"]     = &m_ithrStart;
  fSettings["DACSTOP"]      = &m_ithrStop;
  fSettings["DACSTEP"]      = &m_ithrStep;
  fSettings["NDACSAMPLES"]  = &m_nDacSamples;
  fSettings["SCAN_STEP"]    = &m_scanStep;
  fSettings["TUNINGMAXROW"] = &m_tuningMaxrow;
  fSettings["SPEEDY"]       = &m_speedy;
  fSettings["RAWDATA"]      = &m_rawData;
  fSettings["IVCURVE"]      = &m_ivCurve;
  fSettings["IVPOINTS"]     = &m_ivPoints;
  fSettings["MAXIBIAS"]     = &m_maxIbias;
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
  fSettings["MAXBIAS_3V_IB"]      = &m_powerCutMaxBias3V_IB;
  fSettings["MAXBIAS_3V_OB"]      = &m_powerCutMaxBias3V_OB;
  fSettings["MAXFACTOR_4V_IB"]    = &m_powerMaxFactor4V_IB;
  fSettings["MAXFACTOR_4V_OB"]    = &m_powerMaxFactor4V_OB;

  fSettings["FIFO_MAXERR"]        = &m_fifoCutMaxErr;
  fSettings["FIFO_MAXFAULTY"]     = &m_fifoCutMaxFaulty;

  fSettings["DIGITAL_MAXBAD_CHIP_OB"] = &m_digitalMaxBadPerChipOB;
  fSettings["DIGITAL_MAXBAD_CHIP_IB"] = &m_digitalMaxBadPerChipIB;
  fSettings["DIGITAL_MAXBAD_HIC_OB"]  = &m_digitalMaxBadPerHicOB;
  fSettings["DIGITAL_MAXBAD_HIC_IB"]  = &m_digitalMaxBadPerHicIB;
  fSettings["DIGITAL_MAXDEAD_CHIP_GREEN"] = &m_digitalMaxDeadPerChipGreen;
  fSettings["DIGITAL_MAXDEAD_CHIP_ORANGE"] = &m_digitalMaxDeadPerChipOrange;

  fSettings["DIGITAL_MAXNOMASK_HIC_OB"]       = &m_digitalMaxNoMaskHicOB;
  fSettings["DIGITAL_MAXNOMASK_HIC_IB"]       = &m_digitalMaxNoMaskHicIB;
  fSettings["DIGITAL_MAXNOMASKSTUCK_HIC_OB"]  = &m_digitalMaxNoMaskStuckHicOB;
  fSettings["DIGITAL_MAXNOMASKSTUCK_HIC_IB"]  = &m_digitalMaxNoMaskStuckHicIB;

  fSettings["THRESH_MAXBAD_CHIP_OB"] = &m_threshMaxBadPerChipOB;
  fSettings["THRESH_MAXBAD_CHIP_IB"] = &m_threshMaxBadPerChipIB;
  fSettings["THRESH_MAXBAD_HIC_OB"]  = &m_threshMaxBadPerHicOB;
  fSettings["THRESH_MAXBAD_HIC_IB"]  = &m_threshMaxBadPerHicIB;
  fSettings["THRESH_MAXNOISE_OB"]    = &m_threshMaxNoiseOB;
  fSettings["THRESH_MAXNOISE_IB"]    = &m_threshMaxNoiseIB;

  fSettings["CAL_VPULSEL"]        = &m_calVpulsel;
  fSettings["TARGETTHRESH"]        = &m_targetThresh;
  fSettings["NOMINAL"]            = &m_nominal;

  fSettings["ENDURANCECYCLES"]    = &m_enduranceCycles;
  fSettings["ENDURANCETRIGGERS"]  = &m_enduranceTriggers;

  fSettings["READOUTSPEED"]     = &m_readoutSpeed;
  fSettings["READOUTOCC"]       = &m_readoutOcc;
  fSettings["READOUTDRIVER"]    = &m_readoutDriver;
  fSettings["READOUTPREEMP"]    = &m_readoutPreemp;
  fSettings["READOUTROW"]       = &m_readoutRow;
  fSettings["READOUTPLLSTAGES"] = &m_readoutPllStages;
}


bool TScanConfig::SetParamValue (std::string Name, std::string Value)
{
  if (fSettings.find (Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = std::stoi(Value);
    return true;
  }

  return false;
}


bool TScanConfig::SetParamValue (std::string Name, int Value)
{
  if (fSettings.find (Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = Value;
    return true;
  }

  return false;
}


int TScanConfig::GetParamValue (std::string Name)
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
