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
  WriteResult();
  m_finished = true;
}


THicClassification TPowerAnalysis::GetClassification (THicCurrents currents) 
{
  if (currents.hicType == HIC_IB) return GetClassificationIB(currents);
  else                            return GetClassificationOB(currents);
}


THicClassification TPowerAnalysis::GetClassificationIB (THicCurrents currents)
{
  if ((currents.iddaSwitchon * 1000 < m_config->GetParamValue("MINIDDA_IB")) || 
      (currents.idddSwitchon * 1000 < m_config->GetParamValue("MINIDDD_IB")))
    return CLASS_RED;

  if ((currents.iddaClocked * 1000 < m_config->GetParamValue("MINIDDA_CLOCKED_IB")) ||
      (currents.iddaClocked * 1000 > m_config->GetParamValue("MAXIDDA_CLOCKED_IB")) ||
      (currents.idddClocked * 1000 < m_config->GetParamValue("MINIDDD_CLOCKED_IB")) ||
      (currents.idddClocked * 1000 > m_config->GetParamValue("MAXIDDD_CLOCKED_IB")))
    return CLASS_ORANGE;

  return CLASS_GREEN;

  // TODO: Add orange for back bias
}


THicClassification TPowerAnalysis::GetClassificationOB (THicCurrents currents)
{
  if ((currents.iddaSwitchon * 1000 < m_config->GetParamValue("MINIDDA_OB")) || 
      (currents.idddSwitchon * 1000 < m_config->GetParamValue("MINIDDD_OB")))
      return CLASS_RED;

  if ((currents.iddaClocked * 1000 < m_config->GetParamValue("MINIDDA_CLOCKED_OB")) ||
      (currents.iddaClocked * 1000 > m_config->GetParamValue("MAXIDDA_CLOCKED_OB")) ||
      (currents.idddClocked * 1000 < m_config->GetParamValue("MINIDDD_CLOCKED_OB")) ||
      (currents.idddClocked * 1000 > m_config->GetParamValue("MAXIDDD_CLOCKED_OB")))
    return CLASS_ORANGE;
  return CLASS_GREEN;

  // TODO: Add orange for back bias
}


void TPowerAnalysis::WriteResult() 
{
  char fName[200];
  
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    sprintf (fName, "PowerTestResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(), 
                                                 m_config->GetfNameSuffix());
    m_scan  ->WriteConditions (fName, m_hics.at(ihic));
    
    FILE *fp = fopen (fName, "a");
  
    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->SetResultFile(fName);
    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->WriteToFile(fp);
    fclose(fp);
    //    m_result->WriteToFile     (fName);
 
  }

}


void TPowerResultHic::WriteToFile (FILE *fp) 
{
  fprintf (fp, "HIC Result: \n\n");

  fprintf (fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf (fp, "IDDD at switchon: %.3f\n", idddSwitchon);
  fprintf (fp, "IDDA at switchon: %.3f\n", iddaSwitchon);
  fprintf (fp, "IDDD with clock:  %.3f\n", idddClocked);
  fprintf (fp, "IDDA with clock:  %.3f\n", iddaClocked);
  fprintf (fp, "IDDD configured:  %.3f\n", idddConfigured);
  fprintf (fp, "IDDA configured:  %.3f\n\n", iddaConfigured);

  fprintf (fp, "IBias at 0V:      %.3f\n", ibias0);
  fprintf (fp, "IBias at 3V:      %.3f\n", ibias3);

}