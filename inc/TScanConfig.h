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
  const int VCASN_START    = 40;
  const int VCASN_STOP     = 60;
  const int SPEEDY         = 1;
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
  int *m_ithr; //needed by ITHRthreshold; stores values for each chip
  int *m_vcasn; //needed by tuneITHR+ITHRthreshold; stores values for each HIC
  int  m_ithrStart;  //usually 30
  int  m_ithrStop;   //usually 100
  int  m_vcasnStart; //usually 40
  int  m_vcasnStop;  //usually 60
  int  m_speedy;

 protected: 
 public:
  TScanConfig ();
  ~TScanConfig() {};
  void  InitParamMap    ();
  bool  SetParamValue   (const char *Name, const char *Value);
  int   GetParamValue   (const char *Name) ;
  bool  IsParameter     (const char *Name) {return (fSettings.count(Name) > 0);};
  void  SetVcasnArr     (int hics, float *vcasn);
    //Will set a different value of vcasn for each HIC.
  void  SetIthrArr      (int chips, float *ithr);
    //Will set a different value of ithr for each chip.

  int   GetNInj         () {return m_nInj;};
  int   GetChargeStart  () {return m_chargeStart;};
  int   GetChargeStep   () {return m_chargeStep;};
  int   GetChargeStop   () {return m_chargeStop;};
  int   GetNMaskStages  () {return m_nMaskStages;};
  char *GetfNameSuffix  () {return m_fNameSuffix;};  
};



#endif
