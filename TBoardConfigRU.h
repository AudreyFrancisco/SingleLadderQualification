#ifndef BOARDCONFIGRU_H
#define BOARDCONFIGRU_H

#include <cstdint>
#include <vector>

#include "TBoardConfig.h"

class TBoardConfigRU : public TBoardConfig {
public:
    TBoardConfigRU(const char *fName = 0, int boardIndex = 0);

    // Returns the connected Chip (as chipid) for a given Dataport index on a Dataport
    uint8_t getTransceiverChip(const uint8_t DP, const uint8_t index) const;


    struct TransceiverMapping {
        uint8_t chipId;
        uint8_t moduleId;
        TransceiverMapping(uint8_t chipId, uint8_t moduleId) : chipId(chipId), moduleId(moduleId) {}
    };
    std::vector<TransceiverMapping> getTransceiverMappings();

    uint8_t getConnector() const;

    enum class ReadoutSpeed {RO_400, RO_600, RO_1200};
    ReadoutSpeed getReadoutSpeed() const;
    bool getInvertPolarity() const;
};

#endif //BOARDCONFIGRU_H
