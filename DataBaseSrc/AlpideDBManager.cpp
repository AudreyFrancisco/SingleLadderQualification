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
 * 	2/8/2017   -   Add the X509/Kerberos authentication switch
 *
 */

#include "AlpideDBManager.h"

#include <fstream>

#include <cstdio>
#include <string.h>

AlpideDBManager::AlpideDBManager()
{
  theJarUrl    = SSOURL;
  theCookieJar = new CernSsoCookieJar();
#ifdef AUTH_X509
#ifdef COMPILE_LIBCURL
  theNSSNickName                = NSSCERTNICKNAME;
  theNSSDBPath                  = NSSDATABASEPATH;
  theNSSDBPassword              = NSSDBPASSWD;
  theCliCer                     = USERCERT;
  theCliKey                     = USERKEY;
  theCertificationAuthorityPath = CAPATH;
#endif
#endif

#ifdef COMPILE_LIBCURL
  curl_global_init(CURL_GLOBAL_ALL);
  myHandle = curl_easy_init();
#endif
}

AlpideDBManager::~AlpideDBManager()
{
  delete theCookieJar;
  theCookieJar = 0x0;
}

#ifdef AUTH_KERBEROS
bool AlpideDBManager::Init(string aSslUrl)
{
  theJarUrl = aSslUrl;
  return (Init());
}
#endif

#ifdef AUTH_X509
#ifdef COMPILE_LIBCURL
bool AlpideDBManager::Init(string aSslUrl, string aNickName, string aNSSDBPath, string aNSSDBPass)
{
  theJarUrl        = aSslUrl;
  theNSSNickName   = aNickName;
  theNSSDBPath     = aNSSDBPath;
  theNSSDBPassword = aNSSDBPass;

  string command;
  // now export cert
  command = "certutil -L -d ";
  command += aNSSDBPath;
  command += " -n ";
  command += aNickName;
  command += " -a >";
  command += theCliCer;
  system(command.c_str());
  if (VERBOSITYLEVEL == 1) {
    cout << "Com:" << command << endl;
  }

  // now export  key
  command = "pk12util -o /tmp/theKey.p12 -d . -K ";
  command += theNSSDBPassword;
  command += " -W \"\" -n ";
  command += theNSSNickName;
  system(command.c_str());
  if (VERBOSITYLEVEL == 1) {
    cout << "Com:" << command << endl;
  }

  command = "openssl pkcs12 -in /tmp/theKey.p12 -out ";
  command += theCliKey;
  command += " -nodes -passin pass:";
  system(command.c_str());
  if (VERBOSITYLEVEL == 1) {
    cout << "Com:" << command << endl;
  }

  command = "rm /tmp/theKey.p12";
  system(command.c_str());
  if (VERBOSITYLEVEL == 1) {
    cout << "Com:" << command << endl;
  }
  return (Init());
}
#else
bool AlpideDBManager::Init(string aSslUrl, string aCliCer, string aCliKey, string aCAPath)
{
  theJarUrl                     = aSslUrl;
  theCliCer                     = aCliCer;
  theCliKey                     = aCliKey;
  theCertificationAuthorityPath = aCAPath;
  return (Init());
}
#endif
#endif

bool AlpideDBManager::Init()
{
#ifdef AUTH_KERBEROS
  if (!theCookieJar->fillTheJar(theJarUrl)) {
    cerr << "Error to Init the DB manager with Kerberos authentication. Exit !\n" << endl;
    return (false);
  }
#endif
#ifdef AUTH_X509
  if (!theCookieJar->fillTheJar(theCliCer, theCliKey, theJarUrl)) {
    cerr << "Error to Init the DB manager with X509 authentication. Exit !" << endl;
    return (false);
  }
#endif

#ifdef COMPILE_LIBCURL
  myHandle = curl_easy_init();
  curl_easy_setopt(myHandle, CURLOPT_VERBOSE, CURLVERBOSITYLEVEL);
#ifdef AUTH_X509
  curl_easy_setopt(myHandle, CURLOPT_CAINFO, CAFILE);
  curl_easy_setopt(myHandle, CURLOPT_CAPATH, CAPATH);

  curl_easy_setopt(myHandle, CURLOPT_KEYPASSWD, theNSSDBPassword.c_str());
  curl_easy_setopt(myHandle, CURLOPT_SSLKEY, theNSSNickName.c_str());
  curl_easy_setopt(myHandle, CURLOPT_SSLCERT, theNSSNickName.c_str());
#else
  curl_easy_setopt(myHandle, CURLOPT_HTTPAUTH, CURLAUTH_GSSNEGOTIATE);
  curl_easy_setopt(myHandle, CURLOPT_USERPWD, ":");
#endif
#endif
  thePendingRequests = 0;
  return (true);
}

int AlpideDBManager::makeDBQuery(const string Url, const char *Payload, char **Result,
                                 bool isSOAPrequest, const char *SOAPAction)
{
  string result_filename = std::tmpnam(nullptr);
  string tmp_filename    = std::tmpnam(nullptr);

  // in order to maintain the connection over the 24hours
  if (!theCookieJar->isJarValid()) {
    if (!theCookieJar->fillTheJar()) {
      return (0);
    }
  }

  if (VERBOSITYLEVEL == 1) {
    cout << endl << "DBQuery :" << Url << endl;
    if (strlen(Payload) > 512) {
      printf(" Payload : %.*s ...\n", 500, Payload);
    }
    else {
      cout << " Payload:" << Payload << endl;
    }
  }

#ifdef COMPILE_LIBCURL
  CURLcode res;
  curl_easy_setopt(myHandle, CURLOPT_VERBOSE, CURLVERBOSITYLEVEL);

  // parse  the Url ....
  Uri theUrl = Uri::Parse(Url);

  // Compose the request
  string appo;

  curl_easy_setopt(myHandle, CURLOPT_URL, theUrl.URI.c_str());
  curl_easy_setopt(myHandle, CURLOPT_USERAGENT,
                   "curl/7.19.7 (x86_64-redhat-linux-gnu) "
                   "libcurl/7.19.7 NSS/3.21 Basic ECC zlib/1.2.3 "
                   "libidn/1.18 libssh2/1.4.2");

  // Compose the heasder
  struct curl_slist *headers = NULL;
  appo                       = "Host: ";
  appo += theUrl.Host;
  headers = curl_slist_append(headers, appo.c_str());

  if (isSOAPrequest) {
    if (SOAPVERSION == 11) {
      headers = curl_slist_append(headers, "Content-Type: text/xml; charset=utf-8");
      appo    = "SOAPAction:\"";
      appo += SOAPAction;
      appo += "\"";
      headers = curl_slist_append(headers, appo.c_str());
    }
    else {
      headers = curl_slist_append(headers, "Content-Type: application/soap+xml; charset=utf-8");
    }
  }
  else {
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
  }
  curl_easy_setopt(myHandle, CURLOPT_HTTPHEADER, headers);

  // send all data to this function
  curl_easy_setopt(myHandle, CURLOPT_WRITEFUNCTION, readResponseCB);

  // we pass our 'chunk' struct to the callback function
  struct ReceiveBuffer theBuffer;
  theBuffer.memory = (char *)malloc(1); // will be grown as needed by the realloc above
  theBuffer.size   = 0;                 // no data at this point
  curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, (void *)&theBuffer);

  // Put the Post data ...
  curl_easy_setopt(myHandle, CURLOPT_POSTFIELDS, Payload);
  curl_easy_setopt(myHandle, CURLOPT_POSTFIELDSIZE, strlen(Payload));
  if (VERBOSITYLEVEL == 1) {
    if (strlen(Payload) > 512)
      printf(" Payload : %.*s ...\n", 500, Payload);
    else
      cout << " Payload:" << Payload << endl;
  }
  // https settings ...
  curl_easy_setopt(myHandle, CURLOPT_SSL_VERIFYPEER, true);
  curl_easy_setopt(myHandle, CURLOPT_SSL_VERIFYHOST, true);

  curl_easy_setopt(myHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(myHandle, CURLOPT_COOKIEFILE, theCookieJar->getCookiePackFileName().c_str());

  // Perform the request, res will get the return code
  res = curl_easy_perform(myHandle);
  if (res != CURLE_OK) { // Check for errors
    cerr << "Curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
    return (0);
  }
  else {
    *Result = theBuffer.memory;
  }
  curl_slist_free_all(headers);
  return (1);

#else

  string Command = "curl ";
#ifdef AUTH_X509
  Command += " --cert ";
  Command += theCliCer;
  Command += " --key ";
  Command += theCliKey;
#endif
#ifdef AUTH_KERBEROS
  Command += " --negotiate -u :";
#endif
  Command += " -b";
  Command += theCookieJar->getCookiePackFileName();
  Command += " -c ";
  Command += theCookieJar->getCookiePackFileName();
  if (isSOAPrequest) {
    remove(tmp_filename);
    std::ofstream out(tmp_filename);
    out << Payload;
    out.close();
    Command += " -H 'SOAPACTION: \"";
    Command += SOAPAction;
    Command += "\"' -X POST -H 'Content-type: text/xml' -d @" + tmp_filename + " \"";
    Command += Url;
    Command += "\"";
  }
  else {
    Command += " \"";
    Command += Url;
    Command += "?";
    Command += Payload;
    Command += "\"";
  }
  Command += " > " + result_filename;

  system(Command.c_str());

  if (VERBOSITYLEVEL == 1) {
    cout << "Execute the bash :" << Command << endl;
  }

  if (!fileExists(result_filename)) { // the file doesn't exists. ACH !
    cerr << "Error to Execute the query. Abort !";
    return (false);
  }

  // get the response file size
  FILE *res = fopen(result_filename, "r");
  if (res == NULL) {
    cerr << "Error to Access the File buffer of Query. Abort !" << endl;
    return (false);
  }
  fseek(res, 0L, SEEK_END);
  long sz = ftell(res);
  fseek(res, 0L, SEEK_SET);

  // allocate memory
  char *ptrBuf = (char *)malloc(sz + 10);
  if (ptrBuf == NULL) {
    cerr << "Error to Allocate buffer in memory. Abort !" << endl;
    fclose(res);
    return (false);
  }
  // read the response
  long nre = fread(ptrBuf, 1, sz, res);
  if (nre != sz) {
    cerr << "Error reading the file buffer. Abort !" << endl;
    fclose(res);
    return (false);
  }
  // close and return
  fclose(res);
  *Result = ptrBuf;
  return (true);

#endif
}

#ifdef COMPILE_LIBCURL

size_t AlpideDBManager::readResponseCB(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t                realsize = size * nmemb;
  struct ReceiveBuffer *mem      = (struct ReceiveBuffer *)userp;

  mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    cerr << "not enough memory (realloc returned NULL)" << endl;
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

void AlpideDBManager::print_cookies(CURL *curl)
{
  CURLcode           res;
  struct curl_slist *cookies;
  struct curl_slist *nc;
  int                i;

  printf("Cookies, curl knows:\n");
  res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
  if (res != CURLE_OK) {
    fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
    exit(1);
  }
  nc = cookies;
  i  = 1;
  while (nc) {
    printf("[%d]: %s\n", i, nc->data);
    nc = nc->next;
    i++;
  }
  if (i == 1) {
    printf("(none)\n");
  }
  curl_slist_free_all(cookies);
}

#endif

// ---------------- eof ------------------------
