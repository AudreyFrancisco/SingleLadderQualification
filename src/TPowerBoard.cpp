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
#include "TReadoutBoardMOSAIC.h"
#include <unistd.h>



/* -------------------------
	Constructor

	Parameter Input : board, TReadoutBoardMOSAIC pointer to a valid MOSAIC board object
					config, TPowerBoardConfig pointer to a Power Board Config object
  -------------------------- */
TPowerBoard::TPowerBoard(TReadoutBoardMOSAIC *board, TPowerBoardConfig *config)
{
	fMOSAICPowerBoard = board->GetPowerBoardHandle();
	fPowerBoardConfig = config;
	realTimeRead = false;
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
	fMOSAICPowerBoard = board->GetPowerBoardHandle();
	fPowerBoardConfig = theConfig;
	realTimeRead = false;
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
	thePowerBoardState = new powerboard::pbstate;

	fPBoard.VBset = fPowerBoardConfig->GetBiasVoltage();
	for(int i=0; i<MAX_MOULESPERMOSAIC; i++) {
		fPBoard.Modules[i].AIset = fPowerBoardConfig->GetAnalogCurrent(i);
		fPBoard.Modules[i].AVset = fPowerBoardConfig->GetAnalogVoltage(i);
		fPBoard.Modules[i].DIset = fPowerBoardConfig->GetDigitalCurrent(i);
		fPBoard.Modules[i].DVset = fPowerBoardConfig->GetDigitalVoltage(i);
		fPBoard.Modules[i].BiasOn = fPowerBoardConfig->GetBiasOn(i);
	}
	// first of all test the presence of the power board
	try {
		fMOSAICPowerBoard->isReady();
	} catch (...) {
		std::cerr << "No Power board found ! Abort." << std::endl;
		return;
	}

        fMOSAICPowerBoard->startADC();
	// Get the State
	try {
		fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::getAll);
	} catch (...) {
		std::cerr << "Error accessing the Power board found ! Abort." << std::endl;
		return;
	}

	compareSettings(thePowerBoardState);

	// Switch off all channels before the setting
	fMOSAICPowerBoard->offAllVbias();
	fMOSAICPowerBoard->offAllVout();

	// Finally set up the PowerBoard
	fMOSAICPowerBoard->setVbias( fPBoard.VBset );
	for(int i=0; i< MAX_MOULESPERMOSAIC; i++) {
		fMOSAICPowerBoard->setIth((unsigned char)(i*2), fPBoard.Modules[i].AIset );
		fMOSAICPowerBoard->setIth((unsigned char)(i*2+1), fPBoard.Modules[i].DIset );
		fMOSAICPowerBoard->setVout((unsigned char)(i*2), fPBoard.Modules[i].AVset);
		fMOSAICPowerBoard->setVout((unsigned char)(i*2+1), fPBoard.Modules[i].DVset);
		if(fPBoard.Modules[i].BiasOn)
			fMOSAICPowerBoard->onVbias((unsigned char)(i));
		else
			fMOSAICPowerBoard->offVbias((unsigned char)(i));
	}
	// and also store the values inside the PB memory
	fMOSAICPowerBoard->storeAllVout();

	// first read of monitor values
	fMOSAICPowerBoard->getState(thePowerBoardState,powerboard::getFlags::getAll);
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
	int i;
	for(i=0;i<MAX_MOULESPERMOSAIC;i++) {
		if(aState->Vout[i*2] != fPBoard.Modules[i].AVset) {
			match = false;
			std::cout << "Power board : Module =" << i << " the Analog V Set is different ! Board:" << aState->Vout[i*2] << " Config:" << fPBoard.Modules[i].AVset << std::endl;
		}
		if(aState->Vout[i*2+1] != fPBoard.Modules[i].DVset) {
			match = false;
			std::cout << "Power board : Module =" << i << " the Digital V Set is different ! Board:" << aState->Vout[i*2 + 1] << " Config:" << fPBoard.Modules[i].DVset << std::endl;
		}
	}
	return(match);
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
	if(realTimeRead) {
		time_t now;
		time(&now);
		if(now <= fPBoard.TimeStamp + MINIMUM_TIMELAPSEFORPOLLING) return(false);
	}

	// Read the board
	fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::GetMonitor);

	// Set the data members
	time(&fPBoard.TimeStamp); // mark the time stamp
	fPBoard.VBmon = thePowerBoardState->Vbias;
	fPBoard.IBmon = thePowerBoardState->Ibias;
	fPBoard.Temp = thePowerBoardState->T;

	for(i=0;i<MAX_MOULESPERMOSAIC;i++) { // for each module
		fPBoard.Modules[i].AVmon  = thePowerBoardState->Vmon[i*2];
		fPBoard.Modules[i].DVmon  = thePowerBoardState->Vmon[i*2+1];
		fPBoard.Modules[i].AImon  = thePowerBoardState->Imon[i*2];
		fPBoard.Modules[i].DImon  = thePowerBoardState->Imon[i*2+1];
		fPBoard.Modules[i].BiasOn = thePowerBoardState->biasOn & (0x01 << i);
		fPBoard.Modules[i].AchOn = thePowerBoardState->chOn & (0x0001 << (i*2));
		fPBoard.Modules[i].DchOn = thePowerBoardState->chOn & (0x0001 << (i*2+1));
	}

	return(true);
}


float TPowerBoard::GetAnalogCurrent (int module)
{
  float AIOffset, DIOffset;
  float Current = 0;
  int   N       = 20;
  for (int i = 0; i < N; i ++) {
    readMonitor();
    Current += fPBoard.Modules[module].AImon;
  }
  Current /= N;
  fPowerBoardConfig->GetICalibration(module, AIOffset, DIOffset);
  return (Current - AIOffset);
}


float TPowerBoard::GetDigitalCurrent (int module)
{
  float AIOffset, DIOffset;
  float Current = 0;
  int   N       = 20;
  for (int i = 0; i < N; i ++) {
    readMonitor();
    Current += fPBoard.Modules[module].DImon;
  }
  Current /= N;
  fPowerBoardConfig->GetICalibration(module, AIOffset, DIOffset);
  return (Current - DIOffset);
}


// Calibrate the output voltages of a given module
// set calibration constants back to 1 / 0
// set two different voltages and measure the output voltage for each setting
// determine new calibration constants
void TPowerBoard::CalibrateVoltage(int module)
{
  // two set points that fall on a full bin
  // set the lower voltage second to not risk applying 2.2 V to a HIC
  float set2 = 1.58;
  float set1 = 2.187;
  float analog1, analog2, digital1, digital2;
  float manalog, mdigital, banalog, bdigital;

  // set calibration back to slope 1 / intercept 0
  fPowerBoardConfig->SetVCalibration (module, 1, 1, 0, 0);

  // set and measure first point
  SetAnalogVoltage (module, set1);
  SetDigitalVoltage(module, set1);
  SwitchModule     (module, true);

  sleep(1);

  analog1  = GetAnalogVoltage  (module);
  digital1 = GetDigitalVoltage (module);

  // set and measure second point
  SetAnalogVoltage (module, set2);
  SetDigitalVoltage(module, set2);

  sleep(1);

  analog2  = GetAnalogVoltage  (module);
  digital2 = GetDigitalVoltage (module);

  // calculate slope and intercept for calibration Vout -> Vset
  manalog  = (set1 - set2) / (analog1  - analog2);
  mdigital = (set1 - set2) / (digital1 - digital2);
  banalog  = set1 - manalog  * analog1;
  bdigital = set1 - mdigital * digital1;

  // set new calibration values and switch off module
  fPowerBoardConfig->SetVCalibration (module, manalog, mdigital, banalog, bdigital);

  SwitchModule (module, false);
}


// Calibrate the current offset for a given module
// switch of the channel
// measure the current
void TPowerBoard::CalibrateCurrent(int module)
{
  float aOffset, dOffset;
  fPowerBoardConfig->SetICalibration (module, 0, 0);

  SwitchModule (module, false);

  sleep(1);

  aOffset = GetAnalogCurrent  (module);
  dOffset = GetDigitalCurrent (module);

  fPowerBoardConfig->SetICalibration (module, aOffset, dOffset);
}


// correct the output voltage for the (calculated) voltage drop
// if reset is true, the correction is set to 0
void TPowerBoard::CorrectVoltageDrop (int module, bool reset)
{
  // Measure the channel currents
  // Calculate voltage drop
  // Correct voltage drop for slope of voltage characteristics
  // add corrected voltage drop to channel set voltage
  float RAnalog, RDigital, RGround;
  float dVAnalog, dVDigital;
  float AVScale, DVScale, AVOffset, DVOffset;
  if (reset) {
    dVAnalog  = 0;
    dVDigital = 0;
  }
  else {
    sleep(1);
    float IDDA = GetAnalogCurrent (module);
    float IDDD = GetDigitalCurrent(module);

    fPowerBoardConfig->GetLineResistances (module, RAnalog, RDigital, RGround);
    fPowerBoardConfig->GetVCalibration    (module, AVScale, DVScale,  AVOffset, DVOffset);

    dVAnalog  = IDDA * RAnalog  + (IDDA + IDDD) * RGround;
    dVDigital = IDDD * RDigital + (IDDA + IDDD) * RGround;

    dVAnalog  *= AVScale;
    dVDigital *= DVScale;
  }

  if ((fPBoard.Modules[module].AVset + dVAnalog > SAFE_OUTPUT) ||
      (fPBoard.Modules[module].DVset + dVDigital > SAFE_OUTPUT)) {
    std::cout << "ERROR (CorrectVoltageDrop): Asking for set voltage above safe limit; using uncorrected values." << std::endl;
    dVAnalog  = 0;
    dVDigital = 0;
  }

  // fPBoard contains the voltages corrected with the channel calibration
  fMOSAICPowerBoard->setVout((unsigned char)(module*2),   fPBoard.Modules[module].AVset + dVAnalog);
  fMOSAICPowerBoard->setVout((unsigned char)(module*2+1), fPBoard.Modules[module].DVset + dVDigital);
  sleep(1);
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

	fMOSAICPowerBoard->setVout((unsigned char)(module * 2), AV);
	fMOSAICPowerBoard->setVout((unsigned char)(module * 2+1), DV);
	fMOSAICPowerBoard->setIth((unsigned char)(module * 2), AI);
	fMOSAICPowerBoard->setIth((unsigned char)(module * 2+1), DI);
	if(BiasOn)
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
	if(value) {
		fMOSAICPowerBoard->onVout(module *2);
		fMOSAICPowerBoard->onVout(module *2+1);
	} else {
		fMOSAICPowerBoard->offVout(module *2);
		fMOSAICPowerBoard->offVout(module *2+1);
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
					AChOn, pointer to bool. State of Analogic channel, true := ON, false := OFF
					DChOn, pointer to bool. State of Digital channel, true := ON, false := OFF
  -------------------------- */
void TPowerBoard::GetModule(int module, float* AV, float *AI, float *DV, float *DI, bool *BiasOn, bool *AChOn, bool *DChOn)
{
	readMonitor();
	*AV = fPBoard.Modules[module].AVmon;
	*AI = GetAnalogCurrent(module);
	*DV = fPBoard.Modules[module].DVmon;
	*DI = GetDigitalCurrent(module);
	*BiasOn = fPBoard.Modules[module].BiasOn;
	*AChOn = fPBoard.Modules[module].AchOn;
	*DChOn = fPBoard.Modules[module].DchOn;
	return;
}

/* -------------------------
	IsOk()

	Returns if the power board is connected and operable

	Return : true if the power board is OK
  -------------------------- */
bool TPowerBoard::IsOK()
{
	// first of all test the presence of the power board
	try {
		fMOSAICPowerBoard->isReady();
	} catch (...) {
		std::cerr << "No Power board found ! Abort." << std::endl;
		return(false);
	}
	// Now read the state
	try {
		fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::GetMonitor);
	} catch (...) {
		std::cerr << "Error accessing the Power board found ! Abort." << std::endl;
		return(false);
	}
	return(true);
}

// ------------------ eof ---------------------------
