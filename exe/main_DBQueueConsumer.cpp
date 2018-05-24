#include "DBQueryQue.h"

#include <unistd.h>
#include <iostream>
#include <thread>


int main()
{
	bool fDatabasetype = true;
	DBQueueConsumer* theQueue = new DBQueueConsumer(fDatabasetype);
	// Configuration
	theQueue->cfgExtension = ".dbq";
	theQueue->cfgBasePath = "/tmp";
	theQueue->cfgSpecificPath = "NewAlpide";
	theQueue->cfgStatusFile = "/tmp/DBQueueConsumerState.dat";
	theQueue->cfgMaxReconAtt = 9999;
	
	// run the thread
	thread engine(&DBQueueConsumer::Run, theQueue);
	vector<string> list;
	char ch;
	do {
	  cout<<"\nDBQ>";
	  ch = getchar();
	  switch(ch) {
	  case 'c':
		  cout << "** DBQueueConsumer Configuration **" << endl;
		  cout << " DataBaseType :" << (!theQueue->bDatabasetype ?"PRODUCTION" : "TEST") << endl;
		   cout << " Running on the Project : " << theQueue->GetProjectId() << endl;
		  cout << " Status filename :" << theQueue->cfgStatusFile << endl;
		  cout << " Queue file extension :" << theQueue->cfgExtension << endl;
		  cout << " Queue files base path :" << theQueue->cfgBasePath << endl;
		  cout << " Queue files specific path :" << theQueue->cfgSpecificPath << endl;
		  cout << " Maximum reconnection attempts :" << theQueue->cfgMaxReconAtt << endl;
		  break;
	  case 's':
		  cout << "** DBQueueConsumer Status **" << endl;
		  cout << " File :" << theQueue->theFileName << endl;
		  cout << " State :" << theQueue->theState << endl;
		  cout << " The Queue has " << theQueue->GetQueueLenght() << " elements" << endl;
		  break;
	  case 'q':
		  cout << "** DBQueueConsumer Queue **" << endl;
		  list = theQueue->GetQueue();
		  for(int i=0;i<list.size();i++)
			  cout << list.at(i) << endl;
		  break;
	  case 'h':
	  case '?':
		  cout << "** DBQueueConsumer Help **" << endl;
		  cout << "c := print the configuration" << endl;
		  cout << "s := print the status" << endl;
		  cout << "q := dump the list of queued files" << endl;
		  cout << "e := exit" << endl;
	  }
	}
	while(ch!='e');
	cout <<  "Bye bye !" << endl;

}
