#include <iostream>
#include <unistd.h>

#include "TScanAnalysis.h"

#include "THisto.h"
#include "TScan.h"
#include "TScanConfig.h"

TScanAnalysis::TScanAnalysis(std::deque<TScanHisto> *histoQue, 
                             TScan                  *aScan, 
                             TScanConfig            *aConfig, 
                             std::vector <THic*>     hics, 
                             std::mutex             *aMutex) 
{
  m_histoQue = histoQue;
  m_mutex    = aMutex;
  m_scan     = aScan;
  m_config   = aConfig;
  m_hics     = hics;
  m_first    = true;
  m_chipList.clear ();
}


int TScanAnalysis::ReadChipList() 
{
  TScanHisto histo = m_scan->GetTScanHisto();
  return histo.GetChipList (m_chipList);
}


void TScanAnalysis::CreateChipResults () 
{
  if (m_chipList.size() == 0) {
    std::cout  << "Warning (TScanAnalysis::CreateResult): chip list is empty, doing nothing" << std::endl;
    return;
  }

  for (int i = 0; i < m_chipList.size(); i ++) {
    TScanResultChip    *chipResult = GetChipResult();
    common::TChipIndex idx         = m_chipList.at(i);
    m_result->AddChipResult (idx, chipResult);
  }  
}


int TScanResult::AddChipResult (common::TChipIndex idx,
				TScanResultChip *aChipResult) 
{
  int id = (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf);
  m_chipResults.insert(std::pair<int, TScanResultChip*> (id, aChipResult));
  return m_chipResults.size();
}


int TScanResult::AddChipResult (int aIntIndex, 
				TScanResultChip *aChipResult) 
{
  m_chipResults.insert(std::pair<int, TScanResultChip*> (aIntIndex,aChipResult));
  
  return m_chipResults.size();
}


TScanResultChip *TScanResult::GetChipResult (common::TChipIndex idx) 
{
  for (std::map<int, TScanResultChip*>::iterator it = m_chipResults.begin(); it != m_chipResults.end(); ++it)  {
    int id = (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf);
    if (it->first == id) return it->second;
  }  
  return 0;
}


void TScanResult::WriteToFile(const char *fName) 
{
  FILE *fp = fopen (fName, "a");

  fprintf (fp, "Number of chips: %d\n", m_chipResults.size());

  WriteToFileGlobal (fp);

  for (std::map<int, TScanResultChip*>::iterator it = m_chipResults.begin(); it != m_chipResults.end(); ++it)  {
    int idx = it->first;
    fprintf(fp, "\nBoard %d, Receiver %d, Chip %d:\n", (idx >> 8) & 0xf, (idx >> 4) & 0xf, idx & 0xf);
    it->second->WriteToFile(fp);
  }      

  fclose (fp);
}
