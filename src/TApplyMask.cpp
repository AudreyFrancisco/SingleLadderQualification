#include "TApplyMask.h"

TApplyMask::TApplyMask (std::deque<TScanHisto> *histoQue, 
                        TScan                  *aScan,
                        TScanConfig            *aScanConfig, 
                        std::vector<THic*>      hics,
                        std::mutex             *aMutex,
                        TNoiseResult           *aResult)
  : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{

  if (aResult) {
    m_result = aResult;
  }
  else std::cout << "Error (FATAL): no input result for TApplyMask!" << std::endl;
}


void TApplyMask::Run() 
{
  if (!m_result) return;

  m_config->SetIsMasked (true);
 
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TNoiseResultHic *hicResult = (TNoiseResultHic *) m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    std::map<int, TScanResultChip*>::iterator it;
    
    for (it = hicResult->m_chipResults.begin(); it != hicResult->m_chipResults.end(); ++it) {
      TAlpide          *chip       = m_hics.at(ihic)->GetChipById(it->first);
      TNoiseResultChip *chipResult = (TNoiseResultChip*) it->second;
      
      chip->GetConfig()->SetNoisyPixels(chipResult->GetNoisyPixels());
    }
  }
}
