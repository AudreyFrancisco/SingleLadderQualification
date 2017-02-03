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
#include <deque>

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
#include "MosaicSrc/alpidercv.h"
#include "MosaicSrc/i2csyspll.h"
#include "MosaicSrc/mboard.h"
#include "MosaicSrc/mdatareceiver.h"
#include "MosaicSrc/mdatagenerator.h"
#include "MosaicSrc/TAlpideDataParser.h"

// Constant Definitions
#define DEFAULT_PACKET_SIZE 		1400
#define DEFAULT_UDP_PORT			2000
#define DEFAULT_TCP_BUFFER_SIZE		(512*1024)			// if set to 0 : automatic
#define DEFAULT_TCP_PORT			3333

#define DATA_INPUT_BUFFER_SIZE	64*1024

//class string;
using namespace std;


extern std::vector<unsigned char> fDebugBuffer;

class DummyReceiver : public MDataReceiver
{
public:
	DummyReceiver() {} ;
	~DummyReceiver() {} ;

	long parse(int numClosed) { return(dataBufferUsed); };
};

// -----------------------------------------------------

class TReadoutBoardMOSAIC : public TReadoutBoard, private MBoard
{

// Methods
public:
	TReadoutBoardMOSAIC(TConfig* config, TBoardConfigMOSAIC *boardConfig);
	virtual ~TReadoutBoardMOSAIC();

	int WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId =0);
	int ReadChipRegister  (uint16_t address, uint16_t &value, uint8_t chipId =0);
	int SendOpCode        (uint16_t  OpCode, uint8_t chipId);
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
	void init();
	void enableDefinedReceivers();
	void setPhase(int APhase, int ACii = 0) {
			controlInterface[ACii]->setPhase(APhase);
			controlInterface[ACii]->addSendCmd(ControlInterface::OPCODE_GRST);
			controlInterface[ACii]->execute();
			return;
		};

	void setSpeedMode(Mosaic::TReceiverSpeed ASpeed, int Aindex = -1);
	void setInverted (bool AInverted, int Aindex = -1);

	uint32_t decodeError();


// Properties
private:
	TBoardConfigMOSAIC *fBoardConfig;
  TConfig            *fConfig;
	MDataGenerator 		*dataGenerator;
	I2Cbus 	 			*i2cBus;
	ControlInterface 	*controlInterface[MAX_MOSAICCTRLINT];
	Pulser			 	*pulser;
	ALPIDErcv			*alpideRcv[MAX_MOSAICTRANRECV];
	TAlpideDataParser	*alpideDataParser[MAX_MOSAICTRANRECV];
	DummyReceiver 		*dr;
	TBoardHeader 		theHeaderOfReadData;  // This will host the info catch from Packet header/trailer

private:
	// status register bits
	enum BOARD_STATUS_BITS {
		BOARD_STATUS_FEPLL_LOCK			= 0x0001,
		BOARD_STATUS_EXTPLL_LOCK		= 0x0002,
		BOARD_STATUS_GTPLL_LOCK			= 0x0004,
		BOARD_STATUS_GTP_RESET_DONE		= 0x3ff0000
	};

};
#endif    /* READOUTBOARDMOSAIC_H */
