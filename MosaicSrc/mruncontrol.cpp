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
#include "mruncontrol.h"


MRunControl::MRunControl(WishboneBus *wbbPtr, uint32_t baseAdd): 
			MWbbSlave(wbbPtr, baseAdd)
{
}


void MRunControl::getErrors(uint32_t *errors, bool clear)
{
	wbb->addRead(baseAddress+regErrorState, errors);	// READ error register
	if (clear)
		wbb->addWrite(baseAddress+regErrorState, 0);		// clear it
	wbb->execute();
}

void MRunControl::clearErrors()
{
	wbb->addWrite(baseAddress+regErrorState, 0);
	wbb->execute();
}

void MRunControl::setConfigReg(uint32_t d)
{
	wbb->addWrite(baseAddress+regConfig, d);
	wbb->execute();
}

void MRunControl::getConfigReg(uint32_t *d)
{
	wbb->addRead(baseAddress+regConfig, d);
	wbb->execute();
}

void MRunControl::setAFThreshold(uint32_t d)
{
	wbb->addWrite(baseAddress+regAlmostFullThreshold, d);
	wbb->execute();
}

void MRunControl::getAFThreshold(uint32_t *d)
{
	wbb->addRead(baseAddress+regAlmostFullThreshold, d);
	wbb->execute();
}

void MRunControl::setLatency(uint8_t mode, uint32_t timeout)
{
	uint32_t m = mode;

	wbb->addWrite(baseAddress+regLatency, ((m&0x03)<<30) | (timeout&0x00ffffff));
	wbb->execute();
}

void MRunControl::getLatency(uint8_t *mode, uint32_t *timeout)
{
	uint32_t d;

	wbb->addRead(baseAddress+regLatency, &d);
	wbb->execute();

	*mode = d >> 30;
	*timeout = d & 0x00ffffff;
}

void MRunControl::getStatus(uint32_t *st)
{
	wbb->addRead(baseAddress+regStatus, st);
	wbb->execute();
}


void MRunControl::setSpeed(Mosaic::TReceiverSpeed ASpeed) 
{
	int regSet = 0;

	switch (ASpeed){
	case Mosaic::RCV_RATE_400:
			regSet = CFG_RATE_400;
			break;

	case Mosaic::RCV_RATE_600:
			regSet = CFG_RATE_600;
			break;

	case Mosaic::RCV_RATE_1200:
			regSet = CFG_RATE_1200;
			break;
	}
	std::cout << "Writing " << std::hex << regSet << std::dec << " to config register" << std::endl;
	wbb->addRMWbits(baseAddress+regConfig, ~CFG_RATE_MASK, regSet);
	wbb->execute();

}


void MRunControl::startRun()
{
	wbb->addWrite(baseAddress+regRunCtrl, RUN_CTRL_RUN);
	wbb->execute();
}

void MRunControl::stopRun()
{
	wbb->addWrite(baseAddress+regRunCtrl, 0);
	wbb->execute();
}








