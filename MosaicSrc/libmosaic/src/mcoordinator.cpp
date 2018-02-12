/*
 * Copyright (C) 2018
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2018.
 *
 */
#include "mcoordinator.h"
#include "mexception.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

MCoordinator::MCoordinator(WishboneBus *wbbPtr, uint32_t baseAdd) : MWbbSlave(wbbPtr, baseAdd) {}

void MCoordinator::addEnableExtClock(bool en)
{
  uint32_t tmp;

  tmp = en ? EXT_CLK : 0;
  wbb->addRMWbits(baseAddress + regCfg, ~EXT_CLK, tmp);
}

void MCoordinator::addSetMode(mode_t mode)
{
  uint32_t tmp = 0;

  switch (mode) {
  case Alone:
    tmp = ALONE;
    break;

  case Master:
    tmp = MASTER;
    break;

  case Slave:
    tmp = 0;
    break;
  }

  wbb->addRMWbits(baseAddress + regCfg, ~(ALONE | MASTER), tmp);
}

void MCoordinator::setMode(mode_t mode)
{
  addSetMode(mode);
  wbb->execute();
}

std::string MCoordinator::dumpRegisters()
{
  if (!wbb) throw MIPBusUDPError("No IPBus configured");

  regAddress_e addrs[] = {regCfg};
  uint32_t     nAddrs  = sizeof(addrs) / sizeof(regAddress_e);

  std::stringstream ss;
  ss << std::hex;

  for (uint32_t iAddr = 0; iAddr < nAddrs; ++iAddr) {
    uint32_t result = 0xDEADBEEF;
    try {
      wbb->addRead(baseAddress + addrs[iAddr], &result);
      execute();
    }
    catch (...) {
      std::cerr << "Pulser read error: address 0x" << std::hex << baseAddress + addrs[iAddr]
                << " (0x" << addrs[iAddr] << ")!" << std::dec << std::endl;
    };

    ss << "0x" << addrs[iAddr] << "\t0x" << result << std::endl;
  }

  ss << std::endl;

  return ss.str();
}
