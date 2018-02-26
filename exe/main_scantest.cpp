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
#include "TSCurveScan.h"
#include "TDigitalScan.h"
#include "TScanConfig.h"
#include "THisto.h"
#include "TScanAnalysis.h"
#include "TSCurveAnalysis.h"
#include "TDigitalAnalysis.h"
#include "TFifoTest.h"
#include "TFifoAnalysis.h"
#include "TLocalBusTest.h"
#include "TLocalBusAnalysis.h"
#include "TApplyTuning.h"

#include <ctime>

void scanLoop(TScan *myScan) {
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

int main(int argc, char **argv) {

  decodeCommandParameters(argc, argv);

  TBoardType fBoardType;
  std::vector<TReadoutBoard *> fBoards;
  std::vector<THic *> fHics;
  std::vector<TAlpide *> fChips;
  TConfig *fConfig;
  TSCurveResult *fResult = new TSCurveResult();

  std::deque<TScanHisto> fHistoQue;
  std::mutex fMutex;

  initSetup(fConfig, &fBoards, &fBoardType, &fChips, "Config.cfg", &fHics);

  // TDigitalScan *myScan   = new TDigitalScan(fConfig->GetScanConfig(), fChips, fHics, fBoards,
  // &fHistoQue, &fMutex);
  // TScanAnalysis  *analysis = new TDigitalAnalysis (&fHistoQue, myScan, fConfig->GetScanConfig(),
  // fHics, &fMutex);
  // TScan *myScan   = new TLocalBusTest(fConfig->GetScanConfig(), fChips, fHics, fBoards,
  // &fHistoQue, &fMutex);
  // TScanAnalysis  *analysis = new TLocalBusAnalysis (&fHistoQue, myScan, fConfig->GetScanConfig(),
  // fHics, &fMutex);

  // Now testing full calibration!
  // Timing:
  std::clock_t start;
  double elapsed;
  start = std::clock();
  //...
  // elapsed=(std::clock()-start)/(double)CLOCKS_PER_SEC;
  // std::cout << "Time for scan+analysis: " << elapsed << " sec" << std::endl;

  TtuneVCASNScan *myScan_V =
      new TtuneVCASNScan(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue, &fMutex);
  TSCurveAnalysis *analysis_V = new TSCurveAnalysis(&fHistoQue, myScan_V, fConfig->GetScanConfig(),
                                                    fHics, &fMutex, fResult, 1);
  std::cout << "starting thread VCASN" << std::endl;
  std::thread scanThread_V(scanLoop, myScan_V);
  analysis_V->Initialize();
  std::thread analysisThread_V(&TScanAnalysis::Run, analysis_V);
  scanThread_V.join();
  analysisThread_V.join();
  analysis_V->Finalize();

  elapsed = (std::clock() - start) / (double)CLOCKS_PER_SEC;
  std::cout << "Time for scan+analysis: " << elapsed << " sec" << std::endl;

  std::cout << "Printing mean VCASN thresholds:" << std::endl;
  //  std::map<int,common::TStatVar> thresh_V = analysis_V->DeleteThis();
  // for (std::map<int,common::TStatVar>::iterator it = thresh_V.begin(); it != thresh_V.end();
  // it++) {
  //  std::cout << "Chip " << it->first << ", mean threshold " << it->second.mean << std::endl;
  // }

  // for (unsigned int ihic = 0; ihic < fHics.size(); ihic ++) {
  // TSCurveResultHic *hicResult = (TSCurveResultHic *)
  // fResult->GetHicResult(fHics.at(ihic)->GetDbId());
  //    std::map<int, TScanResultChip*> mp = hicResult->DeleteThisToo();
  // std::map<int, TScanResultChip*>::iterator it;
  // for (it = mp.begin(); it != mp.end(); ++it) {
  // TAlpide              *chip       = fHics.at(ihic)->GetChipById(it->first);
  // TSCurveResultChip *chipResult = (TSCurveResultChip*) it->second;
  //
  // chip->GetConfig()->SetParamValue(GetDACName(), (int)chipResult->GetThresholdMean());
  //  std::cout << "Found in Config: " << it->first << ", thr=" << chipResult->GetThresholdMean() <<
  // std::endl;
  //}
  //}

  if (fResult)
    std::cout << "fResult OK" << std::endl;
  // TApplyVCASNTuning *apply_V = new TApplyVCASNTuning(&fHistoQue, NULL, fConfig->GetScanConfig(),
  // fHics, &fMutex, fResult);
  // std::cout << "starting thread apply_V" << std::endl;
  // apply_V->Initialize();
  // std::thread analysisThread_apply_V(&TScanAnalysis::Run, apply_V);
  // analysisThread_apply_V.join();
  // apply_V->Finalize();
  // std::cout << "Finalized apply_V" << std::endl;

  /*fResult=new TThresholdResult();
  TtuneITHRScan *myScan_I = new TtuneITHRScan(fConfig->GetScanConfig(), fChips, fHics, fBoards,
  &fHistoQue, &fMutex);
  TThresholdAnalysis *analysis_I = new TThresholdAnalysis(&fHistoQue, myScan_I,
  fConfig->GetScanConfig(), fHics, &fMutex, fResult, -1);
  std::cout << "starting thread ITHR" << std::endl;
  std::thread scanThread_I(scanLoop, myScan_I);
  analysis_I->Initialize();
  std::thread analysisThread_I(&TScanAnalysis::Run, analysis_I);
  scanThread_I.join();
  analysisThread_I.join();
  analysis_I->Finalize();
  std::cout << "Printing mean ITHR thresholds:" << std::endl;
  std::map<int,common::TStatVar> thresh_I = analysis_I->DeleteThis();
  for (std::map<int,common::TStatVar>::iterator it = thresh_I.begin(); it != thresh_I.end(); it++) {
    std::cout << "Chip " << it->first << ", mean threshold " << it->second.mean << std::endl;
  }

  if(fResult) std::cout << "fResult OK 2" << std::endl;
  TApplyITHRTuning *apply_I = new TApplyITHRTuning(&fHistoQue, NULL, fConfig->GetScanConfig(),
  fHics, &fMutex, fResult);
  std::cout << "starting thread apply_I" << std::endl;
  apply_I->Initialize();
  std::thread analysisThread_apply_I(&TScanAnalysis::Run, apply_I);
  analysisThread_apply_I.join();
  apply_I->Finalize();
  std::cout << "Finalized apply_I" << std::endl;
  */

  /*fResult=NULL;
  TThresholdScan *myScan_T = new TThresholdScan(fConfig->GetScanConfig(), fChips, fHics, fBoards,
  &fHistoQue, &fMutex);
  TThresholdAnalysis *analysis_T = new TThresholdAnalysis(&fHistoQue, myScan_T,
  fConfig->GetScanConfig(), fHics, &fMutex, fResult);
  std::cout << "starting thread Threshold" << std::endl;
  std::thread scanThread_T(scanLoop, myScan_T);
  analysis_T->Initialize();
  std::thread analysisThread_T(&TScanAnalysis::Run, analysis_T);
  scanThread_T.join();
  analysisThread_T.join();
  analysis_T->Finalize();
  std::cout << "Printing mean thresholds:" << std::endl;
  std::map<int,common::TStatVar> thresh_T = analysis_T->DeleteThis();
  for (std::map<int,common::TStatVar>::iterator it = thresh_T.begin(); it != thresh_T.end(); it++) {
    std::cout << "Chip " << it->first << ", mean threshold " << it->second.mean << std::endl;
  }*/

  // std::vector <TCounter> counters = ((TDigitalAnalysis*)analysis)->GetCounters();

  // std::cout << std::endl << "Counter values: " << std::endl;
  // for (int i = 0; i < counters.size(); i ++) {
  //   std::cout << "Chip " << counters.at(i).chipId <<": nCorrect = " << counters.at(i).nCorrect <<
  // std::endl;
  // }

  delete myScan_V;
  delete analysis_V;
  // delete apply_V;
  // delete myScan_I;
  // delete analysis_I;
  // delete apply_I;
  // delete myScan_T;
  // delete analysis_T;
  delete fResult;

  return 0;
}
