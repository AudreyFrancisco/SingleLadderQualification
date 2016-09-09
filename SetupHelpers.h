#ifndef SETUPHELPERS_H
#define SETUPHELPERS_H

#include <unistd.h>
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"

// definition of standard setup types: 
//   - single chip with DAQ board
//   - IB stave with MOSAIC
//   - OB module with MOSAIC

typedef enum {setupSingle, setupIB, setupOB} TSetupType;

// chip ID that is used in case of single chip setup
extern int fSingleChipId;

// module ID that is used for outer barrel modules 
// (1 will result in master chip IDs 0x10 and 0x18, 2 in 0x20 and 0x28 ...)
extern int fModuleId;

extern TBoardType fBoardType;
extern TSetupType fSetupType;

extern std::vector <TReadoutBoard *> fBoards;
extern std::vector <TAlpide *>       fChips;

extern TConfig *fConfig;

int initSetupOB    ();
int initSetupIB    ();
int initSetupSingle();
int initSetup      ();
int powerOn        (TReadoutBoardDAQ *aDAQBoard);
#endif
