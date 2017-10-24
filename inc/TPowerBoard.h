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
 *		TPowerBoard class definition header
 *
 *		ver.1.0		12/07/2017
 *
 *
 *  		HISTORY
 *
 *
 */
#ifndef TPOWERBOARD_H
#define TPOWERBOARD_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>

#include <powerboard.h>
//#include "TReadoutBoardMOSAIC.h"
#include "TPowerBoardConfig.h"

//class TPowerBoardConfig;
//class powerboard;

class TReadoutBoardMOSAIC;

// The maximum number of modules of the power board
// that the MOSAIC board can drive.
#define MAX_MOULESPERMOSAIC 8

// The validity time for data monitored in seconds,
// this parameter allow the optimization of access
// to the power board
#define MINIMUM_TIMELAPSEFORPOLLING	2

#define SAFE_OUTPUT 2.2

// Class definition
class TPowerBoard {

// Structures and Data types
public:
	struct pbModule {
		bool  AchOn;
		float AVmon;
		float AImon;
		bool  DchOn;
		float DVmon;
		float DImon;
		bool  BiasOn;
		float AVset;
		float AIset;
		float DVset;
		float DIset;
	} ;

	struct pbBoard {
		pbModule Modules[MAX_MOULESPERMOSAIC];
		float VBset;
		float VBmon;
		float IBmon;
		float Temp;
		time_t TimeStamp;
	};

// Members
private:
	powerboard *fMOSAICPowerBoard;  // The handle to the low level object of MOSAIC framework
	pbBoard fPBoard; // The 'powerboard' data structure
	bool realTimeRead; // forces the access to Power Board hardware
	TPowerBoardConfig *fPowerBoardConfig; // the configuration set
	powerboard::pbstate *thePowerBoardState;

// Methods
public:
	TPowerBoard(TReadoutBoardMOSAIC *board);
	TPowerBoard(TReadoutBoardMOSAIC *board, TPowerBoardConfig *config);
	~TPowerBoard () {};

	// Info and Mode
	bool IsOK();
	void SetRealTimeMode(bool realTime) { realTimeRead = realTime; return; };
	TPowerBoardConfig *GetConfigurationHandler() {return(fPowerBoardConfig);};

	// Getters
	float GetTemperature() { readMonitor(); return(fPBoard.Temp);};
	float GetBiasVoltage() { readMonitor(); return(fPBoard.VBmon);};
	float GetBiasCurrent() { readMonitor(); return(fPBoard.IBmon);};

	float GetAnalogVoltage (int module) { readMonitor(); return(fPBoard.Modules[module].AVmon);};
	float GetAnalogCurrent (int module);
	float GetDigitalVoltage(int module) { readMonitor(); return(fPBoard.Modules[module].DVmon);};
	float GetDigitalCurrent(int module);
	bool  IsAnalogChOn     (int module) { readMonitor(); return(fPBoard.Modules[module].AchOn);};
	bool  IsDigitalChOn    (int module) { readMonitor(); return(fPBoard.Modules[module].DchOn);};
	bool  IsBiasChOn       (int module) { readMonitor(); return(fPBoard.Modules[module].BiasOn);};

        void CalibrateVoltage  (int module);
        void CalibrateCurrent  (int module);
        void CorrectVoltageDrop(int module, bool reset = false);
        bool IsCalibrated      (int module) {return GetConfigurationHandler()->IsCalibrated (module);};

	void  GetModule(int module, float* AV, float *AI, float *DV, float *DI, bool *BiasOn, bool *AChOn, bool *DChOn);

	// Setters
	void  SetBiasVoltage(float aVal) { fPBoard.VBset = aVal; fMOSAICPowerBoard->setVbias(aVal);};

	void SetAnalogVoltage(int mod, float val) { fPBoard.Modules[mod].AVset = val; fMOSAICPowerBoard->setVout((unsigned char)(mod*2),val);};
	void SetAnalogCurrent(int mod, float val) { fPBoard.Modules[mod].AIset = val; fMOSAICPowerBoard->setIth((unsigned char)(mod*2),val);};
	void SetDigitalVoltage(int mod, float val) { fPBoard.Modules[mod].DVset = val; fMOSAICPowerBoard->setVout((unsigned char)(mod*2+1),val);};
	void SetDigitalCurrent(int mod, float val) { fPBoard.Modules[mod].DIset = val; fMOSAICPowerBoard->setIth((unsigned char)(mod*2+1),val);};
	void SetModule(int module, float AV, float AI, float DV, float DI, bool BiasOn);
	void SetBiasOn(int mod) { fPBoard.Modules[mod].BiasOn = true; fMOSAICPowerBoard->onVbias((unsigned char)mod);};
	void SetBiasOff(int mod) { fPBoard.Modules[mod].BiasOn = false; fMOSAICPowerBoard->offVbias((unsigned char)mod);};

	void SwitchAnalogOn(int mod) { fPBoard.Modules[mod].AchOn = true; fMOSAICPowerBoard->onVout((unsigned char)(mod*2));};
	void SwitchAnalogOff(int mod) { fPBoard.Modules[mod].AchOn = false; fMOSAICPowerBoard->offVout((unsigned char)(mod*2));};
	void SwitchDigitalOn(int mod) { fPBoard.Modules[mod].DchOn = true; fMOSAICPowerBoard->onVout((unsigned char)(mod*2+1));};
	void SwitchDigitalOff(int mod) { fPBoard.Modules[mod].DchOn = false; fMOSAICPowerBoard->offVout((unsigned char)(mod*2+1));};
	void SwitchModule(int module, bool value);
	void SwitchON() { fMOSAICPowerBoard->onAllVout(); return;};
	void SwitchOFF() { fMOSAICPowerBoard->offAllVout(); return;};

private:
	void Init();
	bool compareSettings(powerboard::pbstate_t *aState);
	bool readMonitor();

};


#endif  /* TOWERBOARD_H */
// ------------------- eof ---------------------
