/*
 * \file AlpideDBManager.cpp
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
 *  Description : Alpide DB Manager Class *
 *  HISTORY
 *
 *
 */

#include "AlpideDBManager.h"
#include "utilities.h"

#include <stdio.h>
#include <string.h>
//#include <curl/curl.h>

AlpideDBManager::AlpideDBManager()
{

	theCliCer = USERCERT;
	theCliKey = USERKEY;
	theCliKeyPem = USERKEYPEM;
	theJarUrl = SSOURL;
	theCertificationAuthorityPath = CAPATH;


//	myHandle = curl_easy_init ( ) ;
	theCookieJar = new CernSsoCookieJar();

}


AlpideDBManager::~AlpideDBManager()
{
}

bool AlpideDBManager::Init(string aCliCer, string aCliKey, string aSslUrl, string aCAPath)
{
	Init(aCliCer, "", aCliKey, aSslUrl, aCAPath);
	return(Init());
}

bool AlpideDBManager::Init(string aCliCer, string aCliKey, string aCliKeyPem, string aSslUrl, string aCAPath)
{
	theCliCer = aCliCer;
	theCliKey = aCliKey;
	theCliKeyPem = aCliKeyPem;
	theJarUrl = aSslUrl;
	theCertificationAuthorityPath = aCAPath;
	return(Init());
}

bool AlpideDBManager::Init()
{

	if(!theCookieJar->fillTheJar(theCliCer,theCliKeyPem,theJarUrl)) {
		cerr << "Error to Init the DB manager. Exit !";
		return(false);
	}

	/*
	myHandle = curl_easy_init ( ) ;
	curl_easy_setopt( myHandle, CURLOPT_VERBOSE, 1L);


	curl_easy_setopt( myHandle,CURLOPT_CAPATH , theCertificationAuthorityPath.c_str());
	curl_easy_setopt( myHandle, CURLOPT_SSH_PRIVATE_KEYFILE, theCliKeyPem.c_str());
	curl_easy_setopt( myHandle, CURLOPT_SSLCERT, theCliCer.c_str());

	cout <<endl<< "the Key file:"<<  theCliKeyPem <<endl;
	thePendingRequests = 0;
    */



	/*
	QList<QSslCertificate> certCA;
	QString s = theCertificationAuthorityPath; s += CA1FILE;
	certCA.append(QSslCertificate::fromPath(QLatin1String(s.toAscii())));
	s = theCertificationAuthorityPath; s += CA2FILE;
	certCA.append(QSslCertificate::fromPath(QLatin1String(s.toAscii())));
	theSslConfig->setCaCertificates(certCA);
    qDebug() << "* CA certificates loaded :"; for(int i=0; i<theSslConfig->caCertificates().size(); i++) qDebug() << i << ")" << theSslConfig->caCertificates().at(i); qDebug() << "-------------------------- ";
*/

	return(true);
}

int  AlpideDBManager::makeDBQuery(const string Url,const char *Payload, char **Result)
{

	 // in order to maintain the connection over the 24hours
	 if(!theCookieJar->isJarValid()) theCookieJar->fillTheJar();

	 cout << endl << "DBQuery :" << Url << " Payload:" << Payload << endl;;

	 string Command = "curl ";
	    Command += " --cert ";
	    Command += theCliCer;
	    Command += " --key ";
	    Command += theCliKeyPem;
	    Command += " -b";
	    Command += "/tmp/cerncookie.txt";
	    Command += " -c ";
	    Command += "/tmp/cerncookie.txt";
	    Command += " ";
	    Command += Url;
	    Command += Payload;
	    Command += " > /tmp/Queryresult.xml";

	    system(Command.c_str());

cout << "Execute the bash :" << Command;

	    if(!fileExists("/tmp/Queryresult.xml")) { //the file doesn't exists. ACH !
	        cerr << "Error to Execute the query. Abort !";
	        return(false);
	    }

	    // get the response file size
	    FILE *res = fopen("/tmp/Queryresult.xml","r");
	    if(res == NULL) {
	        cerr << "Error to Access the File buffer of Query. Abort !";
	        return(false);
	    }
	    fseek(res, 0L, SEEK_END);
	    long sz = ftell(res);
	    fseek(res, 0L, SEEK_SET);

	    // allocate memory
	    char *ptrBuf = (char *)malloc(sz +10);
	    if(ptrBuf == NULL){
	        cerr << "Error to Allocate buffer in memory. Abort !";
	        fclose(res);
	        return(false);
	    }
	    // read the response
	    long nre = fread(ptrBuf, 1, sz, res);
	    if(nre != sz) {
	    	cerr << "Error reading the file buffer. Abort !";
	    	fclose(res);
	    	return(false);
	    }
	    // close and return
	    fclose(res);
	    *Result = ptrBuf;
	    return(true);

	/*
	CURLcode res;
	curl_easy_setopt( myHandle, CURLOPT_VERBOSE, 1L);

    // in order to maintain the connection over the 24hours
    if(!theCookieJar->isJarValid()) theCookieJar->fillTheJar();

    cout << endl << "DBQuery :" << Url << " Payload:" << Payload << endl;;

    // parse  the Url ....
    Uri theUrl = Uri::Parse(Url);


    // encode the url ...
    char *encodedUrl = curl_easy_escape( myHandle, Url.c_str(), Url.size() );

    char encodedUrl[200] = "https://www.cern.ch";
    // Compose the request
    string appo;
    curl_easy_setopt( myHandle, CURLOPT_URL, encodedUrl );
    curl_easy_setopt( myHandle, CURLOPT_USERAGENT, "curl/7.19.7 (x86_64-redhat-linux-gnu) libcurl/7.19.7 NSS/3.21 Basic ECC zlib/1.2.3 libidn/1.18 libssh2/1.4.2" );
*/
 //   curl_easy_setopt( myHandle, CURLOPT_ENCODING, "*/*" );

/*    struct curl_slist *headers=NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    appo = "Host: "; appo += theUrl.Host;
    headers = curl_slist_append(headers, appo.c_str());
//	request.setRawHeader("Content-Length", postDataSize);
    curl_easy_setopt(myHandle, CURLOPT_HTTPHEADER, headers);

    // send all data to this function
    curl_easy_setopt(myHandle, CURLOPT_WRITEFUNCTION, readResponseCB);

    // we pass our 'chunk' struct to the callback function
    struct ReceiveBuffer theBuffer;
    theBuffer.memory = (char *) malloc(1);  //will be grown as needed by the realloc above
    theBuffer.size = 0;    // no data at this point
    curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, (void *)&theBuffer);

    // Put the Post data ...
    curl_easy_setopt(myHandle, CURLOPT_POSTFIELDS, Payload);
    curl_easy_setopt(myHandle, CURLOPT_POSTFIELDSIZE, strlen(Payload) );

    //
    curl_easy_setopt( myHandle, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt( myHandle, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt( myHandle, CURLOPT_COOKIEFILE, "/tmp/cerncookie.txt");

print_cookies(myHandle);
    // Perform the request, res will get the return code
    res = curl_easy_perform(myHandle);
    if(res != CURLE_OK) { // Check for errors
        cerr << "Curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        return(0);
    } else {
    	*Result = theBuffer.memory;
    }
    curl_easy_cleanup(myHandle);

    curl_free(encodedUrl);
*/
	return(1);
}

/*
size_t AlpideDBManager::readResponseCB(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
    struct ReceiveBuffer *mem = (struct ReceiveBuffer *)userp;

	mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		cerr << "not enough memory (realloc returned NULL)" << endl;
	    return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

void  AlpideDBManager::print_cookies(CURL *curl)
{
  CURLcode res;
  struct curl_slist *cookies;
  struct curl_slist *nc;
  int i;
 
  printf("Cookies, curl knows:\n");
  res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
  if(res != CURLE_OK) {
    fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n",
            curl_easy_strerror(res));
    exit(1);
  }
  nc = cookies;
  i = 1;
  while(nc) {
    printf("[%d]: %s\n", i, nc->data);
    nc = nc->next;
    i++;
  }
  if(i == 1) {
    printf("(none)\n");
  }
  curl_slist_free_all(cookies);
}
*/
