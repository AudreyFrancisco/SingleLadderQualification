#ifndef BOARDCONFIGRU_H
#define BOARDCONFIGRU_H

#include <cstdint>

#include "TBoardConfig.h"

class TBoardConfigRU : public TBoardConfig {
public:
    TBoardConfigRU(const char *fName = 0, int boardIndex = 0);

    // Returns the connected Chip (as chipid) for a given Dataport index on a Dataport
    uint8_t getTransceiverChip(const uint8_t DP, const uint8_t index);
};

#endif //BOARDCONFIGRU_H
