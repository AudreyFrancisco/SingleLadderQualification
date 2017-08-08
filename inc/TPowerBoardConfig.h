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
 *		TPowerBoardConfig class header
 *
 *		ver.1.0		12/07/2017
 *
 *
 *  		HISTORY
 *
 *
 */
#ifndef POWERBOARDCONFIG_H
#define POWERBOARDCONFIG_H


#include <stdio.h>
#include "TReadoutBoard.h"

// The maximum number of modules of the power board
// that the MOSAIC board can drive.
#define MAX_MOULESPERMOSAIC 8


// Default values table

// a) Default values used in constructor
#define DEF_BIASVOLTAGE	0.0
#define DEF_ANALOGVOLTAGE	1.80
#define DEF_ANALOGMAXCURRENT	1.00
#define DEF_DIGITALVOLTAGE	1.80
#define DEF_DIGITALMAXCURRENT	1.00
#define DEF_BIASCHANNELON	false

// b) Setup-specific default values - to be refined
// (can be set by setter methods)
#define DEF_BIASVOLTAGE_OB	 0.0
#define DEF_ANALOGVOLTAGE_OB	 1.80
#define DEF_ANALOGMAXCURRENT_OB	 0.3
#define DEF_DIGITALVOLTAGE_OB	 1.80
#define DEF_DIGITALMAXCURRENT_OB 1.50
#define DEF_BIASCHANNELON_OB	 false

#define DEF_BIASVOLTAGE_IB	 0.0
#define DEF_ANALOGVOLTAGE_IB	 1.80
#define DEF_ANALOGMAXCURRENT_IB	 0.3
#define DEF_DIGITALVOLTAGE_IB	 1.80
#define DEF_DIGITALMAXCURRENT_IB 1.00
#define DEF_BIASCHANNELON_IB	 false


// Class definition
class TPowerBoardConfig  {

// structures a data types
public:
	typedef struct Mod {
		bool BiasOn;
		float AVset;
		float AIset;
		float DVset;
		float DIset;
	} Mod_t;

	typedef struct PowBoard {
		Mod_t Modul[MAX_MOULESPERMOSAIC];
		float VBset;
	} PowBoard_t;

// members
private:
	FILE *fhConfigFile; // the file handle of the Configuration File
	PowBoard_t	fPBConfig;
	TBoardType	fBoardType;


// methods
public:
	TPowerBoardConfig(const char *AConfigFileName);

	// Info

	// Getters
	float GetBiasVoltage() { return( fPBConfig.VBset); };

	float GetAnalogVoltage(int mod) { return(fPBConfig.Modul[mod].AVset); };
	float GetAnalogCurrent(int mod) { return(fPBConfig.Modul[mod].AIset); };
	float GetDigitalVoltage(int mod) { return(fPBConfig.Modul[mod].DVset); };
	float GetDigitalCurrent(int mod) { return(fPBConfig.Modul[mod].DIset); };
	bool GetBiasOn(int mod) { return(fPBConfig.Modul[mod].BiasOn); };

	void GetModuleSetUp(int mod, float*AVSet, float*AISet, float*DVSet, float*DISet, bool*isBiasOn);
	void GetAnalogVoltages(float * AVSet);
	void GetDigitalVoltages(float * DVSet);
	void GetAnalogCurrents(float * AISet);
	void GetDigitalCurrents(float * DISet);
	void GetBiasOnSets(bool * BIASOn);

	// Setters
	void SetBiasVoltage(float val) { fPBConfig.VBset = val; };

	void ModuleSetUp(int mod, float AVSet, float AISet, float DVSet, float DISet, bool isBiasOn);
	void SetAnalogVoltage(int mod, float val) { fPBConfig.Modul[mod].AVset = val; };
	void SetAnalogCurrent(int mod, float val) { fPBConfig.Modul[mod].AIset = val; };
	void SetDigitalVoltage(int mod, float val) { fPBConfig.Modul[mod].DVset = val; };
	void SetDigitalCurrent(int mod, float val) { fPBConfig.Modul[mod].DIset = val; };
	void SetBiasOn(int mod, bool val) { fPBConfig.Modul[mod].BiasOn = val; };

        void SetDefaultsOB(int mod);
        void SetDefaultsIB(int mod);
	// Utilities
	bool ReadFromFile(char * AFileName);
	bool WriteToFile(char *AFileName);
	bool DumpConfig() { return false; }; // TODO: not yet implemented

private:
    void readConfiguration();


};

#endif   /* BOARDCONFIGMOSAIC_H */

// ---------------- eof -------------------------
