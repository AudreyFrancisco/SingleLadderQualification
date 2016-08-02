/*
 * Copyright (C) 2014
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2014.
 *
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "mexception.h"
#include "i2csyspll.h"

#define CDCM6208_ADDRESS	0x54

/* System PLL register setup for:
		Primary Input Frequency: 200
		Secondary Input Frequency: 40
		Version 2
		C1: 18.0849p
		R2: 1.003k
		C2: 9.9706n
		R3: 60
		C3: 377.5p
		Charge Pump: 2.5m
		Input MUX set to Primary
		All inputs/Outputs LVDS 3.3V/2.5V
*/

uint16_t regContent [] = {
	/* Register 0: */ 0x02A9,
	/* Register 1: */ 0x0000,
	/* Register 2: */ 0x000E,
	/* Register 3: */ 0x08F5,
	/* Register 4: */ 0x346F,	// set to 0x346f to set reference clock from secondary input (0x246f for primary)
	/* Register 5: */ 0x0023,
	/* Register 6: */ 0x0002,
	/* Register 7: */ 0x0023,
	/* Register 8: */ 0x0002,
	/* Register 9: */ 0x0003,
	/* Register 10: */ 0x0020,
	/* Register 11: */ 0x0000,
	/* Register 12: */ 0x0003,  // 0x0003, 0x2003 bypass Y5 from primary input
	/* Register 13: */ 0x0020,
	/* Register 14: */ 0x0000,
	/* Register 15: */ 0x0003,
	/* Register 16: */ 0x0020,
	/* Register 17: */ 0x0000,
	/* Register 18: */ 0x0003,
	/* Register 19: */ 0x0020,
	/* Register 20: */ 0x0000,
	/* Register 21: */ 0x0006			// RO register
	};



I2CSysPll::I2CSysPll(WishboneBus *wbbPtr, uint32_t baseAdd) : 
			I2Cbus(wbbPtr, baseAdd)
{
}

void I2CSysPll::writeReg(uint8_t add, uint16_t d)
{
	addAddress(CDCM6208_ADDRESS, I2Cbus::I2C_Write);
	addWriteData(0x00);
	addWriteData(add);
	addWriteData(d>>8);
	addWriteData(d&0xff, I2Cbus::RWF_stop);
	execute();
}

void I2CSysPll::readReg (uint8_t add, uint16_t *d)
{
	uint32_t *r = new uint32_t[2];

	addAddress(CDCM6208_ADDRESS, I2Cbus::I2C_Write);
	addWriteData(0x00);
	addWriteData(add, I2Cbus::RWF_stop);

	addAddress(CDCM6208_ADDRESS, I2Cbus::I2C_Read);
	addRead(r);
	addRead(r+1, I2Cbus::RWF_dontAck | I2Cbus::RWF_stop);
	execute();

	*d = ((r[0]&0xff) << 8) | (r[1]&0xff);
}


void I2CSysPll::setup()
{
	uint16_t r;
	int lookTry;
	
	// Write
	for (int i=0; i<20; i++){
		writeReg(i, regContent[i]);
	}

	// Verify
	for (int i=0; i<20; i++){
		readReg(i, &r);
		if (r != regContent[i])
			throw MBoardInitError("System PLL verify error");
	}

	// Cycle the reset 
	writeReg(3, regContent[3] & ~(1<<6));
	writeReg(3, regContent[3]);

	// wait for PLL to lock
	lookTry = 500;
	while (--lookTry){
		readReg(21, &r);
		if ((r&0x0004) == 0)
			break;
	}

	if (lookTry==0)
			throw MBoardInitError("System PLL NOT locked!");
		
}



