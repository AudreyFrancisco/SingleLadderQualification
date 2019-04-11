#include <future>

#include "SetupHelpers.h"
#include "TConfig.h"
#include "TScanFactory.h"
#include "TScanManager.h"

TScanManager::TScanManager() {}

void TScanManager::Init() { initSetup(fConfig, &fBoards, &boardType, &fChips, "", &fHICs); }

TScanFactory::TScanObjects TScanManager::AddScan(TScanType scanType, TScanResult *scanResult)
{
  TScanConfig *config = fConfig->GetScanConfig();

  auto scanObjects = TScanFactory::CreateScanObjects(scanType, config, fChips, fHICs, fBoards,
                                                     &fHistoQue, &fMutex, scanResult);

  // if (scanObjects.analysis) {
  //  fScans.push_back(scanObjects.scan);
  //  fAnalyses.push_back(scanObjects.analysis);
  //  fResults.push_back(scanObjects.result);
  //  fScanTypes.push_back(scanType);
  //  fScanParameters.push_back(fScans.back() ? fScans.back()->GetParameters() : nullptr);
  //}
  fScanObjects.push_back(scanObjects);

  return scanObjects;
}

void TScanManager::Reset() { fScanObjects.clear(); }

void TScanManager::Run()
{
  for (auto scanObj : fScanObjects) {
    scanObj.scan->Init();
    auto future_scan = std::async(std::launch::async, &TScanManager::Scan, scanObj.scan);
    auto future_analysis =
        std::async(std::launch::async, &TScanManager::Analysis, scanObj.analysis);
    future_scan.get();
    future_analysis.get();
  }
}

void TScanManager::Scan(TScan *scan)
{
  scan->LoopStart(2);

  while (scan->Loop(2)) {
    scan->PrepareStep(2);
    scan->LoopStart(1);

    while (scan->Loop(1)) {
      scan->PrepareStep(1);
      scan->LoopStart(0);

      while (scan->Loop(0)) {
        scan->PrepareStep(0);
        scan->Execute();
        scan->Next(0);
      }
      scan->LoopEnd(0);
      scan->Next(1);
    }
    scan->LoopEnd(1);
    scan->Next(2);
  }
  scan->LoopEnd(2);
  scan->Terminate();
}

void TScanManager::Analysis(TScanAnalysis *analysis)
{
  analysis->Initialize();
  analysis->Run();
}
