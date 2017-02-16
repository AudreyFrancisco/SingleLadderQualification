/* ---------------
 * Example of MOSAIC use
 *
 ----------------- */

#include <iostream>
#include <unistd.h>
#include "TReadoutBoard.h"
#include "TReadoutBoardMOSAIC.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "TConfig.h"
#include "TAlpide.h"
#include <exception>

int main()
{
	TBoardConfigMOSAIC 	*theBoardConfiguration;
	TReadoutBoardMOSAIC *theBoard;

	TConfig *config = new TConfig ("Config.cfg");
	theBoard = new TReadoutBoardMOSAIC(config, (TBoardConfigMOSAIC*)config->GetBoardConfig(0));

	theBoard->enableClockOutput(false);

    return 0;
}
