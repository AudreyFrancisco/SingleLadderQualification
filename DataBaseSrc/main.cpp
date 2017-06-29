
#include <iostream>

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"

int main()
{
	// the default setting mode ...
	AlpideDB *theDB = new AlpideDB();

	/*
	// --- alternative setting : NSSDB in the executable directory
	theDB->SetQueryDomain("https://test-alucmsapi.web.cern.ch/AlucmswebAPI.asmx/");

    AlpideDBManager *theDBManager = theDB->GetManageHandle();
	theDBManager->setNSSDBNickName("FrancoAntonio");  // the name associated to the CErt/Key in the DB
	theDBManager->setNSSDBPath(".");  // the path where the: cert8.db,key3.db,secmod.db are stored
	theDBManager->setNSSDBPass("alpide4me");  // the password used to access the NSS DB
	theDBManager->Init();
	// -----
	*/

	AlpideTable::response *theResult;

	ProjectDB *theProjTable = new ProjectDB(theDB);

	vector<ProjectDB::project> ProjList;
	cout << "------  PROJECTS DATABASE -----------" << endl;
	theProjTable->GetList(&ProjList);
	cout << theProjTable->DumpResponse() << endl;

	cout << "The list of Projects is " << ProjList.size() << "  long " << endl;
	for(int i=0;i<ProjList.size();i++) {
		cout << i << " " << ProjList.at(i).ID << "  " << ProjList.at(i).Name << endl;
	}


	MemberDB *theMembTable = new MemberDB(theDB);
	vector<MemberDB::member> MemberList;
	MemberDB::member oneMember;
	cout << endl << "------  MEMBERS DATABASE -----------" << endl;
	int ProjectID = 0;
	printf(" Input the Project id :");
	scanf("%d",&ProjectID);

	theMembTable->GetList(ProjectID, &MemberList);
	cout << theMembTable->DumpResponse() << endl;

	cout << "The list of Members of Project=" << ProjectID << " is " << MemberList.size() << "  long " << endl;
	for(int i=0;i<MemberList.size();i++) {
		cout << i << " " << MemberList.at(i).ID << "  " << MemberList.at(i).PersonalID << "  " << MemberList.at(i).FullName << endl;
	}


	ComponentDB *theCompTable = new ComponentDB(theDB);
	vector<ComponentDB::componentType> ComponentList;
	ComponentDB::componentType oneComponent;
	cout << endl << "------  COMPONENT DATABASE -----------" <<endl;
	ProjectID = 0;
	printf(" Input the Project id :");
	scanf("%d",&ProjectID);

	theCompTable->GetTypeList(ProjectID,&ComponentList);
	cout << theCompTable->DumpResponse() << endl;

	cout << "The list of Type Components of Project=" << ProjectID << " is " << ComponentList.size() << "  long " << endl;
	for(int i=0;i<ComponentList.size();i++) {
		cout << i << " " << ComponentList.at(i).ID << "  " << ComponentList.at(i).Code << "  " << ComponentList.at(i).Name << endl;
	}

	cout << endl << "------  COMPONENT Type SPEC -----------" << endl;
	int ComponentID = 0;
	printf(" Input the Component id : ");
	scanf("%d",&ComponentID);

	theCompTable->GetType(ComponentID,&oneComponent);
	cout << theCompTable->DumpResponse() << endl;

	cout << theCompTable->Print(&oneComponent) << endl;


}





