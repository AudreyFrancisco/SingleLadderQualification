#ifndef ALPIDEDECODER_H
#define ALPIDEDECODER_H

#include <vector>

enum TDataType {DT_IDLE, DT_CHIPHEADER, DT_CHIPTRAILER, DT_EMPTYFRAME, DT_REGIONHEADER, DT_DATASHORT, DT_DATALONG, DT_BUSYON, DT_BUSYOFF, DT_UNKNOWN};

typedef struct {
  int region; 
  int dcol;
  int address;
} TPixHit;

class AlpideDecoder {
 private:
   static TDataType GetDataType        (unsigned char dataWord);
   static void      DecodeChipHeader   (unsigned char *data, int &chipId, int &bunchCounter);
   static void      DecodeChipTrailer  (unsigned char *data, int &flags);
   static void      DecodeRegionHeader (unsigned char *data, int &region);
   static void      DecodeEmptyFrame   (unsigned char *data, int &chipId, int &bunchCounter);
   static void      DecodeDataWord     (unsigned char *data, int region, std::vector <TPixHit> *hits, bool datalong);
 protected:
 public:
};



#endif
