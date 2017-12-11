#ifndef DBHELPERS_H
#define DBHELPERS_H

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include "../inc/TScanConfig.h"
#include <string>
#include <vector>

typedef enum {attachResult, attachLog, attachErrors, attachConfig} TAttachmentType;

int    DbGetMemberId           (AlpideDB *db, string name);
//int  DbGetProjectId        (AlpideDB *db, string Name);
int    DbGetParameterId        (AlpideDB *db, int activityTypeId, string name);
int    DbGetActivityTypeId     (AlpideDB *db, string name);
int    DbGetPrevActivityTypeId (AlpideDB *db, string name, bool &onChildren);
int    DbGetAttachmentTypeId   (AlpideDB *db, string name);
int    DbGetComponentTypeId    (AlpideDB *db, int projectId, string name);
int    DbGetComponentId        (AlpideDB *db, int projectId, int typeId, string name);
int    DbGetListOfChildren     (AlpideDB *db, int Id, std::vector<int> &children);
int    DbGetComponentActivity  (AlpideDB *db, int compId, int activityTypeId);
bool   DbAddParameter          (AlpideDB *db, ActivityDB::activity &activity, string name, float value);
void   DbAddAttachment         (AlpideDB *db, ActivityDB::activity &activity, TAttachmentType attType, string localName, string remoteName);
bool   FileExists              (string fileName);
string CreateActivityName      (string compName, TTestType test);
#endif
