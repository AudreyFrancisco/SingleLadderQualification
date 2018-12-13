/*
 * Copyright (C) 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * ====================================================
 *     __  __   __  _____  __   __
 *    / / /  | / / / ___/ /  | / / SEZIONE di BARI
 *   / / / | |/ / / /_   / | |/ /
 *  / / / /| / / / __/  / /| / /
 * /_/ /_/ |__/ /_/    /_/ |__/
 *
 * ====================================================
 * Written by Antonio Franco  <Anotnio.Franco@ba.infn.it>
 *
 *		TPowerBoard class implementation
 *
 *		ver.1.0		12/07/2017
 *
 *
 *  		HISTORY
 *
 *
 */
#include "TPowerBoard.h"
#include "TConfig.h"
#include "THicConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include <TMatrix.h>
#include <chrono>
#include <numeric>
#include <thread>
#include <unistd.h>
/* -------------------------
        Constructor

        Parameter Input : board, TReadoutBoardMOSAIC pointer to a valid MOSAIC board object
                                        config, TPowerBoardConfig pointer to a Power Board Config
  object
  -------------------------- */
TPowerBoard::TPowerBoard(TReadoutBoardMOSAIC *board, TPowerBoardConfig *config)
{
  fMOSAICPowerBoard = board->GetPowerBoardHandle();
  fPowerBoardConfig = config;
  realTimeRead      = false;
  Init();
}

/* -------------------------
        Constructor

        Parameter Input : board, TReadoutBoardMOSAIC pointer to a valid MOSAIC board object

        NOTE: this constructor allows to load a default configuration into the data set.
  -------------------------- */
TPowerBoard::TPowerBoard(TReadoutBoardMOSAIC *board)
{
  TPowerBoardConfig *theConfig = new TPowerBoardConfig(NULL);
  fMOSAICPowerBoard            = board->GetPowerBoardHandle();
  fPowerBoardConfig            = theConfig;
  realTimeRead                 = false;
  Init();
}

/* -------------------------
        Init()

        Initialize all the members with the configuration values, than
        try to access the power board. Switch off all channels, then store
        the settings and finally read the power board status.

  -------------------------- */
void TPowerBoard::Init()
{
  fPowerBoardConfig->ReadCalibrationFile();
  thePowerBoardState = new powerboard::pbstate;

  fPBoard.VBset = fPowerBoardConfig->GetBiasVoltage();
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    fPBoard.Modules[i].AIset  = fPowerBoardConfig->GetAnalogCurrent(i);
    fPBoard.Modules[i].AVset  = fPowerBoardConfig->GetAnalogVoltage(i);
    fPBoard.Modules[i].DIset  = fPowerBoardConfig->GetDigitalCurrent(i);
    fPBoard.Modules[i].DVset  = fPowerBoardConfig->GetDigitalVoltage(i);
    fPBoard.Modules[i].BiasOn = fPowerBoardConfig->GetBiasOn(i);
  }
  std::lock_guard<std::mutex> lock(mutex_pb);
  // first of all test the presence of the power board
  try {
    fMOSAICPowerBoard->isReady();
  }
  catch (...) {
    std::cerr << "No Power board found ! Abort." << std::endl;
    return;
  }

  fMOSAICPowerBoard->startADC();
  // Get the State
  try {
    fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::getAll);
  }
  catch (...) {
    std::cerr << "Error accessing the Power board found ! Abort." << std::endl;
    return;
  }

  compareSettings(thePowerBoardState);

  // Switch off all channels before the setting
  fMOSAICPowerBoard->offAllVbias();
  fMOSAICPowerBoard->offAllVout();

  // Finally set up the PowerBoard
  fMOSAICPowerBoard->setVbias(fPBoard.VBset);
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    fMOSAICPowerBoard->setIth((unsigned char)(i * 2), fPBoard.Modules[i].AIset);
    fMOSAICPowerBoard->setIth((unsigned char)(i * 2 + 1), fPBoard.Modules[i].DIset);
    fMOSAICPowerBoard->setVout((unsigned char)(i * 2), fPBoard.Modules[i].AVset);
    fMOSAICPowerBoard->setVout((unsigned char)(i * 2 + 1), fPBoard.Modules[i].DVset);
    if (fPBoard.Modules[i].BiasOn)
      fMOSAICPowerBoard->onVbias((unsigned char)(i));
    else
      fMOSAICPowerBoard->offVbias((unsigned char)(i));
  }
  // and also store the values inside the PB memory
  fMOSAICPowerBoard->storeAllVout();

  // first read of monitor values
  fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::getAll);
  return;
}

/* -------------------------
        compareSettings()

        Compares the setup read from the power board firmware
        with that we store in class member

        Return : false is there is a mismatch

  -------------------------- */
bool TPowerBoard::compareSettings(powerboard::pbstate_t *aState)
{
  bool match = true;
  int  i;
  for (i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    if (aState->Vout[i * 2] != fPBoard.Modules[i].AVset) {
      match = false;
      std::cout << "Power board : Module =" << i
                << " the Analog V Set is different ! Board:" << aState->Vout[i * 2]
                << " Config:" << fPBoard.Modules[i].AVset << std::endl;
    }
    if (aState->Vout[i * 2 + 1] != fPBoard.Modules[i].DVset) {
      match = false;
      std::cout << "Power board : Module =" << i
                << " the Digital V Set is different ! Board:" << aState->Vout[i * 2 + 1]
                << " Config:" << fPBoard.Modules[i].DVset << std::endl;
    }
  }
  return (match);
}

/* -------------------------
        readMonitor()

        Retrieve all monitored power board parameters from the hardware, and stores
        the values inside the class data member.
        If the last read time stamp is less then MINIMUM_TIMELAPSEFORPOLLING seconds
        the physical access will be skipped and the actual values are considered valid.

        Return : true if a physical access is done in order to monitor the power board

  -------------------------- */
bool TPowerBoard::readMonitor()
{
  int i;

  // test if the values are considered valid
  if (realTimeRead) {
    time_t now;
    time(&now);
    if (now <= fPBoard.TimeStamp + MINIMUM_TIMELAPSEFORPOLLING) return (false);
  }

  // Read the board
  std::lock_guard<std::mutex> lock(mutex_pb);
  fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::GetMonitor);

  // Set the data members
  time(&fPBoard.TimeStamp); // mark the time stamp
  fPBoard.VBmon = thePowerBoardState->Vbias;
  fPBoard.IBmon = thePowerBoardState->Ibias;
  fPBoard.Temp  = thePowerBoardState->T;

  for (i = 0; i < MAX_MOULESPERMOSAIC; i++) { // for each module
    fPBoard.Modules[i].AVmon         = thePowerBoardState->Vmon[i * 2];
    fPBoard.Modules[i].DVmon         = thePowerBoardState->Vmon[i * 2 + 1];
    fPBoard.Modules[i].AVsetReadback = thePowerBoardState->Vout[i * 2];
    fPBoard.Modules[i].DVsetReadback = thePowerBoardState->Vout[i * 2 + 1];
    fPBoard.Modules[i].AImon         = thePowerBoardState->Imon[i * 2];
    fPBoard.Modules[i].DImon         = thePowerBoardState->Imon[i * 2 + 1];
    fPBoard.Modules[i].BiasOn        = thePowerBoardState->biasOn & (0x01 << i);
    fPBoard.Modules[i].AchOn         = thePowerBoardState->chOn & (0x0001 << (i * 2));
    fPBoard.Modules[i].DchOn         = thePowerBoardState->chOn & (0x0001 << (i * 2 + 1));
  }

  return (true);
}

void TPowerBoard::GetPowerBoardState(powerboard::pbstate *state)
{
  // Read the board
  std::lock_guard<std::mutex> lock(mutex_pb);
  fMOSAICPowerBoard->getState(state, powerboard::getFlags::GetMonitor);
}

float TPowerBoard::GetAnalogCurrent(int module)
{
  float AIOffset, DIOffset;
  float Current = 0;
  int   N       = 20;
  for (int i = 0; i < N; i++) {
    readMonitor();
    Current += fPBoard.Modules[module].AImon;
  }
  Current /= N;
  fPowerBoardConfig->GetICalibration(module, AIOffset, DIOffset);
  return (Current - AIOffset);
}

float TPowerBoard::GetDigitalCurrent(int module)
{
  float AIOffset, DIOffset;
  float Current = 0;
  int   N       = 20;
  for (int i = 0; i < N; i++) {
    readMonitor();
    Current += fPBoard.Modules[module].DImon;
  }
  Current /= N;
  fPowerBoardConfig->GetICalibration(module, AIOffset, DIOffset);
  return (Current - DIOffset);
}

float TPowerBoard::GetBiasCurrent()
{
  float Offset;
  float Current = 0;
  int   N       = 20;
  for (int i = 0; i < N; i++) {
    readMonitor();
    Current += fPBoard.IBmon;
  }
  Current /= N;
  fPowerBoardConfig->GetIBiasCalibration(Offset);

  return (Current - Offset);
}

// Calibrate the bias voltage
// set calibration constants back to 1 / 0
// set two different voltages and measure the output voltage for each setting
// determine new calibration constants
void TPowerBoard::CalibrateVoltage(int module)
{
  // two set points that fall on a full bin
  // set the lower voltage second to not risk applying 2.2 V to a HIC
  float set2 = 1.58;  //-0.4;
  float set1 = 2.187; // -4;
  float analog1, analog2, digital1, digital2;
  float manalog, mdigital, banalog, bdigital;

  // set calibration back to slope 1 / intercept 0
  fPowerBoardConfig->SetVCalibration(module, 1, 1, 0, 0);

  // set and measure first point
  SetAnalogVoltage(module, set1);
  SetDigitalVoltage(module, set1);
  SwitchModule(module, true);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  analog1  = GetAnalogVoltage(module);
  digital1 = GetDigitalVoltage(module);

  // set and measure second point
  SetAnalogVoltage(module, set2);
  SetDigitalVoltage(module, set2);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  analog2  = GetAnalogVoltage(module);
  digital2 = GetDigitalVoltage(module);

  // calculate slope and intercept for calibration Vout -> Vset
  manalog  = (set1 - set2) / (analog1 - analog2);
  mdigital = (set1 - set2) / (digital1 - digital2);
  banalog  = set1 - manalog * analog1;
  bdigital = set1 - mdigital * digital1;

  // set new calibration values and switch off module
  fPowerBoardConfig->SetVCalibration(module, manalog, mdigital, banalog, bdigital);

  SwitchModule(module, false);
}

// Calibrate the current offset for a given module
// switch of the channel
// measure the current
void TPowerBoard::CalibrateCurrent(int module)
{
  float aOffset, dOffset;
  fPowerBoardConfig->SetICalibration(module, 0, 0);

  SwitchModule(module, false);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  aOffset = GetAnalogCurrent(module);
  dOffset = GetDigitalCurrent(module);

  fPowerBoardConfig->SetICalibration(module, aOffset, dOffset);
}

void TPowerBoard::CalibrateBiasCurrent()
{
  float Offset;
  fPowerBoardConfig->SetIBiasCalibration(0);

  SetBiasVoltage(0);
  // this is in principle unneccessary as the calibration should be done without HIC attached
  SwitchOFF();
  for (int imod = 0; imod < MAX_MOULESPERMOSAIC; imod++) {
    SetBiasOff(imod);
  }

  Offset = GetBiasCurrent();
  fPowerBoardConfig->SetIBiasCalibration(Offset);
}

void TPowerBoard::CalibrateBiasVoltage()
{
  // two set points that fall on a full bin
  // set the lower voltage second to not risk applying 2.2 V to a HIC
  float set2 = -0.4; // 1.58;
  float set1 = -4;   // 2.187;
  float measured1, measured2;
  float m, b;

  // set calibration back to slope 1 / intercept 0
  fPowerBoardConfig->SetVBiasCalibration(1, 0);

  // set and measure first point
  SetBiasVoltage(set1);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  measured1 = GetBiasVoltage();

  // set and measure second point
  SetBiasVoltage(set2);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  measured2 = GetBiasVoltage();

  // calculate slope and intercept for calibration Vout -> Vset
  m = (set1 - set2) / (measured1 - measured2);
  b = set1 - m * measured1;

  // set new calibration values and switch off module
  fPowerBoardConfig->SetVBiasCalibration(m, b);
}

// correct the output voltage for the (calculated) voltage drop
// if reset is true, the correction is set to 0
void TPowerBoard::VDropAllMod()
{
  int N_mod  = fConfig->GetNHics();
  int n_iter = 7;

  float  RAnalog, RDigital, RGround;
  float *IDDA           = new float[N_mod];
  float *IDDD           = new float[N_mod];
  float *VDDA           = new float[N_mod];
  float *VDDD           = new float[N_mod];
  float *dVAnalog_iter  = new float[N_mod];
  float *dVDigital_iter = new float[N_mod];
  float *RGnd_part      = new float[N_mod];
  float *RGnd           = new float[N_mod];
  float *R_mod_a        = new float[N_mod];
  float *R_mod_d        = new float[N_mod];

  for (int i = 0; i < N_mod; i++) {
    fPowerBoardConfig->GetLineResistances(i, RAnalog, RDigital, RGround);
    RGnd[i]    = RGround;
    R_mod_a[i] = 0;
    R_mod_d[i] = 0;
    VDDA[i]    = GetAnalogVoltage(i);
    VDDD[i]    = GetDigitalVoltage(i);
    IDDA[i]    = GetAnalogCurrent(i);
    IDDD[i]    = GetDigitalCurrent(i);
  }
  for (int i = 0; i < N_mod; i++) {
    if (i < 1)
      RGnd_part[i] = RGnd[i];
    else
      RGnd_part[i] = RGnd[i] - RGnd[i - 1];
  }
  for (int i = 0; i < n_iter; i++) {
    float *V_drop_part = new float[N_mod];
    for (int i_mod = 0; i_mod < N_mod; i_mod++) {
      float IDDA_tot = 0;
      float IDDD_tot = 0;
      for (int n_mod = i_mod; n_mod < N_mod; n_mod++) {
        IDDA_tot += IDDA[n_mod];
        IDDD_tot += IDDD[n_mod];
      }
      fPowerBoardConfig->GetLineResistances(i_mod, RAnalog, RDigital, RGround);
      if (i_mod < 1)
        V_drop_part[i_mod] = (IDDA_tot + IDDD_tot) * RGnd_part[i_mod];
      else
        V_drop_part[i_mod] = V_drop_part[i_mod - 1] + (IDDA_tot + IDDD_tot) * RGnd_part[i_mod];
      dVAnalog_iter[i_mod] =
          IDDA[i_mod] * RAnalog + V_drop_part[i_mod] + IDDD[i_mod] * RGnd_part[i_mod];
      dVDigital_iter[i_mod] =
          IDDD[i_mod] * RDigital + V_drop_part[i_mod] + IDDA[i_mod] * RGnd_part[i_mod];
      R_mod_a[i_mod] =
          VDDA[i_mod] + V_drop_part[i_mod] + IDDD[i_mod] * RGnd_part[i_mod] / IDDA[i_mod];
      R_mod_d[i_mod] =
          VDDD[i_mod] + V_drop_part[i_mod] + IDDA[i_mod] * RGnd_part[i_mod] / IDDD[i_mod];
      VDDA[i_mod] += dVAnalog_iter[i_mod];
      VDDD[i_mod] += dVDigital_iter[i_mod];
    }
    for (int i_mod = 0; i_mod < N_mod; i_mod++) {
      float IDDA_tot = 0;
      float IDDD_tot = 0;
      for (int i = i_mod; i < N_mod; i++) {
        IDDA_tot += IDDA[i];
        IDDD_tot += IDDD[i];
      }
      fPowerBoardConfig->GetLineResistances(i_mod, RAnalog, RDigital, RGround);
      IDDA[i_mod] = (VDDA[i_mod] - RGround * (IDDA_tot + IDDD_tot + IDDD[i_mod])) / R_mod_a[i_mod];
      IDDD[i_mod] = (VDDD[i_mod] - RGround * (IDDA_tot + IDDD_tot + IDDA[i_mod])) / R_mod_d[i_mod];
    }
  }
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    if (i < N_mod) {
      fPBoard.Modules[i].DVDrop = dVDigital_iter[i];
      fPBoard.Modules[i].AVDrop = dVAnalog_iter[i];
    }
    else {
      fPBoard.Modules[i].DVDrop = 0.0;
      fPBoard.Modules[i].AVDrop = 0.0;
    }
  }
}
void TPowerBoard::CorrectVoltageDrop(int module, bool reset)
{
  float AVScale, DVScale, AVOffset, DVOffset, dVAnalog, dVDigital;
  if (reset) {
    dVAnalog  = 0;
    dVDigital = 0;
  }
  else {

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    fPowerBoardConfig->GetVCalibration(module, AVScale, DVScale, AVOffset, DVOffset);

    dVAnalog  = fPBoard.Modules[module].AVDrop;
    dVDigital = fPBoard.Modules[module].DVDrop;

    dVAnalog *= AVScale;
    dVDigital *= DVScale;

    if (fPBoard.Modules[module].AVset + dVAnalog > SAFE_OUTPUT) {
      std::cout
          << "ERROR (CorrectVoltageDrop): Asking for set voltage AVDD above safe limit; using "
             "safe max value, difference = "
          << fPBoard.Modules[module].AVset + dVAnalog - SAFE_OUTPUT << " V." << std::endl;
      dVAnalog = SAFE_OUTPUT - fPBoard.Modules[module].AVset;
    }
    if (fPBoard.Modules[module].DVset + dVDigital > SAFE_OUTPUT) {
      std::cout
          << "ERROR (CorrectVoltageDrop): Asking for set voltage DVDD above safe limit; using "
             "safe max value, difference = "
          << fPBoard.Modules[module].DVset + dVDigital - SAFE_OUTPUT << " V." << std::endl;
      dVDigital = SAFE_OUTPUT - fPBoard.Modules[module].DVset;
    }

    // fPBoard contains the voltages corrected with the channel calibration
    std::lock_guard<std::mutex> lock(mutex_pb);
    fMOSAICPowerBoard->setVout((unsigned char)(module * 2),
                               fPBoard.Modules[module].AVset + dVAnalog);
    fMOSAICPowerBoard->setVout((unsigned char)(module * 2 + 1),
                               fPBoard.Modules[module].DVset + dVDigital);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
/* -------------------------
   SetModule()

   Sets all the parameters for one module

   Parameter Input : module, integer (0..7) the Number of the power board module
   AV, float. Analogic channel voltage output
   AI, float. Analogic channel current limit
   DV, float. Digital channel voltage output
   DI, float. Digital channel current limit
   BiasOn bool. Set up of the Back Bias channel

   -------------------------- */
void TPowerBoard::SetModule(int module, float AV, float AI, float DV, float DI, bool BiasOn)
{
  fPBoard.Modules[module].AVset  = AV;
  fPBoard.Modules[module].DVset  = DV;
  fPBoard.Modules[module].AIset  = AI;
  fPBoard.Modules[module].DIset  = DI;
  fPBoard.Modules[module].BiasOn = BiasOn;

  std::lock_guard<std::mutex> lock(mutex_pb);
  fMOSAICPowerBoard->setVout((unsigned char)(module * 2), AV);
  fMOSAICPowerBoard->setVout((unsigned char)(module * 2 + 1), DV);
  fMOSAICPowerBoard->setIth((unsigned char)(module * 2), AI);
  fMOSAICPowerBoard->setIth((unsigned char)(module * 2 + 1), DI);
  if (BiasOn)
    fMOSAICPowerBoard->onVbias(module);
  else
    fMOSAICPowerBoard->offVbias(module);
  return;
}

/* -------------------------
   SwitchModule()

   Switch On/Off the Analogic and Digital channel of the specified module

   Parameter Input : module, integer (0..7) the Number of the power board module
   value, bool. The switch value ON := true, OFF := false

   -------------------------- */
void TPowerBoard::SwitchModule(int module, bool value)
{
  fPBoard.Modules[module].AchOn = value;
  fPBoard.Modules[module].DchOn = value;
  std::lock_guard<std::mutex> lock(mutex_pb);
  if (value) {
    fMOSAICPowerBoard->onVout(module * 2);
    fMOSAICPowerBoard->onVout(module * 2 + 1);
  }
  else {
    fMOSAICPowerBoard->offVout(module * 2);
    fMOSAICPowerBoard->offVout(module * 2 + 1);
  }

  return;
}

/* -------------------------
   GetModule()

   Gets all the parameters for one module

   Parameter Input : module, integer (0..7) the Number of the power board module

   Parameter Output : AV, pointer to float. Analogic channel voltage output
   AI, pointer to float. Analogic channel current limit
   DV, pointer to float. Digital channel voltage output
   DI, pointer to float. Digital channel current limit
   BiasOn pointer to bool. Set up of the Back Bias channel
   AChOn, pointer to bool. State of Analogic channel, true :=
   ON, false := OFF
   DChOn, pointer to bool. State of Digital channel, true :=
   ON, false := OFF
   -------------------------- */
void TPowerBoard::GetModule(int module, float *AV, float *AI, float *DV, float *DI, bool *BiasOn,
                            bool *AChOn, bool *DChOn)
{
  readMonitor();
  *AV     = fPBoard.Modules[module].AVmon;
  *AI     = GetAnalogCurrent(module);
  *DV     = fPBoard.Modules[module].DVmon;
  *DI     = GetDigitalCurrent(module);
  *BiasOn = fPBoard.Modules[module].BiasOn;
  *AChOn  = fPBoard.Modules[module].AchOn;
  *DChOn  = fPBoard.Modules[module].DchOn;
  return;
}

/* -------------------------
   IsOk()

   Returns if the power board is connected and operable

Return : true if the power board is OK
-------------------------- */
bool TPowerBoard::IsOK()
{
  std::lock_guard<std::mutex> lock(mutex_pb);
  // first of all test the presence of the power board
  try {
    fMOSAICPowerBoard->isReady();
  }
  catch (...) {
    std::cerr << "No Power board found ! Abort." << std::endl;
    return (false);
  }
  // Now read the state
  try {
    fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::GetMonitor);
  }
  catch (...) {
    std::cerr << "Error accessing the Power board found ! Abort." << std::endl;
    return (false);
  }
  return (true);
}

// ------------------ eof ---------------------------
