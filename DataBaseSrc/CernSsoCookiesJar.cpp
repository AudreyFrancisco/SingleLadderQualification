/*
 * \file CernSsoCookiesJar.cpp
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
 *  Description : Cookie Jar Class extension
 *                for manage the CERN SSO autentication method
 *
 *  HISTORY
 *
 *	2/8/2017   -   Add the X509/Kerberos authentication switch
 *
 */
#include "CernSsoCookiesJar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstdio>
#include <fstream>
#include <iostream>

CernSsoCookieJar::CernSsoCookieJar()
{
  if (!testTheCERNSSO()) {
    std::cerr << "First attempt to obtain the cookie failed, trying once again..." << std::endl;
    if (!testTheCERNSSO()) {
      throw runtime_error("Failed to obtain the SSO cookie");
    }
  }
  theSslUrl         = "";
  theCookiePackFile = std::tmpnam(nullptr);

#ifdef AUTH_X509
  theCliCert = "";
  theCliKey  = "";
#endif
}

CernSsoCookieJar::~CernSsoCookieJar() { remove(theCookiePackFile.c_str()); }

/* -----------------------------------
 * Examine if the Jar is valid
 * ----------------------------------- */
bool CernSsoCookieJar::isJarValid()
{
  int               i;
  int               n             = theJar.size();
  unsigned long int theActualTime = time(NULL);
  unsigned long int theExpiration;

  if (n == 0) return (false); // the Jar is empty
  for (i = 0; i < n; i++) {
    theExpiration = theJar.at(i).expires;
    if (theExpiration <= theActualTime) {
      cerr << "The cookie " << theJar.at(i).name << " is expired !" << endl;
      return (false);
    }
  }
  return (true);
}

/* -----------------------------------
 * gets the CERN SSO Cookies and fills
 * the cookie jar
 * ----------------------------------- */
#ifdef AUTH_X509
bool CernSsoCookieJar::fillTheJar(string aCliCert, string aCliKey, string aSslUrl)
{
  theCliCert = aCliCert;
  theCliKey  = aCliKey;
  theSslUrl  = aSslUrl;
  return (fillTheJar());
}
#endif
#ifdef AUTH_KERBEROS
bool CernSsoCookieJar::fillTheJar(string aSslUrl)
{
  theSslUrl = aSslUrl;
  return (fillTheJar());
}
#endif

bool CernSsoCookieJar::fillTheJar()
{
  // run the CERN SSO in order to obtain the cookies file
  if (remove(theCookiePackFile.c_str())) { // the file exists... delete!
    if (VERBOSITYLEVEL == 1) {
      cout << "The " << theCookiePackFile << " file deleted." << endl;
    }
  }
  string Command = getCookieExe();
#ifdef AUTH_X509
  Command += " --cert ";
  Command += theCliCert;
  Command += " --key ";
  Command += theCliKey;
#endif
#ifdef AUTH_KERBEROS
#ifdef USE_PYTHON_SSO
  Command += " -k";
#else
  Command += " --krb";
#endif
#endif
#ifndef USE_PYTHON_SSO
  Command += " -r ";
#endif
  Command += " -u ";
  Command += theSslUrl;
  Command += " -o ";
  Command += theCookiePackFile;

  if (system(Command.c_str()) != 0) {
    cout << "Failed to execute the command: " << Command << endl;
  }
  if (VERBOSITYLEVEL == 1) {
    cout << "Execute the bash :" << Command << endl;
  }
  if (!fileExists(theCookiePackFile)) { // the file doesn't exists. ACH !
    cerr << "Error to obtain the CERN SSO Cookies pack file. Abort !" << endl;
    return (false);
  }

  int numOfCookies = parseTheJar(theCookiePackFile);
  if (numOfCookies < 1) return (false);
  if (VERBOSITYLEVEL == 1) {
    cout << "The CERN cookie jar contains " << numOfCookies << " valid cookies !" << endl;
  }

  return (true);
}

int CernSsoCookieJar::parseTheJar(string aCookieJarFile)
{
  FILE *fh = fopen(aCookieJarFile.c_str(), "r");
  if (fh == NULL) {
    cerr << "Error to read the CookieJar file ! Abort" << endl;
    return (-1);
  }

  int    NumberOfCookies = 0;
  Cookie rigolo;
  char   Buffer[THECOOKIELENGTH];

  if (!fgets(Buffer, THECOOKIELENGTH, fh)) {
    fclose(fh);
    return 0;
  }
  while (!feof(fh)) {
    if (Buffer[0] != '#' && Buffer[0] != 0 && Buffer[0] != '\n' && Buffer[0] != '\r') {

      char *ptr;
      if ((ptr = strtok(Buffer, "\t"))) {
        rigolo.domain = ptr;
        if ((ptr = strtok(NULL, "\t"))) {
          rigolo.tailmatch = (strcmp(ptr, "FALSE") == 0) ? false : true;
          if ((ptr = strtok(NULL, "\t"))) {
            rigolo.path = ptr;
            if ((ptr = strtok(NULL, "\t"))) {
              rigolo.secure = (strcmp(ptr, "FALSE") == 0) ? false : true;
              if ((ptr = strtok(NULL, "\t"))) {
                rigolo.expires = atol(ptr);
                if ((ptr = strtok(NULL, "\t"))) {
                  rigolo.name = ptr;
                  if ((ptr = strtok(NULL, "\t"))) {
                    rigolo.value = ptr;
                    theJar.push_back(rigolo);
                    NumberOfCookies++;
                  }
                }
              }
            }
          }
        }
      }
    }
    if (!fgets(Buffer, THECOOKIELENGTH, fh)) break;
  }
  fclose(fh);
  return (NumberOfCookies);
}

/* ------------------------------------
 * tests the presence of CERNSSO
 * ------------------------------------ */
bool CernSsoCookieJar::testTheCERNSSO()
{

  string tmp_filename = std::tmpnam(nullptr);

  string Command = getCookieExe() + " > " + tmp_filename;
  if (system(Command.c_str()) != 0) {
    cout << "Failed to execute the command: " << Command << endl;
  }
  if (VERBOSITYLEVEL == 1) {
    cout << "Execute : " << Command << endl;
  }

  FILE *result;
  result = fopen(tmp_filename.c_str(), "r");
  if (result == NULL) {
    cerr << "Error to access /tmp folder ! Abort" << endl;
    remove(tmp_filename.c_str());
    return (false);
  }

  char buffer[512];
  if (fgets(buffer, 511, result) != NULL &&
      strstr(buffer, (getCookieExe() + " is").c_str()) != NULL) {
    if (VERBOSITYLEVEL == 1) {
      cout << "The CERN-SSO package is installed !" << endl;
    }
    fclose(result);
    remove(tmp_filename.c_str());
    return (true);
  }
  else {
    cerr << "CERN-SSO package not Found ! Abort" << endl;
    fclose(result);
    remove(tmp_filename.c_str());
    return (false);
  }
  return (false);
}

// ---------------- eof ------------------------
