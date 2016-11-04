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

#ifndef ALPIDE3RCV_H
#define ALPIDE3RCV_H

#include <stdint.h>
#include "mwbbslave.h"


namespace Mosaic {
  typedef enum rcvRate_e {			// Receiver data rate (in Mbps)
    RCV_RATE_400,
    RCV_RATE_600,
    RCV_RATE_1200
  }  TReceiverSpeed;
}


class Alpide3rcv: public MWbbSlave
{
public:
    Alpide3rcv();
    Alpide3rcv(WishboneBus *wbbPtr, uint32_t baseAddress);
	~Alpide3rcv();
	void setBusAddress(WishboneBus *wbbPtr, uint32_t baseAddress);
	void addSetReg(uint16_t address, uint16_t val);
	void addGetReg(uint16_t address, uint32_t *val);
	void addSetRcvSpeed(Mosaic::TReceiverSpeed sp);
	void addEnable(bool d);
        void addSetInvert(bool inv);
	void addSetRDPReg(uint16_t address, uint16_t val);
	void addGetRDPReg(uint16_t address, uint32_t *val);
	void addSetRDPRegField(uint16_t address, uint16_t size, uint16_t offset, uint16_t val);


private:					// WBB Slave registers map 
	enum regAddress_e {
		regOpMode		= 0,
		rdpBase			= 0x00800000
		};

public:
	enum opModeBits_e {
		OPMODE_RCVENABLE			= (1<<0),
		OPMODE_RATE_MASK			= (0x03<<1),
		OPMODE_RATE_1200			= (0<<1),
		OPMODE_RATE_600				= (0x01<<1),
		OPMODE_RATE_400				= (0x02<<1),
		OPMODE_INVERT_POLARITY		= (1<<3)	};

};



#endif // ALPIDE3RCV_H
