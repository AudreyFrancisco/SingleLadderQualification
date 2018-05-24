/*
 * \file DBQueryQue.h
 * \author A.Franco
 * \date 17/Mag/2018
 *
 * Copyright (C) 2018
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
 *  Description : Header file for the Asyncronous Query DB Class
 *                for manage the DB access
 *
 *  HISTORY
 *
 *
 */
#ifndef DBQUERYQUE_H_
#define DBQUERYQUE_H_

#include <iostream>
#include <string>

#include "utilities.h"

//#include "AlpideDBManager.h"
#include "AlpideDBEndPoints.h"
#include "AlpideDB.h"

using namespace std;


/* -------------------------------------

    Class DBQueryQueue
    
   ---------------------------------- */
#define PROJECT_ID_PROD 383
#define PROJECT_ID_TEST 21

class DBQueryQueue {

//public:
//  enum FieldFormat : int {
//    IntFormat,
//  };

// Members
public:
protected:

private:
    string theBaseRepoPath;
    string theSpecificQueuePath;
    string theExtension;
    string theLocalCopyRepoPath;

    bool isTheLocalCopyEnabled;

// Methods
public:
    DBQueryQueue();
    ~DBQueryQueue();

    bool Push(const string QueryType, ActivityDB::activity *activity);
    bool Pop(string *QueryType, ActivityDB::activity *activity);
    bool Read(string FileName, string *QueryType, ActivityDB::activity *activity);
    bool Write(string FileName, string QueryType, ActivityDB::activity *activity);
    string GetTheFirstFileName();
    vector<string> GetTheQueue();

    bool Serialize(string QueryType, ActivityDB::activity *activity, string *Output);
    bool DeSerialize(string Input, string *QueryType, ActivityDB::activity *activity);

    // getter setter
    string GetQueueBasePath() { return(theBaseRepoPath);};
    string GetSpecificPath() { return(theSpecificQueuePath);};
    string GetEstension() { return(theExtension);};
    string GetLocalCopyPath() { return(theLocalCopyRepoPath);};
    bool IsLocalCopyEnabled() { return(isTheLocalCopyEnabled);};

    void SetQueueBasePath(string s) {theBaseRepoPath = s;};
    void SetSpecificPath(string s) { theSpecificQueuePath = s;};
    void SetEstension(string s) { theExtension = s;};
    void SetLocalCopyPath(string s) { theLocalCopyRepoPath = s;};
    void SetLocalCopyEnabled(bool b) { isTheLocalCopyEnabled = b;};


private:
    void Init();
    bool __makeTheFileName(const string Name, string *QueryFileName, bool IsLocal = false);
    string __addThePathToFileName(const string FileName);
    bool __write(const string FileName, const string Query);
    vector<string> __getTheQueuqeFileList();

};

/* -------------------------------------

    Class DBQueueConsumer
    
   ---------------------------------- */


// Definitions
#define CFG_QUEUEFILEEXETENSION ".dbq"
#define CFG_QUEUEBASEPATH "/tmp"
#define CFG_QUEUESPECIFICPATH "AsyncDBQueue"
#define CFG_STATUS_FILE_PATHNAME "/tmp/DBQueueConsumerState.dat"
#define CFG_MAXRECONATTEMPTS 9999

#define CFG_RECONNECTDELAY_MSEC 500
#define CFG_CONSUMERDELAY_MSEC 20
#define CFG_CREATIONDELAY_MSEC 500

#define ST_START 0
#define ST_OPENACT 1
#define ST_CREATEACT 2
#define ST_CREATEMEM 3
#define ST_CREATEPAR 4
#define ST_CREATEATT 5
#define ST_CREATEURI 6
#define ST_CREATECOMPIN 7
#define ST_CREATECOMPOUT 8
#define ST_CLOSED 9
#define ST_DONE 10

class DBQueueConsumer {

// Members
private:
    time_t theLastSetStatusTS;
    bool isTimeOutFired;
    bool s;
    
    ActivityDB::activity *activ;
    ActivityDB::response *respo;

    AlpideDB *fDB;
    ActivityDB *act;

public:
    string theFileName;
    int theState;
    int theSubState;
    string theQueryType;
    int theResultId;

    bool bDatabasetype;

    int cfgMaxReconAtt;
    string cfgStatusFile = "";
    string cfgExtension = "";
    string cfgBasePath = "";
    string cfgSpecificPath = "";

// Methods
private:
    bool __ReconectDB(bool bDatabasetype);
    void __getTheRunStatus();
    void __setTheRunStatus(int State);
    bool __readTheQueue();
    bool __consumeTheQueue();
    void __checkTimeOut();

public:
    DBQueueConsumer(bool DataBaseType);
    ~DBQueueConsumer();
    int GetQueueLenght();
    vector<string> GetQueue();
    void Run();
    int GetProjectId() { return( fDB->GetProjectId() );};
};


#endif /* DBQUERYQUE_H_ */
