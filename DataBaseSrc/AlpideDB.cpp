/*
 * \file AlpideDB.cpp
 * \author A.Franco
 * \date 16/Mar/2017
 *
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
 *
 *  Description : Alpide DB  Class *
 *  HISTORY
 *
 *
 */
#include "AlpideDB.h"
#include "utilities.h"


AlpideDB::AlpideDB()
{


	theQueryDomain = "https://test-alucmsapi.web.cern.ch/AlucmswebAPI.asmx/";

    theDBmanager = new AlpideDBManager();

    if(theDBmanager->isLibCurlCompiled()){
    	theDBmanager->Init("https://test-alucmsapi.web.cern.ch","FrancoAntonio",".","alpide4me");
    } else {
    	theDBmanager->Init("https://test-alucmsapi.web.cern.ch",
    	    		"/home/fap/.globus/usercert.pem",
    	    		"/home/fap/.globus/userkey.pem",
    				"/etc/ssl/certs");
    }

}


AlpideDB::~AlpideDB()
{
}

// ---------------- eof ------------------------
