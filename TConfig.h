#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include "TChipConfig.h"
#include "TBoardConfig.h"
#include "TReadoutBoard.h"
#include "TScanConfig.h"

const int DEFAULT_MODULE_ID = 1;

enum TDeviceType {TYPE_CHIP, TYPE_TELESCOPE, TYPE_OBHIC, TYPE_IBHIC, TYPE_CHIP_MOSAIC, TYPE_HALFSTAVE, TYPE_UNKNOWN};

class TConfig {
 private:
  std::vector <TBoardConfig *> fBoardConfigs;
  std::vector <TChipConfig *>  fChipConfigs;
  TScanConfig                 *fScanConfig;
  TDeviceType                  fDeviceType;

  void        ReadConfigFile (const char *fName);
  void        Init           (int nBoards, std::vector <int> chipIds, TBoardType boardType = boardMOSAIC); 
  void        Init           (int chipId, TBoardType boardType = boardDAQ);
  void        ParseLine      (const char *Line, char *Param, char *Rest, int *Chip);
  void        DecodeLine     (const char *Line);
  void        SetDeviceType  (TDeviceType AType, int NChips);
  TDeviceType ReadDeviceType (const char *deviceName);
 protected:
 public:
  TConfig (const char *fName);
  TConfig (int nBoards, std::vector <int> chipIds, TBoardType boardType = boardMOSAIC); 
  TConfig (int chipId, TBoardType boardType = boardDAQ);
  
  TDeviceType   GetDeviceType      () {return fDeviceType;};
  int           GetNChips          () {return fChipConfigs.size();};
  int           GetNBoards         () {return fBoardConfigs.size();};
  TChipConfig  *GetChipConfig      (int iChip);
  TChipConfig  *GetChipConfigById  (int chipId);
  TBoardConfig *GetBoardConfig     (int iBoard);
  TScanConfig  *GetScanConfig      () {return fScanConfig;};
  void          WriteToFile        (const char *fName);

};


#endif   /* CONFIG_H */
