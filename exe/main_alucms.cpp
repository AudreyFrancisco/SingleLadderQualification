
#include <iostream>

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"

void projectDB(AlpideDB *theDB)
{
  int                        ProjectID    = 0;
  ProjectDB *                theProjTable = new ProjectDB(theDB);
  vector<ProjectDB::project> ProjList;
  cout << "------  PROJECTS DATABASE -----------" << endl;
  theProjTable->GetList(&ProjList);
  cout << theProjTable->DumpResponse() << endl;
  cout << "The list of Projects is " << ProjList.size() << "  long " << endl;
  for (unsigned int i = 0; i < ProjList.size(); i++) {
    cout << i << "\t" << ProjList.at(i).ID << "\t" << ProjList.at(i).Name << endl;
  }
  cout << "-------------------------------------" << endl;

  // Access the members Data Base
  MemberDB *               theMembTable = new MemberDB(theDB);
  vector<MemberDB::member> MemberList;
  MemberDB::member         oneMember;
  cout << endl << "------  MEMBERS DATABASE -----------" << endl;

  printf(" Input the Project id :");
  int err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return;
  }
  theMembTable->GetList(ProjectID, &MemberList);
  cout << theMembTable->DumpResponse() << endl;
  cout << "The list of Members of Project=" << ProjectID << " is " << MemberList.size() << "  long "
       << endl;
  for (unsigned int i = 0; i < MemberList.size(); i++) {
    cout << i << "\t" << MemberList.at(i).ID << "\t" << MemberList.at(i).PersonalID << "  "
         << MemberList.at(i).FullName << endl;
  }
  cout << "-------------------------------------" << endl;
  return;
}

void componentType(AlpideDB *theDB)
{
  int err;
  // Access the components Data Base
  ComponentDB *                      theCompTable = new ComponentDB(theDB);
  vector<ComponentDB::componentType> ComponentList;
  ComponentDB::componentType         oneComponent;
  cout << endl << "------  COMPONENT DATABASE -----------" << endl;
  int ProjectID = 0;
  printf(" Input the Project id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return;
  }

  theCompTable->GetTypeList(ProjectID, &ComponentList);
  cout << theCompTable->DumpResponse() << endl;
  cout << "The list of Type Components of Project=" << ProjectID << " is " << ComponentList.size()
       << "  long " << endl;
  for (unsigned int i = 0; i < ComponentList.size(); i++) {
    cout << i << "\t" << ComponentList.at(i).ID << "\t" << ComponentList.at(i).Code << "\t"
         << ComponentList.at(i).Name << endl;
  }
  cout << "-------------------------------------" << endl;
  cout << endl << "------  COMPONENT TYPE DUMP -----------" << endl;
  int ComponentID = 0;
  printf(" Input the Component Type id : ");
  err = scanf("%d", &ComponentID);
  if (err != 1) {
    printf(" Failed to read the  component type id ");
    return;
  }

  AlpideTable::response *theresult = theCompTable->GetType(ComponentID, &oneComponent);
  cout << theCompTable->DumpResponse() << endl;
  if (theresult->ID != 0)
    cout << theCompTable->Print(&oneComponent) << endl;
  else
    cout << "Component Type NOT FOUND !" << endl;
  cout << "-------------------------------------" << endl;
}

void componentDB(AlpideDB *theDB)
{
  // Access the components Data Base
  ComponentDB *              theCompTable = new ComponentDB(theDB);
  ComponentDB::componentType oneComponent;
  int                        err;
  cout << endl << "------  COMPONENT LIST -----------" << endl;
  int ProjectID = 0;
  printf(" Input the Project id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return;
  }
  int ComponentID = 0;
  printf(" Input the Component Type id : ");
  err = scanf("%d", &ComponentID);
  if (err != 1) {
    printf(" Failed to read the  component type id ");
    return;
  }
  vector<ComponentDB::componentShort> CompList;
  theCompTable->GetListByType(ProjectID, ComponentID, &CompList);
  cout << theCompTable->DumpResponse() << endl;
  cout << "The list of Components of type=" << ComponentID << " for the Project=" << ProjectID
       << " is " << CompList.size() << "  long " << endl;
  for (unsigned int i = 0; i < CompList.size(); i++) {
    cout << i << "\t" << CompList.at(i).ID << "\t" << CompList.at(i).ComponentID << "\t"
         << CompList.at(i).Description << endl;
  }
  cout << "-------------------------------------" << endl;
  cout << endl << "------  COMPONENT DUMP -----------" << endl;
  printf(" Input the Component id : ");
  err = scanf("%d", &ComponentID);
  if (err != 1) {
    printf(" Failed to read the component id ");
    return;
  }
  ComponentDB::componentLong Compo;
  AlpideTable::response *    theresult = theCompTable->Read(ComponentID, &Compo);
  cout << theCompTable->DumpResponse() << endl;
  if (theresult->ID != 0)
    cout << theCompTable->Dump(&Compo) << endl;
  else
    cout << "Component Type NOT FOUND !" << endl;
  cout << "-------------------------------------" << endl;
}

void activityType(AlpideDB *theDB)
{
  cout << endl << "----------   METHODS ACTIVITY -----------" << endl << endl;
  ActivityDB *theActivityTable = new ActivityDB(theDB);
  int         err;
  cout << endl << "------ Activity type-----------" << endl;
  int ProjectID = 0;
  printf(" Input the Project id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read the project id ");
    return;
  }
  vector<ActivityDB::activityType> *act = theActivityTable->GetActivityTypeList(ProjectID);
  for (unsigned int i = 0; i < act->size(); i++) {
    cout << endl << act->at(i).ID << "\t" << act->at(i).Name << "\t" << act->at(i).Description;
  }
  cout << "-------------------------------------" << endl;

  cout << endl << "------ Parameter type-----------" << endl;
  int ActivityID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ActivityID);
  if (err != 1) {
    printf(" Failed to read Activity Type id ");
    return;
  }
  vector<ActivityDB::parameterType> *par = theActivityTable->GetParameterTypeList(ActivityID);
  for (unsigned int i = 0; i < par->size(); i++) {
    cout << endl
         << par->at(i).ID << "\t" << par->at(i).Name << "\t(" << par->at(i).ParameterID << ")\t"
         << par->at(i).Description;
  }
  cout << "-------------------------------------" << endl;

  cout << endl << "------ locationType type-----------" << endl;
  ProjectID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read Activity Type id ");
    return;
  }
  vector<ActivityDB::locationType> *loc = theActivityTable->GetLocationTypeList(ProjectID);
  for (unsigned int i = 0; i < loc->size(); i++) {
    cout << endl << loc->at(i).ID << "\t" << loc->at(i).Name;
  }
  cout << "-------------------------------------" << endl;

  cout << endl << "------ resultType type-----------" << endl;
  ProjectID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read Activity Type id ");
    return;
  }
  vector<ActivityDB::resultType> *res = theActivityTable->GetResultList(ProjectID);
  for (unsigned int i = 0; i < res->size(); i++) {
    cout << endl << res->at(i).ID << "\t" << res->at(i).Name;
  }
  cout << "-------------------------------------" << endl;

  cout << endl << "------ statusType type-----------" << endl;
  ProjectID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ProjectID);
  if (err != 1) {
    printf(" Failed to read Activity Type id ");
    return;
  }
  vector<ActivityDB::statusType> *sta = theActivityTable->GetStatusList(ProjectID);
  for (unsigned int i = 0; i < sta->size(); i++) {
    cout << endl << sta->at(i).ID << "\t" << sta->at(i).Code << "\t" << sta->at(i).Description;
  }
  cout << "-------------------------------------" << endl;

  cout << endl << "------ componentType type-----------" << endl;
  int ActTypeID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ActTypeID);
  if (err != 1) {
    printf(" Failed to read Activity Type id ");
    return;
  }
  vector<ActivityDB::actTypeCompType> *com = theActivityTable->GetComponentTypeList(ActTypeID);
  for (unsigned int i = 0; i < com->size(); i++) {
    cout << endl << com->at(i).ID << "\t" << com->at(i).Type.Name;
  }
  cout << "-------------------------------------" << endl;
}

void activityDB(AlpideDB *theDB)
{
  ActivityDB *theActivityTable = new ActivityDB(theDB);
  int         err;

  cout << endl << "------ Activity List-----------" << endl;
  int ProjectID = 0;
  printf(" Input the Project id :");
  err                = scanf("%d", &ProjectID);
  int ActivityTypeID = 0;
  printf(" Input the Activity Type id :");
  err = scanf("%d", &ActivityTypeID);
  if (err != 1) {
    printf(" Failed to read the Activity type id ");
    return;
  }
  vector<ActivityDB::activityShort> *act1 =
      theActivityTable->GetActivityList(ProjectID, ActivityTypeID);
  for (unsigned int i = 0; i < act1->size(); i++) {
    cout << endl << act1->at(i).ID << "\t" << act1->at(i).Name;
  }
  cout << "-------------------------------------" << endl;

  int ActivityID;
  cout << endl << "------ Read Activity ----------" << endl;
  printf(" Input the Activity id :");
  err = scanf("%d", &ActivityID);
  if (err != 1) {
    printf(" Failed to read the Activity  id ");
    return;
  }
  ActivityDB::activityLong Act;
  theActivityTable->Read(ActivityID, &Act);
  cout << theActivityTable->DumpResponse() << endl;
  theActivityTable->DumpActivity(&Act);
  cout << "-------------------------------------" << endl;
}

void activityCreation(AlpideDB *theDB)
{
  ActivityDB *theActivityTable = new ActivityDB(theDB);
  cout << endl << "------  CREATE ACTIVITY -----------" << endl;

  // --- create activity test
  //	ActivityDB *theActivityTable = new ActivityDB(theDB);
  ActivityDB::activity  Attiv;
  ActivityDB::member    Mem;
  ActivityDB::parameter Par;
  ActivityDB::attach    Att;
  ActivityDB::actUri    Uri;

  vector<ActivityDB::actUri> Uris;
  // tm ts; ts.tm_year = 2017 - 1900; ts.tm_mon = 6; ts.tm_mday = 13; // caused warning

  Attiv.Location  = 764;
  Attiv.EndDate   = time(NULL);
  Attiv.Lot       = "TestAntonio";
  Attiv.Name      = "Test_db";
  Attiv.Position  = "Position1";
  Attiv.Result    = 842;
  Attiv.StartDate = time(NULL); // mktime(&ts);
  Attiv.Status    = 83;         // open
  Attiv.Type      = 881;
  Attiv.User      = 1;

  Mem.Leader        = 0;
  Mem.ProjectMember = 584;
  Mem.User          = 8791;
  Attiv.Members.push_back(Mem);
  Mem.ProjectMember = 201;
  Mem.User          = 4702;
  Attiv.Members.push_back(Mem);

  Par.ActivityParameter = 468;
  Par.User              = 1;
  Par.Value             = 9.1;
  Attiv.Parameters.push_back(Par);
  Par.ActivityParameter = 494;
  Par.User              = 1;
  Par.Value             = 8.1;
  Attiv.Parameters.push_back(Par);
  Par.ActivityParameter = 381;
  Par.User              = 1;
  Par.Value             = 14;
  Attiv.Parameters.push_back(Par);

  Att.Category       = 41;
  Att.LocalFileName  = "AttachTest.txt";
  Att.User           = 1;
  Att.RemoteFileName = "AttachTest.txt";
  Attiv.Attachments.push_back(Att);

  Uri.Description = "Test Uri";
  Uri.Path        = "http://www.google.com";
  Uris.push_back(Uri);

  theActivityTable->Create(&Attiv);
  cout << " ------- the activity create response ----" << endl;
  cout << theActivityTable->DumpResponse() << endl;
  cout << " ------- the activity create responses ----" << endl;
  AlpideTable::response  res;
  AlpideTable::response *respt;

  for (unsigned int i = 0; i < theActivityTable->GetResponsesNumber(); i++) {
    res = theActivityTable->GetResponse(i);
    cout << theActivityTable->DumpResponse(&res) << endl;
  }
  int ActivityID = theActivityTable->GetResponse().ID;

  respt = theActivityTable->AssignUris(ActivityID, 1, &Uris);
  cout << theActivityTable->DumpResponse(respt) << endl;

  respt = theActivityTable->AssignComponent(ActivityID, 11956, 1221, 1);
  cout << theActivityTable->DumpResponse(respt) << endl;

  respt = theActivityTable->AssignComponent(ActivityID, 11956, 1241, 1);
  cout << theActivityTable->DumpResponse(respt) << endl;
  cout << " ------- ---------------- ----" << endl;

  unsigned int numofchanges = 1;
  int          serr;
  cout << endl << "------ Change parameters burst ----------" << endl;
  printf(" Activity ID :");
  serr = scanf("%d", &ActivityID);
  printf(" Number of changes :");
  serr = scanf("%u", &numofchanges);

  for (unsigned int i = 0; i < numofchanges; i++) {
    theActivityTable->ChangeParameter(ActivityID, "IDDA", 1.1 + i, 1);
  }
  cout << " ------- the parameter change responses ----" << endl;
  for (unsigned int i = 0; i < theActivityTable->GetResponsesNumber(); i++) {
    res = theActivityTable->GetResponse(i);
    cout << theActivityTable->DumpResponse(&res) << endl;
  }
  cout << " ------- ---------------- ----" << serr << endl;
}

int main()
{
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
  int choice = -1;
  int serr;
  while (choice != 0) {
    cout << " ------- MENU ---------" << endl;
    cout << " 1) Project / Members" << endl;
    cout << " 2) Component Types DB" << endl;
    cout << " 3) Component DB" << endl;
    cout << " 4) Activity Types DB" << endl;
    cout << " 5) Activity DB" << endl;
    cout << " 6) Activity Creation" << endl;
    cout << " --------------------- " << endl;
    cout << " Input the Project id :";
    serr = scanf("%d", &choice);
    switch (choice) {
    case 1:
      projectDB(theDB);
      break;
    case 2:
      componentType(theDB);
      break;
    case 3:
      componentDB(theDB);
      break;
    case 4:
      activityType(theDB);
      break;
    case 5:
      activityDB(theDB);
      break;
    case 6:
      activityCreation(theDB);
      break;
    }
  }
  cout << serr;
}
