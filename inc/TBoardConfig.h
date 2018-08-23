#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "stdint.h"
#include <map>
#include <string>

namespace BoardConfig {
  const int PULSEDELAY         = 10000;
  const int STROBEDELAY        = 20;
  const int NCHIPS             = 9;
  const int DIPSWITCH          = 0;
  const int READOUTCRUEMULATOR = 0;
} // namespace BoardConfig

typedef enum { trigInt, trigExt } TTriggerSource;
typedef enum { boardDAQ, boardMOSAIC, boardRU, boardRUv1 } TBoardType;

class TBoardConfig {
private:
protected:
  std::map<std::string, int *> fSettings;
  bool                         fTriggerEnable;
  bool                         fPulseEnable;
  int                          fNTriggers;
  int                          fTriggerDelay;
  int                          fPulseDelay;
  int                          fNChips;
  int                          fDipswitch;
  TTriggerSource               fTriggerSource;
  TBoardType                   fBoardType;
  int                          fReadoutCRUEmulator;

public:
  TBoardConfig(const char *fName = 0, int boardIndex = 0);
  virtual void InitParamMap();
  bool         SetParamValue(std::string Name, std::string Value);
  int          GetParamValue(std::string Name);
  bool         IsParameter(std::string Name) { return (fSettings.count(Name) > 0); };

  virtual TBoardType GetBoardType() { return fBoardType; };

  bool           GetTriggerEnable() { return fTriggerEnable; };
  bool           GetPulseEnable() { return fPulseEnable; };
  int            GetNTriggers() { return fNTriggers; };
  int            GetTriggerDelay() { return fTriggerDelay; };
  int            GetPulseDelay() { return fPulseDelay; };
  TTriggerSource GetTriggerSource() { return fTriggerSource; };

  void SetTriggerEnable(bool trigEnable) { fTriggerEnable = trigEnable; };
  void SetPulseEnable(bool pulseEnable) { fPulseEnable = pulseEnable; };
  void SetNTriggers(int nTriggers) { fNTriggers = nTriggers; };
  void SetTriggerDelay(int trigDelay) { fTriggerDelay = trigDelay; }; // obsolete
  void SetPulseDelay(int pulseDelay) { fPulseDelay = pulseDelay; };   // obsolete
  void SetTriggerSource(TTriggerSource trigSource) { fTriggerSource = trigSource; };
};

#endif /* BOARDCONFIG_H */
