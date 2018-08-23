#ifndef SETUPHELPERS_H
#define SETUPHELPERS_H

#include "TAlpide.h"
#include "THIC.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardRU.h"
#include "TReadoutBoardRUv1.h"
#include <unistd.h>

// definition of standard setup types:
//   - single chip with DAQ board
//   - IB stave with MOSAIC
//   - OB module with MOSAIC

typedef enum { setupSingle, setupIB, setupOB, setupSingleM } TSetupType;

int initSetupEndurance(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                       std::vector<TAlpide *> *chips, std::vector<THic *> *hics,
                       const char **hicIds);
int initSetupOB(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                std::vector<TAlpide *> *chips, std::vector<THic *> *hics, const char **hicIds);
int initSetupIB(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                std::vector<TAlpide *> *chips, std::vector<THic *> *hics, const char **hicIds);
int initSetupSingleRU(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                      std::vector<TAlpide *> *chips);
int initSetupSingleRUv1(TConfig *config, std::vector<TReadoutBoard *> *boards,
                        TBoardType *boardType, std::vector<TAlpide *> *chips,
                        std::vector<THic *> *hics, const char **hicIds);
int initSetupIBRU(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                  std::vector<TAlpide *> *chips, std::vector<THic *> *hics, const char **hicIds);
int initSetupSingle(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                    std::vector<TAlpide *> *chips);
int initSetupSingleMosaic(TConfig *config, std::vector<TReadoutBoard *> *boards,
                          TBoardType *boardType, std::vector<TAlpide *> *chips);
int initSetupHalfStave(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                       std::vector<TAlpide *> *chips, std::vector<THic *> *hics,
                       const char **hicIds);
int initSetupMLStave(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                     std::vector<TAlpide *> *chips, std::vector<THic *> *hics, const char **hicIds);
int initSetupPower(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                   std::vector<TAlpide *> *chips, std::vector<THic *> *hics, const char **hicIds);
int initSetup(TConfig *&config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
              std::vector<TAlpide *> *chips, const char *configFileName = "",
              std::vector<THic *> *hics = 0, const char **hicIds = 0);
int initSetupWithNames(TConfig *&config, std::vector<TReadoutBoard *> *boards,
                       TBoardType *boardType, std::vector<TAlpide *> *chips,
                       const char *configFileName = "", std::vector<THic *> *hics = 0,
                       std::vector<std::string> *hicNames = 0);
int findHic(std::vector<THic *> *hics, int modId);
int powerOn(TReadoutBoardDAQ *aDAQBoard);
int CheckControlInterface(TConfig *config, std::vector<TReadoutBoard *> *boards,
                          TBoardType *boardType, std::vector<TAlpide *> *chips);
int CheckControlInterfaceSingle(TConfig *config, TReadoutBoard *board, TBoardType *boardType,
                                TAlpide *chip);
void MakeDaisyChain(TConfig *config, std::vector<TReadoutBoard *> *boards, TBoardType *boardType,
                    std::vector<TAlpide *> *chips, int startPtr = -1);
int  decodeCommandParameters(int argc, char **argv);

void BaseConfigOBchip(TChipConfig *&chipConfig);
int  initConfig(TConfig *&  config,
                const char *configFileName = ""); // YCM: init config from command parameter
#endif
