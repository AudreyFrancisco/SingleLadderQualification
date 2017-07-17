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
 *		TPowerBoardConfig class implementation
 *
 *		ver.1.0		12/07/2017
 *
 *
 *  		HISTORY
 *
 *
 */
#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstring>

#include "TPowerBoardConfig.h"

using namespace std;

/* -------------------------
	Constructor

	Parameter : AConfigFileName := Path and filename of a configuration ASCII file

  -------------------------- */
TPowerBoardConfig::TPowerBoardConfig(const char *AConfigFileName)
{
	fBoardType = boardMOSAIC;

	// Default values set
	fPBConfig.VBset = DEF_BIASVOLTAGE;
	for(int i=0;i<MAX_MOULESPERMOSAIC;i++) {
		fPBConfig.Modul[i].AVset = DEF_ANALOGVOLTAGE;
		fPBConfig.Modul[i].AIset = DEF_ANALOGMAXCURRENT;
		fPBConfig.Modul[i].DVset = DEF_DIGITALVOLTAGE;
		fPBConfig.Modul[i].DIset = DEF_DIGITALMAXCURRENT;
		fPBConfig.Modul[i].BiasOn = DEF_BIASCHANNELON;
	}

	if (AConfigFileName) { // Read Configuration file
		try {
			if(AConfigFileName == NULL || strlen(AConfigFileName) == 0) throw std::invalid_argument("MOSAIC Config : invalid filename");
			fhConfigFile = fopen(AConfigFileName,"r"); // opens the file
			} catch (...) {
				throw std::invalid_argument("Power Board Config : file not exists !");
			}
			readConfiguration();
			fclose(fhConfigFile);
	}
}

/* -------------------------
	readConfiguration()
	Read the power board configuration from an ASCII file.

	Format : ASCII file with '\n' line termination

	# .....   := Comment line
	<PARAMETER_NAME>\t<VALUE>  := Single value parameter
	<PARAMETER_NAME>\t<VALUE_LIST> := Multi value parameters (for each board module)

	<VALUE_LIST> := <VALUE>[<DELIMITER><VALUE>..]
	<DELIMIDER> := [ ',' | ";" | ":" | "\t' | " "]

	List of Parameters:
	BIASVOLTAGE := single value, float. The voltage value of the Back Bias Output
	ANALOGVOLTAGE := multi value, float. The voltage of analogic source for the 8 modules
	ANALOGCURRENT := multi value, float. The maximum current of analogic source for the 8 modules
	DIGITALVOLTAGE := multi value, float. The voltage of digital source for the 8 modules
	DIGITALCURRENT := multi value, float. The maximum current of digital source for the 8 modules
	BIASON := multi value, bool ("TRUE" | "FALSE"). The switch ON/OFF of the Back Bias line for the 8 modules


  -------------------------- */
void TPowerBoardConfig::readConfiguration()
{
	char buffer[4096];
	char sName[4096];
	char sParam[4096];
	char *tok, *ptr;
	int p;


	fgets(buffer, 4095, fhConfigFile);
	while(!feof(fhConfigFile)){
		if(strlen(buffer) > 0 && buffer[0] != '#') { //a good line
			fscanf(fhConfigFile, "%s\t%s", sName, sParam);
			if(strcasecmp(sName,"BIASVOLTAGE") == 0) {
				fPBConfig.VBset = atof(sParam);
			}
			if(strcasecmp(sName,"ANALOGVOLTAGE") == 0) {
				p =0; ptr = sParam; while ((tok = strsep(&ptr, " ,:;\t")) != NULL) fPBConfig.Modul[p++].AVset = atof(tok);
				while(p<MAX_MOULESPERMOSAIC && p>0) {fPBConfig.Modul[p].AVset = fPBConfig.Modul[p-1].AVset; p++;}
			}
			if(strcasecmp(sName,"ANALOGCURRENT") == 0) {
				p =0; ptr = sParam; while ((tok = strsep(&ptr, " ,:;\t")) != NULL) fPBConfig.Modul[p++].AIset = atof(tok);
				while(p<MAX_MOULESPERMOSAIC && p>0) {fPBConfig.Modul[p].AIset = fPBConfig.Modul[p-1].AIset; p++;}
			}
			if(strcasecmp(sName,"DIGITALVOLTAGE") == 0) {
				p =0; ptr = sParam; while ((tok = strsep(&ptr, " ,:;\t")) != NULL) fPBConfig.Modul[p++].DVset = atof(tok);
				while(p<MAX_MOULESPERMOSAIC && p>0) {fPBConfig.Modul[p].DVset = fPBConfig.Modul[p-1].DVset; p++;}
			}
			if(strcasecmp(sName,"DIGITALCURRENT") == 0) {
				p =0; ptr = sParam; while ((tok = strsep(&ptr, " ,:;\t")) != NULL) fPBConfig.Modul[p++].DIset = atof(tok);
				while(p<MAX_MOULESPERMOSAIC && p>0) {fPBConfig.Modul[p].DIset = fPBConfig.Modul[p-1].DIset; p++;}
			}
			if(strcasecmp(sName,"BIASON") == 0) {
				p =0; ptr = sParam; while ((tok = strsep(&ptr, " ,:;\t")) != NULL) fPBConfig.Modul[p++].BiasOn = (strcasecmp(tok,"TRUE") == 0 ? true: false);
				while(p<MAX_MOULESPERMOSAIC && p>0) {fPBConfig.Modul[p].BiasOn = fPBConfig.Modul[p-1].BiasOn; p++;}
			}
		}
		fgets(buffer, 4095, fhConfigFile);
	}

}
/* -------------------------
	GetModuleSetUp()
	Returns the settings for one specified module.

	Parameter In : mod, integer (0..7) The number of module to be read
	Paramer Out : AVSet, pointer to a float variable. Analogic source output voltage
				AISet, pointer to a float variable. Analogic source output current limit
				DVSet, pointer to a float variable. Digital source output voltage
				DISet, pointer to a float variable. Digital source output current limit
				isBiasOn, pointer to a bool variable. On/Off of the Back Bias channel

  -------------------------- */
void TPowerBoardConfig::GetModuleSetUp(int mod, float*AVSet, float*AISet, float*DVSet, float*DISet, bool*isBiasOn) {
	*AVSet = fPBConfig.Modul[mod].AVset;
	*AISet = fPBConfig.Modul[mod].AIset;
	*DVSet = fPBConfig.Modul[mod].DVset;
	*DISet = fPBConfig.Modul[mod].DIset;
	*isBiasOn = fPBConfig.Modul[mod].BiasOn;
	return;
}

/* -------------------------
	ModuleSetUp()
	Setup for one specified module.

	Parameter In : mod, integer (0..7) The number of module to be read
	 	 	 	AVSet, float. Analogic source output voltage
				AISet, float. Analogic source output current limit
				DVSet, float. Digital source output voltage
				DISet, float. Digital source output current limit
				isBiasOn, bool. On/Off of the Back Bias channel

  -------------------------- */
void TPowerBoardConfig::ModuleSetUp(int mod, float AVSet, float AISet, float DVSet, float DISet, bool isBiasOn) {
	fPBConfig.Modul[mod].AVset = AVSet;
	fPBConfig.Modul[mod].AIset = AISet;
	fPBConfig.Modul[mod].DVset = DVSet;
	fPBConfig.Modul[mod].DIset = DISet;
	fPBConfig.Modul[mod].BiasOn = isBiasOn;
	return;
}

/* -------------------------
	GetAnalogVoltages()
	Returns the array of Analogic Voltage setting for all 8 modules

	Parameter Output : pointer to a float array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetAnalogVoltages(float * AVSet) {
	for(int i=0;i<MAX_MOULESPERMOSAIC;i++) {
		*(AVSet++) = fPBConfig.Modul[i].AVset;
	}
	return;
}

/* -------------------------
	GetDigitalVoltages()
	Returns the array of Digital Voltage setting for all 8 modules

	Parameter Output : pointer to a float array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetDigitalVoltages(float * DVSet) {
	for(int i=0;i<MAX_MOULESPERMOSAIC;i++) {
		*(DVSet++) = fPBConfig.Modul[i].DVset;
	}
	return;
}

/* -------------------------
	GetAnalogCurrents()
	Returns the array of Analogic Current limits setting for all 8 modules

	Parameter Output : pointer to a float array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetAnalogCurrents(float * AISet) {
	for(int i=0;i<MAX_MOULESPERMOSAIC;i++) {
		*(AISet++) = fPBConfig.Modul[i].AIset;
	}
	return;
}

/* -------------------------
	GetDigitalCurrents()
	Returns the array of Digital Current limits setting for all 8 modules

	Parameter Output : pointer to a float array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetDigitalCurrents(float * DISet) {
	for(int i=0;i<MAX_MOULESPERMOSAIC;i++) {
		*(DISet++) = fPBConfig.Modul[i].AVset;
	}
	return;
}

/* -------------------------
	GetBiasOnSets()
	Returns the array of Back Bias On/Off setting for all 8 modules

	Parameter Output : pointer to a bool array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetBiasOnSets(bool * BIASOn) {
	for(int i=0;i<MAX_MOULESPERMOSAIC;i++) {
		*(BIASOn++) = fPBConfig.Modul[i].BiasOn;
	}
	return;
}

/* -------------------------
	ReadFromFile()
	Read a complete configuration from a file and stores it into the class members

	Parameter input : char pointer to a string that specify path and filename of the Configuration file

	Return : false in case of error

  -------------------------- */
bool TPowerBoardConfig::ReadFromFile(char *AFileName)
{
	if(AFileName == NULL || strlen(AFileName) == 0) {
		fhConfigFile = fopen(AFileName,"r"); // opens the file
		if(fhConfigFile != NULL) {
			readConfiguration();
			fclose(fhConfigFile);
			return(true);
		}
	}
	return(false);
}

/* -------------------------
	WriteToFile()
	Write a complete configuration from the class members to a configuration file

	Parameter input : char pointer to a string that specify path and filename of the Configuration file

	Return : false in case of error

  -------------------------- */
bool TPowerBoardConfig::WriteToFile(char *AFileName)
{
	int i;
	if(AFileName == NULL || strlen(AFileName) == 0) {
		fhConfigFile = fopen(AFileName,"w"); // opens the file
		if(fhConfigFile != NULL) {
			fprintf(fhConfigFile,"# ALPIDE POWER BOARD CONFIGURATION  - v0.1\n");
			fprintf(fhConfigFile,"BIASVOLTAGE\t%f\n", fPBConfig.VBset);
			fprintf(fhConfigFile,"# PARAM.\tM0\tM1\tM2\tM3\tM4\tM5\tM6\tM7");
			fprintf(fhConfigFile,"\nANALOGVOLTAGE"); for(i=0;i<MAX_MOULESPERMOSAIC;i++) fprintf(fhConfigFile,"\t%f",fPBConfig.Modul[i].AVset );
			fprintf(fhConfigFile,"\nANALOGCURRENT"); for(i=0;i<MAX_MOULESPERMOSAIC;i++) fprintf(fhConfigFile,"\t%f",fPBConfig.Modul[i].AIset );
			fprintf(fhConfigFile,"\nDIGITALVOLTAGE"); for(i=0;i<MAX_MOULESPERMOSAIC;i++) fprintf(fhConfigFile,"\t%f",fPBConfig.Modul[i].DVset );
			fprintf(fhConfigFile,"\nDIGITALCURRENT"); for(i=0;i<MAX_MOULESPERMOSAIC;i++) fprintf(fhConfigFile,"\t%f",fPBConfig.Modul[i].DIset );
			fprintf(fhConfigFile,"\nBIASON"); for(i=0;i<MAX_MOULESPERMOSAIC;i++) fprintf(fhConfigFile,"\t%s",(fPBConfig.Modul[i].BiasOn ? "TRUE":"FALSE") );
			fprintf(fhConfigFile,"\n# --- eof ----\n");
			fclose(fhConfigFile);
			return(true);
		}
	}
	return(false);
}


// ------------------  eof -----------------------
