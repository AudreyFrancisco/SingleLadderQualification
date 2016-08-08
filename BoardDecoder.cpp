#include <iostream>

#include "BoardDecoder.h"

bool BoardDecoder::DecodeEvent(TBoardType boardType, unsigned char *data, int &nBytesHeader, int &nBytesTrailer, TBoardHeader &boardInfo) {
  if (boardType == boardDAQ) {
    return DecodeEventDAQ(data, nBytesHeader, boardInfo);
  }
  else if (boardType == boardMOSAIC) {
    return DecodeEventMOSAIC(data, nBytesHeader, nBytesTrailer, boardInfo);
  }
  else {
    std::cout << "TBoardDecoder: Unknown board type" << std::endl;
    return false;
  }
}


// Decodes the Event Header and fill the structure.
// The value of nBytes is filled with the length of read header
bool BoardDecoder::DecodeEventMOSAIC(unsigned char *header, int &nBytesHeader, int &nBytesTrailer, TBoardHeader &boardInfo)
{
	boardInfo.size = endianAdjust(header);
	uint32_t flags = endianAdjust(header+4);
	boardInfo.overflow = flags & (1 << 1);
	boardInfo.endOfRun = flags & (1 << 3);
	boardInfo.timeout = flags & (1 << 2);
	boardInfo.closedEvent = flags & (1 << 0);
	boardInfo.eoeCount = endianAdjust(header+8);
	boardInfo.channel = endianAdjust(header+12);
	nBytesHeader = 64; // #define MOSAIC_HEADER_LENGTH 64
	nBytesTrailer = 1; // #define The MOSAIC trailer length

	boardInfo.MOSAICtransmissionFlag = header[20];
	boardInfo.headerError = boardInfo.MOSAICtransmissionFlag & 0x01;
	boardInfo.decoder10b8bError = boardInfo.MOSAICtransmissionFlag & 0x02;
	return false;
};


// Adapt the (char x 4) -> (unsigned int) conversion depending to the endianess
uint32_t BoardDecoder::endianAdjust(unsigned char *buf)
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

