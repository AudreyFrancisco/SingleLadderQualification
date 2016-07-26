#include "TBoardConfigDAQ.h"

TBoardConfigDAQ::TBoardConfigDAQ(const char *fName, int boardIndex) {
  // fill default value from header file
  fCurrentLimitDigital  = LIMIT_DIGITAL;
  fCurrentLimitIo       = LIMIT_IO;
  fCurrentLimitAnalogue = LIMIT_ANALOG;

  fAutoShutdownTime = AUTOSHTDWN_TIME;
  fClockEnableTime  = CLOCK_ENABLE_TIME;
  fSignalEnableTime = SIGNAL_ENABLE_TIME;
  fDrstTime         = DRST_TIME;

  fDDREnable = false;
  fInvertCMUBus = false;


  // read configuration from file
  if (fName) {

  }
}
