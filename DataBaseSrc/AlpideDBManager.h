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
 *	2/8/2017   -   Add the X509/Kerberos authentication switch
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

#ifdef AUTH_X509
	#define CAPATH "/etc/pki/tls/certs"
	#define CAFILE "/etc/pki/tls/certs/ca-bundle.crt"
	#define CA1FILE "CERN_Grid_Certification_Authority.pem"
	#define CA2FILE	"AddTrustExternalCARoot.pem"

	#ifdef COMPILE_LIBCURL
		#define NSSDATABASEPATH "."
		#define NSSCERTNICKNAME "FrancoAntonio"
		#define NSSDBPASSWD "alpide4me"
		#define USERCERT "/tmp/usercert.pem"
		#define USERKEY "/tmp/userkey.pem"
	#else
		#define USERCERT "/home/fap/.globus/usercert.pem"
    	#define USERKEY "/home/fap/.globus/userkey.pem"
	#endif
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
	//CURLcode result; // We’ll store the result of CURL’s webpage retrieval, for simple error checking. YCM:FIXME, not used

	#ifdef AUTH_X509
		string theNSSNickName;
		string theNSSDBPath;
		string theNSSDBPassword;
	#endif
#endif

#ifdef AUTH_X509
	string	theCliCer;
	string	theCliKey;
	string  theCertificationAuthorityPath;
#endif

	CernSsoCookieJar	*theCookieJar;
	string	theJarUrl;
	int		thePendingRequests;

// Methods
public:
	AlpideDBManager();
    ~AlpideDBManager();

#ifdef COMPILE_LIBCURL
    bool isLibCurlCompiled(void) { return(true); };
#else
    bool isLibCurlCompiled(void) { return(false); };
#endif


#ifdef AUTH_KERBEROS
	bool Init(string aSslUrl);
#endif
#ifdef AUTH_X509
	#ifdef COMPILE_LIBCURL
    	bool Init(string aSslUrl, string aNickName, string aNSSDBPath, string aNSSDBPassFile);
    	string getNSSDBNickName() { return(theNSSNickName);};
    	string getNSSDBPath() { return(theNSSDBPath);};
    	string getNSSDBPass() { return(theNSSDBPassword);};
    	void setNSSDBNickName(string aNickName) { theNSSNickName = aNickName;};
    	void setNSSDBPath(string aNSSDBPath) { theNSSDBPath = aNSSDBPath;};
    	void setNSSDBPass(string aNSSDBPass) { theNSSDBPassword = aNSSDBPass;};
	#else
    	bool Init(string aSslUrl, string aCliCer, string aCliKey,  string aCAPath);
        string getClientCertFile() { return(theCliCer);};
        string getClientKeyFile() { return(theCliKey);};
        void setClientCertFile(string aCliCer) { theCliCer = aCliCer;};
        void setClientKeyFile(string aCliKey) { theCliKey = aCliKey;};
        string getCAPath() { return(theCertificationAuthorityPath);};
        void setCAPath(string aCAPath) { theCertificationAuthorityPath = aCAPath;};
	#endif
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
