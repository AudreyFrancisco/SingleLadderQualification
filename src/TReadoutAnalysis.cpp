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
    //TReadoutResultHic *hicResult = (TReadoutResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
    
  }
}


void TReadoutAnalysis::AnalyseHisto  (TScanHisto *histo)
{
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TReadoutResultChip *chipResult = (TReadoutResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
    //int                 channel    = m_chipList.at(ichip).dataReceiver;
    //int                 chipId     = m_chipList.at(ichip).chipId;
    //int                 boardIndex = m_chipList.at(ichip).boardIndex;
  
    if (!chipResult) {
      std::cout << "Warning (TNoiseAnalysis): Missing chip result" << std::endl;
      continue;
    }

    for (int icol = 0; icol < 1024; icol ++) {
      for (int irow = 0; irow < 512; irow ++) {
        // check if pixel selected; if yes -> has to have NTRIG hits, if not has to have 0
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
}
