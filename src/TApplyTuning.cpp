#include "TApplyTuning.h"


TApplyTuning::TApplyTuning (std::deque<TScanHisto> *histoQue, 
                            TScan                  *aScan,
                            TScanConfig            *aScanConfig, 
                            std::vector<THic*>      hics,
                            std::mutex             *aMutex,
                            TThresholdResult       *aResult)
  : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult) m_result = aResult;
  else std::cout << "Error (FATAL): no input result for TApplyTuning!" << std::endl;
}


void TApplyTuning::Run()
{
  if (!m_result) return;

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TThresholdResultHic *hicResult = (TThresholdResultHic *) m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    std::map<int, TScanResultChip*>::iterator it;
    for (it = hicResult->m_chipResults.begin(); it != hicResult->m_chipResults.end(); ++it) {
      TAlpide              *chip       = m_hics.at(ihic)->GetChipById(it->first);
      TThresholdResultChip *chipResult = (TThresholdResultChip*) it->second;
   
      // TODO: check rounding, fix makefile (unresolved reference)
      chip->GetConfig()->SetParamValue(GetDACName(), (int)chipResult->GetThresholdMean());
    }
  }

}
