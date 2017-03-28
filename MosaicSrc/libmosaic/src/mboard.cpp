/*
 * Copyright (C) 2014
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * ====================================================
 *     __  __   __  _____  __   __
 *    / / /  | / / / ___/ /  | / / SEZIONE di BARI
 *   / / / | |/ / / /_   / | |/ /
 *  / / / /| / / / __/  / /| / /
 * /_/ /_/ |__/ /_/    /_/ |__/  	 
 *
 * ====================================================
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2014.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include "mexception.h"
#include "mdatareceiver.h"
#include "mboard.h"

#define PLATFORM_IS_LITTLE_ENDIAN

void MBoard::init()
{
	mIPbus = new IPbusUDP();

	// Data Generator
	mDataGenerator = new MDataGenerator(mIPbus, WbbBaseAddress::dataGenerator);

	// Run control
	mRunControl = new MRunControl(mIPbus, WbbBaseAddress::runControl);

	// Trigger control
	mTriggerControl = new MTriggerControl(mIPbus, WbbBaseAddress::triggerControl);

	// System PLL on I2C bus
	mSysPLL = new I2CSysPll(mIPbus, WbbBaseAddress::i2cSysPLL);	

	tcp_sockfd = -1;
	numReceivers = 0;
}

MBoard::MBoard() 
{
	init();
}

MBoard::MBoard(const char *IPaddr, int port) 
{
	init();
	setIPaddress(IPaddr, port);
}

void MBoard::setIPaddress(const char *IPaddr, int port)
{
	IPaddress = IPaddr;
	mIPbus->setIPaddress(IPaddr, port);
}

MBoard::~MBoard() 
{
	// close the TCP connection
	closeTCP();		

	// delete objects in creation reverse order
	delete mSysPLL;
	delete mDataGenerator;
	delete mRunControl;
	delete mIPbus;
}


void MBoard::addDataReceiver(int id, MDataReceiver *dr)
{
	if (id+1 > numReceivers)
		receivers.resize(id+1, NULL);
	receivers[id] = dr;	
	numReceivers=id+1;
}

void MBoard::flushDataReceivers()
{
	for (int i=0; i<numReceivers; i++)
		if (receivers[i]!=NULL){
			receivers[i]->dataBufferUsed=0;
			receivers[i]->flush();
		}	
}

void MBoard::connectTCP(int port, int rcvBufferSize)
{
	struct sockaddr_in servaddr;

	closeTCP();

	tcp_sockfd=socket(AF_INET,SOCK_STREAM,0);
	if (tcp_sockfd == -1)
		throw MDataConnectError("Socket creation");

	if (rcvBufferSize!=0){
		// Limit the maximum ammount of "in-flight" data
		// In linux setting the buffer size to 128 KB has the affect of limit
		// the TCP receive window to 162 KB
		if (setsockopt(tcp_sockfd, SOL_SOCKET, SO_RCVBUF, &rcvBufferSize,
			sizeof rcvBufferSize) == -1) {
			closeTCP();
			throw MDataConnectError("setsockopt system call");
		}
	}
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(IPaddress.c_str());
	servaddr.sin_port=htons(port);

	if (::connect(tcp_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
		closeTCP ();
		throw MDataConnectError("Can not connect");
	}
}

void MBoard::closeTCP()
{
	if (tcp_sockfd != -1)
		::close (tcp_sockfd);
	tcp_sockfd= -1;
}

ssize_t MBoard::recvTCP(void *rxBuffer, size_t count, int timeout)
{
	struct pollfd ufds;
	int rv;
	ssize_t rxSize;

	ufds.fd = tcp_sockfd;
	ufds.events = POLLIN|POLLNVAL; // check for normal read or error
	rv = poll(&ufds, 1, timeout);	

	if (rv == -1)
		throw MDataReceiveError("Poll system call");

	if (rv == 0)
		return 0;			// timeout
	
	// check for events on sockfd:
	rxSize = 0;
	if (ufds.revents & POLLIN) {
		rxSize = recv(tcp_sockfd, rxBuffer, count, 0);
		if (rxSize==0 || rxSize == -1)
			throw MDataReceiveError("Board connection closed. Fatal error!");
	} else if (ufds.revents & POLLNVAL){
		throw MDataReceiveError("Invalid file descriptor in poll system call");
	}

	return rxSize;
}

ssize_t MBoard::readTCPData(void *buffer, size_t count, int timeout)
{
	ssize_t p=0;
	ssize_t res;
	
	while (count){
		res = recvTCP(buffer, count, timeout);

		if (res == 0)
			return p;
		p+=res;
		buffer = (char*) buffer + res;
		count -= res;
		timeout = -1;		// disable timeout for segments following the first
	}

	return p;
}

unsigned int MBoard::buf2ui(unsigned char *buf)
{
#ifdef PLATFORM_IS_LITTLE_ENDIAN
	return (*(unsigned int *) buf) & 0xffffffff;
#else
	unsigned int d;

	d = *buf++;
	d |= (*buf++) << 8;
	d |= (*buf++) << 16;
	d |= (*buf++) << 24;

	return d;
#endif
}


/*
static void dump(unsigned char *buffer, int size)
{
	int i, j;
	
	for (i=0; i<size;){
		for (j=0; j<16; j++){
			printf(" %02x", buffer[i]);
			i++;
		}
		printf("\n");
	}
}
*/


//
//	Read data from the TCP socket and dispatch them to the receivers
//
long MBoard::pollTCP(int timeout, MDataReceiver **drPtr)
{
	const unsigned int bufferSize = 64*1024;
	unsigned char rcvbuffer[bufferSize];
	const unsigned int headerSize = MOSAIC_HEADER_SIZE;
	unsigned char header[headerSize];
	unsigned int flags;
	long blockSize, readBlockSize;
	long closedDataCounter;
	long readDataSize = headerSize;
	int dataSrc;
	ssize_t n;

	n = readTCPData(header, headerSize, timeout);

	if (n == 0)			// timeout
		return 0;

	blockSize = buf2ui(header);
	flags = buf2ui(header+4);	
	closedDataCounter = buf2ui(header+8);
	dataSrc = buf2ui(header+12);

	if (flags & flagOverflow)
		printf("****** Received data block with overflow flag set from source %d\n", dataSrc);

	// round the block size to the higer 64 multiple
	readBlockSize = (blockSize & 0x3f) ? (blockSize & ~0x3f)+64: blockSize;
	readDataSize+=readBlockSize;

	if (blockSize==0 && (flags & flagCloseRun)==0)
		throw MDataReceiveError("Block size set to zero and not CLOSE_RUN");

//	if (flags & flagCloseRun)
//		printf("Received Data packet with CLOSE_RUN\n");

	// skip data from unregistered source
	if (dataSrc>numReceivers || receivers[dataSrc]== NULL){
		printf("Skipping data block from unregistered source\n");
		while (readBlockSize){
			if (readBlockSize>bufferSize)
				n = readTCPData(rcvbuffer, bufferSize, -1);
			else
				n = readTCPData(rcvbuffer, readBlockSize, -1);
			readBlockSize -= n;
		}
		return readDataSize;
	}

	// read data into the consumer buffer
	MDataReceiver *dr = receivers[dataSrc];
	dr->blockFlags = flags;
	dr->blockSrc = dataSrc;
	memcpy(dr->blockHeader, header, MOSAIC_HEADER_SIZE);
	*drPtr = dr;

	if (readBlockSize != 0) {
		n = readTCPData(dr->getWritePtr(readBlockSize), readBlockSize, -1);
		if (n == 0)
			return 0;
	
		// update the size of data in the buffer
		if (n<blockSize)
			dr->dataBufferUsed += n;
		else
			dr->dataBufferUsed += blockSize;

		// printf("dr->dataBufferUsed: %ld closedDataCounter:%ld flags:0x%04x\n", dr->dataBufferUsed, closedDataCounter, flags);
	}
	dr->numClosedData += closedDataCounter;

	return readDataSize;
}



//
//	Read data from the TCP socket and send it to the receivers
//
long MBoard::pollData(int timeout)
{
	long readDataSize;
	long closedDataCounter;
	MDataReceiver *dr;

	readDataSize = pollTCP(timeout, &dr);
	closedDataCounter = dr->numClosedData;
	if (closedDataCounter>0){
		long parsedBytes = dr->parse(closedDataCounter);

		// move unused bytes to the begin of buffer
		size_t bytesToMove = dr->dataBufferUsed - parsedBytes;
		if (bytesToMove>0)
			memmove(&dr->dataBuffer[0], &dr->dataBuffer[parsedBytes], bytesToMove);
		dr->dataBufferUsed -= parsedBytes;
		dr->numClosedData = 0;
	}

	if ((dr->blockFlags & flagCloseRun) && dr->dataBufferUsed!=0){
		printf("WARNING: MBoard::pollData received data with flagCloseRun but after parsing the databuffer is not empty (%ld bytes)\n",
						dr->dataBufferUsed);		
	//	dump((unsigned char*) &dr->dataBuffer[0], dr->dataBufferUsed);
	}

	return readDataSize;
}




