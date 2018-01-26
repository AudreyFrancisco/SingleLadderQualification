#ifndef ALPIDEDECODER_H
#define ALPIDEDECODER_H

#include <cstdint>
#include <vector>

typedef struct {
  int boardIndex;
  int channel;
  int chipId;
  int region;
  int dcol;
  int address;
} TPixHit;

enum TAlpideDataType {
  DT_IDLE,
  DT_CHIPHEADER,
  DT_CHIPTRAILER,
  DT_EMPTYFRAME,
  DT_REGIONHEADER,
  DT_DATASHORT,
  DT_DATALONG,
  DT_BUSYON,
  DT_BUSYOFF,
  DT_UNKNOWN
};

class AlpideDecoder {
private:
  static void DecodeChipHeader(unsigned char *data, int &chipId, unsigned int &bunchCounter);
  static void DecodeChipTrailer(unsigned char *data, int &flags);
  static void DecodeRegionHeader(unsigned char *data, int &region);
  static void DecodeEmptyFrame(unsigned char *data, int &chipId, unsigned int &bunchCounter);
  static bool DecodeDataWord(unsigned char *data, int chip, int region, std::vector<TPixHit> *hits,
                             bool datalong, int boardIndex, int channel, int &prioErrors,
                             std::vector<TPixHit> *stuck);

protected:
public:
  static TAlpideDataType GetDataType(unsigned char dataWord);
  static int GetWordLength(TAlpideDataType dataType);
  static bool DecodeEvent(unsigned char *data, int nBytes, std::vector<TPixHit> *hits,
                          int boardIndex, int channel, int &prioErrors,
                          std::vector<TPixHit> *stuck = 0, int *chipID = 0,
                          unsigned int *bunchCounter = 0);
  static bool ExtractNextEvent(unsigned char *data, int nBytes, int &eventStart, int &eventEnd,
                               bool &isError, bool logging = false);
};

#endif
