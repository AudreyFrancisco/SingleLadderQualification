// Template to prepare standard test routines
// ==========================================
//
// After successful call to initSetup() the elements of the setup are accessible in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1 readout board, i.e.
// fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for staves and modules,
// all chip accesses should be done with a loop over all elements of the chip vector.
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);
// For an example how to access board-specific functions see the power off at the end of main.
//
// The functions that should be modified for the specific test are configureChip() and main()

#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "TAlpide.h"
#include "TConfig.h"
#include "THIC.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include <deque>
#include <mutex>
#include <thread>
#include <unistd.h>

#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TApplyTuning.h"
#include "TDigitalAnalysis.h"
#include "TDigitalScan.h"
#include "TFifoAnalysis.h"
#include "TFifoTest.h"
#include "THisto.h"
#include "TLocalBusAnalysis.h"
#include "TLocalBusTest.h"
#include "TReadoutAnalysis.h"
#include "TReadoutTest.h"
#include "TSCurveAnalysis.h"
#include "TSCurveScan.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

#include <ctime>

void scanLoop(TScan *myScan)
{
  std::cout << "In scan loop function" << std::endl;
  myScan->Init();

  myScan->LoopStart(2);
  while (myScan->Loop(2)) {
    myScan->PrepareStep(2);
    myScan->LoopStart(1);
    // std::cout << "Loop 1 start" << std::endl;
    while (myScan->Loop(1)) {
      myScan->PrepareStep(1);
      myScan->LoopStart(0);
      // std::cout << "Loop 0 start" << std::endl;
      while (myScan->Loop(0)) {
        myScan->PrepareStep(0);
        myScan->Execute();
        myScan->Next(0);
        // std::cout << "0";
      }
      // std::cout << std::endl << "Loop 0 end";
      myScan->LoopEnd(0);
      // std::cout << "...and...";
      myScan->Next(1);
      // std::cout << "next." << std::endl;
    }
    myScan->LoopEnd(1);
    myScan->Next(2);
    // std::cout << "Loop 1 end" << std::endl;
  }
  myScan->LoopEnd(2);
  std::cout << "Loop 2 end, terminating" << std::endl;
  myScan->Terminate();
}

// TODO:: Clean this UP !!!!

int main(int argc, char **argv)
{

  decodeCommandParameters(argc, argv);

  TBoardType                   fBoardType;
  std::vector<TReadoutBoard *> fBoards;
  std::vector<THic *>          fHics;
  std::vector<TAlpide *>       fChips;
  TConfig *                    fConfig;

  std::deque<TScanHisto> fHistoQue;
  std::mutex             fMutex;

  initSetup(fConfig, &fBoards, &fBoardType, &fChips, "Config_HS.cfg", &fHics);

  while (true) {
    char   buff[20];
    time_t now = time(NULL);
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    std::cout << buff << '\t' << std::endl;

    for (unsigned int ihic = 0; ihic < fHics.size(); ihic++) {
      if (!fHics.at(ihic)->IsEnabled()) continue;
      try {
        if (fHics.at(ihic)->GetPowerBoard()) {
          std::cout << fHics.at(ihic)->GetPowerBoard()->GetStaveTemperature(0);
          std::cout << '\t';
          std::cout << fHics.at(ihic)->GetPowerBoard()->GetStaveTemperature(1);
          std::cout << '\t';
        }
        break;
      }
      catch (std::exception &e) {
      }
    }

    std::cout << '\t';

    for (unsigned int ihic = 0; ihic < fHics.size(); ihic++) {
      if (!fHics.at(ihic)->IsEnabled()) continue;
      try {
        std::cout << fHics.at(ihic)->GetIdda() << '\t';
        std::cout << fHics.at(ihic)->GetIddd() << '\t';
        std::cout << fHics.at(ihic)->GetVdda() << '\t';
        std::cout << fHics.at(ihic)->GetVddd() << '\t';

        std::cout << fHics.at(ihic)->GetTemperature() << '\t';
        std::cout << fHics.at(ihic)->GetAnalogueVoltage() << '\t';
      }
      catch (std::exception &e) {
      }
    }
    std::cout << std::endl;
    sleep(1);
  }

  return 0;
}
