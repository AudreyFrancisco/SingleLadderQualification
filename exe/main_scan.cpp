#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TAlpide.h"
#include "TConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include <unistd.h>

#include "TScanManager.h"

int main(int argc, char **argv)
{
  TScanManager mgr;

  decodeCommandParameters(argc, argv);
  mgr.Init();

  mgr.AddScan(STDigital);

  mgr.Run();

  return 0;
}
