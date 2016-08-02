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
#ifndef WBB_H
#define WBB_H

/*
	Definition of Base address for WishboneBus slaves
*/
class WbbBaseAddress
{
public:
	enum baseAddress_e {
		i2cSysPLL				= (0x00 << 24),
		runControl 				= (0x01 << 24),
		triggerControl			= (0x02 << 24),
		dataGenerator 			= (0x03 << 24),
		pulser					= (0x04 << 24),
		i2cMaster				= (0x05 << 24),
		controlInterface		= (0x06 << 24),
		controlInterfaceB		= (0x07 << 24),
		alpide3rcv				= (0x08 << 24),
		};
};





#endif // WBB_H
