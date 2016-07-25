#include "TBoardConfigDAQ.h"

TBoardConfigDAQ::TBoardConfigDAQ(const char *fName, int boardIndex) {
  // fill default value from header file
  fLimitAnalogue = LIMIT_ANALOG;
  fLimitIo       = LIMIT_IO;
  fLimitDigital  = LIMIT_DIGITAL;

  fAutoShutdownTime = AUTOSHTDWN_TIME;
  fClockEnableTime  = CLOCK_ENABLE_TIME;
  fSignalEnableTime = SIGNAL_ENABLE_TIME;
  fDrstTime         = DRST_TIME;

  // read configuration from file
  if (fName) {
  }
}
