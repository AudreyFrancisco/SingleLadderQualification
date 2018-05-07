#include "TEyeAnalysis.h"
#include "TCanvas.h"
#include "TH2.h"
#include "TLatex.h"
#include <fstream>

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
  FILE *        fp = fopen("EyeDiagram.dat", "w");
  std::ofstream outfile_l("edge_l.dat");
  std::ofstream outfile_r("edge_r.dat");

  // TODO: use proper output path
  // std::string filename_eye = GetHicResult()->GetOutputPath() + "/eye.pdf";
  std::string filename_eye = "eye.pdf";

  TCanvas c;
  c.cd();
  c.Print((filename_eye + "[").c_str());

  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    double step_x = histo->GetStep(m_chipList.at(ichip), 0);
    double step_y = histo->GetStep(m_chipList.at(ichip), 1);

    double min_x = histo->GetMin(m_chipList.at(ichip), 0);
    double min_y = histo->GetMin(m_chipList.at(ichip), 1);

    const int nbin_x = histo->GetNBin(m_chipList.at(ichip), 0);
    const int nbin_y = histo->GetNBin(m_chipList.at(ichip), 1);

    const int nbin_x_half = nbin_x / 2;
    const int nbin_y_half = nbin_y / 2;
    const int yband       = 2;

    TH2F h_eye("h_eye", TString::Format("Eye Diagram chip %i", m_chipList.at(ichip).chipId), nbin_x,
               min_x, min_x + nbin_x * step_x, nbin_y, min_y, min_y + nbin_y * step_y);

    for (int xbin = 0; xbin < nbin_x; xbin++) {
      for (int ybin = 0; ybin < nbin_y; ybin++) {
        double value = ((*histo)(m_chipList.at(ichip), xbin, ybin));

        int x = min_x + xbin * step_x;
        int y = min_y + ybin * step_y;

        if (value != 0) {
          std::cout << "ichip = " << ichip << ", x = " << x << ", y = " << y
                    << ", value = " << value << " bins: " << xbin << ", " << ybin << std::endl;
          h_eye.SetBinContent(xbin + 1, ybin + 1, value);
          fprintf(fp, "%d %d->%d %d->%d %e\n", ichip, xbin, x, ybin, y, value);
        }
      }
    }

    std::cout << "number of bins x/y: " << nbin_x << ", " << nbin_y << std::endl;
    h_eye.SetStats(kFALSE);
    h_eye.Draw("colz");

    double *x_left  = new double[nbin_x_half];
    double *x_right = new double[nbin_x_half];
    for (int xbin = 0; xbin < nbin_x_half; ++xbin) {
      x_left[xbin]  = (xbin != 0) ? x_left[xbin - 1] : 0.;
      x_right[xbin] = (xbin != 0) ? x_right[xbin - 1] : 0.;
      for (int ybin = nbin_y_half - yband; ybin < nbin_y_half + yband; ++ybin) {
        x_left[xbin] += ((*histo)(m_chipList.at(ichip), xbin, ybin));
        x_right[xbin] += ((*histo)(m_chipList.at(ichip), nbin_x - 1 - xbin, ybin));
      }
    }

    int       x_l       = -1;
    int       x_r       = -1;
    const int lastbin_x = std::max(0, nbin_x_half - 1);
    for (int xbin = 0; xbin < nbin_x_half; ++xbin) {
      if ((x_l == -1) && ((x_left[xbin] / x_left[lastbin_x]) > 0.85)) x_l   = xbin;
      if ((x_r == -1) && ((x_right[xbin] / x_right[lastbin_x]) > 0.85)) x_r = xbin;
      printf("%2dL-%3d: %g (%g / %g)\n", ichip, xbin, x_left[xbin] / x_left[31], x_left[xbin],
             x_left[lastbin_x]);
      printf("%2dR-%3d: %g (%g / %g)\n", ichip, xbin, x_right[xbin] / x_right[31], x_right[xbin],
             x_right[lastbin_x]);
      outfile_l << ichip << " " << x_left[xbin] / x_left[lastbin_x] << std::endl;
      outfile_r << ichip << " " << x_right[xbin] / x_right[lastbin_x] << std::endl;
    }
    printf("chip %3d has eye opening left: %3d, right: %3d\n", m_chipList.at(ichip).chipId, x_l,
           x_r);
    TLatex l;
    l.SetTextAlign(21);
    l.SetTextFont(43);
    l.SetTextSize(16);
    l.SetNDC(kTRUE);
    l.DrawLatex(.5, .02, TString::Format("opening: %3d - %3d", x_l, x_r));

    // write result to pdf
    c.Print(filename_eye.c_str());

    delete[] x_left;
    delete[] x_right;
  }
  c.Print((filename_eye + "]").c_str());
  fclose(fp);
  std::cout << "Done" << std::endl;
}
