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
#include <unistd.h>

#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "THisto.h"
#include "TLocalBusTest.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

int main(int argc, char **argv) {

  decodeCommandParameters(argc, argv);

  TBoardType fBoardType;
  std::vector<TReadoutBoard *> fBoards;
  std::vector<THic *> fHics;
  std::vector<TAlpide *> fChips;
  TConfig *fConfig;

  std::deque<TScanHisto> fHistoQue;
  std::mutex fMutex;

  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  TLocalBusTest *myScan =
      new TLocalBusTest(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue, &fMutex);

  // THisto *histo = new THisto("Test", "Test", 1024, 0, 1023, 50, 0, 50);

  // histo->Set(40, 40, 2);
  // histo->Incr(40, 40);
  // std::cout << "histo readback: " << (*histo)(40, 40) << std::endl;

  myScan->Init();

  myScan->LoopStart(2);
  while (myScan->Loop(2)) {
    myScan->PrepareStep(2);
    myScan->LoopStart(1);
    while (myScan->Loop(1)) {
      myScan->PrepareStep(1);
      myScan->LoopStart(0);
      while (myScan->Loop(0)) {
        myScan->PrepareStep(0);
        myScan->Execute();
        myScan->Next(0);
      }
      myScan->LoopEnd(0);
      myScan->Next(1);
    }
    myScan->LoopEnd(1);
    myScan->Next(2);
  }
  myScan->LoopEnd(2);
  myScan->Terminate();

  return 0;
}
