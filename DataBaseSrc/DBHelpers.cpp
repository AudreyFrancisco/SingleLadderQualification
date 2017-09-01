#include "DBHelpers.h"

int DbGetMemberId (AlpideDB *db, string Name)
{
  MemberDB                       *memberDB = new MemberDB (db);
  std::vector <MemberDB::member>  memberList;

  memberDB->GetList(PROJECT_ID, &memberList);

  for (unsigned int i = 0; i < memberList.size(); i++) {
    if (Name == memberList.at(i).FullName) return memberList.at(i).ID;
  }

  return -1;
}


int DbGetParameterId (AlpideDB *db, int ActivityTypeId, string Name)
{
  // TODO: Implement
  return 0;
}


bool DbAddParameter (AlpideDB *db, ActivityDB::activity activity, string name, float value) 
{
  ActivityDB::parameter parameter;
  int                   paramId = DbGetParameterId (db, activity.Type, name);
  if (paramId < 0) {
    std::cout << "Warning: Parameter not found, ignored: " << name << std::endl;
    return false;
  }

  parameter.ActivityParameter = paramId;
  parameter.User              = activity.User;
  parameter.Value             = value;

  activity.Parameters.push_back(parameter);
  return true;
}


