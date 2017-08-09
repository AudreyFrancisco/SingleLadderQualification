#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>
#include "TChipConfig.h"
#include "TBoardConfig.h"
#include "TReadoutBoard.h"
#include "TScanConfig.h"

const int DEFAULT_MODULE_ID = 1;

enum TDeviceType {TYPE_CHIP, TYPE_TELESCOPE, TYPE_OBHIC, TYPE_IBHIC, TYPE_CHIP_MOSAIC,
					TYPE_HALFSTAVE, TYPE_IBHICRU, TYPE_ENDURANCE, TYPE_UNKNOWN};

class TChipConfig;

class TConfig {
 private:
  std::vector <TBoardConfig *> fBoardConfigs;
  std::vector <TChipConfig *>  fChipConfigs;
  TScanConfig                 *fScanConfig;
  TDeviceType                  fDeviceType;
  bool                         fUsePowerBoard;

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

  TDeviceType   GetDeviceType      ()           {return fDeviceType;};
  unsigned int  GetNChips          ()           {return fChipConfigs.size();};
  unsigned int  GetNBoards         ()           {return fBoardConfigs.size();};
  bool          GetUsePowerBoard   ()           {return fUsePowerBoard;};
  void          SetUsePowerBoard   (bool UsePB) {fUsePowerBoard = UsePB;};
  TChipConfig  *GetChipConfig      (unsigned int iChip);
  TChipConfig  *GetChipConfigById  (int chipId);
  TBoardConfig *GetBoardConfig     (unsigned int iBoard);
  TScanConfig  *GetScanConfig      () {return fScanConfig;};
  void          WriteToFile        (const char *fName);

  std::string   GetSoftwareVersion();
};


#endif   /* CONFIG_H */
