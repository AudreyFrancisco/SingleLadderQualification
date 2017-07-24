/* ---------------
 * Example of MOSAIC use
 *
 ----------------- */

#include <iostream>
#include <unistd.h>
#include <vector>
#include "TReadoutBoardMOSAIC.h"
#include "TBoardConfigMOSAIC.h"
#include "TConfig.h"
#include "SetupHelpers.h"
#include <exception>

int main(int argc, char** argv)
{
	TConfig* fConfig;

	decodeCommandParameters(argc,argv);
  initConfig(fConfig);

  TReadoutBoardMOSAIC *theBoard;
  TBoardConfigMOSAIC  *theBoardConfig;
  for(int i=0; i < fConfig->GetNBoards(); i++){
    theBoardConfig = (TBoardConfigMOSAIC*)fConfig->GetBoardConfig(i);
    theBoard       =  new TReadoutBoardMOSAIC(fConfig, theBoardConfig);
    theBoard->enableControlInterfaces(false);
	}

  return 0;
}
