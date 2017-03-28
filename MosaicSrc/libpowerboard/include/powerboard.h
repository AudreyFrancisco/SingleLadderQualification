/*
 * Copyright (C) 2015
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2015.
 *
 */

#ifndef POWERBOARD_H
#define POWERBOARD_H

#include <stdint.h>
#include "i2cbus.h"
#include "pcf8574.h"
#include "ltc2635.h"
#include "ad5254.h"
#include "ad7997.h"
#include "sc18is602.h"
#include "max31865.h"

#define NUM_TSENSOR 3

class powerboard
{
public:
	typedef struct pbstate {
		float Vmon[8];
		float Imon[8];
		int	  chOn;
		float Vout[8];
		float T[NUM_TSENSOR];
	} pbstate_t; 

	enum getFlags {
		GetMonitor	= 0x01,
		GetSettings	= 0x02,
		WaitTconv	= 0x04,
		getAll		= 0x07
	};

public:
	powerboard(I2Cbus *bus, bool master);
	~powerboard();
	bool isReady();
	void setIth(uint8_t ch, float value);
	void setVbias(float value);
	void enVbias(bool en);
	void setVout(uint8_t ch, float value);
	void storeVout(uint8_t ch);
	void storeAllVout();
	void restoreAllVout();
	void onVout(uint8_t ch);
	void onAllVout();

	void getState(pbstate_t *state, getFlags flags=getAll);


protected:
	I2Cbus *i2cBus;	

private:
	PCF8574				*regLuVbuf_18;
	PCF8574				*regCtrl;
	LTC2635				*dacBias;
	LTC2635				*dacThreshold14;
	LTC2635				*dacThreshold58;
	AD5254				*rdacVadj14;
	AD5254				*rdacVadj58;
	AD7997				*adcMon14;
	AD7997				*adcMon58;
	SC18IS602			*spiBridge;
	MAX31865			*temperatureDetector[3];

private:
	typedef struct i2c_baseAddress_s {
		uint8_t dacBias;
		uint8_t dacThreshold_14;
		uint8_t dacThreshold_58;
		uint8_t rdacVadj_14;
		uint8_t rdacVadj_58;
		uint8_t regLuVbuf_18;
		uint8_t regCtrl;
		uint8_t adcMon_14;
		uint8_t adcMon_58;
		uint8_t spiBridge;
	} i2c_baseAddress_t;
	static i2c_baseAddress_t i2c_baseAddress[2];

	enum regCtrlBits{
		CTRL_ResetBar			= 0x01,		// output
		CTRL_AllertBusy1		= 0x02,		// input
		CTRL_AllertBusy2		= 0x04,		// input
		CTRL_BiasShdn			= 0x08,		// output
		CTRL_AllInputs			= 0x06
	};
};



#endif // POWERBOARD_H
