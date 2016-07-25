#include "TBoardConfigDAQ.h"

TBoardConfigDAQ::TBoardConfigDAQ(const char *fName, int boardIndex) {
  // fill default value from header file
  fLimitDigital  = LIMIT_DIGITAL;
  fLimitIo       = LIMIT_IO;
  fLimitAnalogue = LIMIT_ANALOG;

  fAutoShutdownTime = AUTOSHTDWN_TIME;
  fClockEnableTime  = CLOCK_ENABLE_TIME;
  fSignalEnableTime = SIGNAL_ENABLE_TIME;
  fDrstTime         = DRST_TIME;

  fEnableDDR = false;
  fInvertCMU = false;


  // read configuration from file
  if (fName) {

  }
}
