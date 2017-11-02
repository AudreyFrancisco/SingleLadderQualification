#include <iostream>
#include <vector>
#include <string>
#include "TDigitalAnalysis.h"
#include "TDigitalScan.h"
#include "DBHelpers.h"

TDigitalAnalysis::TDigitalAnalysis(std::deque<TScanHisto> *histoQue, 
                                   TScan                  *aScan, 
                                   TScanConfig            *aScanConfig, 
                                   std::vector <THic*>     hics,
                                   std::mutex             *aMutex, 
                                   TDigitalResult         *aResult) 
: TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex) 
{
  m_ninj   = m_config->GetParamValue("NINJ");
  if (aResult) m_result = aResult;
  else         m_result = new TDigitalResult(); 
  FillVariableList ();
}


//TODO: Implement HasData
bool TDigitalAnalysis::HasData(TScanHisto &histo,  common::TChipIndex idx, int col) 
{
  return true;
}


void TDigitalAnalysis::FillVariableList () 
{
  //m_variableList.insert (std::pair <const char *, TResultVariable> ("Chip Status", status));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# of dead Pixels", deadPix));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# of noisy Pixels", noisyPix));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# of ineff Pixels", ineffPix));
  //m_variableList.insert (std::pair <const char *, TResultVariable> ("# of bad double columns", badDcol));
}


void TDigitalAnalysis::Initialize() 
{
  ReadChipList      ();
  CreateHicResults  ();
}


void TDigitalAnalysis::InitCounters () 
{
  m_counters.clear();
  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TDigitalCounter counter;
    counter.boardIndex = m_chipList.at(i).boardIndex;
    counter.receiver   = m_chipList.at(i).dataReceiver;
    counter.chipId     = m_chipList.at(i).chipId;
    counter.nCorrect   = 0;
    counter.nIneff     = 0;
    counter.nNoisy     = 0;
    m_counters.push_back(counter);

    TDigitalResultChip *result = (TDigitalResultChip*) m_result->GetChipResult(m_chipList.at(i));

    result->m_nStuck    = 0;
    result->m_nDead     = 0;
    result->m_nNoisy    = 0;
    result->m_nIneff    = 0;
    result->m_nBadDcols = 0;

  }

  std::map<std::string, TScanResultHic*>::iterator it;

  for (it = m_result->GetHicResults().begin(); it != m_result->GetHicResults().end(); ++it) {
    TDigitalResultHic *result = (TDigitalResultHic *) it->second;
    result->m_nBad      = 0;
    result->m_nStuck    = 0;
    result->m_nBadDcols = 0;
    result->m_lower     = ((TDigitalScan*) m_scan)->IsLower  ();
    result->m_upper     = ((TDigitalScan*) m_scan)->IsUpper  ();
    result->m_nominal   = ((TDigitalScan*) m_scan)->IsNominal();
  }

}


void TDigitalAnalysis::WriteHitData(TScanHisto *histo, int row) 
{
  char fName[100];
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    sprintf(fName, "Digital_%s_B%d_Rcv%d_Ch%d.dat", m_config->GetfNameSuffix(), 
	                                            m_chipList.at(ichip).boardIndex, 
                                                    m_chipList.at(ichip).dataReceiver, 
                                                    m_chipList.at(ichip).chipId);
    FILE *fp = fopen (fName, "a");
    for (int icol = 0; icol < 1024; icol ++) {
      if ((*histo)(m_chipList.at(ichip), icol) > 0) {  // write only non-zero values
        fprintf(fp, "%d %d %d\n", icol, row, (int) (*histo)(m_chipList.at(ichip), icol));
      }
    }
    fclose(fp);
  }
}


void TDigitalAnalysis::WriteResult() 
{

  // should write to file: Conditions, global, results
  // separate files: stuck pixels (how to separate by HIC?)
  // hitmap file? 
  // write both paths to result structure
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    WriteStuckPixels (m_hics.at(ihic));
    sprintf (fName, "DigitalScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(), 
                                                   m_config->GetfNameSuffix());
    m_scan  ->WriteConditions (fName, m_hics.at(ihic));

    FILE *fp = fopen (fName, "a");
    m_result->WriteToFileGlobal(fp);
    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->SetResultFile(fName);
    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->WriteToFile  (fp);
    fclose (fp);
    //    m_result->WriteToFile     (fName);
  }
}


void TDigitalAnalysis::WriteStuckPixels(THic *hic) 
{
  char fName[100];
  sprintf (fName, "StuckPixels_%s_%s.dat", hic->GetDbId().c_str(), 
                                           m_config->GetfNameSuffix());
  
  ((TDigitalResultHic*)m_result->GetHicResult(hic->GetDbId()))->SetStuckFile(fName);

  FILE                 *fp     = fopen (fName, "w");
  std::vector<TPixHit>  pixels = ((TMaskScan*)m_scan)->GetStuckPixels();

  for (unsigned int i = 0; i < pixels.size(); i++) {
    if (!common::HitBelongsToHic(hic, pixels.at(i))) continue;
    fprintf (fp, "%d %d %d %d\n", pixels.at(i).chipId, pixels.at(i).region, pixels.at(i).dcol,pixels.at(i).address);
  }
  fclose(fp);
}


void TDigitalAnalysis::AnalyseHisto (TScanHisto *histo) 
{
  int row = histo->GetIndex();
  std::cout << "ANALYSIS: Found histo for row " << row << ", size = " << m_histoQue->size() << std::endl;
  WriteHitData(histo, row);
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    for (int icol = 0; icol < 1024; icol ++) {
      int hits = (int) (*histo) (m_chipList.at(ichip), icol);
      if      (hits == m_ninj) m_counters.at(ichip).nCorrect ++;         
      else if (hits >  m_ninj) m_counters.at(ichip).nNoisy ++;
      else if (hits >  0)      m_counters.at(ichip).nIneff ++;
    }
  }
}


void TDigitalAnalysis::Finalize() {
  TErrorCounter         errCount = ((TMaskScan*)m_scan)->GetErrorCount();
  TDigitalResult       *result   = (TDigitalResult*) m_result;
  std::vector<TPixHit>  stuck    = ((TMaskScan*)m_scan)->GetStuckPixels();  

  result->m_nTimeout       = errCount.nTimeout;
  result->m_n8b10b         = errCount.n8b10b;
  result->m_nCorrupt       = errCount.nCorruptEvent;
     
  for (unsigned int ichip = 0; ichip < m_chipList.size();ichip ++ ) {
    TDigitalResultChip* chipResult = (TDigitalResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
      
    if (!chipResult) std::cout << "WARNING: chipResult = 0" << std::endl;
    chipResult->m_nDead  = 512 * 1024 - (m_counters.at(ichip).nCorrect + m_counters.at(ichip).nNoisy + m_counters.at(ichip).nIneff);
    chipResult->m_nNoisy = m_counters.at(ichip).nNoisy;
    chipResult->m_nIneff = m_counters.at(ichip).nIneff;
  }
   
  // for the time being divide stuck pixels on different chips here
  // later: change AlpideDecoder?

  for (unsigned int istuck = 0; istuck < stuck.size(); istuck++) {
    int entry = common::FindIndexForHit(m_chipList, stuck.at(istuck));
    if (entry >= 0) {
        TDigitalResultChip* chipResult = (TDigitalResultChip*) m_result->GetChipResult(m_chipList.at(entry));
        chipResult->m_stuck.push_back(stuck.at(istuck));
        chipResult->m_nStuck++;
    }
  }

  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip ++) {
    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
      if (! (m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TDigitalResultChip *chipResult = (TDigitalResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
      TDigitalResultHic  *hicResult  = (TDigitalResultHic*)  m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
      hicResult->m_nBad      += chipResult->m_nDead + chipResult->m_nIneff + chipResult->m_nNoisy;
      hicResult->m_nBadDcols += chipResult->m_nBadDcols;
      hicResult->m_nStuck    += chipResult->m_nStuck;
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TDigitalResultHic *hicResult = (TDigitalResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
    if (m_hics.at(ihic)->GetHicType() == HIC_OB) {
      hicResult->m_class = GetClassificationOB(hicResult);
    }
    else {
      hicResult->m_class = GetClassificationIB(hicResult);
    }
    hicResult->m_errorCounter = m_scan->GetErrorCount(m_hics.at(ihic)->GetDbId());
  }
  WriteResult      ();

  m_finished = true;
}


//TODO: Add readout errors, requires dividing readout errors by hic (receiver)
//TODO: Make two cuts (red and orange)?
THicClassification TDigitalAnalysis::GetClassificationOB(TDigitalResultHic* result) {
  if (result->m_nBad > m_config->GetParamValue("DIGITAL_MAXBAD_HIC_OB")) return CLASS_ORANGE;
  for (unsigned int ichip = 0; ichip < result->m_chipResults.size(); ichip ++) {
    int chipId = m_chipList.at(ichip).chipId & 0xf;
    TDigitalResultChip *chipResult = (TDigitalResultChip*) result->m_chipResults.at(chipId);
    if (chipResult->m_nDead + chipResult->m_nNoisy + chipResult->m_nIneff 
	> m_config->GetParamValue("DIGITAL_MAXBAD_CHIP_OB"))
      return CLASS_ORANGE;
  }
  return CLASS_GREEN;

}


THicClassification TDigitalAnalysis::GetClassificationIB(TDigitalResultHic* result) {
  if (result->m_nBad > m_config->GetParamValue("DIGITAL_MAXBAD_HIC_IB")) return CLASS_ORANGE;
  for (unsigned int ichip = 0; ichip < result->m_chipResults.size(); ichip ++) {
    int chipId = m_chipList.at(ichip).chipId & 0xf;
    TDigitalResultChip *chipResult = (TDigitalResultChip*) result->m_chipResults.at(chipId);
    if (chipResult->m_nDead + chipResult->m_nNoisy + chipResult->m_nIneff 
	> m_config->GetParamValue("DIGITAL_MAXBAD_CHIP_IB"))
      return CLASS_ORANGE;
  }
  return CLASS_GREEN;
}


void TDigitalResult::WriteToFileGlobal (FILE *fp) 
{
  fprintf(fp, "8b10b errors:\t%d\n",    m_n8b10b);
  fprintf(fp, "Corrupt events:\t%d\n",  m_nCorrupt);
  fprintf(fp, "Timeouts:\t%d\n",        m_nTimeout);
}


void TDigitalResultHic::WriteToDB (AlpideDB *db, ActivityDB::activity &activity)
{
  std::string dvdd;
  if (m_nominal) {
    dvdd = string(" (nominal)");
  }
  else if (m_lower) {
    dvdd = string(" (lower)");
  }
  else if (m_upper) {
    dvdd = string(" (upper)");
  }

  DbAddParameter  (db, activity, string ("Timeouts digital") + dvdd,                (float) m_errorCounter.nTimeout);
  DbAddParameter  (db, activity, string ("8b10b errors digital") + dvdd,            (float) m_errorCounter.n8b10b);
  DbAddParameter  (db, activity, string ("Corrupt events digital") + dvdd,          (float) m_errorCounter.nCorruptEvent);
  DbAddParameter  (db, activity, string ("Priority encoder errors digital") + dvdd, (float) m_errorCounter.nPrioEncoder);
  DbAddParameter  (db, activity, string ("Bad double columns digital") + dvdd,      (float) m_nBadDcols);
  DbAddParameter  (db, activity, string ("Bad pixels digital") + dvdd,               (float) m_nBad);
  DbAddAttachment (db, activity, attachResult, string(m_resultFile), string(m_resultFile) + dvdd);

}


void TDigitalResultHic::WriteToFile (FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf (fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "Bad pixels:      %d\n", m_nBad);
  fprintf(fp, "Bad double cols: %d\n", m_nBadDcols);
  fprintf(fp, "Stuck pixels:    %d\n", m_nStuck);

  fprintf(fp, "\nStuck pixel file: %s\n", m_stuckFile);

  fprintf(fp, "\nNumber of chips: %d\n\n", (int) m_chipResults.size());

  std::map<int, TScanResultChip*>::iterator it;
  
  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }

  std::cout << std::endl << "Error counts (Test feature): " << std::endl;
  std::cout << "8b10b errors:  " << m_errorCounter.n8b10b << std::endl;
  std::cout << "corrupt events " << m_errorCounter.nCorruptEvent << std::endl;
  std::cout << "timeouts:      " << m_errorCounter.nTimeout << std::endl;
}


void TDigitalResultChip::WriteToFile (FILE *fp) 
{
  fprintf(fp, "Dead pixels:        %d\n", m_nDead);
  fprintf(fp, "Inefficient pixels: %d\n", m_nIneff);
  fprintf(fp, "Noisy pixels:       %d\n", m_nNoisy);
  fprintf(fp, "Bad double cols:    %d\n", m_nBadDcols);
  fprintf(fp, "Stuck pixels:       %d\n", m_nStuck);
}


float TDigitalResultChip::GetVariable (TResultVariable var) {
  switch (var) {
  case deadPix:
    return (float) m_nDead;
  case noisyPix: 
    return (float) m_nNoisy;
  case ineffPix: 
    return (float) m_nIneff;
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}
