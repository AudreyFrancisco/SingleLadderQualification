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
#include <codecvt>
#include <curl/curl.h>

#include "utilities.h"
#include "CernSsoCookiesJar.h"

#define USERCERT "/home/fap/.globus/usercert.pem"
#define USERKEY "/home/fap/.globus/userkey.key"
#define SSOURL "https://test-alucmsapi.web.cern.ch"
#define CAPATH "/home/fap/.globus/CA/"
#define CA1FILE "CERN_Grid_Certification_Authority.pem"
#define CA2FILE	"AddTrustExternalCARoot.pem"


class AlpideDBManager
{

struct ReceiveBuffer {
	char *memory;
	size_t size;
};



// Members
private:

	CURL * myHandle;
	CURLcode result; // We’ll store the result of CURL’s webpage retrieval, for simple error checking.

	CernSsoCookieJar	*theCookieJar;

	string	theCliCer;
	string	theCliKey;
	string	theJarUrl;
	string theCertificationAuthorityPath;


//	QNetworkAccessManager  *theDBMan;
//	QSslConfiguration	*theSslConfig;
	int		thePendingRequests;

// Methods
public:
	AlpideDBManager();
    ~AlpideDBManager();

    bool Init(string aCliCer, string aCliKey, string aSslUrl, string aCAPath);
    bool Init();

    // getters & setters
    string getClientCertFile() { return(theCliCer);};
    string getClientKeyFile() { return(theCliKey);};
    string getSSOCookieUrl() { return(theJarUrl);};
    string getCAPath() { return(theCertificationAuthorityPath);};

    void setClientCertFile(string aCliCer) { theCliCer = aCliCer;};
    void setClientKeyFile(string aCliKey) { theCliKey = aCliKey;};
    void setSSOCookieUrl(string aJarUrl) { theJarUrl = aJarUrl;};
    void setCAPath(string aCAPath) { theCertificationAuthorityPath = aCAPath;};

public:
	int makeDBQuery(const string Url, const char *Payload, char **Result);
	// int syncDBQuery(string Url, char *Payload, char **Result);

	static size_t readResponseCB(void *contents, size_t size, size_t nmemb, void *userp);

//	void receiveResults(QNetworkRequest *request,QByteArray *result);


private:
	bool makeDBQuery(string Url);

private:
//	void endTheDBQuery(QNetworkReply *reply);
//	void requestAuthenication(QNetworkReply *reply, QAuthenticator *ator);
//	void recieveSslErrors(QNetworkReply*,const QList<QSslError> &errors);


};

#endif // ALPIDEDBMANAGER_H_
