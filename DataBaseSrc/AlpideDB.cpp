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


AlpideDB::AlpideDB(bool isTestDB)
{
  if(isTestDB) {
    Init( "https://test-alucmsapi.web.cern.ch/AlucmswebAPI.asmx", "https://test-alucmsapi.web.cern.ch");
    m_projectId = PROJECT_ID_TEST;
  }
  else {
    Init( "https://alucmsapi.web.cern.ch/AlucmswebAPI.asmx", "https://alucmsapi.web.cern.ch");
    m_projectId = PROJECT_ID_PROD;
  }
}


void AlpideDB::Init(string aQueryDomain, string aJarUrl)
{
  theQueryDomain = aQueryDomain;
  theJarUrl = aJarUrl;

  theDBmanager = new AlpideDBManager();

#ifdef AUTH_KERBEROS
  isConnected = theDBmanager->Init(theJarUrl);
#endif

#ifdef AUTH_X509
  if(theDBmanager->isLibCurlCompiled()){
    isConnected = theDBmanager->Init(theJarUrl,
    		                     "FrancoAntonio",
		                     ".",
		                     "alpide4me");
  } else {
    isConnected = theDBmanager->Init(theJarUrl,
        	    		     "/home/fap/.globus/usercert.pem",
        	    		     "/home/fap/.globus/userkey.pem",
        			     "/etc/ssl/certs");
  }
#endif
}


AlpideDB::~AlpideDB()
{
}

// ---------------- eof ------------------------
