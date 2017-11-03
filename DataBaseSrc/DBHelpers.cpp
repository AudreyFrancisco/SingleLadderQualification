#include "DBHelpers.h"
#include <fstream>

int DbGetActivityTypeId (AlpideDB *db, string name)
{
  ActivityDB                                   *activityDB = new ActivityDB (db);
  static std::vector <ActivityDB::activityType> activityList;

  if (activityList.size() == 0) activityList = *(activityDB->GetActivityTypeList(db->GetProjectId()));

  for (unsigned int i = 0; i < activityList.size(); i++) {
    if (name == activityList.at(i).Name) return activityList.at(i).ID;
  }

  return -1;
}


int DbGetMemberId (AlpideDB *db, string name)
{
  MemberDB                              *memberDB = new MemberDB (db);
  static std::vector <MemberDB::member>  memberList;

  // memberList should not change, so read only once
  if (memberList.size() == 0) memberDB->GetList(db->GetProjectId(), &memberList);

  for (unsigned int i = 0; i < memberList.size(); i++) {
    if (name == memberList.at(i).FullName) return memberList.at(i).ID;
  }

  return -1;
}


int DbGetParameterId (AlpideDB *db, int activityTypeId, string name)
{
  ActivityDB                                    *activityDB = new ActivityDB (db);
  static int                                     myActTypeId;
  static std::vector <ActivityDB::parameterType> parameterList;  

  // for lazy evaluation, check also that activity ID has not changed;
  if ((parameterList.size() == 0) || (activityTypeId != myActTypeId)) {
    myActTypeId   = activityTypeId;
    parameterList = *(activityDB->GetParameterTypeList(myActTypeId));
  }

  for (unsigned int i = 0; i < parameterList.size(); i++) {
    if (name == parameterList.at(i).Name) {
      return parameterList.at(i).ID;
    }
  }
  return -1;
}


int  DbGetAttachmentTypeId (AlpideDB *db, string name) 
{
  ActivityDB                                     *activityDB = new ActivityDB (db);
  static std::vector <ActivityDB::attachmentType> attTypeList;

  if (attTypeList.size() == 0) attTypeList = *(activityDB->GetAttachmentTypeList());

  for (unsigned int i = 0; i < attTypeList.size(); i++) {
    if (name == attTypeList.at(i).Category) {
      return attTypeList.at(i).ID;
    }
  }
  return -1;
}


bool DbAddParameter (AlpideDB *db, ActivityDB::activity &activity, string name, float value) 
{
  ActivityDB::parameter parameter;
  int                   paramId = DbGetParameterId (db, activity.Type, name);
  if (paramId < 0) {
    std::cout << "Warning: Parameter not found, ignored: " << name << std::endl;
    return false;
  }

  parameter.ID                = activity.ID;
  parameter.ActivityParameter = paramId;
  parameter.User              = activity.User;
  parameter.Value             = value;

  activity.Parameters.push_back(parameter);
  return true;
}


bool FileExists(string fileName)
{
    ifstream f(fileName.c_str());
    return f.good();
}


void DbAddAttachment (AlpideDB *db, ActivityDB::activity &activity, TAttachmentType attType, string localName, string remoteName)
{
  if (!FileExists(localName)) {
    std::cout << "Warning: did not find file " << localName << ", ignored" << std::endl;
    return;
  }
  ActivityDB::attach attachment;

  switch (attType) {
  case attachResult:
    attachment.Category = DbGetAttachmentTypeId (db, "HIC Result File");
    break;
  case attachLog: 
    attachment.Category = DbGetAttachmentTypeId (db, "Test Log File");
    break;
  case attachErrors:
    attachment.Category = DbGetAttachmentTypeId (db, "Error Output File");
    break;
  }

  attachment.ID             = activity.ID;
  attachment.User           = activity.User;
  attachment.LocalFileName  = localName;
  attachment.RemoteFileName = remoteName;

  activity.Attachments.push_back(attachment);
}




