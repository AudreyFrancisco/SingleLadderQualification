#ifndef BOARDDECODER_H
#define BOARDDECODER_H 

#include "TReadoutBoard.h"

// put all header and trailer information here 
// (both for mosaic and DAQ board)
typedef struct {
  // common 
  int  size;
  // MOSAIC
  int  channel; 
  int  eoeCount;
  bool timeout;
  bool endOfRun;
  bool overflow;
  bool closedEvent;
  // DAQ board
  bool     almostFull;
  uint64_t eventId;
  uint64_t timestamp;
  bool     truncated;
  int      strobeCount;
  int      trigCountChipBusy;
  int      trigCountDAQBusy;
  int      extTrigCount;
} TBoardHeader;



// Board Decoder decodes the information contained in the readout board header and trailer
// the information is written into TBoardHeader (one common structure for DAQ board and readout board)
// data and nBytes are modified such that after the board decoding they correspond to the chip event only  
class BoardDecoder{
 private:
  static bool DecodeEventMOSAIC(unsigned char *data, int &nBytes, TBoardHeader &boardInfo);
  static bool DecodeEventDAQ   (unsigned char *data, int &nBytes, TBoardHeader &boardInfo) {return false;};
 public:
  static bool DecodeEvent(TBoardType boardType, unsigned char *data, int &nBytes, TBoardHeader &boardInfo);

 private:
  static uint32_t endianAdjust(unsigned char *buf);

};


#endif
