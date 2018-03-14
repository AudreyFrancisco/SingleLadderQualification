#include "TDCTRLAnalysis.h"
#include "AlpideConfig.h"
#include "DBHelpers.h"
#include "TFifoTest.h"
#include <iostream>
#include <string>
#include <vector>


TDctrlAnalysis::TDctrlAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                               TScanConfig *aScanConfig, std::vector<THic *> hics,
                               std::mutex *aMutex, TDctrlResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    m_result = new TDctrlResult();

  FillVariableList();
}

void TDctrlAnalysis::Initialize()
{
  ReadChipList();
  CreateHicResults();
}

// variables to be displayed in the GUI
void TDctrlAnalysis::FillVariableList() {}

string TDctrlAnalysis::GetPreviousTestType()
{
  switch (m_config->GetTestType()) {
  case OBEndurance:
    return string("OB HIC Qualification Test");
  case OBReception:
    return string("OB HIC Qualification Test");
  case OBHalfStaveOL:
    return string("OB HIC Reception Test");
  case OBHalfStaveML:
    return string("OB HIC Reception Test");
  case IBStave:
    return string("IB HIC Qualification Test");
  case IBStaveEndurance:
    return string("IB Stave Qualification Test");
  default:
    return string("");
  }
}

void TDctrlAnalysis::InitCounters()
{
  // m_counters.clear();


  std::map<std::string, TScanResultHic *>::iterator it;
  std::map<int, TScanResultChip *>::iterator        itChip;

  for (it = m_result->GetHicResults()->begin(); it != m_result->GetHicResults()->end(); ++it) {
    TDctrlResultHic *result = (TDctrlResultHic *)it->second;
    for (itChip = result->m_chipResults.begin(); itChip != result->m_chipResults.end(); ++itChip) {
      TDctrlResultChip *resultChip = (TDctrlResultChip *)itChip->second;
      (void)resultChip; // TODO: Set here the initialisations in the chip (and hic) result if
                        // necessary
    }
  }
}

void TDctrlAnalysis::WriteResult()
{
  char fName[200];

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    if (!m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->IsValid()) continue;
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/DctrlScanResult_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "DctrlScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
              m_config->GetfNameSuffix());
    }
    m_scan->WriteConditions(fName, m_hics.at(ihic));

    FILE *fp = fopen(fName, "a");

    hicResult->SetResultFile(fName);
    hicResult->WriteToFile(fp);
    fclose(fp);

    m_scan->WriteChipRegisters(fName);
    m_scan->WriteBoardRegisters(fName);
    m_scan->WriteTimestampLog(fName);
  }
}

void TDctrlAnalysis::AnalyseHisto(TScanHisto *histo)
{
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    // add here the analysis of the single histos
    // access values like:

    float amplitude = ((*histo)(m_chipList.at(ichip), 10));
    std::cout << "Amplitude at driver setting 10: " << amplitude << std::endl;
  }
}

void TDctrlAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TDctrlResultChip *chipResult =
        (TDctrlResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
    if (!chipResult) std::cout << "WARNING: chipResult = 0" << std::endl;

    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      if (!(m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TDctrlResultHic *hicResult =
          (TDctrlResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
      hicResult->SetValidity(true);
      // TODO: Set here the final variables in hic Result, determine hic classification
    }
  }


  WriteResult();

  m_finished = true;
}

THicClassification TDctrlAnalysis::GetClassification(TDctrlResultHic *result)
{
  (void)result;
  return CLASS_GREEN;
}

void TDctrlResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "\nNumber of chips: %d\n\n", (int)m_chipResults.size());

  std::map<int, TScanResultChip *>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
}

void TDctrlResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
{
  std::string fileName, remoteName;

  // to be updated, probably divide according to tested device (IB / OB HIC)
  DbAddParameter(db, activity, string("Slope master 0"), (float)0);

  std::size_t slash = string(m_resultFile).find_last_of("/");
  fileName          = string(m_resultFile).substr(slash + 1); // strip path
  std::size_t point = fileName.find_last_of(".");
  remoteName        = fileName.substr(0, point) + ".dat";
  DbAddAttachment(db, activity, attachResult, string(m_resultFile), remoteName);
}


float TDctrlResultChip::GetVariable(TResultVariable var)
{
  switch (var) {
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}

void TDctrlResultChip::WriteToFile(FILE *fp) { fprintf(fp, "to be definedn"); }
