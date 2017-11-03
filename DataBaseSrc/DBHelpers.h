#ifndef DBHELPERS_H
#define DBHELPERS_H

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include <string>
#include <vector>

typedef enum {attachResult, attachLog, attachErrors} TAttachmentType;

int  DbGetMemberId         (AlpideDB *db, string name);
//int  DbGetProjectId      (AlpideDB *db, string Name);
int  DbGetParameterId      (AlpideDB *db, int activityTypeId, string name);
int  DbGetActivityTypeId   (AlpideDB *db, string name);
int  DbGetAttachmentTypeId (AlpideDB *db, string name);
bool DbAddParameter        (AlpideDB *db, ActivityDB::activity &activity, string name, float value);
void DbAddAttachment       (AlpideDB *db, ActivityDB::activity &activity, TAttachmentType attType, string localName, string remoteName);
bool FileExists            (string fileName);
#endif
