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


void TReadoutAnalysis::Finalize()
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TReadoutResultHic *hicResult = (TReadoutResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
    hicResult->m_errorCounter = m_scan->GetErrorCount(m_hics.at(ihic)->GetDbId());
  }
}
