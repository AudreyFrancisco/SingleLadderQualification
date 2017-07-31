#include <iostream>
#include <vector>
#include "TFifoAnalysis.h"


// TODO: Add number of exceptions to result
// TODO: Add errors per region to chip result

TFifoAnalysis::TFifoAnalysis(std::deque<TScanHisto> *histoQue, 
                             TScan                  *aScan, 
                             TScanConfig            *aScanConfig,
                             std::vector <THic*>     hics, 
                             std::mutex             *aMutex) 
: TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex) 
{
  m_result = new TFifoResult();
  FillVariableList ();
}


void TFifoAnalysis::Initialize() 
{
  ReadChipList     ();
  CreateHicResults ();
}


void TFifoAnalysis::FillVariableList ()
{
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# Errors 0x0000", fifoErr0));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# Errors 0xffff", fifoErrf));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# Errors 0x5555", fifoErr5));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# Errors 0xaaaa", fifoErra));
}


void TFifoAnalysis::InitCounters() 
{
  m_counters.clear();

  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TFifoCounter counter;
    counter.boardIndex = m_chipList.at(i).boardIndex;
    counter.receiver   = m_chipList.at(i).dataReceiver;
    counter.chipId     = m_chipList.at(i).chipId;
    counter.err0       = 0;
    counter.err5       = 0;
    counter.erra       = 0;
    counter.errf       = 0;
    m_counters.push_back(counter);
  }

  std::map<std::string, TScanResultHic*>::iterator it;
 
  for (it = m_result->GetHicResults().begin(); it != m_result->GetHicResults().end(); ++it) {
    TFifoResultHic *result = (TFifoResultHic*) it->second;
    result->m_nExceptions = 0;
    result->m_err0        = 0;
    result->m_err5        = 0;
    result->m_erra        = 0;
    result->m_errf        = 0;
  }

}


void TFifoAnalysis::WriteResult () {
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    sprintf (fName, "FifoScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(), 
                                                m_config->GetfNameSuffix());
    m_scan  ->WriteConditions (fName);
    
    FILE *fp = fopen (fName, "a");
    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->SetResultFile(fName);
    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->WriteToFile(fp);
    fclose(fp);
    //    m_result->WriteToFile     (fName);
  }
}


void TFifoAnalysis::Run() 
{
  while (m_histoQue->size() == 0) {
    sleep(1);
  }

  while ((m_scan->IsRunning() || (m_histoQue->size() > 0))) {
    if (m_histoQue->size() > 0) {
      while (!(m_mutex->try_lock()));
    
      TScanHisto histo = m_histoQue->front();
      if (m_first) {
        histo.GetChipList(m_chipList);
        InitCounters     ();
        m_first = false;
      }

      m_histoQue->pop_front();
      m_mutex   ->unlock();

      for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
        for (int ireg = 0; ireg < 32; ireg ++) {
          m_counters.at(ichip).err0 += (int) histo (m_chipList.at(ichip), ireg, 0x0);
          m_counters.at(ichip).err5 += (int) histo (m_chipList.at(ichip), ireg, 0x5);
          m_counters.at(ichip).erra += (int) histo (m_chipList.at(ichip), ireg, 0xa);
          m_counters.at(ichip).errf += (int) histo (m_chipList.at(ichip), ireg, 0xf);
        }
      }
    }
    else usleep (300);
  }
}


void TFifoAnalysis::Finalize()
{
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip ++) {
    TFifoResultChip* chipResult = (TFifoResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
    if (!chipResult) std::cout << "WARNING: chipResult = 0" << std::endl;

    chipResult->m_err0 = m_counters.at(ichip).err0;
    chipResult->m_err5 = m_counters.at(ichip).err5;
    chipResult->m_erra = m_counters.at(ichip).erra;
    chipResult->m_errf = m_counters.at(ichip).errf;

    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      if (! (m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TFifoResultHic *hicResult = (TFifoResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
      hicResult->m_err0 += chipResult->m_err0;
      hicResult->m_err5 += chipResult->m_err5;
      hicResult->m_erra += chipResult->m_erra;
      hicResult->m_errf += chipResult->m_errf;
    }
  }

  WriteResult ();
}


void TFifoResultHic::WriteToFile (FILE *fp) 
{
  fprintf (fp, "HIC Result:\n\n");

  fprintf(fp, "Errors in pattern 0x0000: %d\n", m_err0);
  fprintf(fp, "Errors in pattern 0x5555: %d\n", m_err5);
  fprintf(fp, "Errors in pattern 0xaaaa: %d\n", m_erra);
  fprintf(fp, "Errors in pattern 0xffff: %d\n", m_errf);
  
  fprintf(fp, "\nNumber of chips: %d\n\n", (int) m_chipResults.size());

  std::map<int, TScanResultChip*>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
}


void TFifoResultChip::WriteToFile (FILE *fp) 
{
  fprintf(fp, "Errors in pattern 0x0000: %d\n", m_err0);
  fprintf(fp, "Errors in pattern 0x5555: %d\n", m_err5);
  fprintf(fp, "Errors in pattern 0xaaaa: %d\n", m_erra);
  fprintf(fp, "Errors in pattern 0xffff: %d\n", m_errf);
}
