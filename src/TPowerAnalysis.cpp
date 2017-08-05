#include "TPowerTest.h"
#include "TPowerAnalysis.h"

TPowerAnalysis::TPowerAnalysis(std::deque<TScanHisto> *histoQue, 
                               TScan                  *aScan, 
                               TScanConfig            *aScanConfig, 
                               std::vector <THic*>     hics,
                               std::mutex             *aMutex, 
                               TPowerResult           *aResult )
  : TScanAnalysis (histoQue, aScan, aScanConfig, hics, aMutex)
{
}


void TPowerAnalysis::Finalize() 
{
  TPowerTest *powerTest = (TPowerTest*) m_scan;
  
  std::map <std::string, THicCurrents> currents = powerTest->GetCurrents();

  std::map <std::string, THicCurrents>::iterator it, itResult;

  for (it = currents.begin(); it != currents.end(); ++it) { 
    TPowerResultHic *hicResult   = (TPowerResultHic *) m_result->GetHicResult (it->first);
    THicCurrents     hicCurrents = it->second;
    // Copy currents from currents to result, apply cuts, write to file
  }

}
