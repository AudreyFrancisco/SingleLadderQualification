#ifndef TESTBEAMTOOLS_H
#define TESTBEAMTOOLS_H

#include "tinyxml.h"
#include "TAlpide.h"

unsigned int Bitmask(int width);
void ParseXML(TAlpide *dut, TiXmlNode *node, int base, int rgn, bool readwrite);

#endif
