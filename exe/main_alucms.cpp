
#include <iostream>

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"

int main() {
  // the default setting mode, it depends from compilation flags
  AlpideDB *theDB = new AlpideDB();

  // Verify if the connection is established
  if (!theDB->isDBConnected()) {
    exit(1); // here we can decide to abort or try to recover ...
  }

  /* If the query domain is not the standard
  theDB->SetQueryDomain("https://test-alucmsapi.web.cern.ch/AlucmswebAPI.asmx");
  */

  /* -- X509 / libcurl setup
  AlpideDBManager *theDBManager = theDB->GetManagerHandle();
  theDBManager->Init("https://test-alucmsapi.web.cern.ch", // the root URL of the web server
                                  "FrancoAntonio",  // the name associated to the CErt/Key in the DB
                                  ".",  // the path where the: cert8.db,key3.db,secmod.db are stored
                                  "alpide4me");  // the password used to access the NSS DB
  */

  /* -- X509 / curl shell setup
  AlpideDBManager *theDBManager = theDB->GetManagerHandle();
  theDBManager->Init(""https://test-alucmsapi.web.cern.ch",
                                          "/home/fap/.globus/usercert.pem",  // the path of the
  Certificate file
                                          "/home/fap/.globus/userkey.pem",  // the path of the Key
  file
                                          "/etc/ssl/certs");  // the path of the CA certificates
  database


  //AlpideTable::response *theResult; // caused warning
*/
  // Access the projects Data Base
  ProjectDB *theProjTable = new ProjectDB(theDB);
  vector<ProjectDB::project> ProjList;
  cout << "------  PROJECTS DATABASE -----------" << endl;
  theProjTable->GetList(&ProjList);
  cout << theProjTable->DumpResponse() << endl;

  cout << "The list of Projects is " << ProjList.size() << "  long " << endl;
  for (unsigned int i = 0; i < ProjList.size(); i++) {
    cout << i << " " << ProjList.at(i).ID << "  " << ProjList.at(i).Name << endl;
  }

  // Access the members Data Base
  MemberDB *theMembTable = new MemberDB(theDB);
  vector<MemberDB::member> MemberList;
  MemberDB::member oneMember;
  cout << endl << "------  MEMBERS DATABASE -----------" << endl;
  int ProjectID = 0;
  printf(" Input the Project id :");
  int err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return -1;
  }

  theMembTable->GetList(ProjectID, &MemberList);
  cout << theMembTable->DumpResponse() << endl;

  cout << "The list of Members of Project=" << ProjectID << " is " << MemberList.size() << "  long "
       << endl;
  for (unsigned int i = 0; i < MemberList.size(); i++) {
    cout << i << " " << MemberList.at(i).ID << "  " << MemberList.at(i).PersonalID << "  "
         << MemberList.at(i).FullName << endl;
  }

  // Access the components Data Base
  ComponentDB *theCompTable = new ComponentDB(theDB);
  vector<ComponentDB::componentType> ComponentList;
  ComponentDB::componentType oneComponent;
  cout << endl << "------  COMPONENT DATABASE -----------" << endl;
  ProjectID = 0;
  printf(" Input the Project id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return -1;
  }

  theCompTable->GetTypeList(ProjectID, &ComponentList);
  cout << theCompTable->DumpResponse() << endl;

  cout << "The list of Type Components of Project=" << ProjectID << " is " << ComponentList.size()
       << "  long " << endl;
  for (unsigned int i = 0; i < ComponentList.size(); i++) {
    cout << i << " " << ComponentList.at(i).ID << "  " << ComponentList.at(i).Code << "  "
         << ComponentList.at(i).Name << endl;
  }

  cout << endl << "------  COMPONENT Type SPEC -----------" << endl;
  int ComponentID = 0;
  printf(" Input the Component id : ");
  err = scanf("%d", &ComponentID);
  if (err != 1) {
    printf(" Failed to read the  component id ");
    return -1;
  }

  theCompTable->GetType(ComponentID, &oneComponent);
  cout << theCompTable->DumpResponse() << endl;

  cout << theCompTable->Print(&oneComponent) << endl;

  cout << endl << "------  NEW METHODS ACTIVITY -----------" << endl << endl;
  ActivityDB *theActivityTable = new ActivityDB(theDB);

  cout << endl << "------ Activity type-----------" << endl;
  ProjectID = 0;
  printf(" Input the Project id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return -1;
  }
  vector<ActivityDB::activityType> *act = theActivityTable->GetActivityTypeList(ProjectID);
  for (unsigned int i = 0; i < act->size(); i++) {
    cout << endl << act->at(i).ID << "\t" << act->at(i).Name << "\t" << act->at(i).Description;
  }

  cout << endl << "------ Parameter type-----------" << endl;
  int ActivityID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ActivityID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return -1;
  }
  vector<ActivityDB::parameterType> *par = theActivityTable->GetParameterTypeList(ActivityID);
  for (unsigned int i = 0; i < par->size(); i++) {
    cout << endl << par->at(i).ID << "\t" << par->at(i).ParameterID << "\t" << par->at(i).Name
         << "\t" << par->at(i).Description;
  }

  cout << endl << "------ locationType type-----------" << endl;
  ProjectID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return -1;
  }
  vector<ActivityDB::locationType> *loc = theActivityTable->GetLocationTypeList(ProjectID);
  for (unsigned int i = 0; i < loc->size(); i++) {
    cout << endl << loc->at(i).ID << " " << loc->at(i).Name;
  }

  cout << endl << "------ resultType type-----------" << endl;
  ProjectID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return -1;
  }
  vector<ActivityDB::resultType> *res = theActivityTable->GetResultList(ProjectID);
  for (unsigned int i = 0; i < res->size(); i++) {
    cout << endl << res->at(i).ID << " " << res->at(i).Name;
  }

  cout << endl << "------ statusType type-----------" << endl;
  ProjectID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return -1;
  }
  vector<ActivityDB::statusType> *sta = theActivityTable->GetStatusList(ProjectID);
  for (unsigned int i = 0; i < sta->size(); i++) {
    cout << endl << sta->at(i).ID << " " << sta->at(i).Code << "   " << sta->at(i).Description;
  }

  cout << endl << "------ componentType type-----------" << endl;
  int ActTypeID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ActTypeID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return -1;
  }
  vector<ActivityDB::actTypeCompType> *com = theActivityTable->GetComponentTypeList(ActTypeID);
  for (unsigned int i = 0; i < com->size(); i++) {
    cout << endl << com->at(i).ID << " " << com->at(i).Type.Name;
  }

  cout << endl << "-----------" << endl;

  //	int ActivityID, ComponentID, ComponentTypeID, UserID;
  //	theActivityTable->AssignComponent(ActivityID, ComponentID, ComponentTypeID, UserID);
  //	cout << theActivityTable->DumpResponse() << endl;

  cout << endl << "------  CREATE ACTIVITY -----------" << endl;

  // --- create activity test
  //	ActivityDB *theActivityTable = new ActivityDB(theDB);
  ActivityDB::activity Attiv;
  ActivityDB::member Mem;
  ActivityDB::parameter Par;
  ActivityDB::attach Att;

  // tm ts; ts.tm_year = 2017 - 1900; ts.tm_mon = 6; ts.tm_mday = 13; // caused warning

  Attiv.Location = 161;
  Attiv.EndDate = time(NULL);
  Attiv.Lot = "TestAntonio";
  Attiv.Name = "Test_db";
  Attiv.Position = "Position1";
  Attiv.Result = 101;
  Attiv.StartDate = time(NULL); // mktime(&ts);
  Attiv.Status = 83;            // open
  Attiv.Type = 221;
  Attiv.User = 1;

  Mem.Leader = 0;
  Mem.ProjectMember = 584;
  Mem.User = 8791;
  Attiv.Members.push_back(Mem);
  Mem.ProjectMember = 201;
  Mem.User = 4702;
  Attiv.Members.push_back(Mem);

  for (int i = 0; i < 3; i++) {
    Par.ActivityParameter = 1;
    Par.User = 1 + i;
    Par.Value = 9.5 + i;
    Attiv.Parameters.push_back(Par);
  }

  for (int i = 0; i < 3; i++) {
    Att.Category = 41;
    Att.LocalFileName = "AttachTest";
    Att.LocalFileName.append(std::to_string(i));
    Att.LocalFileName.append(".txt");
    Att.User = i + 1;
    Att.RemoteFileName = "AttTe";
    Att.RemoteFileName.append(std::to_string(i));
    Att.RemoteFileName.append(".txt");
    Attiv.Attachments.push_back(Att);
  }

  theActivityTable->Create(&Attiv);
  cout << theActivityTable->DumpResponse() << endl;
}
