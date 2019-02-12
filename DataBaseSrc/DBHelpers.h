#ifndef DBHELPERS_H
#define DBHELPERS_H

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScanFactory.h"
#include <string>
#include <vector>

typedef enum { attachResult, attachLog, attachErrors, attachConfig, attachText } TAttachmentType;

typedef struct {
  int    Id;
  int    Type;
  string Name;
  string Position;
} TChild;

int DbGetMemberId(AlpideDB *db, string name);
// int  DbGetProjectId        (AlpideDB *db, string Name);
float            DbGetSoftwareVersion(AlpideDB *db);
int              DbGetParameterId(AlpideDB *db, int activityTypeId, string name);
int              DbGetResultId(AlpideDB *db, int activityTypeId, string resultName);
int              DbGetResultId(AlpideDB *db, int activityTypeId, THicClassification classification);
int              DbGetStatusId(AlpideDB *db, int activityTypeId, string statusCode);
int              DbCountActivities(AlpideDB *db, int activityTypeId, string compName);
std::vector<int> DbGetActivityIds(AlpideDB *db, int activityTypeId, string compName);
std::vector<ActivityDB::activityLong> DbGetActivities(AlpideDB *db, std::vector<int> activityIds);
bool                                  DbCheckActivityExists(AlpideDB *db, int activityId);
int                DbIsNewer(ActivityDB::activityLong act0, ActivityDB::activityLong act1);
int                DbIsNewer(ComponentDB::compActivity act0, ComponentDB::compActivity act1);
bool               DbGetLatestActivity(AlpideDB *db, int activityTypeId, string compName,
                                       ActivityDB::activityLong &activity);
int                DbGetActivityTypeId(AlpideDB *db, string name);
void               DbGetPreviousTests(AlpideDB *db, int compId, int activityTypeId,
                                      vector<ComponentDB::compActivity> &tests, bool &openAct, bool &impedance);
bool               DbCheckCompleteness(AlpideDB *db, int compId);
void               DbEliminateDoubles(vector<ComponentDB::compActivity> &tests);
void               DbGetAllTests(AlpideDB *db, int compId, vector<ComponentDB::compActivity> &tests,
                                 TScanType scanType, bool lastOnly);
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
string DbGetComponentName(AlpideDB *db, int typeId, int compId);
int    DbGetListOfChildren(AlpideDB *db, int Id, std::vector<TChild> &children,
                           bool chipsOnly = false);
int    DbGetPosition(AlpideDB *db, int parentId, int childId);
int    DbGetComponentActivity(AlpideDB *db, int compId, int activityTypeId);
bool   DbAddParameter(AlpideDB *db, ActivityDB::activity &activity, string name, float value,
                      std::string file);
void   DbAddAttachment(AlpideDB *db, ActivityDB::activity &activity, TAttachmentType attType,
                       string localName, string remoteName);
void   DbAddMember(AlpideDB *db, ActivityDB::activity &activity, int memberId);
bool   FileExists(string fileName);
string CreateActivityName(string compName, TScanConfig *config);
string GetServiceAccount(string institute, string &folder);
string GetEosPath(ActivityDB::activityLong activity, THicType hicType, bool doubleComp);
string GetTestDirName(TTestType TestType);
TTestType GetTestType(string activityTypeName);
bool      GetDctrlFileName(ActivityDB::activityLong activity, string &dataName, string &resultName);
bool GetDigitalFileName(ActivityDB::activityLong activity, int chip, int voltPercent, int backBias,
                        string &dataName, string &resultName);
bool GetNoiseFileName(ActivityDB::activityLong activity, bool masked, int backBias,
                      string &dataName, string &hitsName, string &resultName);
bool GetPowerFileName(ActivityDB::activityLong activity, bool &ivFound, string &ivName,
                      string &resultName);
bool GetThresholdFileName(ActivityDB::activityLong activity, int chip, bool nominal, int backBias,
                          string &dataName, string &resultName);

bool GetITHRTuneFileName(ActivityDB::activityLong activity, int chip, int backBias,
                         string &dataName, string &resultName);

bool GetVCASNTuneFileName(ActivityDB::activityLong activity, int chip, int backBias,
                          string &dataName, string &resultName);

#endif
