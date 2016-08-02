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

#ifndef I2CSYSPLL_H
#define I2CSYSPLL_H

#include <stdint.h>
#include "i2cbus.h"

class I2CSysPll: public I2Cbus
{
public:
    I2CSysPll(WishboneBus *wbbPtr, uint32_t baseAddress);
	void writeReg(uint8_t add, uint16_t d);
	void readReg (uint8_t add, uint16_t *d);
	void setup();

private:					// WBB Slave registers map 
/*
	enum regAddress_e {
		regWriteAdd				= 0,
		regReadAdd 				= 1,
		regParInAdd				= 2
		};

	enum writeRegBits_e {
		I2C_STOP_BIT		= (1<<31),
		I2C_START_BIT		= (1<<30),
		I2C_MASTER_ACK_BIT	= (1<<29),
		I2C_IGNORE_ACK_BIT	= (1<<28)
	};
*/

};



#endif // I2CBUS_H
