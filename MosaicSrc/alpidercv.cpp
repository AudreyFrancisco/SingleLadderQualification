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
#include "alpidercv.h"
#include "pexception.h"
#include <stdio.h>
#include <stdlib.h>

ALPIDErcv::ALPIDErcv() {}

ALPIDErcv::ALPIDErcv(WishboneBus *wbbPtr, uint32_t baseAdd) : MWbbSlave(wbbPtr, baseAdd) {}

ALPIDErcv::~ALPIDErcv() {}

//
// set register
//
void ALPIDErcv::addSetReg(uint16_t address, uint16_t val)
{
  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  wbb->addWrite(baseAddress + address, val);
}

//
// Read register
//
void ALPIDErcv::addGetReg(uint16_t address, uint32_t *val)
{
  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  wbb->addRead(baseAddress + address, val);
}

//
// Disable (or enable) the receiver
//
void ALPIDErcv::addEnable(bool d)
{
  wbb->addRMWbits(baseAddress + regOpMode, ~OPMODE_RCVENABLE, d ? OPMODE_RCVENABLE : 0);
}

void ALPIDErcv::addInvertInput(bool d)
{
  wbb->addRMWbits(baseAddress + regOpMode, ~OPMODE_INVERT_POLARITY, d ? OPMODE_INVERT_POLARITY : 0);
}

//
// set RDP register
//
void ALPIDErcv::addSetRDPReg(uint16_t address, uint16_t val)
{
  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  wbb->addWrite(baseAddress + rdpBase + address, val);
}

//
// Read RDP register
//
void ALPIDErcv::addGetRDPReg(uint16_t address, uint32_t *val)
{
  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  wbb->addRead(baseAddress + rdpBase + address, val);
}

//
//  Read Modify Write a RDP register
//
void ALPIDErcv::addSetRDPRegField(uint16_t address, uint16_t size, uint16_t offset, uint16_t val)
{
  uint16_t mask = ((1 << (size)) - 1) << offset;

  val <<= offset;
  val &= mask;

  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  wbb->addRMWbits(baseAddress + rdpBase + address, ~mask, val);
}

//
//  PRBS related functions
//
#define RX_PRBS_ERR_CNT 0x015e // Address int transceiver DRP of RX error counter

void ALPIDErcv::addPRBSsetSel(uint8_t s)
{
  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  wbb->addRMWbits(baseAddress+regPrbs, ~PRBS_SEL_MASK, (s<<PRBS_SEL_SHIFT));
}

void ALPIDErcv::addPRBSreset()
{
  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  wbb->addRMWbits(baseAddress+regPrbs, ~PRBS_RESET, PRBS_RESET);
  wbb->addRMWbits(baseAddress+regPrbs, ~PRBS_RESET, 0);
}

void ALPIDErcv::addGetPRBScounter(uint32_t *ctr)
{
  addGetRDPReg(RX_PRBS_ERR_CNT, ctr);
}

