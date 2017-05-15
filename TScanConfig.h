#ifndef TSCANCONFIG_H
#define TSCANCONFIG_H

#include <map>
#include <string>

namespace ScanConfig {
  const int NINJ           = 50;
  const int CHARGE_START   = 0;
  const int CHARGE_STOP    = 50;
  const int CHARGE_STEP    = 1;
  const int N_MASK_STAGES  = 3;
  const int PIX_PER_REGION = 32;
}


class TScanConfig {
 private: 
  std::map <std::string, int*> fSettings;
  int  m_nInj;
  int  m_chargeStart;
  int  m_chargeStop;
  int  m_chargeStep;
  int  m_nMaskStages;
  int  m_pixPerRegion;
  char m_fNameSuffix[20];
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
  
};



#endif
