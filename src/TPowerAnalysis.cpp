#include "TPowerAnalysis.h"
#include "DBHelpers.h"
#include "TPowerTest.h"

#include <string>

TPowerAnalysis::TPowerAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                               TScanConfig *aScanConfig, std::vector<THic *> hics,
                               std::mutex *aMutex, TPowerResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    m_result = new TPowerResult();
}

string TPowerAnalysis::GetPreviousTestType()
{
  switch (m_config->GetTestType()) {
  case OBQualification:
    return string("ALPIDEB Chip Testing Analysis");
  case OBEndurance:
    return string("OB HIC Qualification Test");
  case OBReception:
    return string("OB HIC Endurance Test");
  case OBHalfStaveOL:
    return string("OB HIC Reception Test");
  case OBHalfStaveML:
    return string("OB HIC Reception Test");
  case IBQualification:
    return string("ALPIDEB Chip Testing Analysis");
  case IBEndurance:
    return string("IB HIC Qualification Test");
  case IBStave:
    return string("IB HIC Qualification Test");
  case IBStaveEndurance:
    return string("IB Stave Qualification Test");
  case OBStaveOL:
    return string("OL HS Qualification Test");
  case OBStaveML:
    return string("ML HS Qualification Test");
  default:
    return string("");
  }
}

void TPowerAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  TPowerTest *powerTest = (TPowerTest *)m_scan;

  std::map<std::string, THicCurrents> currents = powerTest->GetCurrents();

  std::map<std::string, THicCurrents>::iterator it, itResult;

  for (it = currents.begin(); it != currents.end(); ++it) {
    TPowerResultHic *hicResult   = (TPowerResultHic *)m_result->GetHicResult(it->first);
    THicCurrents     hicCurrents = it->second;

    // Copy currents from currents to result, apply cuts, write to file
    hicResult->trip           = hicCurrents.trip;
    hicResult->tripBB         = hicCurrents.tripBB;
    hicResult->iddaSwitchon   = hicCurrents.iddaSwitchon;
    hicResult->idddSwitchon   = hicCurrents.idddSwitchon;
    hicResult->iddaClocked    = hicCurrents.iddaClocked;
    hicResult->idddClocked    = hicCurrents.idddClocked;
    hicResult->iddaConfigured = hicCurrents.iddaConfigured;
    hicResult->idddConfigured = hicCurrents.idddConfigured;
    hicResult->ibias0         = hicCurrents.ibias0;
    hicResult->ibias3         = hicCurrents.ibias3;
    hicResult->maxBias        = hicCurrents.maxBias;

    for (int i = 0; i < m_config->GetParamValue("IVPOINTS"); i++) {
      hicResult->ibias[i] = hicCurrents.ibias[i];
    }
    hicResult->m_class = GetClassification(hicCurrents, hicResult);
    hicResult->SetValidity(true);
  }
  WriteResult();
  m_finished = true;
}

THicClassification TPowerAnalysis::GetClassification(THicCurrents currents, TPowerResultHic *result)
{
  THicClassification returnValue;
  if (currents.trip) {
    std::cout << "Power analysis: HIC classified red due to trip" << std::endl;
    return CLASS_RED;
  }
  if (currents.hicType == HIC_IB)
    returnValue = GetClassificationIB(currents, result);
  else
    returnValue = GetClassificationOB(currents, result);

  // modify class in case of back-bias trip;
  if (currents.tripBB) {
    if (returnValue == CLASS_GOLD) returnValue   = CLASS_GOLD_NOBB;
    if (returnValue == CLASS_SILVER) returnValue = CLASS_SILVER_NOBB;
    if (returnValue == CLASS_BRONZE) returnValue = CLASS_BRONZE_NOBB;
  }
  std::cout << "Power Analysis - Classification: " << WriteHicClassification(returnValue)
            << std::endl;

  return returnValue;
}

THicClassification TPowerAnalysis::GetClassificationIB(THicCurrents     currents,
                                                       TPowerResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  DoCut(returnValue, CLASS_RED, currents.iddaSwitchon * 1000, "MINIDDA_IB", result, true);
  DoCut(returnValue, CLASS_RED, currents.idddSwitchon * 1000, "MINIDDD_IB", result, true);

  DoCut(returnValue, CLASS_SILVER, currents.idddClocked * 1000, "MINIDDD_CLOCKED_IB", result, true);
  DoCut(returnValue, CLASS_SILVER, currents.iddaClocked * 1000, "MINIDDA_CLOCKED_IB", result, true);
  DoCut(returnValue, CLASS_SILVER, currents.idddClocked * 1000, "MAXIDDD_CLOCKED_IB", result);
  DoCut(returnValue, CLASS_SILVER, currents.iddaClocked * 1000, "MAXIDDA_CLOCKED_IB", result);

  // check for absolute value at 3V and for margin from breakthrough
  DoCut(returnValue, CLASS_SILVER, currents.ibias[30], "MAXBIAS_3V_IB", result);

  return returnValue;
}

THicClassification TPowerAnalysis::GetClassificationOB(THicCurrents     currents,
                                                       TPowerResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  DoCut(returnValue, CLASS_RED, currents.iddaSwitchon * 1000, "MINIDDA_OB", result, true);
  DoCut(returnValue, CLASS_RED, currents.idddSwitchon * 1000, "MINIDDD_OB", result, true);

  DoCut(returnValue, CLASS_SILVER, currents.idddClocked * 1000, "MINIDDD_CLOCKED_OB", result, true);
  DoCut(returnValue, CLASS_SILVER, currents.iddaClocked * 1000, "MINIDDA_CLOCKED_OB", result, true);
  DoCut(returnValue, CLASS_SILVER, currents.idddClocked * 1000, "MAXIDDD_CLOCKED_OB", result);
  DoCut(returnValue, CLASS_SILVER, currents.iddaClocked * 1000, "MAXIDDA_CLOCKED_OB", result);

  // check for absolute value at 3V and for margin from breakthrough
  DoCut(returnValue, CLASS_SILVER, currents.ibias[30], "MAXBIAS_3V_OB", result);

  return returnValue;
}

void TPowerAnalysis::WriteIVCurve(THic *hic)
{
  char             fName[200];
  TPowerResultHic *result = (TPowerResultHic *)m_result->GetHicResult(hic->GetDbId());

  if (m_config->GetUseDataPath()) {
    sprintf(fName, "%s/IVCurve_%s.dat", result->GetOutputPath().c_str(),
            m_config->GetfNameSuffix());
  }
  else {
    sprintf(fName, "IVCurve_%s_%s.dat", hic->GetDbId().c_str(), m_config->GetfNameSuffix());
  }

  result->SetIVFile(fName);

  FILE *fp = fopen(fName, "w");

  for (int i = 0; i < m_config->GetParamValue("IVPOINTS"); i++) {
    fprintf(fp, "%.3f %.1f\n", (float)i / 10, result->ibias[i]);
  }
  fclose(fp);
}

void TPowerAnalysis::WriteResult()
{
  char fName[200];

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    WriteIVCurve(m_hics.at(ihic));
    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/PowerTestResult_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "PowerTestResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
              m_config->GetfNameSuffix());
    }
    m_scan->WriteConditions(fName, m_hics.at(ihic));

    FILE *fp = fopen(fName, "a");

    hicResult->SetResultFile(fName);
    hicResult->WriteToFile(fp);
    fclose(fp);

    m_scan->WriteChipRegisters(fName);
    m_scan->WriteBoardRegisters(fName);
    m_scan->WriteTimestampLog(fName);
  }
}

void TPowerResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
{
  string      fileName, ivName;
  std::size_t slash;

  WriteClassToDB(db, activity, string("power test"));
  DbAddParameter(db, activity, string("IDDD"), idddConfigured, GetParameterFile());
  DbAddParameter(db, activity, string("IDDA"), iddaConfigured, GetParameterFile());
  DbAddParameter(db, activity, string("IDDD clocked"), idddClocked, GetParameterFile());
  DbAddParameter(db, activity, string("IDDA clocked"), iddaClocked, GetParameterFile());
  DbAddParameter(db, activity, string("Back bias current 0V"), ibias0, GetParameterFile());
  DbAddParameter(db, activity, string("Back bias current 3V"), ibias3, GetParameterFile());
  DbAddParameter(db, activity, string("Maximum bias voltage"), maxBias, GetParameterFile());

  slash    = string(m_resultFile).find_last_of("/");
  fileName = string(m_resultFile).substr(slash + 1); // strip path

  slash  = string(m_ivFile).find_last_of("/");
  ivName = string(m_ivFile).substr(slash + 1); // strip path

  DbAddAttachment(db, activity, attachResult, string(m_resultFile), fileName);
  DbAddAttachment(db, activity, attachResult, string(m_ivFile), ivName);
}

void TPowerResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result: \n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "Supply trip:       %d\n", trip ? 1 : 0);
  fprintf(fp, "Back bias trip:    %d\n\n", tripBB ? 1 : 0);
  if (tripBB) {
    fprintf(fp, "   max. back bias: %.1f\n\n", maxBias);
  }
  fprintf(fp, "IDDD at switchon:  %.3f\n", idddSwitchon);
  fprintf(fp, "IDDA at switchon:  %.3f\n", iddaSwitchon);
  fprintf(fp, "IDDD with clock:   %.3f\n", idddClocked);
  fprintf(fp, "IDDA with clock:   %.3f\n", iddaClocked);
  fprintf(fp, "IDDD configured:   %.3f\n", idddConfigured);
  fprintf(fp, "IDDA configured:   %.3f\n\n", iddaConfigured);

  fprintf(fp, "IBias at 0V:       %.3f\n", ibias0);
  fprintf(fp, "IBias at 3V:       %.3f\n", ibias3);

  fprintf(fp, "\nI-V-curve file:   %s\n", m_ivFile);
}
