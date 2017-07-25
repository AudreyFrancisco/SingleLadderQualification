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
#include "TDigitalAnalysis.h"
#include "TAnalogScan.h"

void scanLoop (TScan *myScan)
{
  std::cout << "In scan loop function" << std::endl;
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
      myScan->Next   (1);
    }
    myScan->LoopEnd(1);
    myScan->Next   (2);
  }
  myScan->LoopEnd  (2);
  std::cout << "DONE" << std::endl;
  myScan->Terminate();
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
  const char ** hicIds;  //this needs to come from *SOMEWHERE*, OK for now...

  initSetup(fConfig, &fBoards, &fBoardType, &fChips, "", &fHics, hicIds);
  TtuneVCASNScan *myTuneVScan = new TtuneVCASNScan(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue,&fMutex);
  TThresholdAnalysis  *analysisTuneV = new TThresholdAnalysis (&fHistoQue,myTuneVScan, fConfig->GetScanConfig(), fHics, &fMutex, 1); 

  //testing other classes...
  //TThresholdScan *myTuneVScan = new TThresholdScan(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue,&fMutex);
  //TThresholdAnalysis *analysisTuneV = new TThresholdAnalysis(&fHistoQue, myTuneVScan, fConfig->GetScanConfig(), fHics, &fMutex, 1);
  //TDigitalScan *myTuneVScan = new TDigitalScan(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue, &fMutex);
  //TScanAnalysis * analysisTuneV = new TDigitalAnalysis(&fHistoQue, myTuneVScan, fConfig->GetScanConfig(), fHics, &fMutex);

  std::cout << "starting thread" << std::endl;
  std::thread scanThreadV(scanLoop, myTuneVScan);
  analysisTuneV->Initialize(); //should allocate for GetResultThreshold...
  std::thread analysisThreadI(&TScanAnalysis::Run, std::ref(analysisTuneV));

  scanThreadV.join();
  analysisThreadI.join();
  analysisTuneV->Finalize();
  // std::vector <TCounter> counters = ((TDigitalAnalysis*)analysis)->GetCounters();
  
  float * vcasn = new float[fHics.size()];
  std::cout << "Printing VCASN thresholds:" << std::endl; //need to know SPECIFIC chip number!!
  int hicnum = 0;
  for(std::vector<THic*>::iterator it = fHics.begin(); it<fHics.end(); it++) {  //For each HIC, visit each chip:
    std::cout << "HIC " << hicnum << std::endl;
    std::vector<TAlpide*> chips = (*it)->GetChipVector();
    float sum = 0;
    for(std::vector<TAlpide*>::iterator chp = chips.begin(); chp < chips.end(); chp++) {
      unsigned int id = (*chp)->GetChipId();
      std::cout << "ChipId got" << std::endl;
      sum += analysisTuneV->GetResultThreshold(id);
      std::cout << "Chip " << id << ":  " << analysisTuneV->GetResultThreshold(id) << std::endl;
    }
    vcasn[hicnum] = sum / (float)(chips.size());
    hicnum++; ///NOTE THE ORDER
  }

  // std::cout << std::endl << "Counter values: " << std::endl;
  // for (int i = 0; i < counters.size(); i ++) {
  //   std::cout << "Chip " << counters.at(i).chipId <<": nCorrect = " << counters.at(i).nCorrect << std::endl;
  // }
  
  delete myTuneVScan;
  delete analysisTuneV;
  return 0;
}

