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
#include "mexception.h"
#include "pulser.h"


Pulser::Pulser() 
{
}


Pulser::Pulser(WishboneBus *wbbPtr, uint32_t baseAdd) : 
			MWbbSlave(wbbPtr, baseAdd)
{
}

Pulser::~Pulser()
{
}

//
// setup the pulser registers
//
void Pulser::setConfig(uint32_t triggerDelay, uint32_t pulseDelay, uint32_t opMode)
{
	if (!wbb)
		throw MIPBusUDPError("No IPBus configured");

	wbb->addWrite(baseAddress+regTriggerDelay, triggerDelay);
	wbb->addWrite(baseAddress+regPulseDelay, pulseDelay);
	wbb->addWrite(baseAddress+regOpMode, opMode);
	execute();
}

//
// Read the pulser registers
//
void Pulser::getConfig(uint32_t *triggerDelay, uint32_t *pulseDelay, uint32_t *opMode)
{
	if (!wbb)
		throw MIPBusUDPError("No IPBus configured");

	wbb->addRead(baseAddress+regTriggerDelay, triggerDelay);
	wbb->addRead(baseAddress+regPulseDelay, pulseDelay);
	wbb->addRead(baseAddress+regOpMode, opMode);
	execute();
}

//
// generate N pulses. If N==0 stop the generator
//
void Pulser::run(uint32_t numPulses)
{
	if (!wbb)
		throw MIPBusUDPError("No IPBus configured");

	wbb->addWrite(baseAddress+regNumPulses, numPulses);
	execute();
}

//
//	readout the number of pulsed to be generated
//
void Pulser::getStatus(uint32_t *numPulses)
{
	if (!wbb)
		throw MIPBusUDPError("No IPBus configured");

	wbb->addRead(baseAddress+regStatus, numPulses);
	execute();
}


