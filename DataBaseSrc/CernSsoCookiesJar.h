/*
 * \file CernSsoCookiesJar.h
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
 *  Description : Header file for the Cookie Jar Class extension
 *                for manage the CERN SSO autentication method
 *
 *  HISTORY
 *
 *	2/8/2017   -   Add the X509/Kerberos authentication switch
 *
 */
#ifndef CERNSSOCOOKIEJAR_H
#define CERNSSOCOOKIEJAR_H

#include "utilities.h"
#include <iostream>
#include <string>
#include <vector>

#define THECOOKIELENGTH 8196

using namespace std;

class CernSsoCookieJar {
  struct Cookie {
    string            domain;
    bool              tailmatch;
    string            path;
    bool              secure;
    unsigned long int expires;
    string            name;
    string            value;
  };

  // Members
private:
  string         theSslUrl;
  string         theCookiePackFile;
  vector<Cookie> theJar;

#ifdef AUTH_X509
  string theCliCert;
  string theCliKey;
#endif

  // Methods
public:
  CernSsoCookieJar();
  ~CernSsoCookieJar();

  bool isJarValid();

public:
#ifdef AUTH_X509
  bool fillTheJar(string aCliCert, string aCliKey, string aSslUrl);
#endif
#ifdef AUTH_KERBEROS
  bool fillTheJar(string aSslUrl);
#endif

  bool   fillTheJar();
  string getCookiePackFileName() { return theCookiePackFile; }

private:
  bool testTheCERNSSO();
  int parseTheJar(string aCookieJarFile);
};

#endif // CERNSSOCOOKIEJAR_H
