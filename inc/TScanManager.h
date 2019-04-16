#pragma once

#include <future>

#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanFactory.h"

class TScanManager {
public:
  TScanManager();

  TScanFactory::TScanObjects AddScan(TScanType scanType, TScanResult *scanResult = nullptr);
  TScanFactory::TScanObjects InsertScan(int i, TScanType scanType,
                                        TScanResult *scanResult = nullptr);

  void         Init();
  void         Reset();
  void         Run();
  void         PrintClassifications();
  void         UpdateClassifications();
  unsigned int GetNScans() const { return fScanObjects.size(); }

  // spawn in background
  std::future<void> SpawnInit(int i);
  std::future<void> SpawnScan(int i);
  std::future<void> SpawnAnalysis(int i);
  std::future<void> SpawnFinalize(int i);

  static std::string GetResult(THicClassification cl);
  static void        Scan(TScan *scan);
  static void        Analysis(TScanAnalysis *analysis);

protected:
  TConfig *                    fConfig;
  std::vector<TReadoutBoard *> fBoards;
  TBoardType                   fBoardType;
  std::vector<TAlpide *>       fChips;
  std::vector<THic *>          fHICs;
  std::deque<TScanHisto>       fHistoQueue;
  std::mutex                   fMutex;

  std::vector<TScanFactory::TScanObjects> fScanObjects;
  decltype(fScanObjects)::iterator        fScanIterator;

  // legacy members for GUI support
  std::vector<TScan *>           fScans;
  std::vector<TScanAnalysis *>   fAnalyses;
  std::vector<TScanParameters *> fScanParameters;
  std::vector<TScanResult *>     fResults;
  std::vector<TScanType>         fScanTypes;

public:
  auto                 Config() -> decltype(fConfig) & { return fConfig; }
  auto                 Chips() -> decltype(fChips) & { return fChips; }
  auto                 Boards() -> decltype(fBoards) & { return fBoards; }
  auto                 BoardType() -> decltype(fBoardType) & { return fBoardType; }
  auto                 Mutex() -> decltype(fMutex) & { return fMutex; }
  auto                 HistoQueue() -> decltype(fHistoQueue) & { return fHistoQueue; }
  std::vector<THic *> &HICs() { return fHICs; }
  auto                 Scans() -> decltype(fScans) & { return fScans; }
  auto                 Analyses() -> decltype(fAnalyses) & { return fAnalyses; }
  auto                 Parameters() -> decltype(fScanParameters) & { return fScanParameters; }
  auto                 Results() -> decltype(fResults) & { return fResults; }
  auto                 ScanTypes() -> decltype(fScanTypes) & { return fScanTypes; }
};
