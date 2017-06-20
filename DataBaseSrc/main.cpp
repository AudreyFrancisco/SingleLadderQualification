
#include <iostream>

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"

int main()
{
	AlpideDB *theDB = new AlpideDB();

	ProjectDB *theProjTable = new ProjectDB(theDB);
	MemberDB *theMembTable = new MemberDB(theDB);
	ComponentDB *theCompTable = new ComponentDB(theDB);

	vector<ProjectDB::project> ProjList;

	theProjTable->GetList(&ProjList);

	cout << "The list of Projects is " << ProjList.size() << "  long " << endl;




}





