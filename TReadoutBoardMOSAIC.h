/* -------------------------------------------------
 *   Derived TReadOutBoard Class for MOSAIC board
 *
 *   ver.0.1		3/8/2016
 *
 *  Auth.: A.Franco	-  INFN BARI
 *
 *  		HISTORY
 *
 *
 */
#ifndef READOUTBOARDMOSAIC_H
#define READOUTBOARDMOSAIC_H

#include <exception>
#include <string>

#include "TReadoutBoard.h"
#include "TConfig.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "BoardDecoder.h"

#include "MosaicSrc/wbb.h"
#include "MosaicSrc/ipbusudp.h"
#include "MosaicSrc/mruncontrol.h"
#include "MosaicSrc/i2cbus.h"
#include "MosaicSrc/controlinterface.h"
#include "MosaicSrc/pulser.h"
#include "MosaicSrc/mtriggercontrol.h"
#include "MosaicSrc/mdatasave.h"
#include "MosaicSrc/mdatagenerator.h"
#include "MosaicSrc/alpide3rcv.h"
#include "MosaicSrc/i2csyspll.h"

// Constant Definitions
#define DEFAULT_PACKET_SIZE 		1400
#define DEFAULT_UDP_PORT			2000
#define DEFAULT_TCP_BUFFER_SIZE		(512*1024)			// if set to 0 : automatic
#define DEFAULT_TCP_PORT			3333

#define DATA_INPUT_BUFFER_SIZE	64*1024

#define BOARD_STATUS_FEPLL_LOCK			0x0001
#define BOARD_STATUS_EXTPLL_LOCK		0x0002
#define BOARD_STATUS_GT0PLL0_LOCK		0x0004
#define BOARD_STATUS_GT0PLL1_LOCK		0x0008
#define BOARD_STATUS_GTP0RESET_DONE		0x10000

#define BOARD_STATUS_INIT_OK  (BOARD_STATUS_GTP0RESET_DONE | \
								BOARD_STATUS_GT0PLL0_LOCK | \
								BOARD_STATUS_EXTPLL_LOCK | \
								BOARD_STATUS_FEPLL_LOCK )

#define MAX_MOSAICCTRLINT 2
#define MAX_MOSAICTRANRECV 10
#define MOSAIC_HEADER_LENGTH 64

//class string;
using namespace std;

class ForwardReceiver : public MDataReceiver
{
public:
	ForwardReceiver();
	virtual ~ForwardReceiver();

	long parse(int numClosed) { return(0);};

};


class MosaicException : public exception
{
public:
	/** Takes a character string describing the error.  */
	explicit MosaicException();
	explicit MosaicException(const string& __arg);
	~MosaicException() throw();

	/** Returns a C-style character string describing the general cause of
	 *  the current error (the same string passed to the ctor).  */
	virtual const char* what() const throw();

protected:
    string msg;
};

class MosaicSetupError : public MosaicException
{
public:
	explicit MosaicSetupError(const string& __arg);
};

class MosaicRuntimeError : public MosaicException
{
public:
	explicit MosaicRuntimeError(const string& __arg);
};

// -----------------------------------------------------

class TReadoutBoardMOSAIC : public TReadoutBoard {

// Properties
public:

private:

	TBoardConfigMOSAIC *fBoardConfigDAQ;

	IPbusUDP 			*mIPbus;
	MRunControl 		*mRunControl;
	MDataGenerator 		*dataGenerator;
	I2Cbus 	 			*i2cBus;
	ControlInterface 	*controlInterface[MAX_MOSAICCTRLINT];
	Pulser			 	*pulser;
	MDataSave	 		*dr;
	Alpide3rcv			*a3rcv[MAX_MOSAICTRANRECV];
	I2CSysPll			*mSysPLL;
	MTriggerControl 	*mTriggerControl;

	int	tcp_sockfd;
	std::string ipAddress;
	uint16_t TCPport;
	uint16_t numOfSetReceivers = 0;
	std::vector<MDataReceiver *> receivers;

	uint32_t closedEventsPerSource[MAX_MOSAICTRANRECV];
	TBoardHeader theHeaderOfReadData;  // This will host the info catch from Packet header/trailer
	char *theHeaderBuffer[MOSAIC_HEADER_LENGTH+10]; // This will host the info of the header in the original format

	enum dataBlockFlag_e {
		flagClosedEvent			= (1 << 0),
		flagOverflow			= (1 << 1),
		flagTimeout			    = (1 << 2),
		flagCloseRun			= (1 << 3)
		};

protected:
  
// Methods
public:
	TReadoutBoardMOSAIC(const char *AIPaddress, TBoardConfigMOSAIC *config);
	virtual ~TReadoutBoardMOSAIC();
  
	int WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId =0);
	int ReadChipRegister  (uint16_t address, uint16_t &value, uint8_t chipId =0);
	int SendOpCode        (uint16_t  OpCode, uint8_t chipId=0);
	int SendOpCode        (uint16_t  OpCode);
        // Markus: changed trigger delay type from uint32_t to int, since changed upstream
	int SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay);
	void SetTriggerSource  (TTriggerSource triggerSource);
	int Trigger           (int nTriggers);
        // Markus: changed data type from char to unsigned char; check that no problem
        // (should be OK at least for memcpy)
	int ReadEventData     (int &nBytes, unsigned char *buffer);

	void StartRun();
	void StopRun();

	int ReadRegister      (uint16_t Address, uint32_t &Value) { return(0);};
	int WriteRegister     (uint16_t Address, uint32_t Value)  { return(0);};

private:
	void init(TBoardConfigMOSAIC *config);
	void setIPaddress(const char *IPaddr, int UDPport=DEFAULT_UDP_PORT);
	void connectTCP(int port=DEFAULT_TCP_PORT, int rcvBufferSize=DEFAULT_TCP_BUFFER_SIZE);
	void closeTCP();
	ssize_t recvTCP(void *rxBuffer, size_t count, int timeout);
	ssize_t readTCPData(void *buffer, size_t count, int timeout);
//	void cleanHeader(TBoardHeader *AHeader);
//	void copyHeader(const TBoardHeader *SourceHeader, TBoardHeader *DestinHeader);

	void addDataReceiver(int id, MDataReceiver *dr);
	void flushDataReceivers();
	bool waitResetTransreceiver();
	void enableDefinedReceivers();

	void setupPLL() { mSysPLL->setup(); return;};
	void setPhase(int APhase, int ACii = 0) { controlInterface[ACii]->setPhase(APhase); controlInterface[ACii]->addSendCmd(ControlInterface::OPCODE_GRST); controlInterface[ACii]->execute();return;};
	void setSpeedMode(bool AHSpeed, int Aindex = -1);
	void enableExternalTrigger(bool isEnabled, bool levelSensitive = 0) { mTriggerControl->addEnableExtTrigger(isEnabled, levelSensitive);return;};
	uint32_t buf2ui(unsigned char *buf);

//	int returnDataOut(MDataReceiver *AReceiver, int &nBytes, char *buffer);

protected:
  


};
#endif    /* READOUTBOARDMOSAIC_H */
