#ifndef DBHELPERS_H
#define DBHELPERS_H

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include <string>
#include <vector>

#define PROJECT_ID 21   // TODO: make function to get project id?

int DbGetMemberId       (AlpideDB *db, string Name);
//int DbGetProjectId      (AlpideDB *db, string Name);
//int DbGetParameterId    (AlpideDB *db, int ActivityTypeId, string Name);
//int DbGetActivityTypeId (AlpideDB *db, string Name);


#endif
