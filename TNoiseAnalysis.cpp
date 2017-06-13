#include <iostream>
#include <vector>
#include "TNoiseAnalysis.h"

TNoiseAnalysis::TNoiseAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(histoQue, aScan, aScanConfig, aMutex) 
{
  m_nTrig    = m_config->GetParamValue("NTRIG");
  m_noiseCut = m_nTrig / m_config->GetParamValue("NOISECUT_INV");

  m_result = new TNoiseResult();
}


void TNoiseAnalysis::WriteResult() 
{
}


void TNoiseAnalysis::Initialize()
{
  ReadChipList      ();
  CreateChipResults ();
}


void TNoiseAnalysis::Run() 
{
  while (m_histoQue->size() == 0) {
    sleep(1);
  }

  while ((m_scan->IsRunning() || (m_histoQue->size() > 0))) {
    if (m_histoQue->size() > 0) {
      while (!(m_mutex->try_lock()));
    
      TScanHisto histo = m_histoQue->front();
      histo.GetChipList (m_chipList);

      m_histoQue->pop_front();
      m_mutex   ->unlock   ();

      for (int ichip = 0; ichip < m_chipList.size(); ichip ++) {
        TNoiseResultChip *chipResult = (TNoiseResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
        int               channel    = m_chipList.at(ichip).dataReceiver;
        int               chipId     = m_chipList.at(ichip).chipId;
        float             occ        = 0;
        if (!chipResult) {
	  std::cout << "Warning (TNoiseAnalysis): Missing chip result" << std::endl;
          continue;
	}
        for (int icol = 0; icol < 1024; icol ++) {
          for (int irow = 0; irow < 512; irow ++) {
            // if entry > noise cut: add pixel to chipResult->AddNoisyPixel
            if (histo(m_chipList.at(ichip), icol, irow) > m_noiseCut) {
	      TPixHit pixel = {channel, chipId, 0, icol, irow};
              chipResult->AddNoisyPixel(pixel);
	    }
            occ += histo(m_chipList.at(ichip), icol, irow);
	  }
	}
        // divide chipResult->m_occ by m_nTrig * 512 * 1024 and write to chipResult
        occ /= (m_nTrig * 512 * 1024);
        chipResult->SetOccupancy(occ);
      }
    }
   
    else usleep(300);
  }
}
