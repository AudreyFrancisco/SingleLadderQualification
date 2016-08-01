#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H


namespace ChipConfig {     // to avoid clashes with other configs (e.g. for STROBE_DELAY)
  const bool ENABLE_CLUSTERING      = true;
  const int  MATRIX_READOUT_SPEED   = 1;
  const int  SERIAL_LINK_SPEED      = 3;
  const bool ENABLE_SKEWING_GLOBAL  = false;
  const bool ENABLE_SKEWING_STARTRO = false;
  const bool ENABLE_CLOCK_GATING    = false;
  const bool ENABLE_CMU_READOUT     = false;

  // timing values, to be refined
  const int  STROBE_DURATION = 80;       // 2 us
  const int  STROBE_GAP      = 4000;     // .1 ms
  const int  STROBE_DELAY    = 20;       // 500 ns
  const int  TRIGGER_DELAY   = 0;
  const int  PULSE_DURATION  = 2000;     // 50 us

  const int  DCLK_RECEIVER   = 10;
  const int  DCLK_DRIVER     = 10;
  const int  MCLK_RECEIVER   = 10;
  const int  DCTRL_RECEIVER  = 10;
  const int  DCTRL_DRIVER    = 10;

  const int  PREVIOUS_ID        = 0x10;
  const bool INITIAL_TOKEN      = true;
  const bool DISABLE_MANCHESTER = true;
  const bool ENABLE_DDR         = false;
};


class TChipConfig {
 private: 
  int  fChipId;
  // Control register settings
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
  
 protected:
 public:
  TChipConfig   (int chipId, const char *fName = 0);
  int  GetChipId            () {return fChipId;};
  
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
};


#endif   /* CHIPCONFIG_H */
