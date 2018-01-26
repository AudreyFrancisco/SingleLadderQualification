/* ---------------
 * Example of MOSAIC use
 *
 ----------------- */

#include "SetupHelpers.h"
#include "TBoardConfigMOSAIC.h"
#include "TConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include <exception>
#include <iostream>
#include <unistd.h>
#include <vector>

int main(int argc, char **argv) {
  TConfig *fConfig;

  decodeCommandParameters(argc, argv);
  initConfig(fConfig);

  TReadoutBoardMOSAIC *theBoard;
  TBoardConfigMOSAIC *theBoardConfig;
  for (unsigned int i = 0; i < fConfig->GetNBoards(); i++) {
    theBoardConfig = (TBoardConfigMOSAIC *)fConfig->GetBoardConfig(i);
    theBoard = new TReadoutBoardMOSAIC(fConfig, theBoardConfig);
    theBoard->enableControlInterfaces(false);
  }

  return 0;
}
