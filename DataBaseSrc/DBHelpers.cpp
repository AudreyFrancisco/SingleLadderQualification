#include "DBHelpers.h"
#include <fstream>

int DbGetActivityTypeId(AlpideDB *db, string name) {
  ActivityDB *activityDB = new ActivityDB(db);
  static std::vector<ActivityDB::activityType> activityList;

  if (activityList.size() == 0)
    activityList = *(activityDB->GetActivityTypeList(db->GetProjectId()));

  for (unsigned int i = 0; i < activityList.size(); i++) {
    if (name == activityList.at(i).Name)
      return activityList.at(i).ID;
  }

  return -1;
}

// returns the ID of the previous activity
// parameter name is the name of the current activity
// if the previous activity acts on the children, onChildren is set true
int DbGetPrevActivityTypeId(AlpideDB *db, string name, bool &onChildren) {
  // example:
  if (name.compare("OB HIC Endurance Test") == 0) {
    onChildren = false;
    return DbGetActivityTypeId(db, "OB HIC Qualification Test");
  }
  onChildren = false;
  return -1;
}

int DbGetMemberId(AlpideDB *db, string name) {
  MemberDB *memberDB = new MemberDB(db);
  static std::vector<MemberDB::member> memberList;

  // memberList should not change, so read only once
  if (memberList.size() == 0)
    memberDB->GetList(db->GetProjectId(), &memberList);

  for (unsigned int i = 0; i < memberList.size(); i++) {
    if (name == memberList.at(i).FullName)
      return memberList.at(i).ID;
  }

  return -1;
}

int DbGetParameterId(AlpideDB *db, int activityTypeId, string name) {
  ActivityDB *activityDB = new ActivityDB(db);
  static int myActTypeId;
  static std::vector<ActivityDB::parameterType> parameterList;

  // for lazy evaluation, check also that activity ID has not changed;
  if ((parameterList.size() == 0) || (activityTypeId != myActTypeId)) {
    myActTypeId = activityTypeId;
    parameterList = *(activityDB->GetParameterTypeList(myActTypeId));
  }

  for (unsigned int i = 0; i < parameterList.size(); i++) {
    if (name == parameterList.at(i).Name) {
      return parameterList.at(i).ParameterID;
    }
  }
  return -1;
}

int DbGetAttachmentTypeId(AlpideDB *db, string name) {
  ActivityDB *activityDB = new ActivityDB(db);
  static std::vector<ActivityDB::attachmentType> attTypeList;

  if (attTypeList.size() == 0)
    attTypeList = *(activityDB->GetAttachmentTypeList());

  //  std::cout << "TypeListSize: " << attTypeList.size() << std::endl;
  for (unsigned int i = 0; i < attTypeList.size(); i++) {
    //    std::cout << "  Searching " << name << ", found " << attTypeList.at(i).Category <<
    // std::endl;
    if (name == attTypeList.at(i).Category) {
      return attTypeList.at(i).ID;
    }
  }
  return -1;
}

int DbGetResultId(AlpideDB *db, int activityTypeId, THicClassification classification) {
  ActivityDB *activityDB = new ActivityDB(db);
  static int myActTypeId;
  static std::vector<ActivityDB::resultType> resultTypeList;

  if ((resultTypeList.size() == 0) || (activityTypeId != myActTypeId)) {
    myActTypeId = activityTypeId;
    resultTypeList = *(activityDB->GetResultList(myActTypeId));
  }

  for (unsigned int i = 0; i < resultTypeList.size(); i++) {
    string Name = resultTypeList.at(i).Name;
    switch (classification) {
    case CLASS_GREEN:
      if (Name.find("GOLD") != string::npos)
        return resultTypeList.at(i).ID;
      break;
    case CLASS_ORANGE:
      if (Name.find("SILVER") != string::npos)
        return resultTypeList.at(i).ID;
      break;
    case CLASS_PARTIAL:
      if (Name.find("partially") != string::npos)
        return resultTypeList.at(i).ID;
      break;
    case CLASS_RED:
      if (Name.find("not") != string::npos)
        return resultTypeList.at(i).ID;
      break;
    default:
      break;
    }
  }
  return -1;
}

// counts the activities of a given type for a given component
// assumes that the component name is contained in the activity name
int DbCountActivities(AlpideDB *db, int activityTypeId, string compName) {
  ActivityDB *activityDB = new ActivityDB(db);
  static int myActTypeId;
  static std::vector<ActivityDB::activityShort> activityList;
  int count = 0;

  if ((activityList.size() == 0) || (activityTypeId != myActTypeId)) {
    myActTypeId = activityTypeId;
    activityList = *(activityDB->GetActivityList(db->GetProjectId(), activityTypeId));
  }
  for (unsigned int i = 0; i < activityList.size(); i++) {
    if (activityList.at(i).Name.find(compName) != string::npos)
      count++;
  }

  return count;
}

// returns a list of activity IDs of a given type for a given component
// assumes that the component name is contained in the activity name
std::vector<int> DbGetActivityIds(AlpideDB *db, int activityTypeId, string compName) {
  ActivityDB *activityDB = new ActivityDB(db);
  static int myActTypeId;
  static std::vector<ActivityDB::activityShort> activityList;
  std::vector<int> result;

  result.clear();
  if ((activityList.size() == 0) || (activityTypeId != myActTypeId)) {
    myActTypeId = activityTypeId;
    activityList = *(activityDB->GetActivityList(db->GetProjectId(), activityTypeId));
  }
  for (unsigned int i = 0; i < activityList.size(); i++) {
    if (activityList.at(i).Name.find(compName) != string::npos)
      result.push_back(activityList.at(i).ID);
  }

  return result;
}


bool DbFindParamValue (vector<ActivityDB::actParameter> pars, string parName, float &parValue)
{
  for (unsigned int i = 0; i < pars.size(); i++) {
    if (pars.at(i).Type.Parameter.Name == parName) {
      parValue = pars.at(i).Value;
      return true;
    }
  }
  return false;
}


int DbIsNewer (ActivityDB::activityLong act0, ActivityDB::activityLong act1)
{
  float time0, time1;
  // first check the start dates; 
  // not sure about the precision of the member StartDate, but it should be either equal 
  // or have a difference of at least 1 day = 86400 sec
  if (difftime (act0.StartDate, act1.StartDate) < -86000) return 1;  // date of act0 before date of act1
  if (difftime (act0.StartDate, act1.StartDate) > 86000) return 0;

  // if start dates are equal, check the time parameter
  if (DbFindParamValue(act0.Parameters, "Time", time0) && DbFindParamValue(act1.Parameters, "Time", time1)) {
    if (time0 < time1) return 1;
    else if (time1 < time0) return 0;
  }
  // if still no decision, use the activity ids
  if (act0.ID < act1.ID) return 1;
  return 0;
}


// get the latest activity of a certain type, performed on a given component 
// returns false if no activity found, true otherwise
bool DbGetLatestActivity(AlpideDB *db, int activityTypeId, string compName, ActivityDB::activityLong &activity)
{
  std::vector<int>                      ids        = DbGetActivityIds(db, activityTypeId, compName);
  std::vector<ActivityDB::activityLong> activities = DbGetActivities (db, ids);

  int latestIdx = 0;

  if (activities.size() == 0) return false;
  for (unsigned int i = 0; i < activities.size(); i++) {
    if (DbIsNewer(activities.at(latestIdx), activities.at(i)) == 1) {
      latestIdx = i;
    }
  }

  activity = activities.at(latestIdx);
  return true;
}


// returns the vector of activity structures corresponding to the vector of Ids 
std::vector<ActivityDB::activityLong> DbGetActivities(AlpideDB *db, std::vector<int> activityIds)
{
  ActivityDB *activityDB = new ActivityDB (db);
  std::vector<ActivityDB::activityLong> result;

  result.clear();
  for (unsigned int i = 0; i < activityIds.size(); i++) {
    ActivityDB::activityLong activity;
    AlpideTable::response *response = activityDB->Read(activityIds.at(i), &activity);
    if (response->ErrorCode == AlpideTable::NoError)
      result.push_back(activity);
  }
  return result;
}

int DbGetResultId(AlpideDB *db, int activityTypeId, string resultName) {
  ActivityDB *activityDB = new ActivityDB(db);
  static int myActTypeId;
  static std::vector<ActivityDB::resultType> resultTypeList;

  if ((resultTypeList.size() == 0) || (activityTypeId != myActTypeId)) {
    myActTypeId = activityTypeId;
    resultTypeList = *(activityDB->GetResultList(myActTypeId));
  }

  for (unsigned int i = 0; i < resultTypeList.size(); i++) {
    if (resultName == resultTypeList.at(i).Name) {
      return resultTypeList.at(i).ID;
    }
  }
  return -1;
}

int DbGetStatusId(AlpideDB *db, int activityTypeId, string statusCode) {
  ActivityDB *activityDB = new ActivityDB(db);
  static int myActTypeId;
  static std::vector<ActivityDB::statusType> statusTypeList;

  if ((statusTypeList.size() == 0) || (activityTypeId != myActTypeId)) {
    myActTypeId = activityTypeId;
    statusTypeList = *(activityDB->GetStatusList(myActTypeId));
  }

  for (unsigned int i = 0; i < statusTypeList.size(); i++) {
    if (statusCode == statusTypeList.at(i).Code) {
      return statusTypeList.at(i).ID;
    }
  }

  return -1;
}

int DbGetComponentTypeId(AlpideDB *db, int projectId, string name) {
  ComponentDB *componentDB = new ComponentDB(db);
  static std::vector<ComponentDB::componentType> componentTypeList;

  if (componentTypeList.size() == 0)
    componentDB->GetTypeList(projectId, &componentTypeList);

  for (unsigned int i = 0; i < componentTypeList.size(); i++) {
    if (name == componentTypeList.at(i).Name) {
      return componentTypeList.at(i).ID;
    }
  }
  return -1;
}

// method returns the activity component type ID
// (i.e. the id of the component as in or out component of the given activity)
// the general component id is written into the variable componentId
int DbGetActComponentTypeId(AlpideDB *db, int activityTypeId, int &componentId, string Direction) {
  ActivityDB *activityDB = new ActivityDB(db);
  static int myActTypeId;
  static std::vector<ActivityDB::actTypeCompType> actCompTypeList;

  if ((actCompTypeList.size() == 0) || (activityTypeId != myActTypeId)) {
    myActTypeId = activityTypeId;
    actCompTypeList = *(activityDB->GetComponentTypeList(myActTypeId));
  }

  for (unsigned int i = 0; i < actCompTypeList.size(); i++) {
    if (Direction == actCompTypeList.at(i).Direction) {
      componentId = actCompTypeList.at(i).Type.ID;
      return actCompTypeList.at(i).ID;
    }
  }
  return -1;
}

// TODO: complete list of tests
int DbGetComponentId(AlpideDB *db, int projectId, int typeId, string name) {
  ComponentDB *componentDB = new ComponentDB(db);
  static int myTypeId;
  static std::vector<ComponentDB::componentShort> componentList;

  if ((componentList.size() == 0) || (typeId != myTypeId)) {
    myTypeId = typeId;
    componentDB->GetListByType(projectId, myTypeId, &componentList);
  }

  for (unsigned int i = 0; i < componentList.size(); i++) {
    if (name == componentList.at(i).ComponentID) {
      return componentList.at(i).ID;
    }
  }

  return -1;
}

// TODO: check; need also position?
int DbGetListOfChildren(AlpideDB *db, int Id, std::vector<int> &children) {
  ComponentDB *componentDB = new ComponentDB(db);
  ComponentDB::componentLong component;

  children.clear();
  componentDB->Read(Id, &component);

  for (unsigned int i = 0; i < component.Composition.size(); i++) {
    ComponentDB::compComposition child = component.Composition.at(i);
    children.push_back(child.Component.ID);
  }
  return children.size();
}

// looks for a component of type activityTypeId in the history of component compId
int DbGetComponentActivity(AlpideDB *db, int compId, int activityTypeId) {
  ComponentDB *componentDB = new ComponentDB(db);
  std::vector<ComponentDB::compActivity> activities;

  componentDB->GetComponentActivities(compId, &activities);

  for (unsigned int i = 0; i < activities.size(); i++) {
    if (activities.at(i).Type == activityTypeId) {
      return activities.at(i).ID;
    }
  }
  return -1;
}

bool DbAddParameter(AlpideDB *db, ActivityDB::activity &activity, string name, float value) {
  ActivityDB::parameter parameter;
  int paramId = DbGetParameterId(db, activity.Type, name);
  if (paramId < 0) {
    std::cout << "Warning: Parameter not found, ignored: " << name << std::endl;
    return false;
  }

  parameter.ID = activity.ID; // will be set correctly in ActivityDB::Create
  parameter.ActivityParameter = paramId;
  parameter.User = activity.User;
  parameter.Value = value;

  activity.Parameters.push_back(parameter);
  return true;
}

void DbAddMember(AlpideDB *db, ActivityDB::activity &activity, int memberId) {
  ActivityDB::member member;

  member.ID = activity.ID; // will be set correctly in ActivityDB::Create
  member.ProjectMember = memberId;
  member.Leader = 0; // 1 if site leader for this activity; not yet defined
  member.User = -22; // for some reason...

  activity.Members.push_back(member);
}

bool FileExists(string fileName) {
  ifstream f(fileName.c_str());
  return f.good();
}

void DbAddAttachment(AlpideDB *db, ActivityDB::activity &activity, TAttachmentType attType,
                     string localName, string remoteName) {
  if (!FileExists(localName)) {
    std::cout << "Warning: did not find file " << localName << ", ignored" << std::endl;
    return;
  }
  ActivityDB::attach attachment;

  switch (attType) {
  case attachResult:
    attachment.Category = DbGetAttachmentTypeId(db, "HIC Result File");
    break;
  case attachLog:
    attachment.Category = DbGetAttachmentTypeId(db, "HIC Log File");
    break;
  case attachErrors:
    attachment.Category = DbGetAttachmentTypeId(db, "HIC Error Log");
    break;
  case attachConfig:
    attachment.Category = DbGetAttachmentTypeId(db, "HIC Config File");
    break;
  }

  std::cout << "Attaching file " << localName << " with remote name " << remoteName
            << " and category " << attachment.Category << std::endl;

  attachment.ID = activity.ID;
  attachment.User = activity.User;
  attachment.LocalFileName = localName;
  attachment.RemoteFileName = remoteName;

  activity.Attachments.push_back(attachment);
}

string CreateActivityName(string compName, TTestType test) {
  string testName;
  switch (test) {
  case OBQualification:
    testName = string("OB Qualification Test ");
    break;
  case OBEndurance:
    testName = string("OB Endurance Test ");
    break;
  case OBReception:
    testName = string("OB Reception Test ");
    break;
  case OBHalfStaveOL:
    testName = string("OL Half-Stave Test ");
    break;
  case OBHalfStaveML:
    testName = string("ML Half-Stave Test ");
    break;
  case IBQualification:
    testName = string("IB Qualification Test ");
    break;
  case IBEndurance:
    testName = string("IB Endurance Test ");
    break;
  case IBStave:
    testName = string("IB Stave Test ");
    break;
  case IBStaveEndurance:
    testName = string("IB Stave Endurance Test ");
    break;
  default:
    testName = string("");
    break;
  }
  return (testName + compName);
}
