/*
 * \file AlpideBDManager.h
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
 *  Description : Header file for the Alpide DB Manager Class extension
 *                for manage the DB access
 *
 *  HISTORY
 *
 *
 */
#ifndef ALPIDEDBMANAGER_H_
#define ALPIDEDBMANAGER_H_

#include <locale>
#include "utilities.h"

// --- Definition of constants for auth methods
	#define COOKIEPACK "/tmp/cerncookie.txt"

	#define SSOURL "https://test-alucmsapi.web.cern.ch"

#ifdef COMPILE_LIBCURL
	#define PASSWD ":"
#else
    #define USERCERT "/home/fap/.globus/usercert.pem"
    #define USERKEY "/home/fap/.globus/userkey.pem"
#endif


#ifdef COMPILE_LIBCURL
#include <curl/curl.h>
#endif

#include "utilities.h"
#include "CernSsoCookiesJar.h"

class AlpideDBManager
{

struct ReceiveBuffer {
	char *memory;
	size_t size;
};



// Members
private:

#ifdef COMPILE_LIBCURL
	CURL * myHandle;
	CURLcode result; // We’ll store the result of CURL’s webpage retrieval, for simple error checking.

#endif

	CernSsoCookieJar	*theCookieJar;
	string	theJarUrl;

	int		thePendingRequests;

// Methods
public:
	AlpideDBManager();
    ~AlpideDBManager();

#ifdef COMPILE_LIBCURL
    bool Init(string aSslUrl);
    bool isLibCurlCompiled(void) { return(true); };
#else
    bool Init(string aSslUrl, string aCliCer, string aCliKey,  string aCAPath);
    bool isLibCurlCompiled(void) { return(false); };
#endif


    bool Init();

    string getSSOCookieUrl() { return(theJarUrl);};
    void setSSOCookieUrl(string aJarUrl) { theJarUrl = aJarUrl;};

public:
	int makeDBQuery(const string Url, const char *Payload, char **Result, bool isSOAPrequest = false, const char *SOAPAction = NULL);

#ifdef COMPILE_LIBCURL
	static size_t readResponseCB(void *contents, size_t size, size_t nmemb, void *userp);
#endif

private:

#ifdef COMPILE_LIBCURL
	void print_cookies(CURL *curl);
#endif

};

#endif // ALPIDEDBMANAGER_H_
