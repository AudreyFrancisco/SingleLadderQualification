/* -------------------------------------------------
 *   Derived TConfigBoard Class for MOSAIC board
 *
 *   ver.0.1		3/8/2016
 *
 *  Auth.: A.Franco	-  INFN BARI
 *
 *  		HISTORY
 *
 *  22/05/2017    -  Modify the maximum number of Control Interfaces
 *
 */
#ifndef BOARDCONFIGMOSAIC_H
#define BOARDCONFIGMOSAIC_H

#include "TBoardConfig.h"
#include "Mosaic.h"
#include <stdio.h>

#define MAX_MOSAICCTRLINT 12
#define MAX_MOSAICTRANRECV 10
#define MOSAIC_HEADER_LENGTH 64

#define DEF_TCPPORT 2000
#define DEF_CTRLINTPHASE 2
#define DEF_CTRLAFTHR 1250000
#define DEF_CTRLLATMODE 0
#define DEF_CTRLTIMEOUT 0
#define DEF_POLLDATATIMEOUT 500
#define DEF_POLARITYINVERSION 0
#define DEF_SPEEDMODE 0

class TBoardConfigMOSAIC : public TBoardConfig {

private:
	FILE *fhConfigFile; // the file handle of the Configuration File

protected:

    void     InitParamMap ();

    int NumberOfControlInterfaces;
    int TCPPort;
	int ControlInterfacePhase;
	int RunCtrlAFThreshold;
	int RunCtrlLatMode;
	int RunCtrlTimeout;
	int pollDataTimeout;
    int Inverted;
    int SpeedMode;

    char IPAddress[30];
//	Mosaic::TReceiverSpeed  SpeedMode;


public:
	TBoardConfigMOSAIC(const char *fName = 0, int boardIndex = 0);

	// Info methods

	// Getters
	char *   GetIPaddress          () {return(IPAddress);}
	uint16_t GetCtrlInterfaceNum   () {return((uint16_t)NumberOfControlInterfaces);}
	uint16_t GetTCPport            () {return((uint16_t)TCPPort);}
	uint16_t GetCtrlInterfacePhase () {return((uint16_t)ControlInterfacePhase);}
	uint32_t GetCtrlAFThreshold    () {return((uint32_t)RunCtrlAFThreshold);}
	uint16_t GetCtrlLatMode        () {return((uint16_t)RunCtrlLatMode);}
	uint32_t GetCtrlTimeout        () {return((uint32_t)RunCtrlTimeout);}
	uint32_t GetPollingDataTimeout () {return((uint32_t)pollDataTimeout);}
	bool     IsInverted            () {return((bool)Inverted);}
    Mosaic::TReceiverSpeed    GetSpeedMode        ();

	// Setters
	void SetIPaddress          (const char *AIPaddress);
	void SetTCPport            (uint16_t APort)                { TCPPort = (int)APort;}
	void SetCtrlInterfaceNum   (uint16_t ACtrlInterfaceNumber) { NumberOfControlInterfaces = (int)ACtrlInterfaceNumber;}
	void SetCtrlInterfacePhase (uint16_t ACtrlInterfacePhase)  { ControlInterfacePhase = (int)ACtrlInterfacePhase;}
	void SetCtrlAFThreshold    (uint32_t ACtrlAFThreshold)     { RunCtrlAFThreshold = (int)ACtrlAFThreshold;}
	void SetCtrlLatMode        (uint16_t ARunCtrlLatencyMode)  { RunCtrlLatMode = (int)ARunCtrlLatencyMode;}
	void SetCtrlTimeout        (uint32_t ARunCtrlTimeout)      { RunCtrlTimeout = (int)ARunCtrlTimeout;}
        void SetInvertedData       (bool     AIsInverted)          { Inverted       = (int)AIsInverted;};
	void SetPollingDataTimeout (uint32_t APollDataTimeout)     { pollDataTimeout = (int)APollDataTimeout;}
	void SetSpeedMode          (Mosaic::TReceiverSpeed ASpeedMode);

};

//************************************************************

#endif   /* BOARDCONFIGMOSAIC_H */
