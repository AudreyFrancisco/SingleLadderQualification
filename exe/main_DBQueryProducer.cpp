#include "DBQueryQue.h"
#include "DBHelpers.h"

#include <unistd.h>
#include <iostream>
#include <thread>




int main()
{
	AlpideDB *fDB;
	bool fDatabasetype = true;
    fDB = new AlpideDB(fDatabasetype);
    ActivityDB *myactivity = new ActivityDB(fDB);
    ActivityDB::activity activ;

    myactivity->theAsyncronuosQueue->SetLocalCopyEnabled(true);

	        activ.Type      = 881; //fIdofactivitytype;
	        activ.Location  = 161; //fIdoflocationtype;
	        activ.User      = 1126; //fIdofoperator;
	        activ.StartDate = time(0) ;//date.currentDateTime().toTime_t();
	        activ.EndDate   = time(0); //date.currentDateTime().toTime_t();
	        activ.Lot       = " ";
          activ.Name = "Test of asyncronous act creation";//CreateActivityName(fHICs.at(i)->GetDbId(), fConfig->GetScanConfig());
	        activ.Position = " ";
	        activ.Result =
	            -999; // apparently has to stay open here, otherwise activity is considered closed
	        activ.Status = 83;// DbGetStatusId(fDB, fIdofactivitytype, "OPEN");
	            DbAddParameter(fDB, activ, "Number of Working Chips", 10.0,
	                          "");
	            DbAddParameter(fDB, activ, "IDDD", 10.1,"");
	        DbAddAttachment(fDB, activ, attachLog, string("/tmp/Comment.txt"), string("Peppo"));
	        DbAddMember(fDB, activ, 1126);
	        DbAddUri(fDB, activ, "URI test 1" , "http://www.cern.ch");
	        DbAddInComp(fDB, activ, 11956 , 1221);
			DbAddOutComp(fDB, activ, 11956 , 1241);
	        activ.Result = 822; //DbGetResultId(fDB, fIdofactivitytype, fHICs.at(i)->GetClassification());
	        myactivity->CreateAsyncronous(&activ);

}
