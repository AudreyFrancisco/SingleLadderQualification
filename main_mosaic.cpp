/* ---------------
 * Example of MOSAIC use
 *
 ----------------- */

#include <iostream>
#include <unistd.h>
#include "TReadoutBoard.h"
#include "TReadoutBoardMOSAIC.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"

int main()
{
	TBoardConfigMOSAIC *theBoardConfiguration;
	TReadoutBoard      *theBoard;

	// First create an instance of the Configuration
	theBoardConfiguration = new TBoardConfigMOSAIC("Config.cfg", 0); // The file must exists... but could be useful a constructor without param

	// Then create an instance of the board
	theBoard = (TReadoutBoard *) new TReadoutBoardMOSAIC("192.168.168.250", theBoardConfiguration);

	/* Create the Chip Structures */
	/* Reset Chips */
	/* Initialize Chips */

	/* Data Tacking */
	int numberOfReadByte; // the bytes of row event
	unsigned char *theBuffer; // the buffer containing the event

	int enablePulse, enableTrigger, triggerDelay, pulseDelay, nTriggers; // variables that define the trigger/pulse

	theBuffer = (unsigned char*) malloc(200 * 1024); // allocates 200 kilobytes ...

	bool isDataTackingEnd = false; // break the execution of read polling
	int returnCode = 0;
	int timeoutLimit = 10; // ten seconds

	// sets the trigger
	theBoard->SetTriggerConfig (enablePulse, enableTrigger, triggerDelay, pulseDelay);
	theBoard->SetTriggerSource (TTriggerSource::trigInt);

	((TReadoutBoardMOSAIC *)theBoard)->StartRun(); // Activate the data taking ...

	theBoard->Trigger(nTriggers); // Preset end start the trigger

	while(!isDataTackingEnd) { // while we don't receive a timeout
		returnCode = theBoard->ReadEventData(numberOfReadByte, theBuffer);
		if(returnCode != 0) { // we have some thing
			std::cout << "Read an event !  Dimension :" << numberOfReadByte << std::endl;   // Consume the buffer ...
			usleep(20000); // wait
		} else { // read nothing is finished ?
			if(timeoutLimit-- == 0) isDataTackingEnd = true;
			sleep(1);
		}
	}

	((TReadoutBoardMOSAIC *)theBoard)->StopRun(); // Stop run

	exit(0);
        return 0;
}
