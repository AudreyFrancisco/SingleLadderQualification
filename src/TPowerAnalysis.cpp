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
