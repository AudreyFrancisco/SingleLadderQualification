#include "TRUv1WsI2cGbtx.h"
#include "TReadoutBoardRUv1.h"
#include "tinyxml2.h"
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>

TRUv1WsI2cGbtx::TRUv1WsI2cGbtx(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1WsI2cGbtx::LoadConfig(int gbtx_num, const char *filename)
{

  uint16_t              reg[366] = {};
  tinyxml2::XMLDocument doc;
  doc.LoadFile(filename);
  tinyxml2::XMLElement *signals = doc.FirstChildElement("Signals");
  tinyxml2::XMLElement *signal  = signals->FirstChildElement("Signal");

  while (signal != 0) {
    bool trip = signal->BoolAttribute("triplicated");
    // int nob = signal->IntAttribute("numberBits");
    int value = 0;
    signal->FirstChildElement("value")->QueryIntText(&value);
    int startAddr0 = signal->FirstChildElement("location")->IntAttribute("startAddress");
    int startBit0  = signal->FirstChildElement("location")->IntAttribute("startBitIndex");
    // int lastBit0 = signal->FirstChildElement("location")->IntAttribute("lastBitIndex");
    reg[startAddr0] |= (value << startBit0);
    if (trip) {
      int startAddr1 = signal->FirstChildElement("location")
                           ->NextSiblingElement("location")
                           ->IntAttribute("startAddress");
      int startBit1 = signal->FirstChildElement("location")
                          ->NextSiblingElement("location")
                          ->IntAttribute("startBitIndex");
      // int lastBit1 =
      // FirstChildElement("location")->NextSiblingElement("location")->IntAttribute("lastBitIndex");
      reg[startAddr1] |= (value << startBit1);

      int startAddr2 = signal->FirstChildElement("location")
                           ->NextSiblingElement("location")
                           ->NextSiblingElement("location")
                           ->IntAttribute("startAddress");
      int startBit2 = signal->FirstChildElement("location")
                          ->NextSiblingElement("location")
                          ->NextSiblingElement("location")
                          ->IntAttribute("startBitIndex");
      // int lastBit2 =
      // FirstChildElement("location")->NextSiblingElement("location")->NextSiblingElement("location")->IntAttribute("lastBitIndex");
      reg[startAddr2] |= (value << startBit2);
    }

    signal = signal->NextSiblingElement("Signal");
  }
  for (int i = 0; i < 366; i++) {
    WriteGBTXAddress(gbtx_num, i, reg[i]);
    usleep(1000);
  }
}

bool TRUv1WsI2cGbtx::isConfigLoaded(int gbtx_num, const char *filename)
{

  uint16_t              reg[366] = {};
  tinyxml2::XMLDocument doc;
  doc.LoadFile(filename);
  tinyxml2::XMLElement *signals = doc.FirstChildElement("Signals");
  tinyxml2::XMLElement *signal  = signals->FirstChildElement("Signal");

  while (signal != 0) {
    bool trip = signal->BoolAttribute("triplicated");
    // int nob = signal->IntAttribute("numberBits");
    int value = 0;
    signal->FirstChildElement("value")->QueryIntText(&value);
    int startAddr0 = signal->FirstChildElement("location")->IntAttribute("startAddress");
    int startBit0  = signal->FirstChildElement("location")->IntAttribute("startBitIndex");
    // int lastBit0 = signal->FirstChildElement("location")->IntAttribute("lastBitIndex");
    reg[startAddr0] |= (value << startBit0);
    if (trip) {
      int startAddr1 = signal->FirstChildElement("location")
                           ->NextSiblingElement("location")
                           ->IntAttribute("startAddress");
      int startBit1 = signal->FirstChildElement("location")
                          ->NextSiblingElement("location")
                          ->IntAttribute("startBitIndex");
      // int lastBit1 =
      // FirstChildElement("location")->NextSiblingElement("location")->IntAttribute("lastBitIndex");
      reg[startAddr1] |= (value << startBit1);

      int startAddr2 = signal->FirstChildElement("location")
                           ->NextSiblingElement("location")
                           ->NextSiblingElement("location")
                           ->IntAttribute("startAddress");
      int startBit2 = signal->FirstChildElement("location")
                          ->NextSiblingElement("location")
                          ->NextSiblingElement("location")
                          ->IntAttribute("startBitIndex");
      // int lastBit2 =
      // FirstChildElement("location")->NextSiblingElement("location")->NextSiblingElement("location")->IntAttribute("lastBitIndex");
      reg[startAddr2] |= (value << startBit2);
    }

    signal = signal->NextSiblingElement("Signal");
  }
  bool worked = true;
  for (int i = 0; i < 366; i++) {
    if (ReadGBTXAddress(gbtx_num, i) != reg[i]) {
      worked = false;
      std::cout << "REGISTER " << i << " on GBTX " << gbtx_num << " FAILED R/W " << std::endl;
      usleep(1000);
    }
  }
  return worked;
}


void TRUv1WsI2cGbtx::WriteGBTXAddress(int gbtx_num, uint16_t addr, uint16_t val, bool commit)
{

  if (gbtx_num > 2) {
    std::cout << "INVALID GBTX ADDRESS (0, 1 or 2) \n";
  }
  else {
    Write(gbtx_num, addr, false);
    Write(3, val, commit);
  }
}

uint16_t TRUv1WsI2cGbtx::ReadGBTXAddress(int gbtx_num, uint16_t addr)
{

  if (gbtx_num > 2) {
    std::cout << "INVALID GBTX ADDRESS (0, 1 or 2) \n";
    return -1;
  }
  else {
    Write(gbtx_num, addr, false);
    return Read(3);
  }
}


void TRUv1WsI2cGbtx::DumpConfig()
{
  std::cout << "....TRUV1WSI2CGBTX MODULE CONFIG....\n";
  for (int i = 0; i < 4; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
