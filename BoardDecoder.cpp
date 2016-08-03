#include <iostream>

#include "BoardDecoder.h"

bool BoardDecoder::DecodeEvent(TBoardType boardType, unsigned char *data, int &nBytes, TBoardHeader &boardInfo) {
  if (boardType == boardDAQ) {
    return DecodeEventDAQ(data, nBytes, boardInfo);
  }
  else if (boardType == boardMOSAIC) {
    return DecodeEventMOSAIC(data, nBytes, boardInfo);
  }
  else {
    std::cout << "TBoardDecoder: Unknown board type" << std::endl;
    return false;
  }
}

