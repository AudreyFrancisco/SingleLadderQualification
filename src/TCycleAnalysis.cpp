#include "TEnduranceCycle.h"
#include "TCycleAnalysis.h"
#include "DBHelpers.h"

#include <string>

TCycleAnalysis::TCycleAnalysis(std::deque<TScanHisto> *histoQue, 
                               TScan                  *aScan, 
                               TScanConfig            *aScanConfig, 
                               std::vector <THic*>     hics,
                               std::mutex             *aMutex, 
                               TCycleResult           *aResult )
  : TScanAnalysis (histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult) m_result = aResult;
  else         m_result = new TCycleResult();
  InitCounters();
}


void TCycleAnalysis::InitCounters()
{
  std::map<std::string, TScanResultHic*>::iterator it;
  for (it = m_result->GetHicResults().begin(); it != m_result->GetHicResults().end(); ++it) {
    TCycleResultHic *result = (TCycleResultHic *) it->second;
    result->m_nTrips          = 0;
    result->m_minWorkingChips = 14;
    result->m_nChipFailures   = 0;
    result->m_avDeltaT        = 0;
    result->m_maxDeltaT       = 0;
    result->m_avIdda          = 0;
    result->m_maxIdda         = 0;
    result->m_minIdda         = 999;
    result->m_avIddd          = 0;
    result->m_maxIddd         = 0;
    result->m_minIddd         = 999;
  }
}


// TODO write cycle variables to file
// Q: 1 file per HIC? -> yes, since attachment per activity, activity is per HIC
void TCycleAnalysis::Finalize() 
{
  std::vector <std::map <std::string, THicCounter>> counters = ((TEnduranceCycle*) m_scan)->GetCounters ();
  ((TCycleResult*)m_result)->m_nCycles = counters.size();
  for (unsigned int icycle = 0; icycle < counters.size(); icycle ++) {
    std::map <std::string, THicCounter> hicCounters = counters.at(icycle);

    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      THicCounter      hicCounter = hicCounters.at(m_hics.at(ihic)->GetDbId());
      TCycleResultHic *hicResult  = (TCycleResultHic*)m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());

      if (hicCounter.m_trip)
	hicResult->m_nTrips++;
      if (hicCounter.m_tempEnd - hicCounter.m_tempStart > hicResult->m_maxDeltaT)
	hicResult->m_maxDeltaT = hicCounter.m_tempEnd - hicCounter.m_tempStart;
      if (hicCounter.m_nWorkingChips < hicResult->m_minWorkingChips)
	hicResult->m_minWorkingChips = hicCounter.m_nWorkingChips;
      if (hicCounter.m_iddaClocked < hicResult->m_minIdda)
	hicResult->m_minIdda = hicCounter.m_iddaClocked;
      if (hicCounter.m_iddaClocked > hicResult->m_maxIdda)
	hicResult->m_maxIdda = hicCounter.m_iddaClocked;
      if (hicCounter.m_idddClocked < hicResult->m_minIddd)
	hicResult->m_minIddd = hicCounter.m_idddClocked;
      if (hicCounter.m_idddClocked > hicResult->m_maxIddd)
	hicResult->m_maxIddd = hicCounter.m_idddClocked;
      if (hicCounter.m_hicType == HIC_OB)
	hicResult->m_nChipFailures += (14 - hicCounter.m_nWorkingChips);
      else if (hicCounter.m_hicType == HIC_IB)
	hicResult->m_nChipFailures += ( 9 - hicCounter.m_nWorkingChips);

      hicResult->m_avDeltaT += hicCounter.m_tempEnd - hicCounter.m_tempStart;
      hicResult->m_avIdda   += hicCounter.m_iddaClocked;
      hicResult->m_avIddd   += hicCounter.m_idddClocked;
	  
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TCycleResultHic *hicResult  = (TCycleResultHic*)m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());

    hicResult->m_avDeltaT /= ((TCycleResult*)m_result)->m_nCycles;
    hicResult->m_avIdda   /= ((TCycleResult*)m_result)->m_nCycles;
    hicResult->m_avIddd   /= ((TCycleResult*)m_result)->m_nCycles;
  }

  WriteResult();

  m_finished = true;
}


void TCycleAnalysis::WriteResult() 
{
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (m_config->GetUseDataPath()) {
      sprintf (fName, "%s/CycleResult_%s.dat", hicResult->GetOutputPath().c_str(),
                                               m_config->GetfNameSuffix());
    }
    else {
      sprintf (fName, "CycleResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(), 
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


void TCycleResult::WriteToFileGlobal (FILE*fp) 
{
  fprintf (fp, "Number of cycles: %d\n\n", m_nCycles);
}


void TCycleResultHic::WriteToFile(FILE *fp) 
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf (fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf (fp, "Trips:                   %d\n", m_nTrips);
  fprintf (fp, "Min. number of chips:    %d\n", m_minWorkingChips);
  fprintf (fp, "Number of chip failures: %d\n", m_nChipFailures);
  fprintf (fp, "Average delta T:         %.1f\n", m_avDeltaT);
  fprintf (fp, "Maximum delta T:         %.1f\n", m_maxDeltaT);
  fprintf (fp, "Average Idda:            %.3f\n", m_avIdda);
  fprintf (fp, "Maximum Idda:            %.3f\n", m_maxIdda);
  fprintf (fp, "Minimum Idda:            %.3f\n", m_minIdda);
  fprintf (fp, "Average Iddd:            %.3f\n", m_avIddd);
  fprintf (fp, "Maximum Iddd:            %.3f\n", m_maxIddd);
  fprintf (fp, "Minimum Iddd:            %.3f\n", m_minIddd);
}

// TODO: Write to DB, classification
