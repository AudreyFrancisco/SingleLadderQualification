#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "stdint.h"




class TBoardConfig {
 private:

 protected:
  bool fTriggerEnable;
  bool fPulseEnable;
  int  fNTriggers;
  uint32_t  fTriggerDelay;
  int  fPulseDelay;
  int  fTriggerSource;

 public:
  TBoardConfig(const char *fName = 0, int boardIndex = 0) {};
  
  virtual int GetBoardType() = 0;

  bool GetTriggerEnable()             {return fTriggerEnable;};
  bool GetPulseEnable()               {return fPulseEnable;};
  int  GetNTriggers()                 {return fNTriggers;};
  int  GetTriggerDelay()              {return fTriggerDelay;};
  int  GetPulseDelay()                {return fPulseDelay;};
  int  GetTriggerSource()             {return fTriggerSource;};


  void SetTriggerEnable(bool trigEnable)             {fTriggerEnable = trigEnable;};
  void SetPulseEnable  (bool pulseEnable)            {fPulseEnable   = pulseEnable;};
  void SetNTriggers    (int nTriggers)               {fNTriggers     = nTriggers;};
  void SetTriggerDelay (int trigDelay)               {fTriggerDelay  = trigDelay;};
  void SetPulseDelay   (int pulseDelay)              {fPulseDelay    = pulseDelay;};
  void SetTriggerSource(int trigSource)              {fTriggerSource = trigSource;};



};


#endif   /* BOARDCONFIG_H */
