#include <iostream>
#include <vector>
#include "TDigitalAnalysis.h"

TDigitalAnalysis::TDigitalAnalysis(std::deque<TScanHisto> *histoQue, 
                                   TScan                  *aScan, 
                                   TScanConfig            *aScanConfig, 
                                   std::vector <THic*>     hics,
                                   std::mutex             *aMutex) 
: TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex) 
{
  m_ninj   = m_config->GetParamValue("NINJ");
  m_result = new TDigitalResult(); 
  FillVariableList ();
}

/*
TDigitalResult* TDigitalResult::clone()const{

return new TDigitalResult(*this);
}
*/
/*
//Calling the base class assignment operator in my derived class assignment operator
 TDigitalResult* TDigitalResult::operator=(const TDigitalResult& other){
//handle self assignment 
if (&other!=this) return *this;
//handle base class assignemnt
TScanResult::operator=(other);
return *this;
}
*/

//TODO: Implement HasData
bool TDigitalAnalysis::HasData(TScanHisto &histo,  common::TChipIndex idx, int col) 
{
  return true;
}


void TDigitalAnalysis::FillVariableList () 
{
  m_variableList.insert (std::pair <const char *, TResultVariable> ("Chip Status", status));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# of dead Pixels", deadPix));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# of noisy Pixels", noisyPix));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# of ineff Pixels", ineffPix));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# of bad double columns", badDcol));
}


void TDigitalAnalysis::Initialize() 
{
  ReadChipList      ();
  CreateChipResults ();
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
  m_scan  ->WriteConditions (fName);
  m_result->WriteToFile     (fName);
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
  TErrorCounter         errCount = ((TMaskScan*)m_scan)->GetErrorCount();
  TDigitalResult       *result   = (TDigitalResult*) m_result;
  std::vector<TPixHit>  stuck    = ((TMaskScan*)m_scan)->GetStuckPixels();  

  result->m_nTimeout       = errCount.nTimeout;
  result->m_n8b10b         = errCount.n8b10b;
  result->m_nCorrupt       = errCount.nCorruptEvent;
     
  for (int ichip = 0; ichip < m_chipList.size();ichip ++ ) {
    TDigitalResultChip* chipResult = (TDigitalResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
      
    if (!chipResult) std::cout << "WARNING: chipResult = 0" << std::endl;
    chipResult->m_nDead  = 512 * 1024 - (m_counters.at(ichip).nCorrect + m_counters.at(ichip).nNoisy + m_counters.at(ichip).nIneff);
    chipResult->m_nNoisy = m_counters.at(ichip).nNoisy;
    chipResult->m_nIneff = m_counters.at(ichip).nIneff;
  }
   
  // for the time being divide stuck pixels on different chips here
  // later: change AlpideDecoder?

  for (int istuck = 0; istuck < stuck.size(); istuck++) {
    int entry = common::FindIndexForHit(m_chipList, stuck.at(istuck));
    if (entry >= 0) {
        TDigitalResultChip* chipResult = (TDigitalResultChip*) m_result->GetChipResult(m_chipList.at(entry));
        chipResult->m_stuck.push_back(stuck.at(istuck));
        chipResult->m_nStuck++;
    }
  }
  WriteResult      ();
  WriteStuckPixels ();
}


void TDigitalResult::WriteToFileGlobal (FILE *fp) 
{
  fprintf(fp, "8b10b errors:\t%d\n",    m_n8b10b);
  fprintf(fp, "Corrupt events:\t%d\n",  m_nCorrupt);
  fprintf(fp, "Timeouts:\t%d\n",        m_nTimeout);
}


void TDigitalResultChip::WriteToFile (FILE *fp) 
{
  fprintf(fp, "Dead pixels: %d\n", m_nDead);
  fprintf(fp, "Inefficient pixels: %d\n", m_nIneff);
  fprintf(fp, "Noisy pixels: %d\n", m_nNoisy);
  fprintf(fp, "Bad double cols: %d\n", m_nBadDcols);
  fprintf(fp, "Stuck pixels: %d\n", m_nStuck);
}
