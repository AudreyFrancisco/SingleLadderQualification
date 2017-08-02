#include "TChipConfig.h"
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace ChipConfig;

TChipConfig::TChipConfig (TConfig *config, int chipId, const char *fName) {
  fConfig           = config;
  fChipId           = chipId;
  fEnabled          = true;
  fReceiver         = -1;
  fControlInterface = -1;
 
  // fill default values from header file
  fVCASN   = VCASN;
  fVCASN2  = VCASN2;
  fVCLIP   = VCLIP;
  fVRESETD = VRESETD;
  fITHR    = ITHR;
  fIBIAS   = IBIAS;

  fReadoutMode         = READOUT_MODE;
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
  
  fPllPhase            = PLL_PHASE;
  fPllStages           = PLL_STAGES;
  fChargePump          = CHARGE_PUMP;
  fDtuDriver           = DTU_DRIVER;
  fDtuPreemp           = DTU_PREEMP;
 
  if (fName) {
    // read information from file
  }

  InitParamMap();
}


void TChipConfig::InitParamMap () 
{
  fSettings["CHIPID"]           = &fChipId;
  fSettings["RECEIVER"]         = &fReceiver;
  fSettings["CONTROLINTERFACE"] = &fControlInterface;
  fSettings["ENABLED"]          = &fEnabled;
  fSettings["ITHR"]             = &fITHR;
  fSettings["IDB"]              = &fIDB;
  fSettings["VCASN"]            = &fVCASN;
  fSettings["VCASN2"]           = &fVCASN2;
  fSettings["VCLIP"]            = &fVCLIP;
  fSettings["VRESETD"]          = &fVRESETD;
  fSettings["IBIAS"]            = &fIBIAS;
  fSettings["VCASP"]            =  &fVCASP;
  fSettings["VPULSEL"]          = &fVPULSEL;
  fSettings["VPULSEH"]          = &fVPULSEH;
  fSettings["VRESETP"]          = &fVRESETP;
  fSettings["VTEMP"]            = &fVTEMP;
  fSettings["IAUX2"]            = &fIAUX2;
  fSettings["IRESET"]           = &fIRESET;
  fSettings["STROBEDURATION"]   = &fStrobeDuration;
  fSettings["PULSEDURATION"]    = &fPulseDuration;
  fSettings["STROBEDELAYCHIP"]  = &fStrobeDelay;
  fSettings["READOUTMODE"]      = (int*)&fReadoutMode;
  fSettings["LINKSPEED"]        = &fSerialLinkSpeed; 
  fSettings["PLLPHASE"]         = &fPllPhase;
  fSettings["PLLSTAGES"]        = &fPllStages;
  fSettings["CHARGEPUMP"]       = &fChargePump;
  fSettings["DTUDRIVER"]        = &fDtuDriver;
  fSettings["DTUPREEMP"]        = &fDtuPreemp;
}


bool TChipConfig::SetParamValue (const char *Name, const char *Value) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    sscanf (Value, "%d", fSettings.find(Name)->second);
    return true;
  }

  return false;
}


bool TChipConfig::SetParamValue (const char *Name, int Value) 
{
  if (fSettings.find (Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = Value;
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


bool TChipConfig::HasEnabledSlave() {
  if (!IsOBMaster()) return false;
  for (int i = fChipId + 1; i <= fChipId + 6; i++) {
    if (fConfig->GetChipConfigById(i)->IsEnabled()) return true;
  }
  return false;
}