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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>
 *
 *
 *
 *  		HISTORY
 *  3/8/16	- Add the Board decoder class ...
 *  5/8/16  - adapt the read event to new definition
 *  18/01/17 - Review of ReadEventData. Added inheritance from class MBoard
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include "TReadoutBoardMOSAIC.h"
#include "BoardDecoder.h"
#include "AlpideDecoder.h"
#include "TAlpide.h"
#include "SetupHelpers.h"
#include "MosaicSrc/mexception.h"

using namespace std;
std::vector<unsigned char> fDebugBuffer;

// ---- Constructor
TReadoutBoardMOSAIC::TReadoutBoardMOSAIC (TConfig* config, TBoardConfigMOSAIC *boardConfig)
  : TReadoutBoard(boardConfig)
  , fConfig(config)
{
  init();
}



// Distructor
TReadoutBoardMOSAIC::~TReadoutBoardMOSAIC()
{
	delete dr;
	delete pulser;
	for (int i=0; i<MAX_MOSAICTRANRECV; i++)
		delete alpideDataParser[i];

	for(int i=0; i<MAX_MOSAICCTRLINT; i++)
		delete controlInterface[i];

	delete i2cBus;
	delete dataGenerator;
}



/* -------------------------
	Public methods
  -------------------------- */

// Read/Write registers
int TReadoutBoardMOSAIC::WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId)
{
	uint_fast16_t Cii = GetControlInterface(chipId);
	controlInterface[Cii]->addWriteReg(chipId, address, value);
	controlInterface[Cii]->execute();
	return(0);
}

int TReadoutBoardMOSAIC::ReadChipRegister (uint16_t address, uint16_t &value, uint8_t chipId)
{
	uint_fast16_t Cii = GetControlInterface(chipId);
	controlInterface[Cii]->addReadReg( chipId,  address,  &value);
	controlInterface[Cii]->execute();
	return(0);
}

int TReadoutBoardMOSAIC::SendOpCode (uint16_t  OpCode, uint8_t chipId)
{
	uint_fast16_t Cii = GetControlInterface(chipId);
	controlInterface[Cii]->addWriteReg(chipId, Alpide::REG_COMMAND, OpCode);
	controlInterface[Cii]->execute();
	return(0);
}

int TReadoutBoardMOSAIC::SendOpCode (uint16_t  OpCode)
{
	uint8_t ShortOpCode = (uint8_t)OpCode;
	for(int Cii=0;Cii<MAX_MOSAICCTRLINT;Cii++){
		controlInterface[Cii]->addSendCmd(ShortOpCode);
		controlInterface[Cii]->execute();
	}
	return(0);
}

int TReadoutBoardMOSAIC::SetTriggerConfig (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay)
{
	uint16_t pulseMode = 0;

	if(enablePulse)
		pulseMode |= Pulser::OPMODE_ENPLS_BIT;
	if(enableTrigger)
		pulseMode |= Pulser::OPMODE_ENTRG_BIT;
	pulser->setConfig(triggerDelay, pulseDelay, pulseMode);
	return(pulseMode);
}

void TReadoutBoardMOSAIC::SetTriggerSource (TTriggerSource triggerSource)
{
	if(triggerSource == trigInt) {
		// Internal Trigger
		mTriggerControl->addEnableExtTrigger(false, 0);
	} else {
		// external trigger
		mTriggerControl->addEnableExtTrigger(true, 0);
	}
	mTriggerControl->execute();
}

int  TReadoutBoardMOSAIC::Trigger (int nTriggers)
{
	pulser->run(nTriggers);
	return(nTriggers);
}

void TReadoutBoardMOSAIC::StartRun()
{
	enableDefinedReceivers();
	connectTCP(); // open TCP connection
	mRunControl->startRun(); // start run
	usleep(5000);
}

void TReadoutBoardMOSAIC::StopRun()
{
	 pulser->run(0);
	 mRunControl->stopRun();
	 closeTCP();  // FIXME: this could cause the lost of the tail of the buffer ...
}

int  TReadoutBoardMOSAIC::ReadEventData (int &nBytes, unsigned char *buffer)
{
	TAlpideDataParser *dr;
	long readDataSize;

	// check for data in the receivers buffer
	for (int i=0; i<MAX_MOSAICTRANRECV; i++){
		if (alpideDataParser[i]->hasData())
			return (alpideDataParser[i]->ReadEventData(nBytes, buffer));
	}

	// try to read from TCP connection
	for (;;){
		try {
			readDataSize = pollTCP(fBoardConfig->GetPollingDataTimeout(), (MDataReceiver **) &dr);
			if (readDataSize == 0)
				return -1;
		} catch (exception& e) {
			cerr << e.what() << endl;
			StopRun();
			decodeError();
			exit(1);
		}

		// get event data from the selected data receiver
		if (dr->hasData())
			return (dr->ReadEventData(nBytes, buffer));
	}
	return -1;
}


/* -------------------------
 * 		Private Methods
   ------------------------- */
// Private : Init the board
void TReadoutBoardMOSAIC::init()
{
	setIPaddress(fBoardConfig->GetIPaddress(), fBoardConfig->GetTCPport());

	// Data Generator
	dataGenerator = new MDataGenerator(mIPbus, WbbBaseAddress::dataGenerator);

	// I2C master (WBB slave) and connected peripherals
	i2cBus = new I2Cbus(mIPbus, WbbBaseAddress::i2cMaster);

	// System PLL on I2C bus
	mSysPLL = new I2CSysPll(mIPbus, WbbBaseAddress::i2cSysPLL);

	// CMU Control interface
	controlInterface[0] = new ControlInterface(mIPbus, WbbBaseAddress::controlInterface);
	controlInterface[1] = new ControlInterface(mIPbus, WbbBaseAddress::controlInterfaceB);

	// Pulser
	pulser = new Pulser(mIPbus, WbbBaseAddress::pulser);

	// ALPIDE Hi Speed data receiver
	for (int i=0; i<MAX_MOSAICTRANRECV; i++){
		alpideRcv[i] = new ALPIDErcv(mIPbus, WbbBaseAddress::alpideRcv+(i<<24));
		alpideRcv[i]->addEnable(false);
		alpideRcv[i]->addInvertInput(false);
	}

	// The data consumer for hardware generators
	dr = new DummyReceiver;
	addDataReceiver(0, dr);

	for (int i=0; i<MAX_MOSAICTRANRECV; i++){
		alpideDataParser[i] = new TAlpideDataParser();
		addDataReceiver(i+1, alpideDataParser[i]);
	}

	// ----- Now do the initilization -------
	// Initialize the System PLL
	mSysPLL->setup();

	// wait for board to be ready
	uint32_t boardStatusReady = (BOARD_STATUS_GTP_RESET_DONE | \
								BOARD_STATUS_GTPLL_LOCK | \
								BOARD_STATUS_EXTPLL_LOCK | \
								BOARD_STATUS_FEPLL_LOCK );

	// wait 1s for transceivers reset done
	long int init_try;
	for (init_try=1000; init_try>0; init_try--){
		uint32_t st;
		usleep(1000);
		mRunControl->getStatus(&st);
		if ((st & boardStatusReady) == boardStatusReady)
			break;
	}
	if (init_try==0)
		throw MBoardInitError("Timeout setting MOSAIC system PLL");

    for(int i=0;i<MAX_MOSAICCTRLINT;i++)
		setPhase(fBoardConfig->GetCtrlInterfacePhase(),i);  // set the Phase shift on the line

	setSpeedMode (fBoardConfig->GetSpeedMode());// set 400 MHz mode
	setInverted  (fBoardConfig->IsInverted(),-1);

	pulser->run(0);
	mRunControl->stopRun();
	mRunControl->clearErrors();
	mRunControl->setAFThreshold(fBoardConfig->GetCtrlAFThreshold());
	mRunControl->setLatency(fBoardConfig->GetCtrlLatMode(), fBoardConfig->GetCtrlLatMode());

	return;
}

// ============================== DATA receivers private methods =======================================

void TReadoutBoardMOSAIC::enableDefinedReceivers()
{
  bool Used[MAX_MOSAICTRANRECV];
  for (int i = 0; i < MAX_MOSAICTRANRECV; i++) {
    Used[i] = false;
  }

  int dataLink;
  for(int i=0;i< fChipPositions.size(); i++) { //for each defined chip
    dataLink = fChipPositions.at(i).receiver;
    if(dataLink >= 0) { // Enable the data receiver
      if (fConfig->GetChipConfigById(fChipPositions.at(i).chipId)->IsEnabled()) {
        std::cout << "!!!!!! ENabling receiver " << dataLink << std::endl;
        alpideRcv[dataLink]->addEnable(true);
        Used[dataLink] = true;
        //alpideRcv[dataLink]->execute();
      }
      else if (!Used[dataLink]){
        std::cout << "!!!!!! DISabling receiver " << dataLink << std::endl;
        alpideRcv[dataLink]->addEnable(false);
      }
    }
  }
  return;
}

void TReadoutBoardMOSAIC::setSpeedMode(Mosaic::TReceiverSpeed ASpeed, int Aindex)
{
  mRunControl->setSpeed (ASpeed);
}


void TReadoutBoardMOSAIC::setInverted(bool AInverted, int Aindex)
{
	int st,en;
	Aindex = -1;
	st = (Aindex != -1) ? Aindex : 0;
	en = (Aindex != -1) ? Aindex+1 : MAX_MOSAICTRANRECV;
	for(int i=st;i<en;i++) {
		alpideRcv[i]->addInvertInput(AInverted);
		alpideRcv[i]->execute();
	}
	return;
}


// Decode the Mosaic Error register
uint32_t TReadoutBoardMOSAIC::decodeError()
{
	uint32_t runErrors;
	mRunControl->getErrors(&runErrors);
	if (runErrors){
		std::cout << "MOSAIC Error register: 0x" << std::hex << runErrors << std::dec << " ";
		if (runErrors & (1<<0)) std::cout << "Board memory overflow, ";
		if (runErrors & (1<<1)) std::cout << "Board detected TCP/IP connection closed while running, ";
		for (int i=0; i<10; i++)
			if (runErrors & (1<<(8+i)))
				std::cout << " Alpide data receiver " << i << " detected electric idle condition, ";
		std::cout << std::endl;
	}
	return(runErrors);
}


// ================================== EOF ========================================
