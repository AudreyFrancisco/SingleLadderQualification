// Template to prepare standard test routines
// ==========================================
//
// The template is intended to prepare scans that work in the same way for the three setup types
//   - single chip with DAQ board
//   - IB stave with MOSAIC
//   - OB module with MOSAIC
// The setup type has to be set with the global variable fSetupType
//
// After successful call to initSetup() the elements of the setup are accessible in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1 readout board, i.e. fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for staves and modules, 
// all chip accesses should be done with a loop over all elements of the chip vector. 
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);  
// For an example how to access board-specific functions see the power off at the end of main. 
//
// The functions that should be modified for the specific test are configureChip() and main()

// Todo
// 1. SEU 때문에 register 값이 바뀔 수 있음. -> Check하는 코드
// 2. threhold (2~3번 loop) check analogue, digital current, DAC value check 
// 3. Current (D/A) 읽기 ~ 완료
// 4. Output file sutructure 만들기 (예전 자료 참고)
// 5. Temperature 읽기 ~ 완료
// 6. Vbb setup(-3V)에 대비하기
// 7. Configure xml file import (also change makefile)


#include <unistd.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include "TConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include <cstdio>
#include <tinyxml.h>
#include <iomanip> // For using std::setprecision
#include <ctime> // For measuring the time elapsed
#include <iostream> // For writing output file
#include <fstream>
#include <algorithm>
#include <string>
#include "tinyxml.h" // For ParseXML

// Flag
int SEU_CHECKER = 1;
int BB=1; // 0: 0V, 1: -3V
int mySampleDist = 1;

// DAQ Setup
int myStrobeLength = 80;      // strobe length in units of 25 ns
int myStrobeDelay  = 0;
int myPulseLength  = 500;

// Threshold scan setup
int myPulseDelay   = 40;
int myNTriggers    = 50;
int myMaskStages   = 164;    // full: 8192, previous 164

int myChargeStart  = 0;
int myChargeStop   = 65;   // if > 100 points, increase array sizes

int HitData     [200][512][1024];
int ChargePoints[200];

TReadoutBoardDAQ *myDAQBoard;


unsigned int Bitmask(int width)
{
  unsigned int tmp = 0;
  for (int i=0; i<width; i++)
    tmp |= 1 << i;
  return tmp;
}

void ParseXML(TAlpide* dut, TiXmlNode* node, int base, int rgn,
              bool readwrite)
{
  uint16_t value = 0;

  // readwrite (from Chip): true = read; false = write
  for (TiXmlNode* pChild = node->FirstChild("address"); pChild != 0;
       pChild = pChild->NextSibling("address")) {
    if (pChild->Type() != TiXmlNode::TINYXML_ELEMENT)
      continue;
//         printf( "Element %d [%s] %d %d\n", pChild->Type(), pChild->Value(),
//         base, rgn);
    TiXmlElement* elem = pChild->ToElement();
    if (base == -1) {
      if (!elem->Attribute("base")) {
        std::cout << "Base attribute not found!" << std::endl;
        break;
      }
      ParseXML(dut, pChild, atoi(elem->Attribute("base")), -1, readwrite);
    } else if (rgn == -1) {
      if (!elem->Attribute("rgn")) {
        std::cout << "Rgn attribute not found!" << std::endl;
        break;
      }
      ParseXML(dut, pChild, base, atoi(elem->Attribute("rgn")), readwrite);
    } else {
      if (!elem->Attribute("sub")) {
        std::cout << "Sub attribute not found!" << std::endl;
        break;
      }
      int sub = atoi(elem->Attribute("sub"));
      uint16_t address = ((rgn << 11) + (base << 8) + sub);
//      cout << address << endl;
      //std::cout << "region" << rgn << " " << base << " " << sub << std::endl;

      if (readwrite) {
        if (dut->ReadRegister(address, value)) {
          std::cout << "Failure to read chip address " << address << std::endl;
          continue;
        }
      }

      for (TiXmlNode* valueChild = pChild->FirstChild("value"); valueChild != 0;
           valueChild = valueChild->NextSibling("value")) {
        if (!valueChild->ToElement()->Attribute("begin")) {
          std::cout << "begin attribute not found!" << std::endl;
          break;
        }
        int begin = atoi(valueChild->ToElement()->Attribute("begin"));

        int width = 1;
        if (valueChild->ToElement()->Attribute("width")) // width attribute is
                                                         // optional
          width = atoi(valueChild->ToElement()->Attribute("width"));

        if (!valueChild->FirstChild("content") &&
            !valueChild->FirstChild("content")->FirstChild()) {
          std::cout << "content tag not found!" << std::endl;
          break;
        }
        if (readwrite) {
          int subvalue = (value >> begin) & Bitmask(width);
          char tmp[5];
          sprintf(tmp, "%X", subvalue);
          valueChild->FirstChild("content")->FirstChild()->SetValue(tmp);
        } else {
          int content = (int)strtol(
            valueChild->FirstChild("content")->FirstChild()->Value(), 0, 16);



          if (content >= (1 << width)) {
            std::cout << "value too large: " << begin << " " << width << " "
                      << content << " "
                      << valueChild->FirstChild("content")->Value()
                      << std::endl;
            break;
          }
          value += content << begin;
        }
      }
      if (!readwrite) {
        //printf("%d %d %d: %d %d\n", base, rgn, sub, address, value);
	      if (dut->WriteRegister(address, value) != 1)
		      std::cout << "Failure to write chip address " << address << std::endl;
        uint16_t valuecompare = 0;
        if (dut->ReadRegister(address, valuecompare) != 1)
          std::cout << "Failure to read chip address after writing chip address " << address << std::endl;
        if (address != 14 && value != valuecompare)
          std::cout << "Register read back error : write value is : " << value << " and read value is : "<< valuecompare << std::endl;

      }
    }
  }
}

void DumpConfiguration(TAlpide *myAlpide, long int timef) {
  TiXmlDocument doc("full_pALPIDE4.xml");
  if (!doc.LoadFile()) {
    std::string msg = "Failed to load config file!";
    std::cerr << msg.data() << std::endl;
    return;
  }
  
  ParseXML(myAlpide, doc.FirstChild("root")->ToElement(), -1, -1, true);
  
  std::string configStr;
  configStr << doc;

  char buffer[100];
  sprintf(buffer, "Data/Configuration_%ld.xml", timef );
  FILE *fp = fopen (buffer, "w");
  fprintf(fp, "%s", configStr.c_str());
  fclose(fp);
}


void timestamp(int lineChange=0){
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  if(lineChange>0) printf("%02d-%02d-%02d %02d:%02d:%02d>> ", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
  else printf("%02d-%02d-%02d %02d:%02d:%02d\n", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
}

long int timestamp2(){
  time_t       t = time(0);   // get time now
  return t;
}

void SetDACMonSpecial (TAlpide *chip, uint16_t ADac, int IRef = 2) {
    int VDAC, IDAC;
    uint16_t Value;
    switch (ADac) {
        case 9:
            VDAC=9;
            IDAC = 0;
            break;
        case 14:
            VDAC=0;
            IDAC=4;
            break;
        case 16:
            VDAC=0;
            IDAC=6;
            break;
    }
    Value = VDAC & 0xf; // bitwise AND
    Value |= (IDAC & 0x7) << 4;
    Value |= (IRef & 0x3) << 9;

    chip->WriteRegister (Alpide::REG_ANALOGMON, Value);
}


// DAC scan & read start
void SetDACMon (TAlpide *chip, Alpide::TRegister ADac, int IRef = 2) { // copy from DAC scan
  int VDAC, IDAC;
  uint16_t Value;
  switch (ADac) {
    case Alpide::REG_VRESETP:
      VDAC = 4;
      IDAC = 0;
      break;
    case Alpide::REG_VRESETD:
      VDAC = 5;
      IDAC = 0;
      break;
    case Alpide::REG_VCASP:
      VDAC = 1;
      IDAC = 0;
      break;
    case Alpide::REG_VCASN:
      VDAC = 0;
      IDAC = 0;
      break;
    case Alpide::REG_VPULSEH:
      VDAC = 2;
      IDAC = 0;
      break;
    case Alpide::REG_VPULSEL:
      VDAC = 3;
      IDAC = 0;
      break;
    case Alpide::REG_VCASN2:
      VDAC = 6;
      IDAC = 0;
      break;
    case Alpide::REG_VCLIP:
      VDAC = 7;
      IDAC = 0;
      break;
    case Alpide::REG_VTEMP:
      VDAC = 8;
      IDAC = 0;
      break;
    case Alpide::REG_IAUX2:
      IDAC = 1;
      VDAC = 0;
      break;
    case Alpide::REG_IRESET:
      IDAC = 0;
      VDAC = 0;
      break;
    case Alpide::REG_IDB:
      IDAC = 3;
      VDAC = 0;
      break;
    case Alpide::REG_IBIAS:
      IDAC = 2;
      VDAC = 0;
      break;
    case Alpide::REG_ITHR:
      IDAC = 5;
      VDAC = 0;
      break;
    default:
      VDAC = 0;
      IDAC = 0;
      break;
  }
  
  
  Value = VDAC & 0xf; // bitwise AND
  Value |= (IDAC & 0x7) << 4;
  Value |= (IRef & 0x3) << 9;
  
  chip->WriteRegister (Alpide::REG_ANALOGMON, Value);
  
}

void scanCurrentDac(TAlpide *chip, Alpide::TRegister ADac, const char *Name, const char *ChipNum, int sampleDist = 1, long int time = 0) {
  char     fName[200];
  float    Current;
  uint16_t old; 
  sprintf (fName, "Data/%s/DataScanDACS/%ld/IDAC_%s.dat", ChipNum, time, Name);
  FILE *fp = fopen (fName, "w");

  std::cout << "Scanning DAC " << Name << std::endl;

  SetDACMon (chip, ADac);
  usleep(100000);

  chip->ReadRegister (ADac, old);

  for (int i = 0; i < 256; i += sampleDist) {
    chip->WriteRegister (ADac, i);
    Current = myDAQBoard->ReadMonI();
    fprintf (fp, "%d %.3f\n", i, Current);
  }
  
  chip->WriteRegister (ADac, old);
  fclose (fp);
}


void scanVoltageDac(TAlpide *chip, Alpide::TRegister ADac, const char *Name, const char *ChipNum, int sampleDist = 1, long int time = 0) {
  char     fName[200];
  float    Voltage;
  uint16_t old; 
  sprintf (fName, "Data/%s/DataScanDACS/%ld/VDAC_%s.dat", ChipNum, time, Name);
  FILE *fp = fopen (fName, "w");

  std::cout << "Scanning DAC " << Name << std::endl;

  SetDACMon (chip, ADac);
  usleep(100000);

  chip->ReadRegister (ADac, old);

  for (int i = 0; i < 256; i += sampleDist) {
    chip->WriteRegister (ADac, i);
    Voltage = myDAQBoard->ReadMonV();
    fprintf (fp, "%d %.3f\n", i, Voltage);
  }
  
  chip->WriteRegister (ADac, old);
  fclose (fp);
}


// Copy from dacscan. Change it to read single value.
float readCurrentDac(TAlpide *chip, Alpide::TRegister ADac, const char *Name) {
  float    Current;
  uint16_t value;
  
  SetDACMon (chip, ADac);
  usleep(100000);

  
  chip->ReadRegister (ADac, value); // Current value reading
  //std::cout << "Reading DAC(" << Name  << " ," << value << "): ";
  Current = myDAQBoard->ReadMonI();
  //std::cout << std::setprecision(3) << Current << " " << std::endl;
  return Current;
}

float readCurrentSpecial(TAlpide *chip, uint16_t adac, const char *Name) {
    float Current;
    uint16_t value;
    SetDACMonSpecial (chip, adac);
    usleep(100000);

    Current = myDAQBoard->ReadMonI();
    return Current;
}

float readVoltageSpecial(TAlpide *chip, uint16_t adac, const char *Name) {
  float    Voltage;
  uint16_t value;
  
  SetDACMonSpecial (chip, adac);
  usleep(100000);
  
//  chip->ReadRegister (adac, value); // Current value reading
  //std::cout << "Reading DAC(" << Name  << " ," << value << "): ";
  Voltage = myDAQBoard->ReadMonV();
  //std::cout << std::setprecision(3) << Voltage << std::endl;
  return Voltage;
}


// Copy from dacscan. Change it to read single value.
float readVoltageDac(TAlpide *chip, Alpide::TRegister ADac, const char *Name) {
  float    Voltage;
  uint16_t value;
  
  SetDACMon (chip, ADac);
  usleep(100000);
  
  chip->ReadRegister (ADac, value); // Current value reading
  //std::cout << "Reading DAC(" << Name  << " ," << value << "): ";
  Voltage = myDAQBoard->ReadMonV();
  //std::cout << std::setprecision(3) << Voltage << std::endl;
  return Voltage;
}

/*bool checkDAC(TAlpide *chip, int backBias){
  const char VName[7][10] = {"VCASN", "VCASN2", "VRESETP", "VRESETD", "VCLIP", "VPULSEH", "VPULSEL"};
  const char IName[3][10] = {"ITHR", "IDB", "IRESET"};

  int VDAC_compare[2][7]={0}; // Order: VCASN, VCASN2, VRESETP, VRESETD, VCLIP, VPULSEH, VPULSEL
  int IDAC_compare[2][3]={0}; // Order: ITHR, IDB, IRESET

  chip->ReadRegister (Alpide::REG_VCASN,   VDAC_compare[backBias][0]);
  chip->ReadRegister (Alpide::REG_VCASN2,  VDAC_compare[backBias][1]);
  chip->ReadRegister (Alpide::REG_VRESETP, VDAC_compare[backBias][2]);
  chip->ReadRegister (Alpide::REG_VRESETD, VDAC_compare[backBias][3]);
  chip->ReadRegister (Alpide::REG_VCLIP,   VDAC_compare[backBias][4]);
  for (int i=0; i<5; i++){
    if(VDAC_default[backBias][i]==VDAC_compare[backBias][i]){
      std::cout << "Different DAC setting(" << VName[i] <<  "):" <<  
    }
  }

  chip->ReadRegister (Alpide::REG_VPULSEH, VDAC_compare[backBias][5]);
  chip->ReadRegister (Alpide::REG_VPULSEL, VDAC_compare[backBias][6]);
  // Read Current DAC
  chip->ReadRegister (Alpide::REG_ITHR,    IDAC_compare[backBias][0]);
  chip->ReadRegister (Alpide::REG_IDB,     IDAC_compare[backBias][1]);
  chip->ReadRegister (Alpide::REG_IRESET,  IDAC_compare[backBias][2]);
  }*/

void readAllDac (TAlpide *chip){ // Not used
  // Read Voltage DAC
  readVoltageDac (chip, Alpide::REG_VRESETP, "VRESETP");
  readVoltageDac (chip, Alpide::REG_VRESETD, "VRESETD");
  readVoltageDac (chip, Alpide::REG_VCASP,   "VCASP");
  readVoltageDac (chip, Alpide::REG_VCASN,   "VCASN");
  readVoltageDac (chip, Alpide::REG_VPULSEH, "VPULSEH");
  readVoltageDac (chip, Alpide::REG_VPULSEL, "VPULSEL");
  readVoltageDac (chip, Alpide::REG_VCASN2,  "VCASN2");
  readVoltageDac (chip, Alpide::REG_VCLIP,   "VCLIP");
  readVoltageDac (chip, Alpide::REG_VTEMP,   "VTEMP");
  // Read Current DAC
  readCurrentDac (chip, Alpide::REG_IAUX2,   "IAUX2");
  readCurrentDac (chip, Alpide::REG_IRESET,  "IRESET");
  readCurrentDac (chip, Alpide::REG_IDB,     "IDB");
  readCurrentDac (chip, Alpide::REG_IBIAS,   "IBIAS");
  readCurrentDac (chip, Alpide::REG_ITHR,    "ITHR");
}

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

// Threshold scan part start
void ClearHitData() {
  for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
    ChargePoints[icharge-myChargeStart] = icharge;
    for (int icol = 0; icol < 512; icol ++) {
      for (int iaddr = 0; iaddr < 1024; iaddr ++) {
        HitData[icharge-myChargeStart][icol][iaddr] = 0;
      }
    }
  }
}


void CopyHitData(std::vector <TPixHit> *Hits, int charge) {
  for (int ihit = 0; ihit < Hits->size(); ihit ++) {
    HitData[charge-myChargeStart][Hits->at(ihit).dcol + Hits->at(ihit).region * 16][Hits->at(ihit).address] ++;
  }
  Hits->clear();
}


void WriteDataToFile (const char *fName, bool Recreate) {
  FILE *fp;
  bool  HasData;
  if (Recreate) fp = fopen(fName, "w");
  else          fp = fopen(fName, "a");

  for (int icol = 0; icol < 512; icol ++) {
    for (int iaddr = 0; iaddr < 1024; iaddr ++) {
      HasData = false;
      for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
        if (HitData[icharge - myChargeStart][icol][iaddr] > 0) HasData = true;
      }
      
      if (HasData) {
        for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
          fprintf(fp, "%d %d %d %d\n", icol, iaddr, icharge, HitData[icharge - myChargeStart][icol][iaddr]);
  }
      }
    }
  }
  fclose (fp);
}

// initialisation of Fromu
int configureFromu(TAlpide *chip) {
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x20);            // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  myStrobeLength);  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, myStrobeDelay);   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength);   // fromu pulsing 2: pulse length 
  return 0;
}


// initialisation of chip DACs
int configureDACs_threshold(TAlpide *chip) {
  chip->WriteRegister (Alpide::REG_VPULSEH, 170);
  chip->WriteRegister (Alpide::REG_VPULSEL, 169);

  return 0;
}

int configureDACs(TAlpide *chip, int backBias) {
  // Order: VCASN, VCASN2, VRESETP, VRESETD, VCLIP
  int VDAC[2][5] = {
    {50, 62, 117, 147, 0},
    {105, 117, 117, 147, 60}
  };
  // Order: ITHR, IDB, IRESET
  int IDAC[2][3] = {
    {51, 64, 100},
    {51, 64, 100}
  };
  switch (backBias) {
    case 0:
    chip->WriteRegister (Alpide::REG_VCASN,     VDAC[0][0]);
    chip->WriteRegister (Alpide::REG_VCASN2,    VDAC[0][1]);
    chip->WriteRegister (Alpide::REG_VRESETP,   VDAC[0][2]);
    chip->WriteRegister (Alpide::REG_VRESETD,   VDAC[0][3]);
    chip->WriteRegister (Alpide::REG_VCLIP,     VDAC[0][4]);

    chip->WriteRegister (Alpide::REG_ITHR,      IDAC[0][0]);
    chip->WriteRegister (Alpide::REG_IDB,       IDAC[0][1]);
    chip->WriteRegister (Alpide::REG_IRESET,    IDAC[0][2]);
    break;
    case 1:
    chip->WriteRegister (Alpide::REG_VCASN,     VDAC[1][0]);
    chip->WriteRegister (Alpide::REG_VCASN2,    VDAC[1][1]);
    chip->WriteRegister (Alpide::REG_VRESETP,   VDAC[1][2]);
    chip->WriteRegister (Alpide::REG_VRESETD,   VDAC[1][3]);
    chip->WriteRegister (Alpide::REG_VCLIP,     VDAC[1][4]);

    chip->WriteRegister (Alpide::REG_ITHR,      IDAC[1][0]);
    chip->WriteRegister (Alpide::REG_IDB,       IDAC[1][1]);
    chip->WriteRegister (Alpide::REG_IRESET,    IDAC[1][2]);
    break;
  }
    return 0;
}

// initialisation of fixed mask
int configureMask(TAlpide *chip) {
  // unmask all pixels 
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK, true);
  return 0;
}


// setting of mask stage during scan
int configureMaskStage(TAlpide *chip, int istage) {
  int row    = istage / 4;
  int region = istage % 4;

  //uint32_t regionmod = 0x08080808 >> region;

  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   true);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);

  //AlpideConfig::WritePixRegRow (chip, Alpide::PIXREG_MASK,   false, row);
  //AlpideConfig::WritePixRegRow (chip, Alpide::PIXREG_SELECT, true,  row);

  //chip->WriteRegister (Alpide::REG_REGDISABLE_LOW,  (uint16_t) regionmod);
  //chip->WriteRegister (Alpide::REG_REGDISABLE_HIGH, (uint16_t) regionmod);

  for (int icol = 0; icol < 1024; icol += 8) {
    AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_MASK,   false, istage % 512, icol + istage / 512);
    AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_SELECT, true,  istage % 512, icol + istage / 512);   
  }

}

void scan() {   
  unsigned char         buffer[1024*8000]; 
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;


  for (int istage = 0; istage < myMaskStages; istage ++) {
//    if ( !(istage%int(myMaskStages * 0.1)) )
        std::cout << "Mask stage " << istage << std::endl;
    for (int i = 0; i < fChips.size(); i ++) {
      configureMaskStage (fChips.at(i), istage);
    }
//    configureMaskStage (fChips.at(0), istage);

    for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
//      std::cout << "Charge = " << icharge << std::endl;
      fChips.at(0)->WriteRegister (Alpide::REG_VPULSEL, 170 - icharge);
      fBoards.at(0)->Trigger(myNTriggers);

      int itrg = 0;
      while(itrg < myNTriggers) {
        if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
          usleep(100);
          cout << n_bytes_data << endl;
          continue;
        }
        else {
          // decode DAQboard event
          BoardDecoder::DecodeEvent(boardDAQ, buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
          // decode Chip event
          int n_bytes_chipevent=n_bytes_data-n_bytes_header-n_bytes_trailer;
          AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits);

          itrg++;
        }
      } 
      //std::cout << "Number of hits: " << Hits->size() << std::endl;
      CopyHitData(Hits, icharge);
    }
  }
  delete Hits;
  Hits = 0x0;
}

int configureChip_dac(TAlpide *chip) {
  // put all chip configurations before the start of the test here
  chip->WriteRegister (Alpide::REG_MODECONTROL,   0x20);
  chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x60);
}

int configureChip_threshold(TAlpide *chip) {
  // put all chip configurations before the start of the test here
  chip->WriteRegister (Alpide::REG_MODECONTROL, 0x20); // set chip to config mode
  if (fSetupType == setupSingle) {
    chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x30); // CMU/DMU config: turn manchester encoding off etc, initial token=1, disable DDR
  }
  else {
    chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x10); // CMU/DMU config: same as above, but manchester on
  }

  configureFromu(chip);
  configureDACs_threshold (chip);  
  configureDACs (chip, 1);  
  configureMask (chip);

  chip->WriteRegister (Alpide::REG_MODECONTROL, 0x21); // strobed readout mode

}

float readDAQBoardAnalogueCurrent(TReadoutBoardDAQ *aDAQBoard){
    float analogueI;
    //std::cout << "Analog Current  = " << aDAQBoard-> ReadAnalogI() << std::endl;
    analogueI = aDAQBoard->ReadAnalogI();
    return analogueI;
}

float readDAQBoardDigitalCurrent(TReadoutBoardDAQ *aDAQBoard){
    float digitalI;
    //std::cout << "Digital Current = " << aDAQBoard-> ReadDigitalI() << std::endl; 
    digitalI = aDAQBoard->ReadDigitalI();
    return digitalI;
}

int main() {
    // chip ID that is used in case of single chip setup
    fSingleChipId = 16;

    // module ID that is used for outer barrel modules 
    // (1 will result in master chip IDs 0x10 and 0x18, 2 in 0x20 and 0x28 ...)
    fModuleId = 1;

    fSetupType = setupSingle;

    initSetup();

    char Suffix[20], fName[100];

    string platereturn;

    myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

    if (fBoards.size() == 1) {

        fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
        fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

        for (int i = 0; i < fChips.size(); i ++) {
            configureChip_threshold (fChips.at(i));
        }

        fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     


        // put your test here...
        int n = 0;
        long int scan_time;
        std::cout << "Measurement Start" << std::endl;
        std::cout << "Make the Output folder sutructure" << std::endl;
        char ChipNum[10] = "A4W7G7R41";
        char OutputPath[100], OutputPath_DAC[100], OutputPath_Threshold[100];
        // Make defualt folder for chip
        snprintf(OutputPath, 100, "Data/%s", ChipNum);
        char command[120];
        snprintf(command, 120, "mkdir -p %s", OutputPath);
        system(command);
        // Make defualt DAC scan folder for chip
        command[0] = '\0';
        snprintf(OutputPath_DAC, 100, "Data/%s/DataScanDACS", ChipNum);
        snprintf(command, 120, "mkdir -p %s", OutputPath);
        system(command);
        // Make defualt DAC scan folder for chip
        command[0] = '\0';
        snprintf(OutputPath_Threshold, 100, "Data/%s/ThresholdScan", ChipNum);
        snprintf(command, 120, "mkdir -p %s", OutputPath);
        system(command);

        std::ofstream logfile;
        char LogPath[100];
        snprintf(LogPath, 120, "%s/%s.log", OutputPath, ChipNum);
        logfile.open(LogPath, std::ofstream::app);
        logfile << "chipID;Time;Hameg Output Current5V;Hameg Output Current VBB;Analog Current;Digital Current;Output Current;Temperature;VCASN;VCASN2;VCASP;VPULSEH;VPULSEL;VRESETP;VRESETD;VCLIP;VTEMP;IRESET;IAUX2;IBIAS;IDB;ITHR;ADCDAC;IREF;IREFBuffer" << std::endl;

        string hameg5V, hamegVBB;

        while(1) {
            scan_time = timestamp2();
            hameg5V =  exec("./tid/hameg2030.py /dev/ttyHAMEG0 6 0");
            hamegVBB = exec("./tid/hameg2030.py /dev/ttyHAMEG0 6 2");
            hameg5V.erase(std::remove(hameg5V.begin(), hameg5V.end(), '\n'), hameg5V.end());
            hamegVBB.erase(std::remove(hamegVBB.begin(), hamegVBB.end(), '\n'), hamegVBB.end());
            timestamp(1);
            std::cout << "DACread start" << std::endl;
            logfile << setprecision(6) << ChipNum << ";" << scan_time << ";"
                << hameg5V << ";" 
                << hamegVBB << ";" 
                << readDAQBoardAnalogueCurrent(myDAQBoard) << ";" 
                << readDAQBoardDigitalCurrent(myDAQBoard) << ";" 
                << myDAQBoard->ReadTemperature() << ";"
                << readVoltageDac (fChips.at(0), Alpide::REG_VCASN,   "VCASN")    << ";"
                << readVoltageDac (fChips.at(0), Alpide::REG_VCASN2,  "VCASN2")   << ";"
                << readVoltageDac (fChips.at(0), Alpide::REG_VCASP,   "VCASP")    << ";"
                << readVoltageDac (fChips.at(0), Alpide::REG_VPULSEH, "VPULSEH")  << ";"
                << readVoltageDac (fChips.at(0), Alpide::REG_VPULSEL, "VPULSEL")  << ";"
                << readVoltageDac (fChips.at(0), Alpide::REG_VRESETP, "VRESETP")  << ";"
                << readVoltageDac (fChips.at(0), Alpide::REG_VRESETD, "VRESETD")  << ";"
                << readVoltageDac (fChips.at(0), Alpide::REG_VCLIP,   "VCLIP")    << ";"
                << readVoltageDac (fChips.at(0), Alpide::REG_VTEMP,   "VTEMP")    << ";"
                << readCurrentDac (fChips.at(0), Alpide::REG_IRESET,  "IRESET")   << ";"
                << readCurrentDac (fChips.at(0), Alpide::REG_IAUX2,   "IAUX2")    << ";"
                << readCurrentDac (fChips.at(0), Alpide::REG_IBIAS,   "IBIAS")    << ";"
                << readCurrentDac (fChips.at(0), Alpide::REG_IDB,     "IDB")      << ";"
                << readCurrentDac (fChips.at(0), Alpide::REG_ITHR,    "ITHR")     << ";"
                << readVoltageSpecial (fChips.at(0), 9,                   "ADCDAC")     << ";"
                << readCurrentSpecial (fChips.at(0), 14,    "IREF")     << ";"
                << readCurrentSpecial (fChips.at(0), 16,    "IREFBuffer")     << ";"
                << std::endl;
            timestamp(1);
            std::cout << "DACread end" << std::endl;

            if(n%56 == 0) {
 //               platereturn = exec("./moveplate.py 1");
 //               cout << platereturn << endl;
/*                if (!(std::stoi(platereturn))) {
                    cout << "ERROR : PLATE NOT MOVING" << endl;
                    break;
                }*/

                myDAQBoard->PowerOff();
                delete fChips.at(0);
                delete fBoards.at(0);
                fChips.erase(fChips.begin(), fChips.end());
                fBoards.erase(fBoards.begin(), fBoards.end());
                sleep(1);
                system("./tid/hameg2030.py /dev/ttyHAMEG0 9");
                sleep(4);
                system("./tid/hameg2030.py /dev/ttyHAMEG0 7");
                sleep(4);
                system("./program.sh");
                sleep(3);

                initSetup();
                ClearHitData();

                myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));


                fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
                fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

                for (int i = 0; i < fChips.size(); i ++) {
                    configureChip_threshold (fChips.at(i));
                }

                fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

//                for (int i = 0; i < fChips.size(); i ++) {
 //                   configureChip_threshold (fChips.at(i));
  //              }


                /*timestamp(1);
                  std::cout << "Before DAC scan, DACread start" << std::endl;
                  timestamp(1);
                  std::cout << "Before DAC scan, DACread end" << std::endl;*/
                char ScanPath0[100], ScanPath1[100];
                command[0] = '\0';
                snprintf(ScanPath0, 100, "%s/%ld", OutputPath_DAC, scan_time);
                snprintf(command, 120, "mkdir -p %s", ScanPath0);
                system(command);
                command[0] = '\0';
                snprintf(ScanPath1, 100, "%s/%ld", OutputPath_Threshold, scan_time);
                snprintf(command, 120, "mkdir -p %s", ScanPath1);
                system(command);

                timestamp(1);
                std::cout << "Start DAC scan" << std::endl;
                scanVoltageDac (fChips.at(0), Alpide::REG_VRESETP, "VRESETP", ChipNum, mySampleDist, scan_time);
                scanVoltageDac (fChips.at(0), Alpide::REG_VRESETD, "VRESETD", ChipNum, mySampleDist, scan_time);
                scanVoltageDac (fChips.at(0), Alpide::REG_VCASP,   "VCASP",   ChipNum, mySampleDist, scan_time);
                scanVoltageDac (fChips.at(0), Alpide::REG_VCASN,   "VCASN",   ChipNum, mySampleDist, scan_time);
                scanVoltageDac (fChips.at(0), Alpide::REG_VPULSEH, "VPULSEH", ChipNum, mySampleDist, scan_time);
                scanVoltageDac (fChips.at(0), Alpide::REG_VPULSEL, "VPULSEL", ChipNum, mySampleDist, scan_time);
                scanVoltageDac (fChips.at(0), Alpide::REG_VCASN2,  "VCASN2",  ChipNum, mySampleDist, scan_time);
                scanVoltageDac (fChips.at(0), Alpide::REG_VCLIP,   "VCLIP",   ChipNum, mySampleDist, scan_time);
                scanVoltageDac (fChips.at(0), Alpide::REG_VTEMP,   "VTEMP",   ChipNum, mySampleDist, scan_time);

                scanCurrentDac (fChips.at(0), Alpide::REG_IAUX2,   "IAUX2",   ChipNum, mySampleDist, scan_time);
                scanCurrentDac (fChips.at(0), Alpide::REG_IRESET,  "IRESET",  ChipNum, mySampleDist, scan_time);
                scanCurrentDac (fChips.at(0), Alpide::REG_IDB,     "IDB",     ChipNum, mySampleDist, scan_time);
                scanCurrentDac (fChips.at(0), Alpide::REG_IBIAS,   "IBIAS",   ChipNum, mySampleDist, scan_time);
                scanCurrentDac (fChips.at(0), Alpide::REG_ITHR,    "ITHR",    ChipNum, mySampleDist, scan_time);

                timestamp(1);        
                std::cout << "Complete DAC scan" << std::endl;

                

                /*timestamp(1);
                  std::cout << "After DAC scan, DACread start" << std::endl;
                  timestamp(1);
                  std::cout << "After DAC scan, DACread end" << std::endl;*/
                  sleep(2);
                timestamp(1);
                std::cout << "Start threshold scan" << std::endl;
                sprintf(Suffix, "%ld", scan_time);
                
                myDAQBoard->PowerOff();
                delete fChips.at(0);
                delete fBoards.at(0);
                fChips.erase(fChips.begin(), fChips.end());
                fBoards.erase(fBoards.begin(), fBoards.end());
                sleep(1);
                system("./tid/hameg2030.py /dev/ttyHAMEG0 9");
                sleep(4);
                system("./tid/hameg2030.py /dev/ttyHAMEG0 7");
                sleep(4);
                system("./program.sh");
                sleep(3);

                initSetup();
                ClearHitData();

                myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));


                fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
                fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

                for (int i = 0; i < fChips.size(); i ++) {
                    configureChip_threshold (fChips.at(i));
                }

                fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

                /*timestamp(1);
                  std::cout << "Before Threshold scan, DACread start" << std::endl;
                  timestamp(1);
                  std::cout << "Before Threshold scan, DACread end" << std::endl;*/

//                fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);
//

                fBoards.at(0)->SetTriggerConfig (true, false, myStrobeDelay, myPulseDelay);
                fBoards.at(0)->SetTriggerSource (trigExt);
                scan();
                sprintf(fName, "%s/ThresholdScan_%s.dat", ScanPath1, Suffix);
                WriteDataToFile (fName, true);

                timestamp(1);
                std::cout << "Complete threshold scan" << std::endl;

                /*timestamp(1);
                  std::cout << "After Threshold scan, DACread start" << std::endl;
                  readAllDac(fChips.at(0));
                  timestamp(1);
                  std::cout << "After Threshold scan, DACread end" << std::endl;*/
            
                myDAQBoard->PowerOff();
                delete fChips.at(0);
                delete fBoards.at(0);
                fChips.erase(fChips.begin(), fChips.end());
                fBoards.erase(fBoards.begin(), fBoards.end());
                sleep(1);
                system("./tid/hameg2030.py /dev/ttyHAMEG0 9");
                sleep(4);
                system("./tid/hameg2030.py /dev/ttyHAMEG0 7");
                sleep(4);
                system("./program.sh");
                sleep(3);

                initSetup();

                myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));


                fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
                fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

                for (int i = 0; i < fChips.size(); i ++) {
                    configureChip_threshold (fChips.at(i));
                }

                fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     
                
 //               platereturn = exec("./moveplate.py 0");
//                cout << platereturn << endl;
/*                if (!(std::stoi(platereturn))) {
                    cout << "ERROR : PLATE NOT MOVING" << endl;
                    break;
                }*/
            }
            DumpConfiguration(fChips.at(0), scan_time);
            sleep(10);
            n++;
        }

        if (myDAQBoard) {
            myDAQBoard->PowerOff();
            delete myDAQBoard;
        }
    }
    return 0;
}
