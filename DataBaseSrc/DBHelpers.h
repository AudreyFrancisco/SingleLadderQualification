#ifndef DBHELPERS_H
#define DBHELPERS_H

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include <string>
#include <vector>

typedef enum { attachResult, attachLog, attachErrors, attachConfig, attachText } TAttachmentType;

typedef struct {
  int    Id;
  int    Type;
  string Name;
  int    Position;
} TChild;

int DbGetMemberId(AlpideDB *db, string name);
// int  DbGetProjectId        (AlpideDB *db, string Name);
int              DbGetParameterId(AlpideDB *db, int activityTypeId, string name);
int              DbGetResultId(AlpideDB *db, int activityTypeId, string resultName);
int              DbGetResultId(AlpideDB *db, int activityTypeId, THicClassification classification);
int              DbGetStatusId(AlpideDB *db, int activityTypeId, string statusCode);
int              DbCountActivities(AlpideDB *db, int activityTypeId, string compName);
std::vector<int> DbGetActivityIds(AlpideDB *db, int activityTypeId, string compName);
std::vector<ActivityDB::activityLong> DbGetActivities(AlpideDB *db, std::vector<int> activityIds);
int                DbIsNewer(ActivityDB::activityLong act0, ActivityDB::activityLong act1);
int                DbIsNewer(ComponentDB::compActivity act0, ComponentDB::compActivity act1);
bool               DbGetLatestActivity(AlpideDB *db, int activityTypeId, string compName,
                                       ActivityDB::activityLong &activity);
int                DbGetActivityTypeId(AlpideDB *db, string name);
void               DbGetPreviousTests(AlpideDB *db, int compId, int activityTypeId,
                                      vector<ComponentDB::compActivity> &tests, bool &openAct, bool &impedance);
THicClassification DbGetPreviousCategory(AlpideDB *db, int compId, int activityTypeId,
                                         bool &openAct, bool &impedance);
bool   DbFindParamValue(vector<ActivityDB::actParameter> pars, string parName, float &parValue);
int    DbGetPrevActivityTypeId(AlpideDB *db, string name, bool &onChildren);
int    DbGetAttachmentTypeId(AlpideDB *db, string name);
int    DbGetComponentTypeId(AlpideDB *db, int projectId, string name);
int    DbGetComponentTypeId(AlpideDB *db, string name);
int    DbGetActComponentTypeId(AlpideDB *db, int activityTypeId, int componentId, string Direction);
int    DbGetComponentId(AlpideDB *db, int projectId, int typeId, string name);
int    DbGetComponentId(AlpideDB *db, int typeId, string name);
int    DbGetListOfChildren(AlpideDB *db, int Id, std::vector<TChild> &children,
                           bool chipsOnly = false);
int    DbGetComponentActivity(AlpideDB *db, int compId, int activityTypeId);
bool   DbAddParameter(AlpideDB *db, ActivityDB::activity &activity, string name, float value,
                      std::string file);
void   DbAddAttachment(AlpideDB *db, ActivityDB::activity &activity, TAttachmentType attType,
                       string localName, string remoteName);
void   DbAddMember(AlpideDB *db, ActivityDB::activity &activity, int memberId);
bool   FileExists(string fileName);
string CreateActivityName(string compName, TScanConfig *config);
#endif
