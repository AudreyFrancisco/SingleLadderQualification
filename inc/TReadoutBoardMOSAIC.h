/* -------------------------------------------------
 *   Derived TReadOutBoard Class for MOSAIC board
 *
 *   ver.0.1		3/8/2016
 *
 *  Auth.: A.Franco	-  INFN BARI
 *
 *  		HISTORY
 *
 *  23/05/17 - 	Add the support for the endurance Test Board
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
#include "Mosaic.h"
#include "powerboard.h"


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

	long parse(int numClosed) { (void)numClosed; return(dataBufferUsed); };
};

// -----------------------------------------------------

class TReadoutBoardMOSAIC : public TReadoutBoard, private MBoard
{

// Methods
public:
	TReadoutBoardMOSAIC(TConfig* config, TBoardConfigMOSAIC *boardConfig);
	virtual ~TReadoutBoardMOSAIC();

	int WriteChipRegister (uint16_t address, uint16_t value, TAlpide *chipPtr);
	int ReadChipRegister  (uint16_t address, uint16_t &value, TAlpide *chipPtr);
	int SendOpCode        (Alpide::TOpCode OpCode, TAlpide *chipPtr);
	int SendOpCode        (Alpide::TOpCode OpCode);
	int SendCommand       (Alpide::TCommand Command, TAlpide *chipPtr);
        // Markus: changed trigger delay type from uint32_t to int, since changed upstream
	int SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay);
	void SetTriggerSource  (TTriggerSource triggerSource);
	int Trigger           (int nTriggers);
        // Markus: changed data type from char to unsigned char; check that no problem
        // (should be OK at least for memcpy)
	int ReadEventData     (int &nBytes, unsigned char *buffer);
	void StartRun();
	void StopRun();

	int ReadRegister      (uint16_t Address, uint32_t &Value) { (void)Address; (void)Value; return(0);};
	int WriteRegister     (uint16_t Address, uint32_t Value)  { (void)Address; (void)Value; return(0);};
	void enableControlInterfaces(bool en);
	void enableClockOutput(bool en) { enableControlInterfaces(en); return; } // just a wrapper
	void setInverted (bool AInverted, int Aindex = -1);

	bool PowerOn           ();
	void PowerOff          ();

	int  GetFwMajVersion() { return(theVersionMaj); };
	int  GetFwMinVersion() { return(theVersionMin); };
	char *GetFwIdString() { return(theVersionId); };
	powerboard *GetPowerBoardHandle() { return(pb); };



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

	uint32_t decodeError();
	char *getFirmwareVersion();

// Properties
private:
	TBoardConfigMOSAIC *fBoardConfig;
	//TConfig            *fConfig; YCM:FIXME, not used
	I2Cbus 	 			*i2cBus;
	I2Cbus 	 			*i2cBusAux;
	powerboard 			*pb;
	ControlInterface 	*controlInterface[MAX_MOSAICCTRLINT];
	Pulser			 	*pulser;
	ALPIDErcv			*alpideRcv[MAX_MOSAICTRANRECV];
	TAlpideDataParser	*alpideDataParser[MAX_MOSAICTRANRECV];
	DummyReceiver 		*dr;
	//TBoardHeader 		theHeaderOfReadData;  // This will host the info catch from Packet header/trailer YCM: FIXME, not used

	char 				theVersionId[50];  // Version properties
	int					theVersionMaj;
	int					theVersionMin;



private:
	// extend WBB address definitions in mwbb.h
	enum baseAddress_e {
		add_i2cMaster				= (5 << 24),
		add_controlInterface		= (6 << 24),
		add_controlInterfaceB		= (7 << 24),
		add_alpideRcv				= (8 << 24),
		// total of 10 alpideRcv
		add_trgRecorder				= (18 << 24),
		add_controlInterface_0		= (19 << 24),
		add_controlInterface_9		= (28 << 24),
		add_i2cAux					= (29 << 24)
	};

	// status register bits
	enum BOARD_STATUS_BITS {
		BOARD_STATUS_FEPLL_LOCK			= 0x0001,
		BOARD_STATUS_EXTPLL_LOCK		= 0x0002,
		BOARD_STATUS_GTPLL_LOCK			= 0x0004,
		BOARD_STATUS_GTP_RESET_DONE		= 0x3ff0000
	};

	enum configBits_e {
		CFG_EXTCLOCK_SEL_BIT	= (1<<0),		// 0: internal clock - 1: external clock
		CFG_CLOCK_20MHZ_BIT		= (1<<1),		// 0: 40 MHz clock	- 1: 20 MHz clock
		CFG_RATE_MASK			= (0x03<<2),
		CFG_RATE_1200			= (0<<2),
		CFG_RATE_600			= (0x01<<2),
		CFG_RATE_400			= (0x02<<2)
	};

	static I2CSysPll::pllRegisters_t sysPLLregContent;
};
#endif    /* READOUTBOARDMOSAIC_H */
