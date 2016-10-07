/* -------------------------------------------------
 *   Derived TReadOutBoard Class for MOSAIC board
 *
 *   ver.0.1		2/8/2016
 *
 *  Auth.: A.Franco	-  INFN BARI
 *
 *  		HISTORY
 *  3/8/16	-	Add the Board decoder class ...
 *  5/8/16  - adapt the read event to new definition
 *
 */
#include <math.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include "TReadoutBoardMOSAIC.h"
#include "BoardDecoder.h"
#include "TAlpide.h"

using namespace std;
ForwardReceiver::ForwardReceiver() : MDataReceiver()
{
}
ForwardReceiver::~ForwardReceiver()
{
}

MosaicException::MosaicException()
{
}
MosaicException::MosaicException(const string& arg)
{
	msg = arg;
}
MosaicException::~MosaicException() throw()
{
}
const char* MosaicException::what() const throw()
{
	return msg.c_str();
}


MosaicSetupError::MosaicSetupError(const string& __arg)
{
		 msg = "MOSAIC Board Setup error: " + __arg;
}

MosaicRuntimeError::MosaicRuntimeError(const string& __arg)
{
	 msg = "MOSAIC Board Run error: " + __arg;
}






// ---- Constructor
TReadoutBoardMOSAIC::TReadoutBoardMOSAIC (TBoardConfigMOSAIC *AConfig) : TReadoutBoard(AConfig)
{
  fBoardConfig = AConfig;
  init(AConfig);
}

// Distructor
TReadoutBoardMOSAIC::~TReadoutBoardMOSAIC()
{
	closeTCP(); // close the TCP connection
	delete dr;
	delete pulser;
	for(int i=0;i<MAX_MOSAICCTRLINT;i++) if(controlInterface[i] != NULL) delete controlInterface[i];
	delete i2cBus;
	delete dataGenerator;
	delete mRunControl;
	delete mIPbus;
	delete mTriggerControl;

}



/* -------------------------
	Public methods
  -------------------------- */

// Read/Write registers
int TReadoutBoardMOSAIC::WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId)
{

	uint_fast16_t Cii = GetControlInterface(chipId);
	controlInterface[Cii]->addWriteReg(chipId, address, value);
	controlInterface[Cii]->execute();
	return(0);
}

int TReadoutBoardMOSAIC::ReadChipRegister  (uint16_t address, uint16_t &value, uint8_t chipId)
{
	uint_fast16_t Cii = GetControlInterface(chipId);
	controlInterface[Cii]->addReadReg( chipId,  address,  &value);
	controlInterface[Cii]->execute();
	return(0);
}

int TReadoutBoardMOSAIC::SendOpCode        (uint16_t  OpCode, uint8_t chipId)
{
	uint_fast16_t Cii = GetControlInterface(chipId);
	controlInterface[Cii]->addSendCmd(OpCode);
	controlInterface[Cii]->execute();
	return(0);
}

int TReadoutBoardMOSAIC::SendOpCode        (uint16_t  OpCode)
{
	for(int Cii=0;Cii<MAX_MOSAICCTRLINT;Cii++){
		controlInterface[Cii]->addSendCmd(OpCode);
		controlInterface[Cii]->execute();
	}
	return(0);
}


int TReadoutBoardMOSAIC::SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay)
{

	uint16_t pulseMode;
	if(enablePulse  && enableTrigger)  pulseMode = 3; 
	if(enablePulse  && !enableTrigger) pulseMode = 1;
	if(!enablePulse && enableTrigger)  pulseMode = 2;
	if(!enablePulse && !enableTrigger) pulseMode = 0;
	pulser->setConfig(triggerDelay, pulseDelay, pulseMode);
	return(pulseMode);
}

void TReadoutBoardMOSAIC::SetTriggerSource  (TTriggerSource triggerSource)
{
	if(triggerSource == trigInt) { // Internal Trigger
		enableExternalTrigger(false,0);

	} else { // external trigger
		enableExternalTrigger(true,0);
	}

}

int  TReadoutBoardMOSAIC::Trigger           (int nTriggers)
{
	pulser->run(nTriggers);
	return(nTriggers);
}

void TReadoutBoardMOSAIC::StartRun()
{
        enableDefinedReceivers();
	connectTCP(); // open TCP connection
	mRunControl->startRun(); // start run
	usleep(5000); 	//
	return;
}

void TReadoutBoardMOSAIC::StopRun()
{
	 pulser->run(0);
	 mRunControl->stopRun();
	 closeTCP();  // FIXME: this could cause the lost of the tail of the buffer ...
	 return;
}

/*
//
//	Read data from the TCP socket version to unpack the stream one event for call
//
int  TReadoutBoardMOSAIC::ReadEventData(int &nBytes, char *buffer)
{

	const unsigned int bufferSize = DATA_INPUT_BUFFER_SIZE;
	unsigned char rcvbuffer[bufferSize];
	const unsigned int headerSize = 64;
	unsigned char header[headerSize];
	unsigned int flags;
	long blockSize, readBlockSize;
	long closedDataCounter;
	long movedEvents;
	long readDataSize = headerSize;
	int dataSrc;
	ssize_t n;

	for (int i=0; i<numOfSetReceivers; i++) {
		if(closedEventsPerSource[i] > 0) { // ok we have some thing to return !
			movedEvents = returnDataOut(receivers[i], nBytes, buffer); // move and clean the buffer
			closedEventsPerSource[i] -= movedEvents; // update the data counter
			return(1);  // FIXME: what is the return parameter ??
		}
	}

	// The Data Counters are all empty then we read some thing ...
	n = readTCPData(header, headerSize, fBoardConfigDAQ->GetPollingDataTimeout() ); 		// Read the TCP/IP socket
	cleanHeader(&theHeaderOfReadData);
	if (n == 0)	{		// timeout
		theHeaderOfReadData.timeout = true;
		return 0;
	}
	memcpy(theHeaderBuffer, header, MOSAIC_HEADER_LENGTH);// save the 16 bytes of the header

	blockSize = buf2ui(header);	// Decodes the received block ...
	theHeaderOfReadData.size = blockSize;

	flags = buf2ui(header+4);
	theHeaderOfReadData.overflow = flags & flagOverflow;
	theHeaderOfReadData.endOfRun = flags & flagCloseRun;
	theHeaderOfReadData.timeout = flags & flagTimeout;
	theHeaderOfReadData.closedEvent = flags & flagClosedEvent;

	closedDataCounter = buf2ui(header+8);
	theHeaderOfReadData.eoeCount = closedDataCounter;

	dataSrc = buf2ui(header+12);
	theHeaderOfReadData.channel = dataSrc;

	//	printf("Received TCP Header (%d) : Block Size = %d  Flags = %04x(Ovr %d, Tim %d Run %d Eve %d) DataCounters = %d DataSource = %04x",headerSize, blockSize,flags,flags & flagOverflow, flags & flagTimeout, flags & flagCloseRun, flags & flagClosedEvent, closedDataCounter,dataSrc);

	readBlockSize = (blockSize & 0x3f) ? (blockSize & ~0x3f)+64: blockSize; // round the block size to the higher 64 multiple
	readDataSize+=readBlockSize;

	if (blockSize==0 && (flags & flagCloseRun)==0) { // Abort operation on error
		throw MosaicRuntimeError("Block size set to zero and not CLOSE_RUN");
	}
	if (dataSrc>numOfSetReceivers || receivers[dataSrc] == NULL) {  	// skip data from unregistered source
		// printf("Skipping data block from unregistered source = %d ",dataSrc);
		while (readBlockSize) {
			n = readTCPData(rcvbuffer, (readBlockSize>bufferSize) ? bufferSize : readBlockSize, -1);
			readBlockSize -= n;
		}
		return(0);
	}
	if((flags & flagOverflow) != 0) { // track the event
		std::cerr << " ERROR  ATTENTION !! We receive an Overflow Flag error. Pay attention! "<< std::endl; // FIXME : Need to decide what we will do ?
	}

	MDataReceiver *dr = receivers[dataSrc];  // finally ...we can read data into the consumer buffer
	if (readBlockSize != 0) { // Added By G.DeRobertis
		n = readTCPData(dr->getWritePtr(readBlockSize), readBlockSize, -1); // read from TCP one Block of data and stores at the tail of buffer
		if (n == 0) {		// timeout
			// printf("Data block not received. Exit for Timeout !");
			return(0);
		}
		dr->dataBufferUsed += (n<blockSize) ? n : blockSize;		// update the size of data in the buffer
		// printf("We read the TCP block. Data read=%d Block size in the header=%d Block size to read=%d ",n,blockSize,readBlockSize);
	}
	if (closedDataCounter>0) { // We have closed transfers
		closedEventsPerSource[dataSrc] += closedDataCounter; // update the number of Events
		movedEvents = returnDataOut(receivers[dataSrc], nBytes, buffer); // move and clean the buffer
		closedEventsPerSource[dataSrc] -= movedEvents; // update the data counter
		return(1);  // FIXME: what is the return parameter ??
	}

	if ((flags & flagCloseRun) && dr->dataBufferUsed!=0) {  // Here we have some mismatch between the buffer and the status  ?
		std::cout << "WARNING Received data with flagCloseRun but after parsing the databuffer is not empty (" << dr->dataBufferUsed<<" bytes)" << std::endl;
		//	dump((unsigned char*) &dr->dataBuffer[0], dr->dataBufferUsed);
	}
	return(0);
}
*/

//	Read data from the TCP socket second version
//
int  TReadoutBoardMOSAIC::ReadEventData(int &nBytes, unsigned char *buffer)
{
	const unsigned int bufferSize = DATA_INPUT_BUFFER_SIZE;
	unsigned char rcvbuffer[bufferSize];
	const unsigned int headerSize = MOSAIC_HEADER_LENGTH;
	unsigned char header[headerSize];
	unsigned int flags;
	long blockSize, readBlockSize;
	long closedDataCounter;
	long movedEvents;
	long readDataSize = headerSize;
	int dataSrc;
	ssize_t n;

	n = readTCPData(header, headerSize, fBoardConfig->GetPollingDataTimeout() ); 		// Read the TCP/IP socket
	if (n == 0)	{		// timeout
		theHeaderOfReadData.timeout = true;
		std::cout << "Header timeout " << std::endl;
		return(-1);
	}
	memcpy(theHeaderBuffer, header, headerSize);// save the 64 bytes of the header

	blockSize = buf2ui(header);	// Decodes the received block ...
	theHeaderOfReadData.size = blockSize;

	flags = buf2ui(header+4);
	theHeaderOfReadData.overflow = flags & flagOverflow;
	theHeaderOfReadData.endOfRun = flags & flagCloseRun;
	theHeaderOfReadData.timeout = flags & flagTimeout;
	theHeaderOfReadData.closedEvent = flags & flagClosedEvent;

	closedDataCounter = buf2ui(header+8);
	theHeaderOfReadData.eoeCount = closedDataCounter;

	dataSrc = buf2ui(header+12);
	theHeaderOfReadData.channel = dataSrc;

      	//printf("Received TCP Header (%d) : Block Size = %d  Flags = %04x(Ovr %d, Tim %d Run %d Eve %d) DataCounters = %d DataSource = %04x",headerSize, blockSize,flags,flags & flagOverflow, flags & flagTimeout, flags & flagCloseRun, flags & flagClosedEvent, closedDataCounter,dataSrc);

	readBlockSize = (blockSize & 0x3f) ? (blockSize & ~0x3f)+64: blockSize; // round the block size to the higher 64 multiple
	readDataSize+=readBlockSize;

	if (blockSize==0 && (flags & flagCloseRun)==0) { // Abort operation on error
		throw MosaicRuntimeError("Block size set to zero and not CLOSE_RUN");
	}
	if (dataSrc>numOfSetReceivers || receivers[dataSrc] == NULL) {  	// skip data from unregistered source
		// printf("Skipping data block from unregistered source = %d ",dataSrc);
		while (readBlockSize) {
			n = readTCPData(rcvbuffer, (readBlockSize>bufferSize) ? bufferSize : readBlockSize, -1);
			readBlockSize -= n;
		}
		return(-1);
	}

	if((flags & flagOverflow) != 0) { // track the event
		std::cerr << " ERROR  ATTENTION !! We receive an Overflow Flag error. Pay attention! "<< std::endl;
	}

	MDataReceiver *dr = receivers[dataSrc];  // finally ...we can read data into the consumer buffer
	if (readBlockSize != 0) { // Added By G.DeRobertis
		n = readTCPData(dr->getWritePtr(readBlockSize), readBlockSize, -1); // read from TCP one Block of data and stores at the tail of buffer
		if (n == 0) {		// timeout
			// printf("Data block not received. Exit for Timeout !");
			return(-1);
		}
		dr->dataBufferUsed += (n<blockSize) ? n : blockSize;		// update the size of data in the buffer
		// printf("We read the TCP block. Data read=%d Block size in the header=%d Block size to read=%d ",n,blockSize,readBlockSize);
	}
	if (closedDataCounter>0) { // We have closed transfers
		memcpy(buffer, theHeaderBuffer, headerSize); // first copy the header
		buffer = buffer + headerSize; // move the pointer
		nBytes = headerSize;
		*(buffer+20) = dr->dataBuffer[dr->dataBufferUsed-1]; // copy the last trailer byte (MOSAIC status) into the header in order to decode it later

		memcpy(buffer, &dr->dataBuffer[0], dr->dataBufferUsed); // sets the output
		nBytes += dr->dataBufferUsed;
		dr->dataBufferUsed = 0; // flush the buffer
		return(true);
	}
	if ((flags & flagCloseRun) && dr->dataBufferUsed!=0) {  // Here we have some mismatch between the buffer and the status  ?
		std::cout << "WARNING Received data with flagCloseRun but after parsing the databuffer is not empty (" << dr->dataBufferUsed<<" bytes)" << std::endl;
		//	dump((unsigned char*) &dr->dataBuffer[0], dr->dataBufferUsed);
	}
	return(-1);
}





/* -----------------------------------------
 * This acts as a pre parser move the event into the out buffer
-----------------------------------------
int TReadoutBoardMOSAIC::returnDataOut(MDataReceiver *AReceiver, int &nBytes, char *buffer)
{
	if(AReceiver->dataBufferUsed == 0){ // nothing to do return null !
		std::cerr << "WARNING Open Events, but buffer empty !!!" << std::endl;
		buffer[0] = '\0';
		nBytes = 0;
		return(0); // zero events removed
	}
	char *ptrScan, *ptrStartEvent, *ptrEndEvent, *ptrEnd, *ptrBegin;
	size_t bytesToMove;

	ptrBegin = &AReceiver->dataBuffer[0];
	ptrEnd = &AReceiver->dataBuffer[AReceiver->dataBufferUsed-1];
	ptrScan = ptrBegin;
	ptrStartEvent = NULL;
	ptrEndEvent = NULL;

	while(ptrScan <= ptrEnd) { // start the light Parser
		if(*ptrScan == (char)0xff || *ptrScan == (char)0xbc || *ptrScan == (char)0xf1  || *ptrScan == (char)0xf0) {//return DT_IDLE,DT_COMMA,DT_BUSYON,DT_BUSYOFF;
			ptrScan++;
  	    } else if ((*ptrScan & 0xf0) == 0xa0) { // The Chip Id CHIPHEADER START of EVENT
  	    	ptrStartEvent = ptrScan;
  	    	ptrScan+=2;
  	    } else if ((*ptrScan & 0xf0) == 0xb0) { // The Chip Id DT_CHIPTRAILER END of EVENT
  	    	if(ptrStartEvent == NULL) { // we receive a End Event without start event
  	    		std::cerr << "WARNING received a frame with EndOfEvent without StartOfEvent !" << std::endl;
  	    		ptrScan+=3;
  	    	} else {
  	    		if(ptrScan[2] != 0) {
  	    			std::cerr << "WARNING receive a MOSAIC packet error=" << ptrScan[2] << endl;
  	    			//theNumberOfMOSAICError++;
  	    		}
  	    		ptrEndEvent = ptrScan+1; // we remove the MOSAIC Flag byte
  	    		bytesToMove = (ptrEndEvent - ptrStartEvent +1);
  	    		if (bytesToMove>0) {
  	    			memcpy(buffer, theHeaderBuffer, MOSAIC_HEADER_LENGTH); // first mount the header
  	    			buffer = buffer + MOSAIC_HEADER_LENGTH; // move the pointer
  	    			memcpy(buffer, ptrStartEvent, bytesToMove); // sets the output
  	  	    		nBytes = bytesToMove + MOSAIC_HEADER_LENGTH;
  	    		}
  	    		bytesToMove = AReceiver->dataBufferUsed - (ptrScan+3 - ptrBegin);
  	    		if (bytesToMove>0) memmove(ptrBegin, ptrScan+3, bytesToMove);	// move unused bytes to the begin of buffer
  	    		AReceiver->dataBufferUsed -= bytesToMove;

  	    		// NOW WE CAN EXIT ...
  	    		return(1); // one event removed
  	    	}
  	    } else if ((*ptrScan & 0xf0) == 0xe0) { //return DT_CHIPEMPTYFRAME;
  	    	//theNumberOfEmptyFrame++;
  	    	ptrScan+=3;
		} else if ((*ptrScan & 0xe0) == 0xc0) { //return DT_REGHEADER;
			ptrScan++;
		} else if ((*ptrScan & 0xc0) == 0x40) { //return DT_DATASHORT;
			ptrScan+=2;
		} else if ((*ptrScan & 0xc0) == 0x00) { //  return DT_DATALONG;
			ptrScan+=3;
		} else {
			std::cerr << "WARNING received an unknown byte !" << std::endl;
			ptrScan++; //return DT_UNKNOWN;
		}
	}
	if(ptrStartEvent != NULL) { // we found a StartEvent without an End of Event
		buffer[0] = '\0';
		nBytes = 0;

  		bytesToMove = AReceiver->dataBufferUsed - (ptrStartEvent - ptrBegin); // we clean the buffer up to the start of event
  		if (bytesToMove>0) memmove(ptrBegin, ptrStartEvent, bytesToMove);	// move unused bytes to the begin of buffer
  		AReceiver->dataBufferUsed -= bytesToMove;
  		return(0);
	}
	// at this point the buffer is full of trash
	AReceiver->dataBufferUsed = 0; // flush the buffer
	return(0);
}

// clean all fields of the structure
void TReadoutBoardMOSAIC::cleanHeader(TBoardHeader *AHeader)
{
	if(AHeader == NULL) return;
	AHeader->channel = 0;
	AHeader->eoeCount = 0;
	AHeader->timeout = false;
	AHeader->endOfRun = false;
	AHeader->overflow = false;
	return;
}

void TReadoutBoardMOSAIC::copyHeader(const TBoardHeader *SourceHeader, TBoardHeader *DestinHeader)
{
	if(SourceHeader == NULL || DestinHeader == NULL) return;
	DestinHeader->channel = SourceHeader->channel;
	DestinHeader->eoeCount = SourceHeader->eoeCount;
	DestinHeader->timeout = SourceHeader->timeout;
	DestinHeader->endOfRun = SourceHeader->endOfRun;
	DestinHeader->overflow = SourceHeader->overflow;
	return;
}
*/

/* -------------------------
 * 		Private Methods
   ------------------------- */
// Private : Init the board
void TReadoutBoardMOSAIC::init(TBoardConfigMOSAIC *config)
{
	mIPbus = new IPbusUDP();  // Create the Instance for the IPbus
	mRunControl = new MRunControl(mIPbus, WbbBaseAddress::runControl); 	// Run control
	tcp_sockfd = -1;

	// I2C master (WBB slave) and connected peripherals
	i2cBus = new I2Cbus(mIPbus, WbbBaseAddress::i2cMaster);

	// System PLL on I2C bus
	mSysPLL = new I2CSysPll(mIPbus, WbbBaseAddress::i2cSysPLL);

	setIPaddress(config->GetIPaddress(), config->GetTCPport());

	// CMU Control interface
	controlInterface[0] = new ControlInterface(mIPbus, WbbBaseAddress::controlInterface);
	controlInterface[1] = new ControlInterface(mIPbus, WbbBaseAddress::controlInterfaceB);

	// Pulser
	pulser = new Pulser(mIPbus, WbbBaseAddress::pulser);

	// ALPIDE3 Hi Speed data receiver
	for(int i=0; i<MAX_MOSAICTRANRECV-1;i++)
		a3rcv[i] = new Alpide3rcv(mIPbus, WbbBaseAddress::alpide3rcv+(i<<24) );

	// Data Generator
	dataGenerator = new MDataGenerator(mIPbus, WbbBaseAddress::dataGenerator);

	// The data consumer for hardware generator
	numOfSetReceivers = 0;
	dr = new MDataSave;
	addDataReceiver(0, (MDataReceiver *)dr);

	for(int i=1; i<MAX_MOSAICTRANRECV;i++) {
		dr =(MDataSave *) new ForwardReceiver();
		addDataReceiver(i, (MDataReceiver *)dr);
		a3rcv[i-1]->addDisable(true);
	}

	// Trigger control
	mTriggerControl = new MTriggerControl(mIPbus, WbbBaseAddress::triggerControl);

	// ----- Now do the initilization -------
	setupPLL(); // set the PLL ! in order to start the communication
        if(!waitResetTransreceiver()) {
  		exit(-1);
  	}
        for(int i=0;i<MAX_MOSAICCTRLINT;i++) setPhase(config->GetCtrlInterfacePhase(),i);  // set the Phase shift on the line

	setSpeedMode(config->IsLowSpeedMode(), -1);// set 400 MHz mode

	pulser->run(0);
	mRunControl->stopRun();
	mRunControl->clearErrors();
	mRunControl->setAFThreshold(config->GetCtrlAFThreshold());
	mRunControl->setLatency(config->GetCtrlLatMode(), config->GetCtrlLatMode());
	mRunControl->setConfigReg(0);

	enableDefinedReceivers(); // enable all the defined data link

	return;
}

// ============================== DATA receivers private methods =======================================

// adds a data receiver ...
void TReadoutBoardMOSAIC::addDataReceiver(int id, MDataReceiver *dr)
{
	if (id+1 > numOfSetReceivers) receivers.resize(id+1, NULL);
	receivers[id] = dr;
	numOfSetReceivers=id+1;
	return;
}

// flush the data receivers...
void TReadoutBoardMOSAIC::flushDataReceivers()
{
	for (int i=0; i<numOfSetReceivers; i++) {
		if (receivers[i]!=NULL) {
			receivers[i]->dataBufferUsed=0;
			receivers[i]->flush();
		}
		closedEventsPerSource[i] = 0;
	}
	return;
}

// Wait
bool TReadoutBoardMOSAIC::waitResetTransreceiver()
{
	uint32_t st;
	long int init_try; // wait 1s for transceivers reset done
	for (init_try=1000; init_try>0; init_try--){
		usleep(1000);
		mRunControl->getStatus(&st);
		if ((st & BOARD_STATUS_INIT_OK) == BOARD_STATUS_INIT_OK) break;
	}
	if (init_try==0) {
		throw MosaicRuntimeError("Failed to reset the transceivers of the MOSAIC board ! Abort");
	    return(false);
	}
	return(true);
}

void TReadoutBoardMOSAIC::enableDefinedReceivers()
{
	int dataLink;
	for(int i=0;i< fChipPositions.size(); i++) { //for each defined chip
		dataLink = fChipPositions.at(i).receiver;
		if(dataLink >= 0) { // Enable the data receiver
		  if (fChipPositions.at(i).enabled) {
			a3rcv[dataLink]->addDisable(false);
		  }
                  else {
			a3rcv[dataLink]->addDisable(true);
		  }
		}
	}
	return;
}

// ==============================TCP/IP private methods =======================================

// Just sets the IP address and the UDP port into the private variables
void TReadoutBoardMOSAIC::setIPaddress(const char *IPaddr, int UDPport)
{
	ipAddress.assign(IPaddr);
	mIPbus->setIPaddress(IPaddr, UDPport);
	return;
}

// Do the TCP/IP connection setting the port and the maximum buffer size
void TReadoutBoardMOSAIC::connectTCP(int Aport, int rcvBufferSize)
{
	struct sockaddr_in servaddr;
	closeTCP();
	tcp_sockfd=socket(AF_INET,SOCK_STREAM,0);
	if (tcp_sockfd == -1)
		throw MosaicSetupError("Error in the TCP/IP socket creation."); // Socket creation

	if (rcvBufferSize!=0) { // Limit the maximum ammount of "in-flight" data. In linux setting the buffer size to 128 KB has the affect of limit the TCP receive window to 162 KB
		if (setsockopt(tcp_sockfd, SOL_SOCKET, SO_RCVBUF, &rcvBufferSize, sizeof rcvBufferSize) == -1) {
			closeTCP();
			throw MosaicSetupError("Error in the TCP/IP socket set. setsockopt() ");
		}
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(ipAddress.c_str());
	//servaddr.sin_port=htons(TCPport);
	servaddr.sin_port=htons(Aport);

	if (::connect(tcp_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
		closeTCP ();
		throw MosaicSetupError("Error connecting the TCP/IP socket");;
	}
	std::cout << "MOSAIC board : TCP/IP socket opened & connected !" << std::endl;
	return;
}

// Close the Connection ...
void TReadoutBoardMOSAIC::closeTCP()
{
	if (tcp_sockfd != -1)
		::close (tcp_sockfd);
	tcp_sockfd= -1;
	std::cout << "MOSAIC board : TCP/IP socket closed !" << std::endl;
	return;
}


// Poll the TCP/IP socket waiting for Input data, returns the size of the received data
ssize_t TReadoutBoardMOSAIC::recvTCP(void *rxBuffer, size_t count, int timeout)
{
	struct pollfd ufds;
	int rv;
	ssize_t rxSize=0;

	ufds.fd = tcp_sockfd;
	ufds.events = POLLIN|POLLNVAL; // check for normal read or error
     
	rv = poll(&ufds, 1, timeout);

	if (rv == -1) throw MosaicRuntimeError("TCP/IP Poll system call fails");

	if (rv > 0) { // There are Data !
		if (ufds.revents & POLLIN) { // check for events on sockfd:
			rxSize = recv(tcp_sockfd, rxBuffer, count, 0);
			if (rxSize==0 || rxSize == -1) throw MosaicRuntimeError("Board connection closed. Buffer overflow or fatal error!");
		} else if (ufds.revents & POLLNVAL){
			throw MosaicRuntimeError("Invalid file descriptor in poll system call");
		}
	}
	return(rxSize);
}

// Read data from the TCP/IP socket, returns the number of bytes read & fills the buffer
ssize_t TReadoutBoardMOSAIC::readTCPData(void *buffer, size_t count, int timeout)
{
	ssize_t p=0;
	ssize_t res;
	while (count){
		res = recvTCP(buffer, count, timeout);

		if (res == 0) {
			return p;
		}
		p+=res;
		buffer = (char*) buffer + res;
		count -= res;
		timeout = -1;		// disable timeout for segments following the first
	}
	return p;
}
// =========================================================================================

void TReadoutBoardMOSAIC::setSpeedMode(bool ALSpeed, int Aindex)
{
	int st,en;
	Aindex = -1;
	st = (Aindex != -1) ? Aindex : 0;
	en = (Aindex != -1) ? Aindex+1 : MAX_MOSAICTRANRECV-1;
	for(int i=st;i<en;i++) {
		a3rcv[i]->addSetLowSpeed(ALSpeed);
		a3rcv[i]->execute();
	}
	return;
}


// Adjust the read data by the internal configuration
uint32_t TReadoutBoardMOSAIC::buf2ui(unsigned char *buf)
{
#ifdef PLATFORM_IS_LITTLE_ENDIAN
	return (*(uint32_t *) buf) & 0xffffffff;
#else
	uint32_t d;
	d = *buf++;
	d |= (*buf++) << 8;
	d |= (*buf++) << 16;
	d |= (*buf++) << 24;
	return d;
#endif
}

// ================================== EOF ========================================





