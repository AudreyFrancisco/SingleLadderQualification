#include <iostream>
#include <vector>
#include <string>
#include "TFifoAnalysis.h"
#include "TFifoTest.h"
#include "DBHelpers.h"

// TODO: Add number of exceptions to result
// TODO: Add errors per region to chip result

TFifoAnalysis::TFifoAnalysis(std::deque<TScanHisto> *histoQue, 
                             TScan                  *aScan, 
                             TScanConfig            *aScanConfig,
                             std::vector <THic*>     hics, 
                             std::mutex             *aMutex, 
                             TFifoResult            *aResult)
: TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex) 
{
  if (aResult) 
    m_result = aResult;
  else
    m_result = new TFifoResult();

  
  FillVariableList ();
}


void TFifoAnalysis::Initialize() 
{
  ReadChipList     ();
  CreateHicResults ();
}


void TFifoAnalysis::FillVariableList ()
{
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# Errors 0x0000", Err0));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# Errors 0xffff", Errf));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# Errors 0x5555", Err5));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# Errors 0xaaaa", Erra));
}


void TFifoAnalysis::InitCounters() 
{
  m_counters.clear();

  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TFifoCounter counter;
    counter.boardIndex = m_chipList.at(i).boardIndex;
    counter.receiver   = m_chipList.at(i).dataReceiver;
    counter.chipId     = m_chipList.at(i).chipId;
    counter.err0       = 0;
    counter.err5       = 0;
    counter.erra       = 0;
    counter.errf       = 0;
    m_counters.push_back(counter);
  }

  std::map<std::string, TScanResultHic* >::iterator it;
  std::map<int,         TScanResultChip*>::iterator itChip;

  for (it = m_result->GetHicResults().begin(); it != m_result->GetHicResults().end(); ++it) {
    TFifoResultHic *result = (TFifoResultHic*) it->second;
    result->m_nExceptions  = 0;
    result->m_nFaultyChips = 0;
    result->m_err0         = 0;
    result->m_err5         = 0;
    result->m_erra         = 0;
    result->m_errf         = 0;
    result->m_lower        = ((TFifoTest*) m_scan)->IsLower  ();
    result->m_upper        = ((TFifoTest*) m_scan)->IsUpper  ();
    result->m_nominal      = ((TFifoTest*) m_scan)->IsNominal();
    for (itChip = result->m_chipResults.begin(); itChip != result->m_chipResults.end(); ++itChip) {
      TFifoResultChip *resultChip = (TFifoResultChip*) itChip->second;
      resultChip->m_err0       = 0;
      resultChip->m_err5       = 0;
      resultChip->m_erra       = 0;
      resultChip->m_errf       = 0;
    }
  }

}


void TFifoAnalysis::WriteResult () {
  char fName[200];

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId()); 
    if (m_config->GetUseDataPath()) {
      sprintf (fName, "%s/FifoScanResult_%s.dat", hicResult->GetOutputPath().c_str(),
                                                  m_config->GetfNameSuffix());
    }
    else {
      sprintf (fName, "FifoScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(), 
                                                  m_config->GetfNameSuffix());
    }
    m_scan  ->WriteConditions (fName, m_hics.at(ihic));
    
    FILE *fp = fopen (fName, "a");
  
    hicResult->SetResultFile(fName);
    hicResult->WriteToFile(fp);
    fclose(fp);
  }
}


void TFifoAnalysis::AnalyseHisto (TScanHisto *histo)
{
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    for (int ireg = 0; ireg < 32; ireg ++) {
      m_counters.at(ichip).err0 += (int) ((*histo) (m_chipList.at(ichip), ireg, 0x0));
      m_counters.at(ichip).err5 += (int) ((*histo) (m_chipList.at(ichip), ireg, 0x5));
      m_counters.at(ichip).erra += (int) ((*histo) (m_chipList.at(ichip), ireg, 0xa));
      m_counters.at(ichip).errf += (int) ((*histo) (m_chipList.at(ichip), ireg, 0xf));
    }
  }
}


void TFifoAnalysis::Finalize()
{
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip ++) {
    TFifoResultChip* chipResult = (TFifoResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
    if (!chipResult) std::cout << "WARNING: chipResult = 0" << std::endl;

    chipResult->m_err0 = m_counters.at(ichip).err0;
    chipResult->m_err5 = m_counters.at(ichip).err5;
    chipResult->m_erra = m_counters.at(ichip).erra;
    chipResult->m_errf = m_counters.at(ichip).errf;

    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      if (! (m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TFifoResultHic *hicResult = (TFifoResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
      hicResult->m_err0 += chipResult->m_err0;
      hicResult->m_err5 += chipResult->m_err5;
      hicResult->m_erra += chipResult->m_erra;
      hicResult->m_errf += chipResult->m_errf;
      if (chipResult->m_err0 + chipResult->m_err5 + chipResult->m_erra + chipResult->m_errf > 0)
        hicResult->m_nFaultyChips ++;
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++)  {
    TFifoResultHic *hicResult = (TFifoResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
    hicResult->m_class = GetClassification(hicResult);
  }

  WriteResult ();

  m_finished = true;
}


THicClassification TFifoAnalysis::GetClassification (TFifoResultHic *result)
{
  if (result->m_nExceptions > 0) return CLASS_RED;
  if (result->m_err0 + result->m_err5 + result->m_erra + result->m_errf == 0) return CLASS_GREEN;
  
  if ((result->m_err0 < m_config->GetParamValue("FIFO_MAXERR")) && 
      (result->m_err5 < m_config->GetParamValue("FIFO_MAXERR")) && 
      (result->m_erra < m_config->GetParamValue("FIFO_MAXERR")) && 
      (result->m_errf < m_config->GetParamValue("FIFO_MAXERR")) && 
      (result->m_nFaultyChips < m_config->GetParamValue("FIFO_MAXFAULTY"))) return CLASS_ORANGE;

  return CLASS_RED;
}


void TFifoResultHic::WriteToFile (FILE *fp) 
{
  fprintf (fp, "HIC Result:\n\n");

  fprintf (fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "Errors in pattern 0x0000: %d\n", m_err0);
  fprintf(fp, "Errors in pattern 0x5555: %d\n", m_err5);
  fprintf(fp, "Errors in pattern 0xaaaa: %d\n", m_erra);
  fprintf(fp, "Errors in pattern 0xffff: %d\n", m_errf);
  
  fprintf(fp, "\nNumber of chips: %d\n\n", (int) m_chipResults.size());

  std::map<int, TScanResultChip*>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
}


void TFifoResultHic::GetParameterSuffix (std::string &suffix, std::string &file_suffix) {
  if (m_nominal) {
    suffix      = string(" (nominal)");
    file_suffix = string("_nominal");
  }
  else if (m_lower) {
    suffix      = string(" (lower)");
    file_suffix = string("_lower");
  }
  else if (m_upper) {
    suffix      = string(" (upper)");
    file_suffix = string("_upper");
  }
}


void TFifoResultHic::WriteToDB (AlpideDB *db, ActivityDB::activity &activity)
{
  std::string suffix, file_suffix, fileName;
  GetParameterSuffix (suffix, file_suffix);

  DbAddParameter (db, activity, string ("FIFO errors") + suffix,            (float) (m_err0 + m_err5 + m_erra + m_errf));
  DbAddParameter (db, activity, string ("Chips with FIFO errors") + suffix, (float)m_nFaultyChips);

  std::size_t point = string(m_resultFile).find_last_of(".");
  fileName = string(m_resultFile).substr (0, point) + file_suffix + ".dat";
  DbAddAttachment (db, activity, attachResult, string(m_resultFile), fileName);
}


float TFifoResultChip::GetVariable (TResultVariable var) {
  switch (var) {
  case Err0: 
    return (float) m_err0;
  case Err5: 
    return (float) m_err5;
  case Erra: 
    return (float) m_erra;
  case Errf: 
    return (float) m_errf;
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}


void TFifoResultChip::WriteToFile (FILE *fp) 
{
  fprintf(fp, "Errors in pattern 0x0000: %d\n", m_err0);
  fprintf(fp, "Errors in pattern 0x5555: %d\n", m_err5);
  fprintf(fp, "Errors in pattern 0xaaaa: %d\n", m_erra);
  fprintf(fp, "Errors in pattern 0xffff: %d\n", m_errf);
}
