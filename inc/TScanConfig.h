#ifndef TSCANCONFIG_H
#define TSCANCONFIG_H

/* Eventually, this should get moved back to TScanConfig. */

#include <map>
#include <string>
#include <string.h>

namespace ScanConfig {
  const int NINJ           = 50;        // number of injections in digital/threshold scans
  const int NTRIG          = 1000000;   // number of triggers for noise occupancy scans
  const int CHARGE_START   = 0;
  const int CHARGE_STOP    = 50;
  const int CHARGE_STEP    = 1;
  const int N_MASK_STAGES  = 3;
  const int PIX_PER_REGION = 32;
  const int NOISECUT_INV   = 100000;   // inverse of pixel noise cut (e.g. 100000 = 1e-5)

  const int ITHR_START     = 30;
  const int ITHR_STOP      = 100;
  const int ITHR_STEP      = 1;
  const int VCASN_START    = 30;
  const int VCASN_STOP     = 70;
  const int VCASN_STEP     = 1;
  const int SCAN_STEP      = 16; //Grab every Xth row (for tuneITHR/VCASN scan only).
                                 //Speeds up scan; changing this has little effect on result accuracy.
  const int LOCALBUSCUTRED = 1;

  // current limits for powering test in mA
  const int POWER_CUT_MINIDDA_OB = 20;
  const int POWER_CUT_MINIDDD_OB = 50;
  const int POWER_CUT_MINIDDD_IB = 50;
  const int POWER_CUT_MINIDDA_IB = 20;

  const int POWER_CUT_MINIDDA_CLOCKED_OB = 150;
  const int POWER_CUT_MINIDDD_CLOCKED_OB = 600;
  const int POWER_CUT_MAXIDDA_CLOCKED_OB = 250;
  const int POWER_CUT_MAXIDDD_CLOCKED_OB = 850;

  const int POWER_CUT_MINIDDA_CLOCKED_IB = 90;
  const int POWER_CUT_MINIDDD_CLOCKED_IB = 400;
  const int POWER_CUT_MAXIDDA_CLOCKED_IB = 180;
  const int POWER_CUT_MAXIDDD_CLOCKED_IB = 550;

  // cuts for fifo test
  const int FIFO_CUT_MAXERR    = 128;  // max number of errors per pattern and hic
  const int FIFO_CUT_MAXFAULTY = 1;   // max number of chips with errors

  // cuts for digital scan
  const int DIGITAL_MAXBAD_CHIP_OB = 1024;   // max number of bad pixels: 1 dcol
  const int DIGITAL_MAXBAD_CHIP_IB = 524;    // 1 per mille
  const int DIGITAL_MAXBAD_HIC_OB  = 7340;   // 1 per mille
  const int DIGITAL_MAXBAD_HIC_IB  = 4700;   // 1 per mille

  // cuts for threshold scan
  const int THRESH_MAXBAD_CHIP_OB = 1024;   // max number of bad pixels: 1 dcol
  const int THRESH_MAXBAD_CHIP_IB = 524;    // 1 per mille
  const int THRESH_MAXBAD_HIC_OB  = 7340;   // 1 per mille
  const int THRESH_MAXBAD_HIC_IB  = 4700;   // 1 per mille
  const int THRESH_MAXNOISE_OB    = 10;     // max noise of a single chip
  const int THRESH_MAXNOISE_IB    = 10;

  const int SPEEDY         = 0;  //Use slow fit if 0, differentiate->mean if 1.
  const int RAWDATA        = 0;
  const int CAL_VPULSEL    = 160; //VPULSEH assumed 170.  Used for ITHR and VCASN scans.
  const int IVCURVE        = 1;   //Do I-V-curve on back bias
  const int IVPOINTS       = 40;  //number of 100 mV-points for back bias IV curve (max. 50 = 5V)
  const float VOLTAGE_SCALE = 1.0;
}


class TScanConfig {
 private:
  std::map <std::string, int*> fSettings;
  int  m_nInj;
  int  m_nTrig;
  int  m_chargeStart;
  int  m_chargeStop;
  int  m_chargeStep;
  int  m_nMaskStages;
  int  m_pixPerRegion;
  int  m_noiseCutInv;
  char m_fNameSuffix[20];
  //NEW--added for additional scans
  int  m_ithrStart;  //usually 30
  int  m_ithrStop;   //usually 100
  int  m_ithrStep;
  int  m_vcasnStart; //usually 40
  int  m_vcasnStop;  //usually 60
  int  m_vcasnStep;
  int  m_scanStep;   //16
  int  m_speedy;
  int  m_rawData;
  int  m_ivCurve;
  int  m_ivPoints;
  int  m_localBusCutRed;
  int  m_powerCutMinIdda_OB;
  int  m_powerCutMinIddd_OB;
  int  m_powerCutMinIddaClocked_OB;
  int  m_powerCutMinIdddClocked_OB;
  int  m_powerCutMaxIddaClocked_OB;
  int  m_powerCutMaxIdddClocked_OB;
  int  m_powerCutMinIdda_IB;
  int  m_powerCutMinIddd_IB;
  int  m_powerCutMinIddaClocked_IB;
  int  m_powerCutMinIdddClocked_IB;
  int  m_powerCutMaxIddaClocked_IB;
  int  m_powerCutMaxIdddClocked_IB;
  int  m_fifoCutMaxErr;
  int  m_fifoCutMaxFaulty;
  int  m_digitalMaxBadPerChipOB;
  int  m_digitalMaxBadPerChipIB;
  int  m_digitalMaxBadPerHicOB;
  int  m_digitalMaxBadPerHicIB;
  int  m_threshMaxBadPerChipOB;
  int  m_threshMaxBadPerChipIB;
  int  m_threshMaxBadPerHicOB;
  int  m_threshMaxBadPerHicIB;
  int  m_threshMaxNoiseIB;
  int  m_threshMaxNoiseOB;
  int  m_calVpulsel;
  float m_voltageScale;
 protected:
 public:
  TScanConfig ();
  ~TScanConfig() {};
  void  InitParamMap    ();
  bool  SetParamValue   (const char *Name, const char *Value);
  int   GetParamValue   (const char *Name) ;
  bool  IsParameter     (const char *Name) {return (fSettings.count(Name) > 0);};

  int   GetNInj          () {return m_nInj;};
  int   GetChargeStart   () {return m_chargeStart;};
  int   GetChargeStep    () {return m_chargeStep;};
  int   GetChargeStop    () {return m_chargeStop;};
  int   GetNMaskStages   () {return m_nMaskStages;};
  char *GetfNameSuffix   () {return m_fNameSuffix;};
  int   GetIthrStart     () {return m_ithrStart;};
  int   GetIthrStop      () {return m_ithrStop;};
  int   GetIthrStep      () {return m_ithrStep;};
  int   GetVcasnStart    () {return m_vcasnStart;};
  int   GetVcasnStop     () {return m_vcasnStop;};
  int   GetVcasnStep     () {return m_vcasnStep;};
  int   GetScanStep      () {return m_scanStep;};
  int   GetSpeedy        () {return m_speedy;};
  int   GetLocalBusCutRed() {return m_localBusCutRed;};
  void  SetfNameSuffix   (const char *aSuffix) {strcpy (m_fNameSuffix, aSuffix);};
  int   GetCalVpulsel    () {return m_calVpulsel;};
  void  SetVoltageScale  (float aScale) {m_voltageScale = aScale;};
  float GetVoltageScale  () {return m_voltageScale;};
};


#endif
