#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include "TChipConfig.h"
#include "TBoardConfig.h"
#include "TReadoutBoard.h"

enum TDeviceType {TYPE_CHIP, TYPE_TELESCOPE, TYPE_MODULE, TYPE_UNKNOWN};

class TConfig {
 private:
  std::vector <TBoardConfig *> fBoardConfigs;
  std::vector <TChipConfig *>  fChipConfigs;
  void ReadConfigFile (const char *fName);
  void Init           (int nBoards, std::vector <int> chipIds, TBoardType boardType = boardMOSAIC); 
  void Init           (int chipId, TBoardType boardType = boardDAQ);
 protected:
 public:
  TConfig (const char *fName);
  TConfig (int nBoards, std::vector <int> chipIds, TBoardType boardType = boardMOSAIC); 
  TConfig (int chipId, TBoardType boardType = boardDAQ);

  int GetNChips () {return fChipConfigs.size();};
  TChipConfig  *GetChipConfig  (int chipId);
  TBoardConfig *GetBoardConfig (int iBoard);
  void WriteToFile (const char *fName);

};


#endif   /* CONFIG_H */
