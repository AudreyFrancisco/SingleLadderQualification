#include "TBoardConfigRU.h"

TBoardConfigRU::TBoardConfigRU(const char *fName, int boardIndex) : TBoardConfig(fName,boardIndex) {
    this->fBoardType = TBoardType::boardRU;
}
