#include "TApplyMask.h"

TApplyMask::TApplyMask (std::deque<TScanHisto> *histoQue, 
                        TScan                  *aScan,
                        TScanConfig            *aScanConfig, 
                        std::vector<THic*>      hics,
                        std::mutex             *aMutex,
                        TNoiseResult           *aResult)
  : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{

  if (aResult) {m_result = aResult;
      std::cout << "In constructor: Number of hic results: " << aResult->GetNHics() << std::endl;
  }
  else std::cout << "Error (FATAL): no input result for TApplyMask!" << std::endl;
}


void TApplyMask::Run() 
{
  if (!m_result) return;
 
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    std::cout<<"before hicresult"<<std::endl;
    std::cout << "HIC id: " << m_hics.at(ihic)->GetDbId() << std::endl;
    std::cout << "Number of hic results: " << m_result->GetNHics() << std::endl;
    TNoiseResultHic *hicResult = (TNoiseResultHic *) m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    std::cout<<"after hic result"<<std::endl;
    std::map<int, TScanResultChip*>::iterator it;
    
    std::cout<<"the size of mcipresult isvv   "<<hicResult<<std::endl;
    for (it = hicResult->m_chipResults.begin(); it != hicResult->m_chipResults.end(); ++it) {
      std::cout<<"before chip"<<std::endl;
      TAlpide          *chip       = m_hics.at(ihic)->GetChipById(it->first);
      std::cout<<"before hic result"<<std::endl;
      TNoiseResultChip *chipResult = (TNoiseResultChip*) it->second;
      
      // TODO: check rounding, fix makefile (unresolved reference)
      chip->GetConfig()->SetNoisyPixels(chipResult->GetNoisyPixels());
    }
  }
}
