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
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "pexception.h"
#include "alpide3rcv.h"


Alpide3rcv::Alpide3rcv() 
{
}


Alpide3rcv::Alpide3rcv(WishboneBus *wbbPtr, uint32_t baseAdd) : 
			MWbbSlave(wbbPtr, baseAdd)
{
}

Alpide3rcv::~Alpide3rcv()
{
}

//
// set register
//
void Alpide3rcv::addSetReg(uint16_t address, uint16_t val)
{
	if (!wbb)
		throw PControlInterfaceError("No IPBus configured");

	wbb->addWrite(baseAddress+address, val);
}

//
// Read register
//
void Alpide3rcv::addGetReg(uint16_t address, uint32_t *val)
{
	if (!wbb)
		throw PControlInterfaceError("No IPBus configured");

	wbb->addRead(baseAddress+address, val);
}


void Alpide3rcv::addSetInvert (bool inv)
{
	wbb->addRMWbits(baseAddress+regOpMode, ~OPMODE_INVERT_POLARITY, inv ? OPMODE_INVERT_POLARITY : 0);
        wbb->execute();
}

//
// Disable (or enable) the receiver 
//
void Alpide3rcv::addEnable(bool d)
{
  wbb->addRMWbits(baseAddress+regOpMode, ~OPMODE_RCVENABLE, d ? OPMODE_RCVENABLE : 0);
  wbb->execute();
}

//
// set RDP register
//
void Alpide3rcv::addSetRDPReg(uint16_t address, uint16_t val)
{
	if (!wbb)
		throw PControlInterfaceError("No IPBus configured");

	wbb->addWrite(baseAddress+rdpBase+address, val);
}

//
// Read RDP register
//
void Alpide3rcv::addGetRDPReg(uint16_t address, uint32_t *val)
{
	if (!wbb)
		throw PControlInterfaceError("No IPBus configured");

	wbb->addRead(baseAddress+rdpBase+address, val);
}

//
//  Read Modify Write a RDP register
//
void Alpide3rcv::addSetRDPRegField(uint16_t address, uint16_t size, uint16_t offset, uint16_t val)
{
	uint16_t mask = ((1 << (size))-1) << offset;

	val <<= offset;
	val &= mask; 

	if (!wbb)
		throw PControlInterfaceError("No IPBus configured");

	wbb->addRMWbits(baseAddress+rdpBase+address, ~mask, val);
}
