#ifndef TSCANCONFIG_H
#define TSCANCONFIG_H

/* Eventually, this should get moved back to TScanConfig. */

#include <map>
#include <string>

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
  const int VCASN_START    = 40;
  const int VCASN_STOP     = 60;
  const int VCASN_STEP     = 1;
  const int SCAN_STEP      = 16; //Grab every Xth row (for tuneITHR/VCASN scan only).
                                 //Speeds up scan; changing this has little effect on result accuracy.
  const int SPEEDY         = 1;  //Use slow fit if 0, differentiate->mean if 1.
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

 protected: 
 public:
  TScanConfig ();
  ~TScanConfig() {};
  void  InitParamMap    ();
  bool  SetParamValue   (const char *Name, const char *Value);
  int   GetParamValue   (const char *Name) ;
  bool  IsParameter     (const char *Name) {return (fSettings.count(Name) > 0);};

  int   GetNInj         () {return m_nInj;};
  int   GetChargeStart  () {return m_chargeStart;};
  int   GetChargeStep   () {return m_chargeStep;};
  int   GetChargeStop   () {return m_chargeStop;};
  int   GetNMaskStages  () {return m_nMaskStages;};
  char *GetfNameSuffix  () {return m_fNameSuffix;};  
  int   GetIthrStart    () {return m_ithrStart;};
  int   GetIthrStop     () {return m_ithrStop;};
  int   GetIthrStep     () {return m_ithrStep;};
  int   GetVcasnStart   () {return m_vcasnStart;};
  int   GetVcasnStop    () {return m_vcasnStop;};
  int   GetVcasnStep    () {return m_vcasnStep;};
  int   GetScanStep     () {return m_scanStep;};
  int   GetSpeedy       () {return m_speedy;};

};



#endif