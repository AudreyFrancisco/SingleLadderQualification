#ifndef SETUPHELPERS_H
#define SETUPHELPERS_H

#include <unistd.h>
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardRU.h"


// definition of standard setup types: 
//   - single chip with DAQ board
//   - IB stave with MOSAIC
//   - OB module with MOSAIC

typedef enum {setupSingle, setupIB, setupOB, setupSingleM} TSetupType;

int  initSetupOB          (TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips);
int  initSetupIB          (TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips);
int  initSetupIBRU        (TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips);
int  initSetupSingle      (TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips);
int  initSetupSingleMosaic(TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips);
int  initSetupHalfStave   (TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips);
int  initSetup            (TConfig*& config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips, const char *configFileName = "");
int  powerOn              (TReadoutBoardDAQ *aDAQBoard);
int  CheckControlInterface(TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips);
void MakeDaisyChain       (TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips);
int  decodeCommandParameters(int argc, char **argv);

#endif
