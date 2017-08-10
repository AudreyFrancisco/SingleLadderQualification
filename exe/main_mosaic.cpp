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
#include "TConfig.h"
#include "TAlpide.h"
#include <exception>
#include "SetupHelpers.h"

int main(int argc, char** argv) {

  decodeCommandParameters(argc, argv);
	TReadoutBoard      *theBoard;

	// First create an instance of the Configuration
	//theBoardConfiguration = new TBoardConfigMOSAIC("Config.cfg", 0); // The file must exists... but could be useful a constructor without param
	// Then create an instance of the board


	std::vector <int>      chipIDs;
	std::vector <TAlpide*> fChips;

  for (int i = 0; i < 30; i++) chipIDs.push_back(i);

  //TConfig *config = new TConfig (5);
  TConfig *config = new TConfig (1, chipIDs);

	theBoard = (TReadoutBoard *) new TReadoutBoardMOSAIC(config, (TBoardConfigMOSAIC*)config->GetBoardConfig(0));

  for (unsigned int i = 0; i < config->GetNChips(); i++) {
    fChips.push_back(new TAlpide(config->GetChipConfigById(chipIDs.at(i))));
    fChips.at(i) -> SetReadoutBoard(theBoard);
    theBoard     -> AddChip        (chipIDs.at(i), 0, 0);
	}

  uint16_t Value;

  for (unsigned int i = 0; i < fChips.size(); i++) {
	  // std::cout << "About to write chip " << chipIDs.at(i) << std::endl;
    //for (int ii = 5; ii >0; ii --) {
	  //  std::cout << "  in " << ii << " s." << std::endl;
    //  sleep(1);
	  //}
    fChips.at(i)->WriteRegister (0x60d, 10);
    try {
      fChips.at(i)->ReadRegister (0x60d, Value);
      std::cout << "Chip ID " << chipIDs.at(i) << ", Value = 0x" << std::hex << (int) Value << std::dec << std::endl;
	  }
    catch (exception &e) {
      std::cout << "Chip ID " << chipIDs.at(i) << ", not answering, exception: " << e.what() << std::endl;
	  }
	}


  return 0;


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
