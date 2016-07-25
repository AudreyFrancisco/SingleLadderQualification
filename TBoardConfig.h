#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H


// DAQ board section 
const int LIMIT_DIGITAL = 300; //300;
const int LIMIT_IO      = 50;  //10, deprecated but leave in to write the register to some defined value
const int LIMIT_ANALOG  = 300; //100;

const int AUTOSHTDWN_TIME    = 10;      // time until enabling of auto shutdown
const int CLOCK_ENABLE_TIME  = 12;      // time until clock is enabled
const int SIGNAL_ENABLE_TIME = 12;      // time until signals are enabled
const int DRST_TIME          = 13;      // time until drst is deasserted

class TBoardConfig {
 private:
 protected:

 public:
  TBoardConfig(const char *fName = 0, int boardIndex = 0);

  // should these be private with getters? 
  int fLimitDigital;
  int fLimitIo;
  int fLimitAnalogue;

  int fAutoShutdownTime;
  int fClockEnableTime;
  int fSignalEnableTime;
  int fDrstTime;

};


#endif   /* BOARDCONFIG_H */
