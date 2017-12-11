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


// returns the ID of the previous activity
// parameter name is the name of the current activity
// if the previous activity acts on the children, onChildren is set true
int  DbGetPrevActivityTypeId (AlpideDB *db, string name, bool &onChildren)
{
  // example:
  if (name.compare ("OB HIC Endurance Test") == 0) {
    onChildren = false;
    return DbGetActivityTypeId(db, "OB HIC Qualification Test");
  }
  onChildren = false;
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
      return parameterList.at(i).ParameterID;
    }
  }
  return -1;
}


int  DbGetAttachmentTypeId (AlpideDB *db, string name) 
{
  ActivityDB                                     *activityDB = new ActivityDB (db);
  static std::vector <ActivityDB::attachmentType> attTypeList;

  if (attTypeList.size() == 0) attTypeList = *(activityDB->GetAttachmentTypeList());

  //  std::cout << "TypeListSize: " << attTypeList.size() << std::endl;
  for (unsigned int i = 0; i < attTypeList.size(); i++) {
    //    std::cout << "  Searching " << name << ", found " << attTypeList.at(i).Category << std::endl;
    if (name == attTypeList.at(i).Category) {
      return attTypeList.at(i).ID;
    }
  }
  return -1;
}


int DbGetResultId (AlpideDB *db, int activityTypeId, THicClassification classification)
{
  ActivityDB                                 *activityDB = new ActivityDB (db);
  static std::vector <ActivityDB::resultType> resultTypeList;

  if (resultTypeList.size() == 0) resultTypeList = *(activityDB->GetResultList(activityTypeId));

  for (unsigned int i = 0; i < resultTypeList.size(); i++) {
    string Name = resultTypeList.at(i).Name;
    switch (classification) {
    case CLASS_GREEN:
      if (Name.find("GOLD") != string::npos) 
        return resultTypeList.at(i).ID; 
    case CLASS_ORANGE:
      if (Name.find("SILVER") != string::npos) 
        return resultTypeList.at(i).ID; 
    case CLASS_PARTIAL:
      if (Name.find("partially") != string::npos) 
        return resultTypeList.at(i).ID; 
    case CLASS_RED:
      if (Name.find("not") != string::npos) 
        return resultTypeList.at(i).ID; 
    default:
      break;
    }
  }
  return -1;
}


int DbGetResultId (AlpideDB *db, int activityTypeId, string resultName) 
{
  ActivityDB                                 *activityDB = new ActivityDB (db);
  static std::vector <ActivityDB::resultType> resultTypeList;

  if (resultTypeList.size() == 0) resultTypeList = *(activityDB->GetResultList(activityTypeId));

  for (unsigned int i = 0; i < resultTypeList.size(); i++) {
    if (resultName == resultTypeList.at(i).Name) {
      return resultTypeList.at(i).ID;
    }
  }
  return -1;
}


int DbGetStatusId (AlpideDB *db, int activityTypeId, string statusCode)
{
  ActivityDB                                 *activityDB = new ActivityDB (db);
  static std::vector <ActivityDB::statusType> statusTypeList;

  if (statusTypeList.size() == 0) statusTypeList = *(activityDB->GetStatusList(activityTypeId));

  for (unsigned int i = 0; i < statusTypeList.size(); i++) {
    if (statusCode == statusTypeList.at(i).Code) {
      return statusTypeList.at(i).ID;
    }
  }

  return -1;
}


int DbGetComponentTypeId  (AlpideDB *db, int projectId, string name)
{
  ComponentDB *componentDB = new ComponentDB (db);
  static std::vector <ComponentDB::componentType> componentTypeList;

  if (componentTypeList.size() == 0) componentDB->GetTypeList(projectId, &componentTypeList);

  for (unsigned int i = 0; i < componentTypeList.size(); i++) {
    if (name == componentTypeList.at(i).Name) {
      return componentTypeList.at(i).ID;
    }
  }
  return -1;
}


//TODO: complete list of tests
int DbGetComponentId (AlpideDB *db, int projectId, int typeId, string name)
{
  ComponentDB *componentDB = new ComponentDB (db);
  static std::vector <ComponentDB::componentShort> componentList;  

  if (componentList.size() == 0) componentDB->GetListByType(projectId, typeId, &componentList);

  for (unsigned int i = 0; i < componentList.size(); i++) {
    if (name == componentList.at(i).ComponentID) {
      return componentList.at(i).ID;
    }
  }

  return -1;
}  


// TODO: check; need also position?
int DbGetListOfChildren     (AlpideDB *db, int Id, std::vector<int> &children)
{
  ComponentDB               *componentDB = new ComponentDB (db);
  ComponentDB::componentLong component;

  children    .clear();
  componentDB->Read(Id, &component);

  for (unsigned int i = 0; i < component.Composition.size(); i++) {
    ComponentDB::compComposition child = component.Composition.at(i);
    children.push_back(child.Component.ID);
  }
  return children.size();
}


// looks for a component of type activityTypeId in the history of component compId
int DbGetComponentActivity  (AlpideDB *db, int compId, int activityTypeId)
{
  ComponentDB *componentDB = new ComponentDB (db);
  std::vector <ComponentDB::compActivity> activities;
  
  componentDB->GetComponentActivities(compId, &activities);

  for (unsigned int i = 0; i < activities.size(); i++) {
    if (activities.at(i).Type == activityTypeId) {
      return activities.at(i).ID;
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

  parameter.ID                = activity.ID; // will be set correctly in ActivityDB::Create
  parameter.ActivityParameter = paramId;
  parameter.User              = activity.User;
  parameter.Value             = value;

  activity.Parameters.push_back(parameter);
  return true;
}


void DbAddMember (AlpideDB *db, ActivityDB::activity &activity, int memberId) 
{
  ActivityDB::member member;

  member.ID            = activity.ID; // will be set correctly in ActivityDB::Create
  member.ProjectMember = memberId;
  member.Leader        = 0;           // 1 if site leader for this activity; not yet defined
  member.User          = -22;         // for some reason...
  
  activity.Members.push_back(member);
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
    attachment.Category = DbGetAttachmentTypeId (db, "HIC Log File");
    break;
  case attachErrors:
    attachment.Category = DbGetAttachmentTypeId (db, "HIC Error Log");
    break;
  case attachConfig:
    attachment.Category = DbGetAttachmentTypeId (db, "HIC Config File");
    break;    
  }

  std::cout << "Attaching file " << localName << " with remote name " << remoteName << " and category " << attachment.Category << std::endl;

  attachment.ID             = activity.ID;
  attachment.User           = activity.User;
  attachment.LocalFileName  = localName;
  attachment.RemoteFileName = remoteName;

  activity.Attachments.push_back(attachment);
}


string CreateActivityName (string compName, TTestType test) 
{
  string testName;
  switch (test) {
  case OBQualification: 
    testName = string ("OB Qualification Test ");
    break;
  case OBEndurance: 
    testName = string ("OB Endurance Test ");
    break;
  case OBReception: 
    testName = string ("OB Reception Test ");
    break;
  case OBHalfStaveOL: 
    testName = string ("OL Half-Stave Test ");
    break;
  case OBHalfStaveML: 
    testName = string ("ML Half-Stave Test ");
    break;
  case IBQualification:
    testName = string ("IB Qualification Test ");
    break;
  case IBEndurance: 
    testName = string ("IB Endurance Test ");
    break;
  case IBStave: 
    testName = string ("IB Stave Test ");
    break;
  case IBStaveEndurance: 
    testName = string ("IB Stave Endurance Test ");
    break;
  default:
    testName = string ("");
    break;
  }
  return (testName + compName);
}



