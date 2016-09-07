#include "TChipConfig.h"

using namespace ChipConfig;

TChipConfig::TChipConfig (int chipId, const char *fName) {
  fChipId  = chipId;
  fEnabled = true;
 
  // fill default values from header file
  fEnableClustering    = ENABLE_CLUSTERING;
  fMatrixReadoutSpeed  = MATRIX_READOUT_SPEED;
  fSerialLinkSpeed     = SERIAL_LINK_SPEED;
  fEnableSkewingGlobal = ENABLE_SKEWING_GLOBAL;
  fEnableClockGating   = ENABLE_CLOCK_GATING;
  fEnableCMUReadout    = ENABLE_CMU_READOUT;
  
  fStrobeDuration      = STROBE_DURATION;
  fStrobeGap           = STROBE_GAP;
  fStrobeDelay         = STROBE_DELAY;
  fTriggerDelay        = TRIGGER_DELAY;
  fPulseDuration       = PULSE_DURATION;
  
  fDclkReceiver        = DCLK_RECEIVER;
  fDclkDriver          = DCLK_DRIVER;
  fMclkReceiver        = MCLK_RECEIVER;
  fDctrlReceiver       = DCTRL_RECEIVER;
  fDctrlDriver         = DCTRL_DRIVER;
  
  fPreviousId          = PREVIOUS_ID;
  fInitialToken        = INITIAL_TOKEN;
  fDisableManchester   = DISABLE_MANCHESTER;
  fEnableDdr           = ENABLE_DDR;
 
  if (fName) {
    // read information from file
  }
}
