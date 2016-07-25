#include "AlpideDecoder.h"
#include <stdint.h>

//using namespace AlpideDecoder;


TDataType AlpideDecoder::GetDataType(unsigned char dataWord) {
  if      (dataWord == 0xff)          return DT_IDLE;
  else if (dataWord == 0xf1)          return DT_BUSYON;
  else if (dataWord == 0xf0)          return DT_BUSYOFF;
  else if ((dataWord & 0xf0) == 0xa0) return DT_CHIPHEADER;
  else if ((dataWord & 0xf0) == 0xb0) return DT_CHIPTRAILER;
  else if ((dataWord & 0xf0) == 0xe0) return DT_EMPTYFRAME;
  else if ((dataWord & 0xe0) == 0xc0) return DT_REGIONHEADER;
  else if ((dataWord & 0xc0) == 0x40) return DT_DATASHORT;
  else if ((dataWord & 0xc0) == 0x0)  return DT_DATALONG;
  else return DT_UNKNOWN;
}


void AlpideDecoder::DecodeChipHeader (unsigned char *data, int &chipId, int &bunchCounter) {
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  bunchCounter = data_field & 0xff;
  chipId       = (data_field >> 8) & 0xf;
}


void AlpideDecoder::DecodeChipTrailer (unsigned char *data, int &flags) {
  flags = data[0] & 0xf;
}


void AlpideDecoder::DecodeRegionHeader (unsigned char *data, int &region) {
  region = data[0] & 0x1f;
}


void AlpideDecoder::DecodeEmptyFrame (unsigned char *data, int &chipId, int &bunchCounter) {
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  bunchCounter = data_field & 0xff;
  chipId       = (data_field >> 8) & 0xf;
}


void AlpideDecoder::DecodeDataWord (unsigned char *data, int region, std::vector <TPixHit> *hits, bool datalong) {
  TPixHit hit;
  int     address, hitmap_length;

  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  hit.region = region;
  hit.dcol   = (data_field & 0x3c00) >> 10;
  address    = (data_field & 0x03ff);

  if (datalong) {
    hitmap_length = 7;
  }
  else {
    hitmap_length = 0;
  }

  for (int i = -1; i < hitmap_length; i ++) {
    if ((i >= 0) && (! (data[2] >> i) & 0x1)) continue;     
    hit.address = address + (i + 1);
    hits->push_back (hit);
  }
}
