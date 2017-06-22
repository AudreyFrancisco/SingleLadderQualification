#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H

#include "TConfig.h"
#include <map>
#include <string>

class TConfig;

namespace ChipConfig {     // to avoid clashes with other configs (e.g. for STROBE_DELAY)
  const int  VCASN   = 50;
  const int  VCASN2  = 64;
  const int  VCLIP   = 0;
  const int  VRESETD = 147;
  const int  ITHR    = 51;
  const int  IBIAS   = 64;
  const int  VCASP   = 86;

  const bool READOUT_MODE           = false;// triggered
  const bool ENABLE_CLUSTERING      = true;
  const int  MATRIX_READOUT_SPEED   = 1;
  const int  SERIAL_LINK_SPEED      = 1200;
  const bool ENABLE_SKEWING_GLOBAL  = false;
  const bool ENABLE_SKEWING_STARTRO = false;
  const bool ENABLE_CLOCK_GATING    = false;
  const bool ENABLE_CMU_READOUT     = false;

  // timing values, to be refined
  const int  STROBE_DURATION = 80;       // 2 us
  const int  STROBE_GAP      = 4000;     // .1 ms
  const int  STROBE_DELAY    = 20;       // 500 ns
  const int  TRIGGER_DELAY   = 0;
  const int  PULSE_DURATION  = 500;      // 12.5 us

  const int  DCLK_RECEIVER   = 10;
  const int  DCLK_DRIVER     = 10;
  const int  MCLK_RECEIVER   = 10;
  const int  DCTRL_RECEIVER  = 10;
  const int  DCTRL_DRIVER    = 10;

  const int  PLL_PHASE       = 8;
  const int  PLL_STAGES      = 1;
  const int  CHARGE_PUMP     = 8;
  const int  DTU_DRIVER      = 15;
  const int  DTU_PREEMP      = 15;

  const int  PREVIOUS_ID        = 0x10;
  const bool INITIAL_TOKEN      = true;
  const bool DISABLE_MANCHESTER = false;
  const bool ENABLE_DDR         = true;

};


class TChipConfig {
 private: 
  std::map <std::string, int*> fSettings;
  TConfig *fConfig;
  int      fChipId;
  int      fEnabled;                 // variable to exclude (non-working) chip from tests, default true
  int      fReceiver;
  int      fControlInterface;
  // DACs used
  int  fITHR;
  int  fIDB;
  int  fVCASN;
  int  fVCASN2; 
  int  fVCLIP;
  int  fVRESETD;
  int  fVCASP;
  int  fVPULSEL;
  int  fVPULSEH;
  int  fIBIAS;
  // DACs unused
  int  fVRESETP;
  int  fVTEMP;
  int  fIAUX2;
  int  fIRESET;
  // Control register settings
  bool fReadoutMode;        // false = triggered, true = continuous (influences busy handling)
  bool fEnableClustering;
  int  fMatrixReadoutSpeed;
  int  fSerialLinkSpeed;
  bool fEnableSkewingGlobal;
  bool fEnableSkewingStartRO;
  bool fEnableClockGating;
  bool fEnableCMUReadout;
  // Fromu settings
  int  fStrobeDuration;
  int  fStrobeGap;          // gap between subsequent strobes in sequencer mode
  int  fStrobeDelay;        // delay from pulse to strobe if generated internally
  int  fTriggerDelay;       // delay between external trigger command and internally generated strobe
  int  fPulseDuration;
  // Buffer current settings
  int  fDclkReceiver;
  int  fDclkDriver;
  int  fMclkReceiver;
  int  fDctrlReceiver;
  int  fDctrlDriver;
  // CMU / DMU settings
  int  fPreviousId;
  bool fInitialToken;
  bool fDisableManchester;
  bool fEnableDdr;
  // DTU settings
  int  fPllPhase;
  int  fPllStages;
  int  fChargePump;
  int  fDtuDriver;
  int  fDtuPreemp;
 protected:
 public:
  TChipConfig               (TConfig *config, int chipId, const char *fName = 0);
  void InitParamMap         (); 
  bool SetParamValue        (const char *Name, const char *Value);
  bool SetParamValue        (const char *Name, int Value);
  int  GetParamValue        (const char *Name) ;
  bool IsParameter          (const char *Name) {return (fSettings.count(Name) > 0);};
  int  GetChipId            () {return fChipId;};
  int  GetCtrInt            () {return fControlInterface;};
  int  GetDataLink          () {return fReceiver;};
  bool IsEnabled            () {return (fEnabled != 0);};
  void SetEnable            (bool Enabled) {fEnabled = Enabled?1:0;};
  int  GetModuleId          () {return (fChipId & 0x70) >> 4;};
  bool IsOBMaster           () {return ((fChipId % 8 == 0) && (GetModuleId() > 0));};
  bool HasEnabledSlave      (); 

  bool GetReadoutMode          () {return fReadoutMode;};
  bool GetEnableClustering     () {return fEnableClustering;};
  int  GetMatrixReadoutSpeed   () {return fMatrixReadoutSpeed;};
  int  GetSerialLinkSpeed      () {return fSerialLinkSpeed;};
  bool GetEnableSkewingGlobal  () {return fEnableSkewingGlobal;};
  bool GetEnableSkewingStartRO () {return fEnableSkewingStartRO;};
  bool GetEnableClockGating    () {return fEnableClockGating;};
  bool GetEnableCMUReadout     () {return fEnableCMUReadout;};
  
  int  GetTriggerDelay         () {return fTriggerDelay;};
  int  GetStrobeDuration       () {return fStrobeDuration;};
  int  GetStrobeDelay          () {return fStrobeDelay;};
  int  GetPulseDuration        () {return fPulseDuration;};

  int  GetDclkReceiver         () {return fDclkReceiver;};
  int  GetDclkDriver           () {return fDclkDriver;};
  int  GetMclkReceiver         () {return fMclkReceiver;};
  int  GetDctrlReceiver        () {return fDctrlReceiver;};
  int  GetDctrlDriver          () {return fDctrlDriver;};

  int  GetPreviousId           () {return fPreviousId;};
  bool GetInitialToken         () {return fInitialToken;};
  bool GetDisableManchester    () {return fDisableManchester;};
  bool GetEnableDdr            () {return fEnableDdr;};

  void SetPreviousId           (int APreviousId)         {fPreviousId   = APreviousId;};
  void SetInitialToken         (bool AInitialToken)      {fInitialToken = AInitialToken;};
  void SetEnableDdr            (bool AEnableDdr)         {fEnableDdr    = AEnableDdr;};
  void SetDisableManchester    (bool ADisableManchester) {fDisableManchester = ADisableManchester;};
};


#endif   /* CHIPCONFIG_H */
