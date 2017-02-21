#include <iostream>
#include "TScanAnalysis.h"
#include "THisto.h"

TScanAnalysis::TScanAnalysis(std::deque<TScanHisto> *histoQue) 
{
  m_histoQue = histoQue;
}


void TScanAnalysis::Run() 
{

  TChipIndex idx = {0, 3, 0};
  while (m_histoQue->size() > 0) {
    TScanHisto histo = m_histoQue->front();
    m_histoQue->pop_front();

    for (int i = 0; i < 50; i ++) {
      std::cout << histo(idx, 0, i) << " ";
    }
    std::cout << std::endl;
  }

}
