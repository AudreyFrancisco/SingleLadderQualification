/*
 * \file AlpideBD.h
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
 *  Description : Header file for the Alpide DB Class
 *                for manage the DB access
 *
 *  HISTORY
 *
 *
 */
#ifndef ALPIDEDB_H_
#define ALPIDEDB_H_

#include <iostream>
#include <string>

#include "utilities.h"

#include "AlpideDBManager.h"

#define PROJECT_ID_PROD 383
#define PROJECT_ID_TEST 21

using namespace std;

class AlpideDB {

public:
  enum FieldFormat : int {
    IntFormat,
    FloatFormat,
    CharFormat,
    StringFormat,
    BoolFormat,
    ArrayFormat
  };

  // Members
public:
protected:
  AlpideDBManager *theDBmanager;
  string theQueryDomain;
  string theJarUrl;
  bool isConnected;
  int m_projectId;
  // Methods
public:
  AlpideDB(bool isTestDB = true);

  ~AlpideDB();

  string GetQueryDomain() { return (theQueryDomain); };
  void SetQueryDomain(string aQueryDomain) { theQueryDomain = aQueryDomain; };

  AlpideDBManager *GetManagerHandle() { return (theDBmanager); };
  bool isDBConnected() { return (isConnected); };
  int GetProjectId() { return m_projectId; };

private:
  void Init(string aQueryDomain, string aJarUrl);
};

#endif /* ALPIDEDB_H_ */
