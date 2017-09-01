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


