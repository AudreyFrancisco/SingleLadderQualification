// Template to prepare standard test routines
// ==========================================
//
// After successful call to initSetup() the elements of the setup are accessible in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1 readout board, i.e. fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for staves and modules, 
// all chip accesses should be done with a loop over all elements of the chip vector. 
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);  
// For an example how to access board-specific functions see the power off at the end of main. 
//
// The functions that should be modified for the specific test are configureChip() and main()


#include <unistd.h>
#include <deque>
#include <thread>
#include <mutex> 
#include "TAlpide.h"
#include "THIC.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include "TConfig.h"
#include "AlpideDecoder.h"
#include "AlpideConfig.h"

#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TScan.h"
#include "TThresholdScan.h"
#include "TDigitalScan.h"
#include "TScanConfig.h"
#include "THisto.h"
#include "TScanAnalysis.h"
#include "TThresholdAnalysis.h"

void scanLoop (TScan *myScan)
{
  std::cout << "In scan loop functiokn" << std::endl;
  myScan->Init();

  myScan->LoopStart(2);
  while (myScan->Loop(2)) {
    myScan->PrepareStep(2);
    myScan->LoopStart  (1);
    while (myScan->Loop(1)) {
      myScan->PrepareStep(1);
      myScan->LoopStart  (0);
      while (myScan->Loop(0)) {
        myScan->PrepareStep(0);
        myScan->Execute    ();
        myScan->Next       (0);  
      }
      myScan->LoopEnd(0);
      // To avoid race hazard w.r.t. analysis (fit takes time). 
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      myScan->Next   (1);
    }
    myScan->LoopEnd(1);
    myScan->Next   (2);
  }
  myScan->LoopEnd  (2);
  myScan->Terminate();
}

void scanAnalysis (TScanAnalysis *myAnalysis){
 
  myAnalysis->Initialize();
  myAnalysis->Run();
  myAnalysis->Finalize();
  
}


int main(int argc, char** argv) {
  
  decodeCommandParameters(argc, argv);
  
  TBoardType fBoardType;
  std::vector <TReadoutBoard *> fBoards;
  std::vector <THic *>          fHics;
  std::vector <TAlpide *>       fChips;
  TConfig *fConfig;
  
  std::deque<TScanHisto>  fHistoQue;
  std::mutex              fMutex;
  
  initSetup(fConfig, &fBoards, &fBoardType, &fChips);
  
  TThresholdScan *myScan = new TThresholdScan(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue, &fMutex);
  TScanAnalysis *myAnalysis = new TThresholdAnalysis (&fHistoQue, myScan, fConfig->GetScanConfig(), fHics, &fMutex);
  
  std::thread scanThread(scanLoop, myScan);
  std::thread analysisThread(scanAnalysis, myAnalysis);
  //std::thread analysisThread(&TScanAnalysis::Run, std::ref(analysis));
  
  scanThread.join();
  analysisThread.join();
  
  delete myScan;
  delete myAnalysis;
  
  return 0;
}

