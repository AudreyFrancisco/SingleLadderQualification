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
#include <unistd.h>
#include "powerboard.h"

powerboard::i2c_baseAddress_t powerboard::i2c_baseAddress[2] = 
	{
		{	// Master board					
			.dacBias 			= 0x32,		// LTC2635		U3  0110010	0x32
			.dacThreshold_14	= 0x31,		// LTC2635		U20 0110001 0x31
			.dacThreshold_58	= 0x33,		// LTC2635		U21 0110011 0x33
			.rdacVadj_14		= 0x2c,		// AD5254		U2	0101100	0x2c
			.rdacVadj_58		= 0x2d,		// AD5254		U8	0101101	0x2d
			.regLuVbuf_18		= 0x20,		// PCF8574		U32	0100000 0x20
			.regCtrl			= 0x26,		// PCF8574		U39	0100110 0x26
			.adcMon_14			= 0x21,		// AD7997		U4	0100001	0x21
			.adcMon_58			= 0x23,		// AD7997		U15	0100011	0x23
			.spiBridge			= 0x2a		// SC18IS602B 	U51	0101010	0x2a
		},
		{	// Slave board					
			.dacBias 			= 0x50,		// LTC2635		U3  1010000	0x50
			.dacThreshold_14	= 0x43,		// LTC2635		U20 1000011 0x43
			.dacThreshold_58	= 0x51,		// LTC2635		U21 1010001 0x51
			.rdacVadj_14		= 0x2e,		// AD5254		U2	0101110	0x2e
			.rdacVadj_58		= 0x2f,		// AD5254		U8	0101111	0x2f
			.regLuVbuf_18		= 0x27,		// PCF8574		U32	0100000 0x27
			.regCtrl			= 0x25,		// PCF8574		U39	0100101 0x25
			.adcMon_14			= 0x22,		// AD7997		U4	0100010	0x22
			.adcMon_58			= 0x24,		// AD7997		U15	0100100	0x24
			.spiBridge			= 0x2b		// SC18IS602B 	U51	0101011	0x2b
		}
	};



powerboard::powerboard(I2Cbus *bus, bool master)
{
	i2cBus = bus;
	i2c_baseAddress_t *baseAddrPtr = master ? i2c_baseAddress : i2c_baseAddress+1;

	regLuVbuf_18 = new PCF8574(i2cBus, baseAddrPtr->regLuVbuf_18);
	regCtrl = new PCF8574(i2cBus, baseAddrPtr->regCtrl);
	dacBias = new LTC2635(i2cBus, baseAddrPtr->dacBias);
	dacThreshold14 = new LTC2635(i2cBus, baseAddrPtr->dacThreshold_14);
	dacThreshold58 = new LTC2635(i2cBus, baseAddrPtr->dacThreshold_58);
	rdacVadj14 = new AD5254(i2cBus, baseAddrPtr->rdacVadj_14);
	rdacVadj58 = new AD5254(i2cBus, baseAddrPtr->rdacVadj_58);
	adcMon14 = new AD7997(i2cBus, baseAddrPtr->adcMon_14);
	adcMon58 = new AD7997(i2cBus, baseAddrPtr->adcMon_58);
	spiBridge = new SC18IS602(i2cBus, baseAddrPtr->spiBridge);
	for (int i=0; i<NUM_TSENSOR; i++)
		temperatureDetector[i] = new MAX31865(spiBridge, i);
}


powerboard::~powerboard()
{
	// delete objects in creation reverse order
	for (int i=0; i<NUM_TSENSOR; i++)
		delete temperatureDetector[i];
	delete adcMon14;
	delete adcMon58;
	delete rdacVadj14;
	delete rdacVadj58;
	delete dacBias;
	delete dacThreshold14;
	delete dacThreshold58;
	delete regCtrl;
	delete regLuVbuf_18;
}

bool powerboard::isReady()
{
	uint16_t data;

	// Write 0x550 in the hysteresys register of channel 1
	adcMon14->write(AD7997::REG_Hysteresis_CH1, (uint16_t) 0x0550);
	adcMon14->read(AD7997::REG_Hysteresis_CH1, &data);
	if (data != 0x0550)
		return false;

	// reset hysteresys register
	adcMon14->write(AD7997::REG_Hysteresis_CH1, (uint16_t) 0x0000);
	return true;
}


/*
	Set the current threshold of a single channel
*/
void powerboard::setIth(uint8_t ch, float value)
{
	LTC2635 *dac;
	uint16_t data;

	// select the DAC
	dac = (ch<4) ? dacThreshold14 : dacThreshold58;

	// evaluate the dac value from current value
	if ((ch&0x01) == 0)
		data = (255.0 * value)/4.31;	// Digital power supply
	else
		data = (255.0 * value)/0.43;	// analog power supply

	if (data > 255)
		data = 255;

	dac->WriteUpdateReg(ch&0x03, data);
}

/*
	Set the substrate bias
*/
void powerboard::setVbias(float value)
{
	uint16_t data;

	if (value>0)
		return;
	
	data = value * (-6528.0/125.0);
	
	if (data>255)
		data = 255;
	
	dacBias->WriteUpdateReg(0, data);
}

void powerboard::enVbias(bool en)
{
	if (en == false)
		regCtrl->write(CTRL_BiasShdn | CTRL_AllInputs | CTRL_ResetBar);
	else
		regCtrl->write(CTRL_AllInputs | CTRL_ResetBar);
}


void powerboard::setVout(uint8_t ch, float value)
{
	int data;
	AD5254 *rdac;

	// select the RDAC
	rdac = (ch<4) ? rdacVadj14 : rdacVadj58;

	data = 200.0 * (2.8-value); 
	if (data<0)
		data = 0;
	if (data>255)
		data = 255;

	rdac->setRDAC(ch&0x03, data);
}

void powerboard::storeVout(uint8_t ch)
{
	AD5254 *rdac;

	// select the RDAC
	rdac = (ch<4) ? rdacVadj14 : rdacVadj58;
	rdac->storeRDAC(ch&0x03);
}


void powerboard::storeAllVout()
{
	for (int i=0; i<4; i++){
		rdacVadj14->storeRDAC(i);
		rdacVadj58->storeRDAC(i);
	}
}

void powerboard::restoreAllVout()
{
	rdacVadj14->restoreAll();
	rdacVadj58->restoreAll();
}

// turn on all selected channel
void powerboard::onVout(uint8_t ch)
{
	regLuVbuf_18->write(~(1<<ch));
	usleep(100000);
	regLuVbuf_18->write(0xff);
}

// turn on all channels
void powerboard::onAllVout()
{
	regLuVbuf_18->write(0x00);
	usleep(100000);
	regLuVbuf_18->write(0xff);
}

// polinomial interpolation of RTD to Temperature function for PT100 sensor
static float RTD2T(float r)
{
	const float R0 = 100.0;		// PT100 ressitance at 0 C

	const float a = 2.3E-3;
	const float b = 2.5579;
	const float c = 1E-3;
	
	r -= R0;
	return a+r*(b+r*c);

}

/*
	Read board state from all chips
*/
void powerboard::getState(pbstate_t *state, getFlags flags)
{
	// configure the temperature sensors to start converting
	for (int i=0; i<NUM_TSENSOR; i++)
		temperatureDetector[i]->configure();

	if (flags & WaitTconv)
		usleep(50000);		// wait few conversion periods

	// Latch register
	state->chOn = regLuVbuf_18->read();
	
	if (flags & GetSettings){
		// Voltage adjust
		for (int i=0; i<8; i++){
			int rdata;
			AD5254 *rdac= (i<4) ? rdacVadj14 : rdacVadj58;

			rdata = rdac->getRDAC(i&0x03);
			state->Vout[i] = 2.8 - ((float)rdata / 200.0);
		}		
	}

	if (flags & GetMonitor){
		// Volatage/Currents monitor 
		adcMon14->setConfiguration(0xff);	// All channels
		adcMon58->setConfiguration(0xff);	// All channels
	
		uint16_t adcData[16];
		uint16_t *dataPtr = adcData;

		adcMon14->convert(8, adcData);
		adcMon58->convert(8, adcData+8);

		for (int i=0; i<8; i++){
			float data;
			data = (*dataPtr++ >> 2) & 0x3ff;
			state->Vmon[i] = data * (3.0 / 1024.0);

			data = (*dataPtr++ >> 2) & 0x3ff;
			if (i&0x01){
				// analog
				state->Imon[i] = data * ((3.0 / 1024.0) * (1.0/100.0) * (1.0/0.058));
			} else {
				// digital
				state->Imon[i] = data * ((3.0 / 1024.0) * (1.0/10.0) * (1.0/0.058));
			}
		}

		for (int i=0; i<NUM_TSENSOR; i++){
			float rtd = (temperatureDetector[i]->getRTD() >> 1) * (400.0/32768.0);
			state->T[i] = RTD2T(rtd);
		}
	}
}


