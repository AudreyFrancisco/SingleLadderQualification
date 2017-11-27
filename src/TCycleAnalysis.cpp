#include "TEnduranceCycle.h"
#include "TCycleAnalysis.h"
#include "DBHelpers.h"

#include <string>

TCycleAnalysis::TCycleAnalysis(std::deque<TScanHisto> *histoQue, 
                               TScan                  *aScan, 
                               TScanConfig            *aScanConfig, 
                               std::vector <THic*>     hics,
                               std::mutex             *aMutex, 
                               TCycleResult           *aResult )
  : TScanAnalysis (histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult) m_result = aResult;
  else         m_result = new TCycleResult(); 
}


void TCycleAnalysis::Finalize() 
{
}
