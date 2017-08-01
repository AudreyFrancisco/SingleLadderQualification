#include "TConfig.h"
#include "TBoardConfigDAQ.h"
#include "TBoardConfigMOSAIC.h"
#include "TBoardConfigRU.h"
#include <iostream>
#include <string.h>

//construct Config from config file
TConfig::TConfig (const char *fName) {
  fDeviceType = TYPE_UNKNOWN;   // will be overwritten in read config file
  fUsePowerBoard = false;
  ReadConfigFile (fName);
}


// construct Config in the application using only number of boards and number of chips / vector of chip Ids
// for the time being use one common config for all board types (change this?)
// this constructor does not set the device type correctly
// (not clear right now, which setup this constructor will be used for)
TConfig::TConfig (int nBoards, std::vector <int> chipIds, TBoardType boardType) {
  std::cout << "Warning: using deprecated constructur that does not set setup type correctly" << std::endl;
  fDeviceType = TYPE_UNKNOWN;
  Init(nBoards, chipIds, boardType);
}


// construct a config for a single chip setup (one board and one chip only)
TConfig::TConfig (int chipId, TBoardType boardType) {
  fUsePowerBoard = false;
  Init(chipId, boardType);
}

void TConfig::Init (int nBoards, std::vector <int> chipIds, TBoardType boardType) {
  for (int iboard = 0; iboard < nBoards; iboard ++) {
    if (boardType == boardDAQ) {
      fBoardConfigs.push_back (new TBoardConfigDAQ());
    }
    else if (boardType == boardMOSAIC) {
      fBoardConfigs.push_back (new TBoardConfigMOSAIC());
    }
    else if (boardType == boardRU) {
      fBoardConfigs.push_back (new TBoardConfigRU());
    }
    else {
      std::cout << "TConfig: Unknown board type" << std::endl;
    }
  }
  for (unsigned int ichip = 0; ichip < chipIds.size(); ichip ++) {
    fChipConfigs.push_back (new TChipConfig(this, chipIds.at(ichip)));
  }
}


void TConfig::Init (int chipId, TBoardType boardType) {
  if (boardType == boardDAQ) {
    fDeviceType = TYPE_CHIP;
    fBoardConfigs.push_back (new TBoardConfigDAQ());
  }
  else if (boardType == boardMOSAIC) {
    fDeviceType = TYPE_CHIP_MOSAIC;
    fBoardConfigs.push_back (new TBoardConfigMOSAIC());
  }
  else {
    fDeviceType = TYPE_UNKNOWN;
    std::cout << "TConfig: Unknown board type" << std::endl;
  }

  fChipConfigs. push_back (new TChipConfig (this, chipId));
}


// getter functions for chip and board config
TChipConfig *TConfig::GetChipConfigById  (int chipId) {
  for (unsigned int i = 0; i < fChipConfigs.size(); i++) {
    if (fChipConfigs.at(i)->GetChipId() == chipId)
      return fChipConfigs.at(i);
  }
  // throw exception here.
  std::cout << "Chip id " << chipId << " not found" << std::endl;
  return 0;
}


TChipConfig *TConfig::GetChipConfig (unsigned int iChip) {
  if (iChip < fChipConfigs.size()) {
    return fChipConfigs.at(iChip);
  }
  else {
    return 0;
  }
}


TBoardConfig *TConfig::GetBoardConfig (int iBoard){
  if (iBoard < fBoardConfigs.size()) {
    return fBoardConfigs.at(iBoard);
  }
  else {  // throw exception
    return 0;
  }
}


TDeviceType TConfig::ReadDeviceType (const char *deviceName) {
  TDeviceType type = TYPE_UNKNOWN;
  if (!strcmp (deviceName, "CHIP")) {
    type = TYPE_CHIP;
  }
  else if (!strcmp(deviceName, "TELESCOPE")) {
    type = TYPE_TELESCOPE;
  }
  else if (!strcmp(deviceName, "OBHIC")) {
    type = TYPE_OBHIC;
  }
  else if (!strcmp(deviceName, "OBHIC_PB")) {
    SetUsePowerBoard(true);
    type = TYPE_OBHIC;
  }
  else if (!strcmp(deviceName, "IBHIC")) {
    type = TYPE_IBHIC;
  }
  else if (!strcmp(deviceName, "CHIPMOSAIC")) {
    type = TYPE_CHIP_MOSAIC;
  }
  else if (!strcmp(deviceName, "HALFSTAVE")) {
    type = TYPE_HALFSTAVE;
  }
  else if (!strcmp(deviceName, "ENDURANCETEST")) {
    type = TYPE_ENDURANCE;
  }
  else if (!strcmp(deviceName, "IBHICRU")) {
    type = TYPE_IBHICRU;
  }
  else {
    std::cout << "Error, unknown setup type found: " << deviceName << std::endl;
    exit (EXIT_FAILURE);
  }
  return type;
}


void TConfig::SetDeviceType (TDeviceType AType, int NChips) {
  std::vector <int> chipIds;

  fDeviceType = AType;
  if (AType == TYPE_CHIP) {
    Init(16, boardDAQ);
  }
  else if (AType == TYPE_CHIP_MOSAIC) {
    Init(0, boardMOSAIC);
  }
  else if (AType == TYPE_TELESCOPE) {
    for (int i = 0; i < NChips; i++) {
      chipIds.push_back(16);
    }
    Init(NChips, chipIds, boardDAQ);
  }
  else if (AType == TYPE_OBHIC) {
    for (int i = 0; i < 15; i++) {
      if (i == 7) continue;
      int ModuleId = (NChips <= 0 ? DEFAULT_MODULE_ID : NChips) & 0x07;
      chipIds.push_back(i + (ModuleId << 4));
    }
    Init (1, chipIds, boardMOSAIC);
  }
  else if (AType == TYPE_ENDURANCE) {
  	for(int mod=0; mod < 5; mod++) {
  	  for (int i = 0; i < 15; i++) {
        if (i == 7) continue;
        int ModuleId = (NChips <= 0 ? DEFAULT_MODULE_ID : NChips) & 0x07;
        chipIds.push_back(i + (ModuleId << 4));
  	  }
  	}
  	Init (1, chipIds, boardMOSAIC);
  }
  else if (AType == TYPE_IBHIC) {
    for (int i = 0; i < 9; i++) {
      chipIds.push_back(i);
    }
    Init (1, chipIds, boardMOSAIC);
  }
  else if (AType == TYPE_IBHICRU) {
    for (int i = 0; i < 9; i++) {
      chipIds.push_back(i);
    }
    Init (1, chipIds, boardRU);
  }
  else if (AType == TYPE_HALFSTAVE) {
    // in case of half stave NChips contains number of modules
    for (int imod = 0; imod < NChips; imod++) {
      int moduleId = imod + 1;
      for (int i = 0; i < 15; i++) {
        if (i == 7) continue;
        chipIds.push_back(i + ((moduleId & 0x7) << 4));
      }
    }
    Init (2, chipIds, boardMOSAIC);
  }
}


void TConfig::ReadConfigFile (const char *fName)
{
  char        Line[1024], Param[50], Rest[50];
  bool        Initialised = false;
  int         NChips      = 0;
  int         NModules    = 0;
  int         ModuleId    = DEFAULT_MODULE_ID;
  int         Chip;
  TDeviceType type        = TYPE_UNKNOWN;
  FILE       *fp          = fopen (fName, "r");

  if (!fp) {
    std::cout << "WARNING: Config file " << fName << " not found, using default configuration." << std::endl;
    return;
  }

  // first look for the type of setup in order to initialise config structure
  while ((!Initialised) && (fgets(Line, 1023, fp) != NULL)) {
    if ((Line[0] == '\n') || (Line[0] == '#')) continue;
    ParseLine (Line, Param, Rest, &Chip);
    if (!strcmp(Param,"NCHIPS")){
      sscanf(Rest, "%d", &NChips);
    }
    if (!strcmp(Param,"NMODULES")){
      sscanf(Rest, "%d", &NModules);
    }
    if (!strcmp(Param,"MODULE")){
      sscanf(Rest, "%d", &ModuleId);
    }
    if (!strcmp(Param, "DEVICE")) {
      type = ReadDeviceType (Rest);
    }
    if ((type != TYPE_UNKNOWN) && ((type != TYPE_TELESCOPE) || (NChips > 0)) && ((type != TYPE_HALFSTAVE) || (NModules == 0))) {   // type and nchips has been found (nchips not needed for type chip)
      // SetDeviceType calls the appropriate init method, which in turn calls
      // the constructors for board and chip configs
      if (type == TYPE_OBHIC) {
    	  SetDeviceType(type, ModuleId);
      } else if (type == TYPE_ENDURANCE) {
  	      SetDeviceType(type, ModuleId);
      } else if (type == TYPE_HALFSTAVE) {
    	  SetDeviceType(type, NModules);
      } else {
    	  SetDeviceType(type, NChips);
      }
      Initialised = true;
    }
  }

  fScanConfig = new TScanConfig();

  // now read the rest
  while (fgets(Line, 1023, fp) != NULL) {
    DecodeLine(Line);
  }
}


void TConfig::ParseLine(const char *Line, char *Param, char *Rest, int *Chip) {
  char MyParam[132];
  char *MyParam2;
  if (!strchr(Line, '_')) {
    *Chip = -1;
    sscanf (Line,"%s\t%s",Param, Rest);
  }
  else {
    sscanf (Line,"%s\t%s", MyParam, Rest);
    MyParam2 = strtok(MyParam, "_");
    sprintf(Param, "%s", MyParam2);
    sscanf (strpbrk(Line, "_")+1, "%d", Chip);
  }
}


void TConfig::DecodeLine(const char *Line)
{
  int Index, ChipStart, ChipStop, BoardStart, BoardStop;
  char Param[128], Rest[896];
  if ((Line[0] == '\n') || (Line[0] == '#')) {   // empty Line or comment
      return;
  }

  ParseLine(Line, Param, Rest, &Index);

  if (Index == -1) {
    ChipStart = 0;
    ChipStop  = fChipConfigs.size();
    BoardStart = 0;
    BoardStop  = fBoardConfigs.size();
  }
  else {
    ChipStart  = (Index<fChipConfigs.size())  ? Index   : -1;
    ChipStop   = (Index<fChipConfigs.size())  ? Index+1 : -1;
    BoardStart = (Index<fBoardConfigs.size()) ? Index   : -1;
    BoardStop  = (Index<fBoardConfigs.size()) ? Index+1 : -1;
  }

  // Todo: correctly handle the number of readout boards
  // currently only one is written
  // Note: having a config file with parameters for the mosaic board, but a setup with a DAQ board
  // (or vice versa) will issue unknown-parameter warnings...
  if (ChipStart>=0 && fChipConfigs.at(ChipStart)->IsParameter(Param)) {
    for (int i = ChipStart; i < ChipStop; i++) {
      fChipConfigs.at(i)->SetParamValue (Param, Rest);
    }
  }
  else if (BoardStart>=0 && fBoardConfigs.at(BoardStart)->IsParameter(Param)) {
    for (int i = BoardStart; i < BoardStop; i++) {
      fBoardConfigs.at(i)->SetParamValue (Param, Rest);
    }
  }
  else if (fScanConfig->IsParameter(Param)) {
    fScanConfig->SetParamValue (Param, Rest);
  }
  else if (BoardStart>=0 && !strcmp(Param, "ADDRESS")) {
    for (int i = BoardStart; i < BoardStop; i++) {
      if (fBoardConfigs.at(BoardStart)->GetBoardType() == boardMOSAIC) {
        ((TBoardConfigMOSAIC *)fBoardConfigs.at(i))->SetIPaddress(Rest);
      }
      else if (fBoardConfigs.at(BoardStart)->GetBoardType() == boardDAQ) {
        if (!strchr(Rest, '.')) {
          int address = -1;
          sscanf (Rest, "%d", &address);
          ((TBoardConfigDAQ *)fBoardConfigs.at(i))->SetBoardAddress(address);
        }
      }
    }
  }
  else {
    std::cout << "Warning: Unknown parameter " << Param << std::endl;
  }


}

// write config to file, has to call same function for all sub-configs (chips and boards)
void TConfig::WriteToFile (const char *fName) {

}
