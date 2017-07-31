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

  for (unsigned int i = 0; i < m_chipList.size(); i ++) {
    TScanResultChip    *chipResult = GetChipResult();
    common::TChipIndex idx         = m_chipList.at(i);
    m_result->AddChipResult (idx, chipResult);
  }  
}


void TScanAnalysis::CreateHicResults ()
{
  if (m_hics.size() == 0) {
    std::cout  << "Warning (TScanAnalysis::CreateResult): hic list is empty, doing nothing" << std::endl;
    return;
  }  
  if (m_chipList.size() == 0) {
    std::cout  << "Warning (TScanAnalysis::CreateResult): chip list is empty, doing nothing" << std::endl;
    return;
  }

  for (unsigned int i = 0; i < m_chipList.size(); i ++) {
    TScanResultChip    *chipResult = GetChipResult();
    common::TChipIndex idx         = m_chipList.at(i);
    m_result->AddChipResult (idx, chipResult);
  }  


  for (unsigned int i = 0; i < m_hics.size(); i ++) {
    TScanResultHic *hicResult = GetHicResult();
    for (unsigned int iChip = 0; iChip < m_chipList.size(); iChip ++) {
      if (m_hics.at(i)->ContainsChip(m_chipList.at(i))) {
        hicResult->AddChipResult(m_chipList.at(i).chipId & 0xf, 
                                 m_result->GetChipResult(m_chipList.at(i)));
      }
    }

    m_result->AddHicResult (m_hics.at(i)->GetDbId(), hicResult);
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


int TScanResult::AddHicResult (std::string hicId, TScanResultHic *aHicResult) 
{
  m_hicResults.insert(std::pair<std::string, TScanResultHic*> (hicId, aHicResult));

  return m_hicResults.size();
}


int TScanResultHic::AddChipResult (int aChipId, TScanResultChip *aChipResult) 
{
  m_chipResults.insert(std::pair<int, TScanResultChip*> (aChipId, aChipResult));
 
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


TScanResultHic *TScanResult::GetHicResult (std::string hic)
{
  std::map<std::string, TScanResultHic*>::iterator it;
  it = m_hicResults.find(hic);
  if (it != m_hicResults.end()) return it->second;
  return 0;
}


void TScanResult::WriteToFile(const char *fName) 
{
  FILE *fp = fopen (fName, "a");

  fprintf (fp, "Number of chips: %d\n", (int)m_chipResults.size());

  WriteToFileGlobal (fp);

  for (std::map<int, TScanResultChip*>::iterator it = m_chipResults.begin(); it != m_chipResults.end(); ++it)  {
    int idx = it->first;
    fprintf(fp, "\nBoard %d, Receiver %d, Chip %d:\n", (idx >> 8) & 0xf, (idx >> 4) & 0xf, idx & 0xf);
    it->second->WriteToFile(fp);
  }      

  fclose (fp);
}
