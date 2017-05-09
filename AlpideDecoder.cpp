#include "AlpideDecoder.h"
#include <stdint.h>
#include <iostream>

//using namespace AlpideDecoder;


bool newEvent;

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


int AlpideDecoder::GetWordLength(TDataType dataType) {
  if (dataType == DT_DATALONG) {
    return 3;
  }
  else if ((dataType == DT_DATASHORT) || (dataType == DT_CHIPHEADER) || (dataType == DT_EMPTYFRAME)) {
    return 2;
  }
  else return 1;
}


void AlpideDecoder::DecodeChipHeader (unsigned char *data, int &chipId, unsigned int &bunchCounter) {
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  bunchCounter = data_field & 0xff;
  chipId       = (data_field >> 8) & 0xf;
  newEvent     = true;
}


void AlpideDecoder::DecodeChipTrailer (unsigned char *data, int &flags) {
  flags = data[0] & 0xf;
}


void AlpideDecoder::DecodeRegionHeader (unsigned char *data, int &region) {
  region = data[0] & 0x1f;
}


void AlpideDecoder::DecodeEmptyFrame (unsigned char *data, int &chipId, unsigned int &bunchCounter) {
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  bunchCounter = data_field & 0xff;
  chipId       = (data_field >> 8) & 0xf;
}


void AlpideDecoder::DecodeDataWord (unsigned char *data, int chip, int region, std::vector <TPixHit> *hits, bool datalong, int channel, int &prioErrors, std::vector <TPixHit> *stuck) {
  TPixHit hit;
  int     address, hitmap_length;

  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  if (chip == -1) {std::cout << "Warning, found chip id -1, dataword = 0x" <<std::hex << (int) data_field << std::dec << std::endl;}
  hit.channel = channel;
  hit.chipId = chip;
  hit.region = region;
  hit.dcol   = (data_field & 0x3c00) >> 10;
  address    = (data_field & 0x03ff);

  if ((hits->size() > 0) && (!newEvent)) {
    if ((hit.region == hits->back().region) && (hit.dcol == hits->back().dcol) && (address == hits->back().address)) {
      std::cout << "Warning (chip "<< chip << "/ channel "<< channel << "), received pixel " << hit.region << "/" << hit.dcol << "/" << address <<  " twice." << std::endl;
      prioErrors ++;
      if (stuck) stuck->push_back(hit);
    }
    else if ((hit.region == hits->back().region) && (hit.dcol == hits->back().dcol) && (address < hits->back().address)) {
      std::cout << "Warning (chip "<< chip << "/ channel "<< channel << "), address of pixel " << hit.region << "/" << hit.dcol << "/" << address <<  " is lower than previous one ("<< hits->back().address << ") in same double column." << std::endl;
      prioErrors ++;
      if (stuck) stuck->push_back(hit);
    }
  }

  if (datalong) {
    hitmap_length = 7;
  }
  else {
    hitmap_length = 0;
  }

  for (int i = -1; i < hitmap_length; i ++) {
    if ((i >= 0) && (! (data[2] >> i) & 0x1)) continue;
    hit.address = address + (i + 1);
  if (hit.chipId == -1) {std::cout << "Warning, found chip id -1" << std::endl;}
    hits->push_back (hit);
  }
  newEvent = false;
}

bool AlpideDecoder::ExtractNextEvent(unsigned char *data, int nBytes, int &eventStart, int &eventEnd, bool &isError, bool logging) {
  int       byte    = 0;
  bool      started = false; // event has started, i.e. chip header has been found
  bool      finished = false; // event trailer found
  TDataType type;

  unsigned char last;

  eventStart = 0;

  isError = false;
  while (byte < nBytes) {
    last = data[byte];
    type = GetDataType (data[byte]);
    switch (type) {
    case DT_IDLE:
      byte +=1;
      break;
    case DT_BUSYON:
      byte += 1;
      break;
    case DT_BUSYOFF:
      byte += 1;
      break;
    case DT_EMPTYFRAME:
      started = true;
      eventStart = byte;
      byte += 2;
      finished = true;
      break;
    case DT_CHIPHEADER:
      started = true;
      eventStart = byte;
      finished = false;
      byte += 2;
      break;
    case DT_CHIPTRAILER:
      if (!started) {
          if(logging)
              std::cout << "Error, chip trailer found before chip header" << std::endl;
        isError = true;
      }
      if (finished) {
          if(logging)
        std::cout << "Error, chip trailer found after event was finished" << std::endl;
        isError = true;
      }
      finished = true;
      byte += 1;
      break;
    case DT_REGIONHEADER:
      if (!started) {
          if(logging)
              std::cout << "Error, region header found before chip header or after chip trailer" << std::endl;
        isError = true;
      }
      byte +=1;
      break;
    case DT_DATASHORT:
      if (!started) {
          if(logging)
              std::cout << "Error, hit data found before chip header or after chip trailer" << std::endl;
        isError = true;
      }
      byte += 2;
      break;
    case DT_DATALONG:
      if (!started) {
          if(logging)
        std::cout << "Error, hit data found before chip header or after chip trailer" << std::endl;
          isError = true;
      }
      byte += 3;
      break;
    case DT_UNKNOWN:
        if(logging)
            std::cout << "Error, data of unknown type 0x" << std::hex << (int)data[byte] << std::dec << std::endl;
        isError=true;
        byte += 1;
    }
    if(finished || (started && isError))
        break;
    if(!started && isError)
        isError=false;
  }

  eventEnd = byte;

  // return true if Event end is found, false otherwise
  return finished or isError;
}


bool AlpideDecoder::DecodeEvent (unsigned char *data, int nBytes, std::vector <TPixHit> *hits, int channel, int &prioErrors, std::vector <TPixHit> *stuck) {
  int       byte    = 0;
  int       region  = -1;
  int       chip    = -1;
  int       flags   = 0;
  bool      started = false; // event has started, i.e. chip header has been found
  bool      finished = false; // event trailer found
  bool      corrupt  = false; // corrupt data found (i.e. data without region or chip)
  TDataType type;

  unsigned char last;

  unsigned int BunchCounterTmp;

  while (byte < nBytes) {
    last = data[byte];
    type = GetDataType (data[byte]);
    switch (type) {
    case DT_IDLE:
      byte +=1;
      break;
    case DT_BUSYON:
      byte += 1;
      break;
    case DT_BUSYOFF:
      byte += 1;
      break;
    case DT_EMPTYFRAME:
      started = true;
      DecodeEmptyFrame (data + byte, chip, BunchCounterTmp);
      byte += 2;
      finished = true;
      break;
    case DT_CHIPHEADER:
      started = true;
      finished = false;
      DecodeChipHeader (data + byte, chip, BunchCounterTmp);
      byte += 2;
      break;
    case DT_CHIPTRAILER:
      if (!started) {
        std::cout << "Error, chip trailer found before chip header" << std::endl;
        return false;
      }
      if (finished) {
        std::cout << "Error, chip trailer found after event was finished" << std::endl;
        return false;
      }
      DecodeChipTrailer (data + byte, flags);
      finished = true;
      chip = -1;
      byte += 1;
      break;
    case DT_REGIONHEADER:
      if (!started) {
        std::cout << "Error, region header found before chip header or after chip trailer" << std::endl;
        return false;
      }
      DecodeRegionHeader (data + byte, region);
      byte +=1;
      break;
    case DT_DATASHORT:
      if (!started) {
        std::cout << "Error, hit data found before chip header or after chip trailer" << std::endl;
        return false;
      }
      if (region == -1) {
	std::cout << "Warning: data word without region, skipping (Chip " << chip << ")" << std::endl;
        corrupt = true;
      }
      else if (hits) {
        if (chip == -1) {
          for (int i = 0; i < nBytes; i++) {
            printf("%02x ", data[i]);
	  }
          printf("\n");
	}
        DecodeDataWord (data + byte, chip, region, hits, false, channel, prioErrors, stuck);
      }
      byte += 2;
      break;
    case DT_DATALONG:
      if (!started) {
        std::cout << "Error, hit data found before chip header or after chip trailer" << std::endl;
        return false;
      }
      if (region == -1) {
	std::cout << "Warning: data word without region, skipping (Chip " << chip << ")" << std::endl;
        corrupt = true;
      }
      else if (hits) {
        if (chip == -1) {
          for (int i = 0; i < nBytes; i++) {
            printf("%02x ", data[i]);
	  }
          printf("\n");
	}
        DecodeDataWord (data + byte, chip, region, hits, true, channel, prioErrors, stuck);
      }
      byte += 3;
      break;
    case DT_UNKNOWN:
      std::cout << "Error, data of unknown type 0x" << std::hex << data[byte] << std::dec << std::endl;
      return false;
    }
  }
  //std::cout << "Found " << Hits->size() - NOldHits << " hits" << std::endl;
  if (started && finished) return (!corrupt);
  else {
    if (started && !finished) {
      std::cout << "Warning (chip "<< chip << "), event not finished at end of data, last byte was 0x" << std::hex << (int) last << std::dec << ", event length = " << nBytes <<std::endl;
      return false;
    }
    if (!started) {
      std::cout << "Warning, event not started at end of data" << std::endl;
      return false;
    }
  }

  return (!corrupt);
}
