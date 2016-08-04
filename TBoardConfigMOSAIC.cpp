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
#include "TBoardConfigMOSAIC.h"

using namespace std;


TBoardConfigMOSAIC::TBoardConfigMOSAIC(const char *AConfigFileName, int ABoardIndex)
{
	// Default values set
	NumberOfControlInterfaces = 2;
	strcpy(IPAddress, "192.168.168.250");
	TCPPort = 2000;

	ControlInterfacePhase = 6;
	RunCtrlAFThreshold = 1250000;
	RunCtrlLatMode =1; // 1 := latencyModeEoe, 2 := latencyModeMemory, 3 := latencyModeTimeout
	RunCtrlTimeout = 0;
	LowSpeedMode = true;

	pollDataTimeout = 500; // milliseconds

	// Read Configuration file
	try {
		if(AConfigFileName == NULL || strlen(AConfigFileName) == 0) throw std::invalid_argument("MOSAIC Config : invalid filename");
		fhConfigFile = fopen(AConfigFileName,"r"); // opens the file
	} catch (...) {
		throw std::invalid_argument("MOSAIC Config : file not exists !");
	}
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
