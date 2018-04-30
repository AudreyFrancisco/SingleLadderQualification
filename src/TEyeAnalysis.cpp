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
    double step_x = histo->GetStep(m_chipList.at(ichip), 0);
    double step_y = histo->GetStep(m_chipList.at(ichip), 1);

    double min_x = histo->GetMin(m_chipList.at(ichip), 0);
    double min_y = histo->GetMin(m_chipList.at(ichip), 1);

    for (int xbin = 0; xbin < 255; xbin++) {
      for (int ybin = 0; ybin < 64; ybin++) {
        double value = ((*histo)(m_chipList.at(ichip), xbin, ybin));

        int x = min_x + xbin * step_x;
        int y = min_y + ybin * step_y;
        if (value != 0) {
          std::cout << "ichip = " << ichip << ", x = " << x << ", y = " << y
                    << ", value = " << value << std::endl;
          fprintf(fp, "%d %d %d %e\n", ichip, x, y, value);
        }
      }
    }
  }
  fclose(fp);
  std::cout << "Done" << std::endl;
}
