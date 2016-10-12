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
#ifndef BOARDCONFIGMOSAIC_H
#define BOARDCONFIGMOSAIC_H

#include "TBoardConfig.h"
#include <stdio.h>

class TBoardConfigMOSAIC : public TBoardConfig {

private:
	FILE *fhConfigFile; // the file handle of the Configuration File

protected:
        void     InitParamMap ();
	uint16_t NumberOfControlInterfaces;
	char IPAddress[30];
	uint16_t TCPPort;

	uint16_t ControlInterfacePhase;
	uint32_t RunCtrlAFThreshold;
	uint16_t RunCtrlLatMode;
	uint32_t RunCtrlTimeout;
	bool LowSpeedMode;
        bool Inverted;

	uint32_t pollDataTimeout;

public:
	TBoardConfigMOSAIC(const char *fName = 0, int boardIndex = 0);

	// Info methods

	// Getters
	char * GetIPaddress() {return(IPAddress);}
	uint16_t GetTCPport() {return(TCPPort);}
	uint16_t GetCtrlInterfaceNum() {return(NumberOfControlInterfaces);}
	uint16_t GetCtrlInterfacePhase() {return(ControlInterfacePhase);}
	uint32_t GetCtrlAFThreshold() {return(RunCtrlAFThreshold);}
	uint16_t GetCtrlLatMode() {return(RunCtrlLatMode);}
	uint32_t GetCtrlTimeout() {return(RunCtrlTimeout);}
	bool IsLowSpeedMode() { return(LowSpeedMode);}
	bool IsInverted    () { return(Inverted);}
	uint32_t GetPollingDataTimeout() {return(pollDataTimeout);}

	// Setters
	void SetIPaddress(const char *AIPaddress);
	void SetTCPport(uint16_t APort) { TCPPort = APort;}
	void SetCtrlInterfaceNum(uint16_t ACtrlInterfaceNumber) { NumberOfControlInterfaces = ACtrlInterfaceNumber;}
	void SetCtrlInterfacePhase(uint16_t ACtrlInterfacePhase) { ControlInterfacePhase = ACtrlInterfacePhase;}
	void SetCtrlAFThreshold(uint32_t ACtrlAFThreshold) { RunCtrlAFThreshold = ACtrlAFThreshold;}
	void SetCtrlLatMode(uint16_t ARunCtrlLatencyMode) { RunCtrlLatMode = ARunCtrlLatencyMode;}
	void SetCtrlTimeout(uint32_t ARunCtrlTimeout) { RunCtrlTimeout = ARunCtrlTimeout;}
	void SetLowSpeedMode(bool IsLowSpeedMode) { LowSpeedMode = IsLowSpeedMode;}
	void SetPollingDataTimeout(uint32_t APollDataTimeout) { pollDataTimeout = APollDataTimeout;}

};

//************************************************************

#endif   /* BOARDCONFIGMOSAIC_H */
