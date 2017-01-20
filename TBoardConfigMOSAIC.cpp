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
	NumberOfControlInterfaces = MAX_MOSAICCTRLINT;
	TCPPort = DEF_TCPPORT;
	ControlInterfacePhase = DEF_CTRLINTPHASE;
	RunCtrlAFThreshold = DEF_CTRLAFTHR;
	RunCtrlLatMode = DEF_CTRLLATMODE; // 0 := latencyModeEoe, 1 := latencyModeTimeout, 2 := latencyModeMemory
	RunCtrlTimeout = DEF_CTRLTIMEOUT;
	pollDataTimeout = DEF_POLLDATATIMEOUT; // milliseconds
	Inverted = DEF_POLARITYINVERSION;
	SpeedMode = DEF_SPEEDMODE;

	if (AConfigFileName) { // Read Configuration file
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
	fSettings["NUMBEROFCONTROLINTERFACES"] = &NumberOfControlInterfaces;
	fSettings["TCPPORTNUMBER"] = &TCPPort;
	fSettings["CONTROLINTERFACEPHASE"] = &ControlInterfacePhase;
	fSettings["CONTROLAFTHRESHOLD"] = &RunCtrlAFThreshold;
	fSettings["CONTROLLATENCYMODE"] = &RunCtrlLatMode;
	fSettings["CONTROLTIMEOUT"] = &RunCtrlTimeout;
	fSettings["POLLINGDATATIMEOUT"] = &pollDataTimeout;
	fSettings["DATALINKPOLARITY"] = &Inverted;
	fSettings["DATALINKSPEED"] = &SpeedMode;

	TBoardConfig::InitParamMap();
}


Mosaic::TReceiverSpeed TBoardConfigMOSAIC::GetSpeedMode()
{
	switch(SpeedMode) {
	case 0:
		return(Mosaic::RCV_RATE_400);
		break;
	case 1:
		return(Mosaic::RCV_RATE_600);
		break;
	case 2:
		return(Mosaic::RCV_RATE_1200);
		break;
	default:
		return(Mosaic::RCV_RATE_400);
		break;
	}
}

void TBoardConfigMOSAIC::SetSpeedMode(Mosaic::TReceiverSpeed ASpeedMode)
{
	switch(ASpeedMode) {
	case Mosaic::RCV_RATE_400:
		SpeedMode = 0;
		break;
	case Mosaic::RCV_RATE_600:
		SpeedMode = 1;
		break;
	case Mosaic::RCV_RATE_1200:
		SpeedMode = 2;
		break;
	default:
		SpeedMode = 0;
		break;
	}
	return;
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
