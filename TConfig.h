#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include "TChipConfig.h"
#include "TBoardConfig.h"
#include "TReadoutBoard.h"


class TConfig {
 private:
  std::vector <TBoardConfig *> fBoardConfigs;
  std::vector <TChipConfig *>  fChipConfigs;
 protected:
 public:
  TConfig (const char *fName);
  TConfig (int nBoards, std::vector <int> chipId, TBoardType boardType = boardMOSAIC); 
  TConfig (int chipId, TBoardType boardType = boardDAQ);

  TChipConfig  *GetChipConfig  (int chipId);
  TBoardConfig *GetBoardConfig (int iBoard);
  void WriteToFile (const char *fName);

};


#endif   /* CONFIG_H */
