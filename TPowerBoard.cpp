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
}

/* -------------------------
	Constructor

	Parameter Input : board, TReadoutBoardMOSAIC pointer to a valid MOSAIC board object

	NOTE: this constructor allows to load a default configuration into the data set.
  -------------------------- */
TPowerBoard::TPowerBoard(TReadoutBoardMOSAIC *board)
{
	TPowerBoardConfig *theConfig = new TPowerBoardConfig("");
	fMOSAICPowerBoard = board->GetPowerBoardHandle();
	fPowerBoardConfig = theConfig;
	realTimeRead = false;
}

/* -------------------------
	Init()

	Initialize all the members with the configuration values, than
	try to access the power board. Switch off all channels, then store
	the settings and finally read the power board status.

  -------------------------- */
void TPowerBoard::Init()
{
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

	// Get the State
	powerboard::pbstate *thePowerBoardState;
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
		if(aState->Vout[i*2]+1 != fPBoard.Modules[i].DVset) {
			match = false;
			std::cout << "Power board : Module =" << i << " the Digital V Set is different ! Board:" << aState->Vout[i*2] << " Config:" << fPBoard.Modules[i].AVset << std::endl;
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
	powerboard::pbstate_t *thePowerBoardState;
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
	fPBoard.Modules[module].AVmon  = AV;
	fPBoard.Modules[module].DVmon  = DV;
	fPBoard.Modules[module].AImon  = AI;
	fPBoard.Modules[module].DImon  = DI;
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
	fMOSAICPowerBoard->onVout(module *2);
	fMOSAICPowerBoard->onVout(module *2+1);
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
	*AI = fPBoard.Modules[module].AImon;
	*DV = fPBoard.Modules[module].DVmon;
	*DI = fPBoard.Modules[module].DImon;
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
	powerboard::pbstate_t *thePowerBoardState;
	try {
		fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::GetMonitor);
	} catch (...) {
		std::cerr << "Error accessing the Power board found ! Abort." << std::endl;
		return(false);
	}
	return(true);
}

// ------------------ eof ---------------------------
