#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H

#include "AlpideDecoder.h"
#include "TConfig.h"
#include <map>
#include <string.h>
#include <string>

class TConfig;

namespace ChipConfig { // to avoid clashes with other configs (e.g. for STROBE_DELAY)
  const int VCASN   = 50;
  const int VCASN2  = 64;
  const int VCLIP   = 0;
  const int VRESETD = 147;
  const int ITHR    = 51;
  const int IBIAS   = 64;
  const int VCASP   = 86;

  const int READOUT_MODE           = 0; // triggered
  const int ENABLE_CLUSTERING      = 1;
  const int MATRIX_READOUT_SPEED   = 1;
  const int SERIAL_LINK_SPEED      = 1200;
  const int ENABLE_SKEWING_GLOBAL  = 1;
  const int ENABLE_SKEWING_STARTRO = 1;
  const int ENABLE_CLOCK_GATING    = 0;
  const int ENABLE_CMU_READOUT     = 0;

  // timing values, to be refined
  const int STROBE_DURATION = 80;   // 2 us
  const int STROBE_GAP      = 4000; // .1 ms
  const int STROBE_DELAY    = 20;   // 500 ns
  const int TRIGGER_DELAY   = 0;
  const int PULSE_DURATION  = 500; // 12.5 us

  const int DCLK_RECEIVER  = 10;
  const int DCLK_DRIVER    = 10;
  const int MCLK_RECEIVER  = 10;
  const int DCTRL_RECEIVER = 10;
  const int DCTRL_DRIVER   = 10;

  const int PLL_PHASE   = 8;
  const int PLL_STAGES  = 1;
  const int CHARGE_PUMP = 10;
  const int DTU_DRIVER  = 4;
  const int DTU_PREEMP  = 4;

  const int PREVIOUS_ID        = 0x10;
  const int INITIAL_TOKEN      = 1;
  const int DISABLE_MANCHESTER = 1;
  const int ENABLE_DDR         = 1;
} // namespace ChipConfig

class TChipConfig {
private:
  std::map<std::string, int *> fSettings;
  TConfig *                    fConfig;
  int                          fChipId;
  int fEnabled;       // variable to exclude (non-working) chip from tests, default true
  int fEnabledWithBB; // variable to exclude chips from tests when BB on, default true
  int fReceiver;
  int fControlInterface;
  // DACs used
  int fITHR;
  int fIDB;
  int fVCASN;
  int fVCASN2;
  int fVCLIP;
  int fVRESETD;
  int fVCASP;
  int fVPULSEL;
  int fVPULSEH;
  int fIBIAS;
  // DACs unused
  int fVRESETP;
  int fVTEMP;
  int fIAUX2;
  int fIRESET;
  // Control register settings
  int fReadoutMode; // 0 = triggered, 1 = continuous (influences busy handling)
  int fEnableClustering;
  int fMatrixReadoutSpeed;
  int fSerialLinkSpeed;
  int fEnableSkewingGlobal;
  int fEnableSkewingStartRO;
  int fEnableClockGating;
  int fEnableCMUReadout;
  // Fromu settings
  int fStrobeDuration;
  int fStrobeGap;    // gap between subsequent strobes in sequencer mode
  int fStrobeDelay;  // delay from pulse to strobe if generated internally
  int fTriggerDelay; // delay between external trigger command and internally generated strobe
  int fPulseDuration;
  // Buffer current settings
  int fDclkReceiver;
  int fDclkDriver;
  int fMclkReceiver;
  int fDctrlReceiver;
  int fDctrlDriver;
  // CMU / DMU settings
  int fPreviousId;
  int fInitialToken;
  int fDisableManchester;
  int fEnableDdr;
  // DTU settings
  int fPllPhase;
  int fPllStages;
  int fChargePump;
  int fDtuDriver;
  int fDtuPreemp;
  // Mask file
  char                 fMaskFile[200];
  std::vector<TPixHit> m_noisyPixels;
  unsigned int         fDoubleColumnMask[32];

protected:
public:
  TChipConfig(TConfig *config, int chipId, const char *fName = 0);
  int  fEnduranceDisabled; // temporary fix to re-enabled chips that were disabled in end. test
  void InitParamMap();
  bool SetParamValue(std::string Name, std::string Value);
  bool SetParamValue(std::string Name, int Value);
  int  GetParamValue(std::string Name);
  bool IsParameter(std::string Name) { return (fSettings.count(Name) > 0); };
  int  GetChipId() { return fChipId; };
  int  GetCtrInt() { return fControlInterface; };
  int  GetDataLink() { return fReceiver; };
  bool IsEnabled() const;
  bool IsEnabledNoBB() const { return fEnabled != 0; }
  bool IsEnabledWithBB() const { return (fEnabled != 0) && (fEnabledWithBB != 0); }
  void SetEnable(bool Enabled) { fEnabled = Enabled ? 1 : 0; };
  void SetEnableWithBB(bool Enabled) { fEnabledWithBB = Enabled ? 1 : 0; };
  int  GetModuleId() { return (fChipId & 0x70) >> 4; };
  bool IsOBMaster() { return ((fChipId % 8 == 0) && (GetModuleId() > 0)); };
  bool HasEnabledSlave();

  bool GetReadoutMode() { return (bool)fReadoutMode; };
  bool GetEnableClustering() { return (bool)fEnableClustering; };
  int  GetMatrixReadoutSpeed() { return fMatrixReadoutSpeed; };
  int  GetSerialLinkSpeed() { return fSerialLinkSpeed; };
  bool GetEnableSkewingGlobal() { return (bool)fEnableSkewingGlobal; };
  bool GetEnableSkewingStartRO() { return (bool)fEnableSkewingStartRO; };
  bool GetEnableClockGating() { return (bool)fEnableClockGating; };
  bool GetEnableCMUReadout() { return (bool)fEnableCMUReadout; };

  int GetTriggerDelay() { return fTriggerDelay; };
  int GetStrobeDuration() { return fStrobeDuration; };
  int GetStrobeDelay() { return fStrobeDelay; };
  int GetPulseDuration() { return fPulseDuration; };

  int GetDclkReceiver() { return fDclkReceiver; };
  int GetDclkDriver() { return fDclkDriver; };
  int GetMclkReceiver() { return fMclkReceiver; };
  int GetDctrlReceiver() { return fDctrlReceiver; };
  int GetDctrlDriver() { return fDctrlDriver; };

  int  GetPreviousId() { return fPreviousId; };
  bool GetInitialToken() { return (bool)fInitialToken; };
  bool GetDisableManchester() { return (bool)fDisableManchester; };
  bool GetEnableDdr() { return (bool)fEnableDdr; };

  void SetPreviousId(int APreviousId) { fPreviousId = APreviousId; };
  void SetInitialToken(bool AInitialToken) { fInitialToken = (AInitialToken) ? 1 : 0; };
  void SetEnableDdr(bool AEnableDdr) { fEnableDdr = (AEnableDdr) ? 1 : 0; };
  void SetDisableManchester(bool ADisableManchester)
  {
    fDisableManchester = (ADisableManchester) ? 1 : 0;
  };

  void                 SetMaskFile(const char *fName) { strcpy(fMaskFile, fName); };
  void                 SetNoisyPixels(std::vector<TPixHit> noisy) { m_noisyPixels = noisy; };
  void                 ClearNoisyPixels() { m_noisyPixels.clear(); };
  std::vector<TPixHit> GetNoisyPixels() { return m_noisyPixels; };

  void         SetDoubleColumnMask(unsigned int dcol, bool mask = true);
  unsigned int GetDoubleColumnMask(unsigned int region);
};

#endif /* CHIPCONFIG_H */
