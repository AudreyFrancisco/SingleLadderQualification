#include <iostream>
#include <vector>
#include "TFifoAnalysis.h"

TFifoAnalysis::TFifoAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(histoQue, aScan, aScanConfig, aMutex) 
{
}



void TFifoAnalysis::InitCounters() 
{
  m_counters.clear();

  for (int i = 0; i < m_chipList.size(); i++) {
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
}


void TFifoAnalysis::WriteResult () {
  char fName[100];
  sprintf (fName, "FifoScanResult_%s.dat", m_config->GetfNameSuffix());
  
  FILE         *fp       = fopen (fName, "w");

  fprintf(fp, "NChips\t%d\n\n", m_chipList.size());

  for (int ichip = 0; ichip < m_chipList.size();ichip ++ ) {
    fprintf(fp, "\nBoard %d, Receiver %d, Chip %d\n", m_chipList.at(ichip).boardIndex,
	    m_chipList.at(ichip).dataReceiver, 
            m_chipList.at(ichip).chipId);
 
    fprintf(fp, "Errors in pattern 0x0000: %d\n", m_counters.at(ichip).err0);
    fprintf(fp, "Errors in pattern 0x5555: %d\n", m_counters.at(ichip).err5);
    fprintf(fp, "Errors in pattern 0xaaaa: %d\n", m_counters.at(ichip).erra);
    fprintf(fp, "Errors in pattern 0xffff: %d\n", m_counters.at(ichip).errf);
  }
  
  fclose (fp);  
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

      for (int ichip = 0; ichip < m_chipList.size(); ichip++) {
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
  WriteResult ();
}
