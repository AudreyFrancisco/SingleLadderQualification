#include <iostream>
#include <vector>
#include "TDigitalAnalysis.h"

TDigitalAnalysis::TDigitalAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(histoQue, aScan, aScanConfig, aMutex) 
{
  m_ninj = m_config->GetParamValue("NINJ");
}


//TODO: Implement HasData
bool TDigitalAnalysis::HasData(TScanHisto &histo,  common::TChipIndex idx, int col) 
{
  return true;
}


void TDigitalAnalysis::InitCounters () 
{
  m_counters.clear();
  for (int i = 0; i < m_chipList.size(); i++) {
    TDigitalCounter counter;
    counter.boardIndex = m_chipList.at(i).boardIndex;
    counter.receiver   = m_chipList.at(i).dataReceiver;
    counter.chipId     = m_chipList.at(i).chipId;
    counter.nCorrect   = 0;
    counter.nIneff     = 0;
    counter.nNoisy     = 0;
    m_counters.push_back(counter);
  }
}


void TDigitalAnalysis::WriteHitData(TScanHisto histo, int row) 
{
  char fName[100];
  for (int ichip = 0; ichip < m_chipList.size(); ichip++) {
    sprintf(fName, "Digital_%s_B%d_Rcv%d_Ch%d.dat", m_config->GetfNameSuffix(), 
	                                            m_chipList.at(ichip).boardIndex, 
                                                    m_chipList.at(ichip).dataReceiver, 
                                                    m_chipList.at(ichip).chipId);
    FILE *fp = fopen (fName, "a");
    for (int icol = 0; icol < 1024; icol ++) {
      if (histo(m_chipList.at(ichip), icol) > 0) {  // write only non-zero values
        fprintf(fp, "%d %d %d\n", icol, row, (int) histo(m_chipList.at(ichip), icol));
      }
    }
    fclose(fp);
  }
}


void TDigitalAnalysis::WriteResult() 
{
  char fName[100];
  sprintf (fName, "DigitalScanResult_%s.dat", m_config->GetfNameSuffix());
  
  FILE         *fp       = fopen (fName, "w");
  TErrorCounter errCount = ((TMaskScan*)m_scan)->GetErrorCount();

  fprintf(fp, "NChips\t%d\n\n", m_chipList.size());

  
  fprintf(fp, "8b10b errors:\t%d\n",    errCount.n8b10b);
  fprintf(fp, "Corrupt events:\t%d\n",  errCount.nCorruptEvent);
  fprintf(fp, "Timeouts:\t%d\n",        errCount.nTimeout);
  fprintf(fp, "Priority errors:\t%d\n", errCount.nPrioEncoder);

  for (int ichip = 0; ichip < m_chipList.size();ichip ++ ) {
    fprintf(fp, "\nBoard %d, Receiver %d, Chip %d\n", m_chipList.at(ichip).boardIndex,
	    m_chipList.at(ichip).dataReceiver, 
            m_chipList.at(ichip).chipId);
    int dead = 512 * 1024 - (m_counters.at(ichip).nCorrect + m_counters.at(ichip).nNoisy + m_counters.at(ichip).nIneff);
    fprintf(fp, "Dead pixels: %d\n", dead);
    fprintf(fp, "Pixels with < %d hits: %d\n", m_ninj, m_counters.at(ichip).nIneff);
    fprintf(fp, "Pixels with > %d hits: %d\n", m_ninj, m_counters.at(ichip).nNoisy);
  }
  
  fclose (fp);  
}


void TDigitalAnalysis::WriteStuckPixels() 
{
  char fName[100];
  sprintf (fName, "StuckPixels_%s.dat", m_config->GetfNameSuffix());
  
  FILE                 *fp     = fopen (fName, "w");
  std::vector<TPixHit>  pixels = ((TMaskScan*)m_scan)->GetStuckPixels();

  for (int i = 0; i < pixels.size(); i++) {
    fprintf (fp, "%d %d %d %d %d\n", pixels.at(i).channel, pixels.at(i).chipId, pixels.at(i).region, pixels.at(i).dcol,pixels.at(i).address);
  }
  fclose(fp);
}


void TDigitalAnalysis::Run() 
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

      int row = histo.GetIndex();
      std::cout << "ANALYSIS: Found histo for row " << row << ", size = " << m_histoQue->size() << std::endl;
      WriteHitData(histo, row);
      for (int ichip = 0; ichip < m_chipList.size(); ichip++) {
        for (int icol = 0; icol < 1024; icol ++) {
          int hits = (int) histo (m_chipList.at(ichip), icol);
          if      (hits == m_ninj) m_counters.at(ichip).nCorrect ++;         
          else if (hits >  m_ninj) m_counters.at(ichip).nNoisy ++;
          else if (hits >  0)      m_counters.at(ichip).nIneff ++;
        }
      }
    }
    else usleep (300);
  }
}


void TDigitalAnalysis::Finalize() {
  WriteResult      ();
  WriteStuckPixels ();
}
