#include "TApplyTuning.h"
#include <cstring>

TApplyTuning::TApplyTuning(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                           std::vector<THic *> hics, std::mutex *aMutex, TSCurveResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    std::cout << "Error (FATAL): no input result for TApplyTuning!" << std::endl;
}

void TApplyTuning::Run()
{
  if (!m_result) return;

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TSCurveResultHic *hicResult =
        (TSCurveResultHic *)m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    std::map<int, TScanResultChip *>::iterator it;
    for (it = hicResult->m_chipResults.begin(); it != hicResult->m_chipResults.end(); ++it) {
      TAlpide *          chip       = m_hics.at(ihic)->GetChipById(it->first);
      TSCurveResultChip *chipResult = (TSCurveResultChip *)it->second;

      // TODO: check rounding, fix makefile (unresolved reference)
      chip->GetConfig()->SetParamValue(GetDACName(), (int)chipResult->GetThresholdMean());
      if (strcmp(GetDACName(), "VCASN") == 0)
        chip->GetConfig()->SetParamValue("VCASN2", (int)chipResult->GetThresholdMean() + 12);
      std::cout << "Setting chip " << it->first << ", thr=" << chipResult->GetThresholdMean()
                << std::endl;
    }
  }
}
