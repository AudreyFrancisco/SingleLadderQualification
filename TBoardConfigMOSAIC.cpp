/* -------------------------------------------------
 *   Derived TConfigBoard Class for MOSAIC board
 *
 *   ver.0.1		3/8/2016
 *
 *  Auth.: A.Franco	-  INFN BARI
 *
 *  		HISTORY
 *
 *
 */
#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstring>
#include "TBoardConfigMOSAIC.h"

using namespace std;


TBoardConfigMOSAIC::TBoardConfigMOSAIC(const char *AConfigFileName, int ABoardIndex)
{
  fBoardType = boardMOSAIC;
	// Default values set
	NumberOfControlInterfaces = 2;

	TCPPort = 2000;

	ControlInterfacePhase = 2;
	RunCtrlAFThreshold = 1250000;
	RunCtrlLatMode = 0; // 0 := latencyModeEoe, 1 := latencyModeTimeout, 2 := latencyModeMemory
	RunCtrlTimeout = 0;
	//	LowSpeedMode = false;
        //Inverted     = false;

	pollDataTimeout = 500; // milliseconds

        if (AConfigFileName) {
  	// Read Configuration file
  	  try {
	  	if(AConfigFileName == NULL || strlen(AConfigFileName) == 0) throw std::invalid_argument("MOSAIC Config : invalid filename");
		fhConfigFile = fopen(AConfigFileName,"r"); // opens the file
	  } catch (...) {
		throw std::invalid_argument("MOSAIC Config : file not exists !");
	  }
	}
        InitParamMap();
}



void TBoardConfigMOSAIC::InitParamMap() 
{
  TBoardConfig::InitParamMap();
}


// ----- private methods ----

// sets the IP address
void TBoardConfigMOSAIC::SetIPaddress(const char *AIPaddress)
{
	try {
		if(AIPaddress == NULL) throw std::invalid_argument("MOSAIC Config : invalid IP number");
		strcpy(IPAddress, AIPaddress);
	} catch (...) {
		throw std::invalid_argument("MOSAIC Config : bad IP parameter specification");
	}
	return;
}
