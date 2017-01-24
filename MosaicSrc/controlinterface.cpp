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
#include "pexception.h"
#include "controlinterface.h"


ControlInterface::ControlInterface() 
{
	readReqest = NULL;
	numReadRequest = 0;
}


ControlInterface::ControlInterface(WishboneBus *wbbPtr, uint32_t baseAdd) : 
			MWbbSlave(wbbPtr, baseAdd)
{
	readRequestSize = wbb->getBufferSize() / (5*4);	// every read on IPBus requires 5 word
	readReqest = new CiReadRequest [readRequestSize];	
	numReadRequest = 0;
}

void ControlInterface::setBusAddress(WishboneBus *wbbPtr, uint32_t baseAdd)
{
	// set the WBB 
	MWbbSlave::setBusAddress(wbbPtr, baseAdd);

	readRequestSize = wbbPtr->getBufferSize() / (5*4);	// every read on IPBus requires 5 word
	if (readReqest)
		delete readReqest;
	readReqest = new CiReadRequest [readRequestSize];	
	numReadRequest = 0;
}

ControlInterface::~ControlInterface()
{
	if (readReqest)
		delete readReqest;
}

//
//	set the output phase
//
void ControlInterface::setPhase(uint8_t phase)
{
	wbb->addWrite(baseAddress+regDataPhase, phase);
	wbb->execute();
}


//
// schedule a broadcast command
//
void ControlInterface::addSendCmd(uint8_t cmd)
{
	if (!wbb)
		throw PControlInterfaceError("No IPBus configured");

	wbb->addWrite(baseAddress+regWriteCtrl, cmd << 24);
// printf("Sd_com-> 0x%04x : 0x%04x\n", regWriteCtrl, cmd);
}

//
// schedule a broadcast command
//
void ControlInterface::addSendCmd(uint16_t cmd)
{
	if (!wbb)
		throw PControlInterfaceError("No IPBus configured");

	wbb->addWrite(baseAddress+regWriteCtrl, cmd << 16);
// printf("Sd_com-> 0x%04x : 0x%04x\n", regWriteCtrl, cmd);
}

//
// schedule a register write
//
void ControlInterface::addWriteReg(uint8_t chipID, uint16_t address, uint16_t data)
{
	if (!wbb)
		throw PControlInterfaceError("No IPBus configured");

	wbb->addWrite(baseAddress+regWriteData, data);
	wbb->addWrite(baseAddress+regWriteCtrl, 
					(OPCODE_WROP << 24) |
					((chipID & 0xff) << 16) |
					(address & 0xffff)
					);
// printf("wr_reg-> 0x%04x : 0x%04x : 0x%04x\n", chipID, address, data);
}



//
// schedule a register read
//
void ControlInterface::addReadReg(uint8_t chipID, uint16_t address, uint16_t *dataPtr)
{
	if (!wbb)
		throw PControlInterfaceError("No IPBus configured");

	if ( numReadRequest >= readRequestSize )
		execute();

	// queue read command
	wbb->addWrite(baseAddress+regWriteCtrl, 
					(OPCODE_RDOP << 24) |
					((chipID & 0xff) << 16) |
					(address & 0xffff)
					);
	// read answer
	wbb->addRead(baseAddress+regReadData, &readReqest[numReadRequest].IPBusReadData); 

	// Put the request into the list
	readReqest[numReadRequest].chipID = chipID;
	readReqest[numReadRequest].address = address;
	readReqest[numReadRequest].IPBusReadData = 0;
	readReqest[numReadRequest].readDataPtr = dataPtr;
	numReadRequest++;
}

void ControlInterface::execute()
{
	try {
		MWbbSlave::execute();

		// check the read results
		for (int i=0; i<numReadRequest; i++){
			uint32_t d = readReqest[i].IPBusReadData;
			uint8_t rxChipID = (d >> 16) & 0xff;
			uint8_t rxFlags  = (d >> 24) & 0x0f;

			// check the flags
			if ((rxFlags & FLAG_SYNC_BIT) == 0)
				throw PControlInterfaceError("Sync error reading data");

			if ((rxFlags & FLAG_CHIPID_BIT) == 0)
				throw PControlInterfaceError("No ChipID reading data");

			if ((rxFlags & FLAG_DATAL_BIT) == 0)
				throw PControlInterfaceError("No Data Low byte reading data");

			if ((rxFlags & FLAG_DATAH_BIT) == 0)
				throw PControlInterfaceError("No Data High byte reading data");

			// check the sender
			if (rxChipID!=readReqest[i].chipID)
				throw PControlInterfaceError("ChipID mismatch");

			*readReqest[i].readDataPtr = (d & 0xffff);
		}
		numReadRequest = 0;
	} catch (...) {
		numReadRequest = 0;
		throw;
	}
}



