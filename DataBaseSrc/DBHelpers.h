#ifndef DBHELPERS_H
#define DBHELPERS_H

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include <string>
#include <vector>

#define PROJECT_ID 21   // TODO: make function to get project id?

int DbGetMemberId       (AlpideDB *db, string name);
//int DbGetProjectId      (AlpideDB *db, string Name);
int DbGetParameterId    (AlpideDB *db, int activityTypeId, string name);
int DbGetActivityTypeId (AlpideDB *db, string name);
bool DbAddParameter     (AlpideDB *db, ActivityDB::activity activity, string name, float value);
void DbAddAttachment    (AlpideDB *db, ActivityDB::activity activity, int attCategory, string localName, string remoteName);
#endif
