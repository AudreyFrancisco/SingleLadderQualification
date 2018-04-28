#include "TEyeAnalysis.h"

TEyeAnalysis::TEyeAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                           std::vector<THic *> hics, std::mutex *aMutex, TEyeResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    m_result = new TEyeResult();
  FillVariableList();
}

void TEyeAnalysis::Initialize()
{
  ReadChipList();
  CreateHicResults();
}


void TEyeAnalysis::AnalyseHisto(TScanHisto *histo)
{
  std::cout << "in analyse histo, chipList.size = " << m_chipList.size() << std::endl;
  FILE *fp = fopen("EyeDiagram.dat", "w");
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    // TODO: ad hoc loop, fix later
    for (int xbin = 0; xbin < 255; xbin++) {
      for (int ybin = 0; ybin < 255; ybin++) {
        double value = ((*histo)(m_chipList.at(ichip), xbin, ybin));
        int    x     = -124 + xbin;
        int    y     = -124 + ybin;
        fprintf(fp, "%d %d %d %e\n", ichip, x, y, value);
        if (value != 0)
          std::cout << "ichip = " << ichip << ", x = " << x << ", y = " << y
                    << ", value = " << value << std::endl;
      }
    }
  }
  fclose(fp);
  std::cout << "Done" << std::endl;
}
