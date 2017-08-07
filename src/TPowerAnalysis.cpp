#include "TPowerTest.h"
#include "TPowerAnalysis.h"

TPowerAnalysis::TPowerAnalysis(std::deque<TScanHisto> *histoQue, 
                               TScan                  *aScan, 
                               TScanConfig            *aScanConfig, 
                               std::vector <THic*>     hics,
                               std::mutex             *aMutex, 
                               TPowerResult           *aResult )
  : TScanAnalysis (histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult) m_result = aResult;
  else         m_result = new TPowerResult(); 
}


void TPowerAnalysis::Finalize() 
{
  TPowerTest *powerTest = (TPowerTest*) m_scan;
  
  std::map <std::string, THicCurrents> currents = powerTest->GetCurrents();

  std::map <std::string, THicCurrents>::iterator it, itResult;

  for (it = currents.begin(); it != currents.end(); ++it) { 
    TPowerResultHic *hicResult   = (TPowerResultHic *) m_result->GetHicResult (it->first);
    THicCurrents     hicCurrents = it->second;

    // Copy currents from currents to result, apply cuts, write to file
    hicResult->iddaSwitchon   = hicCurrents.iddaSwitchon;
    hicResult->idddSwitchon   = hicCurrents.idddSwitchon;
    hicResult->iddaClocked    = hicCurrents.iddaClocked;
    hicResult->idddClocked    = hicCurrents.idddClocked;
    hicResult->iddaConfigured = hicCurrents.iddaConfigured;
    hicResult->idddConfigured = hicCurrents.idddConfigured;
    hicResult->ibias0         = hicCurrents.ibias0;
    hicResult->ibias3         = hicCurrents.ibias3;

    hicResult->m_class        = GetClassification(hicCurrents);
  }

}


THicClassification TPowerAnalysis::GetClassification (THicCurrents currents) 
{
  if (currents.hicType == HIC_IB) return GetClassificationIB(currents);
  else                            return GetClassificationOB(currents);
}


THicClassification TPowerAnalysis::GetClassificationIB (THicCurrents currents)
{
  // TODO: Check if currents in mA or in A
  if ((currents.iddaSwitchon < m_config->GetParamValue("MINIDDA_IB")) || 
      (currents.idddSwitchon < m_config->GetParamValue("MINIDDD_IB")))
    return CLASS_RED;

  if ((currents.iddaClocked < m_config->GetParamValue("MINIDDA_CLOCKED_IB")) ||
      (currents.iddaClocked > m_config->GetParamValue("MAXIDDA_CLOCKED_IB")) ||
      (currents.idddClocked < m_config->GetParamValue("MINIDDD_CLOCKED_IB")) ||
      (currents.idddClocked > m_config->GetParamValue("MAXIDDD_CLOCKED_IB")))
    return CLASS_ORANGE;

  return CLASS_GREEN;

  // TODO: Add orange for back bias
}


THicClassification TPowerAnalysis::GetClassificationOB (THicCurrents currents)
{
  // TODO: Check if currents in mA or in A
  if ((currents.iddaSwitchon < m_config->GetParamValue("MINIDDA_OB")) || 
      (currents.idddSwitchon < m_config->GetParamValue("MINIDDD_OB")))
      return CLASS_RED;
  if ((currents.iddaClocked < m_config->GetParamValue("MINIDDA_CLOCKED_OB")) ||
      (currents.iddaClocked > m_config->GetParamValue("MAXIDDA_CLOCKED_OB")) ||
      (currents.idddClocked < m_config->GetParamValue("MINIDDD_CLOCKED_OB")) ||
      (currents.idddClocked > m_config->GetParamValue("MAXIDDD_CLOCKED_OB")))
    return CLASS_ORANGE;
  return CLASS_GREEN;

  // TODO: Add orange for back bias
}
