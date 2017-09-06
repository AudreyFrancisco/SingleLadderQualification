#include "DBHelpers.h"

int DbGetMemberId (AlpideDB *db, string name)
{
  MemberDB                              *memberDB = new MemberDB (db);
  static std::vector <MemberDB::member>  memberList;

  // memberList should not change, so read only once
  if (memberList.size() == 0) memberDB->GetList(PROJECT_ID, &memberList);

  for (unsigned int i = 0; i < memberList.size(); i++) {
    if (name == memberList.at(i).FullName) return memberList.at(i).ID;
  }

  return -1;
}


int DbGetParameterId (AlpideDB *db, int activityTypeId, string name)
{
  //ActivityDB                               *activityDB = new ActivityDB (db);
  //static int                                myActTypeId;
  //static std::vector <ActivityDB::activity> activityList;  

  // for lazy evaluation, here one has to check also that activity ID has not changed;
  //if ((activityList.size() == 0) || (activityTypeId != myActTypeId)) {
  //  myActTypeId  = activityTypeId;
  //  activityList = activityDB->GetActivityTypeList(PROJECT_ID);
  //}

  //for (unsigned int i = 0; i < activityList.size(); i++) {
  //  if (name == activityList.at(i).Name) return activityList.at(i).ID;
  //}
  return -1;
}


bool DbAddParameter (AlpideDB *db, ActivityDB::activity activity, string name, float value) 
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


void DbAddAttachment (AlpideDB *db, ActivityDB::activity activity, int attCategory, string localName, string remoteName)
{
  ActivityDB::attach attachment;

  attachment.ID             = activity.ID;
  attachment.User           = activity.User;
  attachment.Category       = attCategory;
  attachment.LocalFileName  = localName;
  attachment.RemoteFileName = remoteName;

  activity.Attachments.push_back(attachment);
}




