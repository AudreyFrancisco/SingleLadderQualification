#include <iostream>
#include <vector>
#include <string>
#include "TReadoutAnalysis.h"
#include "TReadoutTest.h"
#include "DBHelpers.h"

TReadoutAnalysis::TReadoutAnalysis(std::deque<TScanHisto> *histoQue, 
                                   TScan                  *aScan, 
                                   TScanConfig            *aScanConfig,
                                   std::vector <THic*>     hics, 
                                   std::mutex             *aMutex, 
                                   TReadoutResult           *aResult) 
: TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex) 
{
  m_nTrig = m_config->GetParamValue("NTRIG");
  m_occ   = m_config->GetParamValue("READOUTOCC");
  m_row   = ((TReadoutTest *) m_scan)->GetRow();
  if (aResult) m_result = aResult;
  else         m_result = new TReadoutResult();
  FillVariableList();
}


void TReadoutAnalysis::Initialize() 
{
  ReadChipList      ();
  CreateHicResults  ();
}


void TReadoutAnalysis::InitCounters ()
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TReadoutResultHic *hicResult = (TReadoutResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
    
    hicResult->m_missingHits = 0;
    hicResult->m_deadPixels  = 0;
    hicResult->m_ineffPixels = 0;
    hicResult->m_extraHits   = 0;
    hicResult->m_noisyPixels = 0;    
  }

  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TReadoutResultChip *chipResult = (TReadoutResultChip*) m_result->GetChipResult(m_chipList.at(i));
    chipResult->m_missingHits = 0;
    chipResult->m_deadPixels  = 0;
    chipResult->m_ineffPixels = 0;
    chipResult->m_extraHits   = 0;
    chipResult->m_noisyPixels = 0;
  }
 
}


bool TReadoutAnalysis::IsInjected (int col, int row)
{
  if (row != m_row) return false;   // wrong row
  if (m_occ == 32)  return true;    // correct row, all pixels in row injected

  int colStep = 32 / m_occ;
  if (col % colStep == 0) return true;
  return false;
}


void TReadoutAnalysis::AnalyseHisto  (TScanHisto *histo)
{
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TReadoutResultChip *chipResult = (TReadoutResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
    //int                 channel    = m_chipList.at(ichip).dataReceiver;
    //int                 chipId     = m_chipList.at(ichip).chipId;
    //int                 boardIndex = m_chipList.at(ichip).boardIndex;
  
    if (!chipResult) {
      std::cout << "Warning (TReadoutAnalysis): Missing chip result" << std::endl;
      continue;
    }

    for (int icol = 0; icol < 1024; icol ++) {
      for (int irow = 0; irow < 512; irow ++) {
        // check if pixel selected; if yes -> has to have NTRIG hits, if not has to have 0
	if (IsInjected(icol, irow)) {
	  if ((*histo)(m_chipList.at(ichip), icol, irow) == 0) {
            chipResult->m_missingHits += m_nTrig;
	    chipResult->m_deadPixels  ++;
	  }
          else if (!((*histo)(m_chipList.at(ichip), icol, irow) == m_nTrig)) {
            chipResult->m_missingHits += m_nTrig - (*histo)(m_chipList.at(ichip), icol, irow);
	    chipResult->m_ineffPixels ++;
	  }
	}
	else {
	  if ((*histo)(m_chipList.at(ichip), icol, irow) != 0) {
            chipResult->m_extraHits   += (*histo)(m_chipList.at(ichip), icol, irow);
	    chipResult->m_noisyPixels ++;
	  }	  
	}
      }
    }
  }
}


void TReadoutAnalysis::Finalize()
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TReadoutResultHic *hicResult = (TReadoutResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
    hicResult->m_errorCounter = m_scan->GetErrorCount(m_hics.at(ihic)->GetDbId());
    std::cout << "8b10b errors: " << hicResult->m_errorCounter.n8b10b<< std::endl;
  }

  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip ++) {
    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
      if (! (m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TReadoutResultChip *chipResult = (TReadoutResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
      TReadoutResultHic  *hicResult  = (TReadoutResultHic*)  m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());

      hicResult->m_missingHits += chipResult->m_missingHits;
      hicResult->m_deadPixels  += chipResult->m_deadPixels;
      hicResult->m_ineffPixels += chipResult->m_ineffPixels;
      hicResult->m_extraHits   += chipResult->m_extraHits;
      hicResult->m_noisyPixels += chipResult->m_noisyPixels;
    }
  }

  WriteResult      ();

  m_finished = true;
}


void TReadoutAnalysis::WriteResult()
{
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (m_config->GetUseDataPath()) {
      sprintf (fName, "%s/ReadoutScanResult_%s.dat", hicResult->GetOutputPath().c_str(),
                                                     m_config->GetfNameSuffix());
    }
    else {
      sprintf (fName, "ReadoutScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(), 
                                                     m_config->GetfNameSuffix());
    }
    m_scan  ->WriteConditions (fName, m_hics.at(ihic));

    FILE *fp = fopen (fName, "a");
    m_result->WriteToFileGlobal(fp);
    hicResult->SetResultFile(fName);
    hicResult->WriteToFile  (fp);
    fclose (fp);
  }
}


void TReadoutResultHic::WriteToFile (FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf (fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf (fp, "8b10b errors:   %d\n", m_errorCounter.n8b10b);
  fprintf (fp, "Corrupt events: %d\n", m_errorCounter.nCorruptEvent);
  fprintf (fp, "Timeouts:       %d\n", m_errorCounter.nTimeout);  

  fprintf (fp, "Missing hits:       %d\n", m_missingHits);
  fprintf (fp, "Extra hits:         %d\n", m_extraHits);
  fprintf (fp, "Dead pixels:        %d\n", m_deadPixels);
  fprintf (fp, "Inefficient pixels: %d\n", m_ineffPixels);
  fprintf (fp, "Noisy pixels:       %d\n", m_noisyPixels);

  fprintf(fp, "\nNumber of chips: %d\n\n", (int) m_chipResults.size());

  std::map<int, TScanResultChip*>::iterator it;
  
  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
}


void TReadoutResultChip::WriteToFile (FILE *fp)
{
  fprintf (fp, "Missing hits:       %d\n", m_missingHits);
  fprintf (fp, "Extra hits:         %d\n", m_extraHits);
  fprintf (fp, "Dead pixels:        %d\n", m_deadPixels);
  fprintf (fp, "Inefficient pixels: %d\n", m_ineffPixels);
  fprintf (fp, "Noisy pixels:       %d\n", m_noisyPixels);
}
