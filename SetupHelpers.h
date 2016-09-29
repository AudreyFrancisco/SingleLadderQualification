#ifndef SETUPHELPERS_H
#define SETUPHELPERS_H

#include <unistd.h>
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"

// definition of standard setup types: 
//   - single chip with DAQ board
//   - IB stave with MOSAIC
//   - OB module with MOSAIC

typedef enum {setupSingle, setupIB, setupOB, setupSingleM} TSetupType;

extern TBoardType fBoardType;

extern std::vector <TReadoutBoard *> fBoards;
extern std::vector <TAlpide *>       fChips;

extern TConfig *fConfig;

int initSetupOB          ();
int initSetupIB          ();
int initSetupSingle      ();
int initSetupSingleMosaic();
int initSetup            (const char *configFileName = "Config.cfg");
int powerOn              (TReadoutBoardDAQ *aDAQBoard);
#endif
