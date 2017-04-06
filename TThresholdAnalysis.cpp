#include <iostream>
#include <vector>
#include "TThresholdAnalysis.h"

TThresholdAnalysis::TThresholdAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(histoQue, aScan, aScanConfig, aMutex) 
{
}


//TODO: Implement HasData
bool TThresholdAnalysis::HasData(TScanHisto &histo, TChipIndex idx, int col) 
{
  return true;
}

//TODO: clean up
void TThresholdAnalysis::Run() 
{
  int chargeSteps = ((float)m_config->GetChargeStop() - (float) m_config->GetChargeStart()) / m_config->GetChargeStep();
  std::vector <TChipIndex> chipList;

  while (m_histoQue->size() == 0) {
    sleep(1);
  }

  while ((m_scan->IsRunning() || (m_histoQue->size() > 0))) {
    if (m_histoQue->size() > 0) {
      while (!(m_mutex->try_lock()));
    
      TScanHisto histo = m_histoQue->front();
      histo.GetChipList(chipList);

      m_histoQue->pop_front();
      m_mutex   ->unlock();

      int row = histo.GetIndex();
      std::cout << "ANALYSIS: Found histo for row " << row << ", size = " << m_histoQue->size() << std::endl;
      for (int ichip = 0; ichip < chipList.size(); ichip++) {
        char fName[100];
        //TODO: Write file name correctly, including time stamp

        sprintf(fName, "TestData_%d_%d_%d.dat", chipList.at(ichip).boardIndex, chipList.at(ichip).dataReceiver, chipList.at(ichip).chipId);
        FILE *fp = fopen (fName, "a");
        for (int icol = 0; icol < 1024; icol ++) {
          if (HasData(histo, chipList.at(ichip), icol)) {
            for (int icharge = 0; icharge < chargeSteps; icharge ++) {
              fprintf(fp, "%d %d %d %d\n", icol, row, icharge, (int)histo(chipList.at(ichip), icol, icharge));
  	    }
	  }
        }
        fclose(fp);
      }
     
    }
    else usleep (300);
  }
}
