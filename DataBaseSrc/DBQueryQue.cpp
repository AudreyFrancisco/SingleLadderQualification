/*
 * \file DBQueryQue.cpp
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
 *  Description : Implementation file for the Asyncronous Query DB Class
 *                for manage the DB access
 *
 *  HISTORY
 *
 *
 */
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <stdio.h>
#include <string.h>

#include "DBQueryQue.h"
#include "cJSON.h"

DBQueryQueue::DBQueryQueue()
{
    theBaseRepoPath = "/tmp";
    theSpecificQueuePath = "NewAlpide";
    theExtension = ".dbq";
    theLocalCopyRepoPath = "/tmp/localRepo";
    isTheLocalCopyEnabled = false;
}

DBQueryQueue::~DBQueryQueue()
{

}

string DBQueryQueue::__addThePathToFileName(const string FileName, bool isQueue)
{
    return( (!isQueue ? theLocalCopyRepoPath : theBaseRepoPath)  + "/" + theSpecificQueuePath + "/" + FileName);
}

bool DBQueryQueue::__makeTheFileName(const string Name, string *QueryFileName, bool IsLocal)
{
    string CompletePath;
    string FileName;
    time_t now;
    time( &now );
    FileName = time2name(now) + "_" + Name + theExtension;

    CompletePath = (IsLocal ? theLocalCopyRepoPath : theBaseRepoPath) + "/" + theSpecificQueuePath + "/";
    if(!pathExists(CompletePath)) {
        cerr << "DBQueue Error : the Path to locate the Queue not exists ! (" << CompletePath << ")" << endl;
        return(false);
    }
    CompletePath += FileName;
    QueryFileName->assign( CompletePath );
    return(true);
}

vector<string> DBQueryQueue::__getTheQueuqeFileList()
{
    vector<string> fileList;
    string Path;
    Path = theBaseRepoPath + "/" + theSpecificQueuePath;
    DIR *dir;
    dir = opendir( Path.c_str() );
    if (dir == NULL) {
        cerr << "DBQueue Error : to scan the directory (" << errno << ")  " << Path << endl;
        return fileList;
    }
    struct dirent *dirp;
    struct stat filestat;
    string filepath;
    while ((dirp = readdir( dir ))) {
        filepath = Path + "/" + dirp->d_name;
        if (stat( filepath.c_str(), &filestat )) continue;
        if (S_ISDIR( filestat.st_mode ))         continue;
        fileList.push_back(dirp->d_name);
    }
    closedir( dir );
    std::sort(fileList.begin(), fileList.end());
    return(fileList);
}

bool DBQueryQueue::__write(const string FileName, const string Query)
{
    ofstream newFile;
    newFile.open (FileName.c_str(), ios::out|ios::trunc);
    if(newFile.is_open() ) {
        newFile << Query << endl;
    } 
    else {
        cerr << "DBQueue Error in File Creation ! (" << FileName << ")" << endl;
        return(false);
    }
    newFile.close();
    return(true);
}

bool DBQueryQueue::Push(const string QueryType, ActivityDB::activity *activity)
{
    string OutBuffer;
    if(!Serialize(QueryType, activity, &OutBuffer)) {
        cerr << "DBQueue Error in Serialization !" << endl;
        return(false);
    }
    string sQueryFileName;
    if(!__makeTheFileName(activity->Name, &sQueryFileName)) {
        cerr << "DBQueue Error in File Name Creation !" << endl;
        return(false);
    }
    bool res = __write(sQueryFileName,OutBuffer);
    if(isTheLocalCopyEnabled) {
        if(!__makeTheFileName(activity->Name, &sQueryFileName, true)) {
            cerr << "DBQueue Error in File Name Creation !" << endl;
            return(false);
        }
        res &= __write(sQueryFileName,OutBuffer);
    }
    return(res);
}

vector<string> DBQueryQueue::GetTheQueue()
{
    vector<string> Files = __getTheQueuqeFileList();
    return(Files);
}

string DBQueryQueue::GetTheFirstFileName()
{
    vector<string> Files = __getTheQueuqeFileList();
    if(Files.size() > 0) {
        return(Files.at(0));
    }
    return("");
}

bool DBQueryQueue::Pop(string *QueryType, ActivityDB::activity *activity)
{
    vector<string> Files = __getTheQueuqeFileList();
    if( Files.size() == 0) {
        cerr << "DBQueue Queue empy !" << endl;
        return(false);
    }
    if( !Read(Files.at(0), QueryType, activity)) {
        cerr << "DBQueue Error reading the file  :"<< Files.at(0) << endl;
        return(false);
    }
    string FileWithPath = __addThePathToFileName(Files.at(0));
    if( remove(FileWithPath.c_str()) != 0 ) {
        cerr << "DBQueue Error deleting the file :"<< FileWithPath << endl;
        return(false);
    }
    return(true);
}


bool DBQueryQueue::Read(string FileName, string *QueryType, ActivityDB::activity *activity, bool isQueue)
{
    string FileWithPath = __addThePathToFileName(FileName, isQueue);
    if(!fileExists(FileWithPath)) {
        cerr << "DBQueue Error the file :"<< FileWithPath << " not exists !" << endl;
        return(false);
    }
    ifstream inpFile;
    inpFile.open (FileWithPath.c_str(), ios::in);
    if(inpFile.is_open() ) {
        inpFile.seekg(0, std::ios::end);
        size_t size = inpFile.tellg();
        string InpBuffer(size, ' ');
        inpFile.seekg(0);
        inpFile.read(&InpBuffer[0], size); 
        inpFile.close();
        if(!DeSerialize(InpBuffer, QueryType, activity)) {
            cerr << "DBQueue Error in De-Serialization !" << endl;
            return(false);
        }
        return(true);
    }
    cerr << "DBQueue Error opening the file :" << FileWithPath << endl;
    return(false);
}

bool DBQueryQueue::Write(string FileName, string QueryType, ActivityDB::activity *activity, bool isQueue)
{
    string FileWithPath = __addThePathToFileName(FileName, isQueue);
    string OutBuffer;
    if(!Serialize(QueryType, activity, &OutBuffer)) {
        cerr << "DBQueue Error in Serialization !" << endl;
        return(false);
    }
    bool res = __write(FileWithPath,OutBuffer);
    return(res);
}


bool DBQueryQueue::Serialize(string QueryType, ActivityDB::activity *activity, string *Output)
{
    cJSON *root = NULL;
    cJSON *que = NULL;
    cJSON *arr = NULL;
    cJSON *rec = NULL;
    time_t now;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "QueryType", cJSON_CreateString(QueryType.c_str()));
    time( &now );
    cJSON_AddItemToObject(root, "QueryTS", cJSON_CreateString( time2str(now).c_str()));
    que = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "Query", que);

    cJSON_AddNumberToObject(que, "ID", activity->ID);
    cJSON_AddNumberToObject(que, "Type", activity->Type);
    cJSON_AddNumberToObject(que, "Location", activity->Location);
    cJSON_AddNumberToObject(que, "Result", activity->Result);
    cJSON_AddNumberToObject(que, "Status", activity->Status);
    cJSON_AddNumberToObject(que, "User", activity->User);
    cJSON_AddStringToObject(que, "Lot", activity->Lot.c_str());
    cJSON_AddStringToObject(que, "Name", activity->Name.c_str());
    cJSON_AddStringToObject(que, "Position", activity->Position.c_str());
    cJSON_AddStringToObject(que, "StartDate", time2str(activity->StartDate).c_str());
    cJSON_AddStringToObject(que, "EndDate", time2str(activity->EndDate).c_str());

    arr = cJSON_CreateArray();
    cJSON_AddItemToObject(que, "Members", arr);
    for(int i=0;i<activity->Members.size();i++) {
        cJSON_AddItemToArray(arr, rec = cJSON_CreateObject());
        cJSON_AddNumberToObject(rec, "ID", activity->Members.at(i).ID);
        cJSON_AddNumberToObject(rec, "Leader", activity->Members.at(i).Leader);
        cJSON_AddNumberToObject(rec, "ProjectMember", activity->Members.at(i).ProjectMember);
        cJSON_AddNumberToObject(rec, "User", activity->Members.at(i).User);
    }
    arr = cJSON_CreateArray();
    cJSON_AddItemToObject(que, "Parameters", arr);
    for(int i=0;i<activity->Parameters.size();i++) {
        cJSON_AddItemToArray(arr, rec = cJSON_CreateObject());
        cJSON_AddNumberToObject(rec, "ID", activity->Parameters.at(i).ID);
        cJSON_AddNumberToObject(rec, "User", activity->Parameters.at(i).User);
        cJSON_AddNumberToObject(rec, "ActivityParameter", activity->Parameters.at(i).ActivityParameter);
        cJSON_AddNumberToObject(rec, "Value", activity->Parameters.at(i).Value);
    }
    arr = cJSON_CreateArray();
    cJSON_AddItemToObject(que, "Attachments", arr);
    for(int i=0;i<activity->Attachments.size();i++) {
        cJSON_AddItemToArray(arr, rec = cJSON_CreateObject());
        cJSON_AddNumberToObject(rec, "ID", activity->Attachments.at(i).ID);
        cJSON_AddNumberToObject(rec, "User", activity->Attachments.at(i).User);
        cJSON_AddNumberToObject(rec, "Category", activity->Attachments.at(i).Category);
        cJSON_AddStringToObject(rec, "LocalFileName", activity->Attachments.at(i).LocalFileName.c_str());
        cJSON_AddStringToObject(rec, "RemoteFileName", activity->Attachments.at(i).RemoteFileName.c_str());
    }
    arr = cJSON_CreateArray();
    cJSON_AddItemToObject(que, "Uris", arr);
    for(int i=0;i<activity->Uris.size();i++) {
        cJSON_AddItemToArray(arr, rec = cJSON_CreateObject());
        cJSON_AddNumberToObject(rec, "ID", activity->Uris.at(i).ID);
        cJSON_AddNumberToObject(rec, "User", activity->Uris.at(i).User);
        cJSON_AddStringToObject(rec, "Path", activity->Uris.at(i).Path.c_str());
        cJSON_AddStringToObject(rec, "Description", activity->Uris.at(i).Description.c_str());
    }
    arr = cJSON_CreateArray();
    cJSON_AddItemToObject(que, "InComps", arr);
    for(int i=0;i<activity->InComps.size();i++) {
        cJSON_AddItemToArray(arr, rec = cJSON_CreateObject());
        cJSON_AddNumberToObject(rec, "ID", activity->InComps.at(i).ID);
        cJSON_AddNumberToObject(rec, "User", activity->InComps.at(i).User);
        cJSON_AddNumberToObject(rec, "CompID", activity->InComps.at(i).CompID);
        cJSON_AddNumberToObject(rec, "CompTypeID", activity->InComps.at(i).CompTypeID);
    }
    arr = cJSON_CreateArray();
    cJSON_AddItemToObject(que, "OutComps", arr);
    for(int i=0;i<activity->OutComps.size();i++) {
        cJSON_AddItemToArray(arr, rec = cJSON_CreateObject());
        cJSON_AddNumberToObject(rec, "ID", activity->OutComps.at(i).ID);
        cJSON_AddNumberToObject(rec, "User", activity->OutComps.at(i).User);
        cJSON_AddNumberToObject(rec, "CompID", activity->OutComps.at(i).CompID);
        cJSON_AddNumberToObject(rec, "CompTypeID", activity->OutComps.at(i).CompTypeID);
    }
    Output->assign( cJSON_Print(root) );
    cJSON_Delete(root);
    return(true);
}

bool DBQueryQueue::DeSerialize(string Input, string *QueryType, ActivityDB::activity *activity)
{
    cJSON *root = cJSON_Parse(Input.c_str());
    const cJSON *obj = NULL;
    const cJSON *obj1 = NULL;
    const cJSON *arr = NULL;
    const cJSON *ele = NULL;
    string sAp;
    int iAp;
    float fAp;

    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
            cerr << "Deserialization Error NULL root document :" << error_ptr << endl;
        return(false);
    }
    obj = cJSON_GetObjectItemCaseSensitive(root, "QueryType");
    if (cJSON_IsString(obj) && (obj->valuestring != NULL)) {
        QueryType->assign( obj->valuestring);
    }
    obj = cJSON_GetObjectItemCaseSensitive(root, "QueryTS");
    if (cJSON_IsString(obj) && (obj->valuestring != NULL)) {
        //  nothing TODO: add info about the query time execution ....
    }
    if (activity == NULL) {
        cerr << "Deserialization Error NULL pointer on return Activity parameter. Abort !" << endl;
        return(false);
    }
    obj = cJSON_GetObjectItemCaseSensitive(root, "Query");
    if (cJSON_IsObject(obj)) {
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "ID");
        if (cJSON_IsNumber(obj1)) {
            activity->ID = obj1->valueint;
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "Type");
        if (cJSON_IsNumber(obj1)) {
            activity->Type = obj1->valueint;
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "Location");
        if (cJSON_IsNumber(obj1)) {
            activity->Location = obj1->valueint;
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "Result");
        if (cJSON_IsNumber(obj1)) {
            activity->Result = obj1->valueint;
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "Status");
        if (cJSON_IsNumber(obj1)) {
            activity->Status = obj1->valueint;
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "User");
        if (cJSON_IsNumber(obj1)) {
            activity->User = obj1->valueint;
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "Lot");
        if (cJSON_IsString(obj1)  && (obj1->valuestring != NULL)) {
            activity->Lot.assign( obj1->valuestring );
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "Name");
        if (cJSON_IsString(obj1)  && (obj1->valuestring != NULL)) {
            activity->Name.assign( obj1->valuestring );
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "Position");
        if (cJSON_IsString(obj1)  && (obj1->valuestring != NULL)) {
            activity->Position.assign( obj1->valuestring );
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "StartDate");
        if (cJSON_IsString(obj1)  && (obj1->valuestring != NULL)) {
            str2time(obj1->valuestring , &activity->StartDate);
        }
        obj1 = cJSON_GetObjectItemCaseSensitive(obj, "EndDate");
        if (cJSON_IsString(obj1)  && (obj1->valuestring != NULL)) {
            str2time(obj1->valuestring , &(activity->EndDate) );
        }

        arr = cJSON_GetObjectItemCaseSensitive(obj, "Members");
        ActivityDB::member sMem;
        activity->Members.clear();
        cJSON_ArrayForEach(ele, arr) {
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "ID");
            if (cJSON_IsNumber(obj1)) {
                sMem.ID = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "Leader");
            if (cJSON_IsNumber(obj1)) {
                sMem.Leader = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "ProjectMember");
            if (cJSON_IsNumber(obj1)) {
                sMem.ProjectMember = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "User");
            if (cJSON_IsNumber(obj1)) {
                sMem.User = obj1->valueint;
            }
            activity->Members.push_back(sMem);
        }

        arr = cJSON_GetObjectItemCaseSensitive(obj, "Parameters");
        ActivityDB::parameter sPar;
        activity->Parameters.clear();
        cJSON_ArrayForEach(ele, arr) {
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "ID");
            if (cJSON_IsNumber(obj1)) {
                sPar.ID = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "User");
            if (cJSON_IsNumber(obj1)) {
                sPar.User = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "ActivityParameter");
            if (cJSON_IsNumber(obj1)) {
                sPar.ActivityParameter = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "Value");
            if (cJSON_IsNumber(obj1)) {
                sPar.Value = obj1->valuedouble;
            }
            activity->Parameters.push_back(sPar);
        }

        arr = cJSON_GetObjectItemCaseSensitive(obj, "Attachments");
        ActivityDB::attach sAtt;
        activity->Attachments.clear();
        cJSON_ArrayForEach(ele, arr) {
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "ID");
            if (cJSON_IsNumber(obj1)) {
                sAtt.ID = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "User");
            if (cJSON_IsNumber(obj1)) {
                sAtt.User = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "Category");
            if (cJSON_IsNumber(obj1)) {
                sAtt.Category = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "LocalFileName");
            if (cJSON_IsString(obj1)  && (obj1->valuestring != NULL)) {
                sAtt.LocalFileName.append( obj1->valuestring );
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "RemoteFileName");
            if (cJSON_IsString(obj1)  && (obj1->valuestring != NULL)) {
                sAtt.RemoteFileName.append( obj1->valuestring );
            }
            activity->Attachments.push_back(sAtt);
        }

        arr = cJSON_GetObjectItemCaseSensitive(obj, "Uris");
        ActivityDB::uri sUri;
        activity->Uris.clear();
        cJSON_ArrayForEach(ele, arr) {
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "ID");
            if (cJSON_IsNumber(obj1)) {
                sUri.ID = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "User");
            if (cJSON_IsNumber(obj1)) {
                sUri.User = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "Description");
            if (cJSON_IsString(obj1)  && (obj1->valuestring != NULL)) {
                sUri.Description.append( obj1->valuestring );
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "Path");
            if (cJSON_IsString(obj1)  && (obj1->valuestring != NULL)) {
                sUri.Path.append( obj1->valuestring );
            }
            activity->Uris.push_back(sUri);
        }

        arr = cJSON_GetObjectItemCaseSensitive(obj, "InComps");
        ActivityDB::incomp sCom;
        activity->InComps.clear();
        cJSON_ArrayForEach(ele, arr) {
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "ID");
            if (cJSON_IsNumber(obj1)) {
                sCom.ID = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "User");
            if (cJSON_IsNumber(obj1)) {
                sCom.User = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "CompID");
            if (cJSON_IsNumber(obj1)) {
                sCom.CompID = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "CompTypeID");
            if (cJSON_IsNumber(obj1)) {
                sCom.CompTypeID = obj1->valueint;
            }
            activity->InComps.push_back(sCom);
        }

        arr = cJSON_GetObjectItemCaseSensitive(obj, "OutComps");
        ActivityDB::outcomp sComo;
        activity->OutComps.clear();
        cJSON_ArrayForEach(ele, arr){
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "ID");
            if (cJSON_IsNumber(obj1)) {
                sComo.ID = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "User");
            if (cJSON_IsNumber(obj1)) {
                sComo.User = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "CompID");
            if (cJSON_IsNumber(obj1)) {
                sComo.CompID = obj1->valueint;
            }
            obj1 = cJSON_GetObjectItemCaseSensitive(ele, "CompTypeID");
            if (cJSON_IsNumber(obj1)) {
                sComo.CompTypeID = obj1->valueint;
            }
            activity->OutComps.push_back(sComo);
        }
    }
    cJSON_Delete( root );
    return(true);
}

/* ------------ DBQueueConsumer Class ------------- */
DBQueueConsumer::DBQueueConsumer(bool DataBaseType)
{
    bDatabasetype = DataBaseType;

    cfgStatusFile = CFG_STATUS_FILE_PATHNAME;
    cfgExtension = CFG_QUEUEFILEEXETENSION;
    cfgBasePath = CFG_QUEUEBASEPATH;
    cfgSpecificPath = CFG_QUEUESPECIFICPATH;
    cfgMaxReconAtt = CFG_MAXRECONATTEMPTS;

    activ = new ActivityDB::activity();
    
    isTimeOutFired = false;
}

DBQueueConsumer::~DBQueueConsumer()
{
}


bool DBQueueConsumer::__ReconectDB(bool bDatabasetype)
{
    if(fDB == NULL) {
        fDB = new AlpideDB(bDatabasetype);
    }

    bool bDone = false;
    int iCount = cfgMaxReconAtt; // Maximum attempts
    while(iCount>0 && !bDone) {
        if (fDB->isDBConnected()) {
            if(act == NULL) {
                act = new ActivityDB(fDB);
                act->theAsyncronuosQueue->SetEstension(cfgExtension);
                act->theAsyncronuosQueue->SetQueueBasePath(cfgBasePath);
                act->theAsyncronuosQueue->SetSpecificPath(cfgSpecificPath);
            }
            bDone = true;
        } 
        else {
            iCount--;
            if(fDB != NULL) {
                delete fDB;
                fDB = NULL;
            }
            fDB = new AlpideDB(bDatabasetype);
            if(act != NULL) {
                delete act;
                act = NULL;
            }
            usleep(CFG_RECONNECTDELAY_MSEC * 1000); // takes delay
        }
    }
    return(bDone);
}


void DBQueueConsumer::__getTheRunStatus()
{
    char buf[999];
    FILE *fh;
    fh = fopen( cfgStatusFile.c_str() , "r");
    if(fh && !feof(fh)) {
        fgets(buf, 998, fh);
        theState = atoi(buf);
        fgets(buf, 998, fh);
        if(buf[ strlen(buf) -1 ] == '\n') buf[ strlen(buf) -1 ] = 0;
        theFileName.assign(buf);
        fclose(fh);
    } 
    else {
        theState = ST_START;
    }
    return;
}
void DBQueueConsumer::__setTheRunStatus(int State)
{
    char buf[999];
    FILE *fh;

    theState = State;
    theLastSetStatusTS = time(0);
    theSubState = 0;
    
    strftime(buf, 40, "%d-%m-%Y %H:%M:%S", (const tm *)(localtime(&theLastSetStatusTS)));
    fh = fopen( cfgStatusFile.c_str() , "w");
    if(fh) {
        fprintf(fh, "%d\n", theState);
        fprintf(fh, "%s\n", theFileName.c_str());
        fprintf(fh, "%s\n", buf);
        fclose(fh);
    } 
    else {
        cerr << "DBQueueConsumer : Unable to write the status file ! ABORT." << endl;
        exit(-1);
    }
    return;
}

void DBQueueConsumer::__checkTimeOut()
{
    time_t now = time(0);
    if((now - theLastSetStatusTS) > 3600) {
        if(!isTimeOutFired) {
            cerr << "DBQueueConsumer : One Query take more then one hour to execute !" << endl;
            isTimeOutFired = true;
        }
    }
    else {
        isTimeOutFired = false;
    }
    usleep(CFG_CONSUMERDELAY_MSEC * 1000);
}

bool DBQueueConsumer::__readTheQueue()
{
    bool bRes = true;
    string fileName = act->theAsyncronuosQueue->GetTheFirstFileName();
    if(fileName == "") bRes = false;
    theFileName = fileName;
    return(bRes);
}

bool DBQueueConsumer::__consumeTheQueue()
{
    int s;
    unsigned int i;
    bool isClosable = false;
    std::vector<ActivityDB::statusType> *statusTypeList;
    
    theState = 0;
    __getTheRunStatus();
    
    while(theState != ST_DONE) {
        switch(theState) {
        case ST_START:    // we can take a queue element
            s = (!__readTheQueue()) ? ST_DONE : ST_OPENACT;
            __setTheRunStatus(s);
            break;
        case ST_OPENACT: // Create the activity
            if (act->theAsyncronuosQueue->Read(theFileName, &theQueryType, activ)) {
                theResultId = activ->Result; // save the result
                activ->Result = -999;
                respo = act->CreateActivity_1(activ);
                if(respo->ErrorCode == 0){
                    activ->Result = theResultId; // restore the Result after creation
                    __setTheRunStatus(ST_CREATEACT);
                    act->theAsyncronuosQueue->Write(theFileName, theQueryType, activ);
                }
            }
            break;
        case ST_CREATEACT: // Assign the Member
            if (act->theAsyncronuosQueue->Read(theFileName, &theQueryType, activ)) {
                respo = act->CreateMember_2(activ);
                if(respo->ErrorCode == 0){
                    __setTheRunStatus(ST_CREATEMEM);
                    act->theAsyncronuosQueue->Write(theFileName, theQueryType, activ);
                }
            }
            break;
        case ST_CREATEMEM: // Assign the Parameters
            if (act->theAsyncronuosQueue->Read(theFileName, &theQueryType, activ)) {
                respo = act->CreateParameter_3(activ);
                if(respo->ErrorCode == 0){
                    __setTheRunStatus(ST_CREATEPAR);
                    act->theAsyncronuosQueue->Write(theFileName, theQueryType, activ);
                }
            }
            break;
        case ST_CREATEPAR: // Assign the Attachment
            if (act->theAsyncronuosQueue->Read(theFileName, &theQueryType, activ)) {
                respo = act->CreateAttachments_4(activ);
                if(respo->ErrorCode == 0){
                    __setTheRunStatus(ST_CREATEATT);
                    act->theAsyncronuosQueue->Write(theFileName, theQueryType, activ);
                }
            }
            break;
        case ST_CREATEATT: // Assign the Uri
            if (act->theAsyncronuosQueue->Read(theFileName, &theQueryType, activ)) {
                respo = act->AssignUris(activ->ID, activ->User, &(activ->Uris));
                if(respo->ErrorCode == 0) {
                    __setTheRunStatus(ST_CREATEURI);
                    act->theAsyncronuosQueue->Write(theFileName, theQueryType, activ);
                }
            }
            break;
        case ST_CREATEURI: // Assign theComponents
            if (act->theAsyncronuosQueue->Read(theFileName, &theQueryType, activ)) {
                s = ST_CREATECOMPIN;
                for(i=theSubState;i<activ->InComps.size();i++){
                    respo = act->AssignComponent(activ->ID, activ->InComps.at(i).CompID,
                            activ->InComps.at(i).CompTypeID, activ->InComps.at(i).User);
                    if(respo->ErrorCode != 0) {
                        cerr << "Assign Components In Error :" << act->DumpResponse()  << endl;
                        s = ST_CREATEURI;
                        theSubState = i;
                        break;
                    } 
                    else {
                        activ->InComps.at(i).ID = respo->ID;    
                    }
                }
                if( i == activ->InComps.size() ) { // we finish the assignment
                    __setTheRunStatus(s);
                    act->theAsyncronuosQueue->Write(theFileName, theQueryType, activ);
                }
            }
            break;
        case ST_CREATECOMPIN: // Assign theComponents
            if (act->theAsyncronuosQueue->Read(theFileName, &theQueryType, activ)) {
                s = ST_CREATECOMPOUT;
                for(i=theSubState;i<activ->OutComps.size();i++){
                    respo = act->AssignComponent(activ->ID, activ->OutComps.at(i).CompID,
                            activ->OutComps.at(i).CompTypeID, activ->OutComps.at(i).User);
                    if(respo->ErrorCode != 0) {
                        cerr << "Assign Components Out Error :" << act->DumpResponse()  << endl;
                        s = ST_CREATECOMPIN;
                        theSubState = i;
                        break;
                    }
                    else {
                        activ->OutComps.at(i).ID = respo->ID;    
                    }
                }
                if( i == activ->OutComps.size() ) { // we finish the assignment
                    __setTheRunStatus(s);
                    act->theAsyncronuosQueue->Write(theFileName, theQueryType, activ);
                }
            }
            break;
        case ST_CREATECOMPOUT: // Close the activity
            isClosable = false;
            if (act->theAsyncronuosQueue->Read(theFileName, &theQueryType, activ)) {
                statusTypeList = act->GetStatusList(activ->Type);
                for (i = 0; i < statusTypeList->size(); i++) {
                    if ("CLOSED" == statusTypeList->at(i).Code) {
                        isClosable = true;
                        activ->Status = statusTypeList->at(i).ID;
                        respo = act->Change(activ);
                        if(respo->ErrorCode == 0) {
                            __setTheRunStatus(ST_CLOSED);
                            act->theAsyncronuosQueue->Write(theFileName, theQueryType, activ);
                        }
                        else {
                            cerr << "Close activity Error :" << act->DumpResponse()  << endl;
                        }
                        break;
                    }
                }
                if(!isClosable) { // --- escape without the close 
                    __setTheRunStatus(ST_CLOSED);
                    act->theAsyncronuosQueue->Write(theFileName, theQueryType, activ);
                }
            }
            break;
        case ST_CLOSED:  // Remove the element from the queue
            if (act->theAsyncronuosQueue->Pop(&theQueryType, activ)) {
                __setTheRunStatus(ST_DONE);
                cout << "DBQueueConsumer : Asyncronous Activity created :" <<  activ->ID << "  " << activ->Name << endl;
            }
        }
        __checkTimeOut();
    }
    __setTheRunStatus(ST_START);
    return(true);
}

void DBQueueConsumer::Run()
{
    bool bExit = false;
    while(!bExit) {
        if (!__ReconectDB(bDatabasetype)) {
            cerr << "DBQueueConsumer : We loose the DB connection..." << endl;
            bExit = true;
        } else {
            __consumeTheQueue(); // manage one queue element
            usleep(CFG_CREATIONDELAY_MSEC * 1000);
        }
    }
}

int DBQueueConsumer::GetQueueLenght()
{
    vector<string> list = act->theAsyncronuosQueue->GetTheQueue();
    return(list.size());
}

vector<string> DBQueueConsumer::GetQueue()
{
    return( act->theAsyncronuosQueue->GetTheQueue() );
}

