#include "TChipConfig.h"
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace ChipConfig;

TChipConfig::TChipConfig (int chipId, const char *fName) {
  fChipId  = chipId;
  fEnabled = true;
 
  // fill default values from header file
  fVCASN   = VCASN;
  fVCASN2  = VCASN2;
  fVCLIP   = VCLIP;
  fVRESETD = VRESETD;
  fITHR    = ITHR;
  fIBIAS   = IBIAS;

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

  InitParamMap();
}


void TChipConfig::InitParamMap () 
{
  fSettings["CHIPID"]  = &fChipId;
  fSettings["ENABLED"] = &fEnabled;
  fSettings["ITHR"]    = &fITHR;
  fSettings["IDB"]     = &fIDB;
  fSettings["VCASN"]   = &fVCASN;
  fSettings["VCASN2"]  = &fVCASN2;
  fSettings["VCLIP"]   = &fVCLIP;
  fSettings["VRESETD"] = &fVRESETD;
  fSettings["IBIAS"]   = &fIBIAS;
  fSettings["VCASP"]   = &fVCASP;
  fSettings["VPULSEL"] = &fVPULSEL;
  fSettings["VPULSEH"] = &fVPULSEH;
  fSettings["VRESETP"] = &fVRESETP;
  fSettings["VTEMP"]   = &fVTEMP;
  fSettings["IAUX2"]   = &fIAUX2;
  fSettings["IRESET"]  = &fIRESET;
}


bool TChipConfig::SetParamValue (const char *Name, const char *Value) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    sscanf (Value, "%d", fSettings.find(Name)->second);
    return true;
  }

  return false;
}


int TChipConfig::GetParamValue (const char *Name) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    return *(fSettings.find(Name)->second);
  }
  return -1;
}


