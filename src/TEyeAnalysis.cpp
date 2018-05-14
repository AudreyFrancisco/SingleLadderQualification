#include "TEyeAnalysis.h"
#include "TCanvas.h"
#include "TEyeMeasurement.h"
#include "TFile.h"
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
  FILE *fp = fopen("EyeDiagram.dat", "w");

  std::string filename_eye = FindHicResultForChip(m_chipList.at(0))->GetOutputPath() + "/eye.pdf";
  std::string filename_eye_root =
      FindHicResultForChip(m_chipList.at(0))->GetOutputPath() + "/eye.root";

  TCanvas c;
  c.cd();
  c.Print((filename_eye + "[").c_str());

  TFile *rootfile_eye = TFile::Open(filename_eye_root.c_str(), "RECREATE");

  for (auto chip : m_chipList) {
    const double step_x = histo->GetStep(chip, 0);
    const double step_y = histo->GetStep(chip, 1);

    const double min_x = histo->GetMin(chip, 0);
    const double min_y = histo->GetMin(chip, 1);

    const int nbin_x = histo->GetNBin(chip, 0);
    const int nbin_y = histo->GetNBin(chip, 1);

    const int nbin_x_half = nbin_x / 2;
    const int nbin_y_half = nbin_y / 2;
    const int yband       = 2;

    TEyeResultHic *hicResult = (TEyeResultHic*) FindHicResultForChip(chip);
    Int_t driverStrength = ((TEyeParameters*) hicResult->GetScanParameters())->driverStrength;
    Int_t preemphasis    = ((TEyeParameters*) hicResult->GetScanParameters())->driverStrength;

    const std::string hname  = TString::Format("h_eye_%i_d%i_p%i", chip.chipId, driverStrength, preemphasis).Data();
    const std::string htitle = TString::Format("Eye Diagram chip %i (%s - D: %i, P: %i)", chip.chipId,
                                               hicResult->GetName().c_str(), driverStrength, preemphasis)
                                   .Data();

    TH2F h_eye(hname.c_str(), htitle.c_str(), nbin_x, min_x, min_x + nbin_x * step_x, nbin_y, min_y,
               min_y + nbin_y * step_y);
    h_eye.SetDirectory(0);

    // fill histogram
    for (int xbin = 0; xbin < nbin_x; xbin++) {
      for (int ybin = 0; ybin < nbin_y; ybin++) {
        int    x     = min_x + xbin * step_x;
        int    y     = min_y + ybin * step_y;
        double value = (*histo)(chip, xbin, ybin);

        if (value != 0) {
          fprintf(fp, "%d %d->%d %d->%d %e\n", chip.chipId, xbin, x, ybin, y, value);
          h_eye.SetBinContent(xbin + 1, ybin + 1, value);
        }
      }
    }

    // calculate cumulative function along x (within [-yband, yband])
    std::vector<double> x_l, x_r;
    for (int xbin = 0; xbin < nbin_x_half; ++xbin) {
      x_l.push_back((xbin != 0) ? x_l[xbin - 1] : 0.);
      x_r.push_back((xbin != 0) ? x_r[xbin - 1] : 0.);
      for (int ybin = nbin_y_half - yband; ybin < nbin_y_half + yband; ++ybin) {
        x_l.back() += (*histo)(chip, xbin, ybin);
        x_r.back() += (*histo)(chip, nbin_x - 1 - xbin, ybin);
      }
    }

    // calculate opening as 85 % per-centile
    int open_l = -1;
    int open_r = -1;
    for (int xbin = 0; xbin < nbin_x_half; ++xbin) {
      if ((open_l == -1) && ((x_l[xbin] / x_l.back()) > 0.85)) open_l = xbin;
      if ((open_r == -1) && ((x_r[xbin] / x_r.back()) > 0.85)) open_r = xbin;
    }

    // draw histogram
    h_eye.SetStats(kFALSE);
    h_eye.Draw("colz");
    TLatex l;
    l.SetTextAlign(21);
    l.SetTextFont(43);
    l.SetTextSize(16);
    l.SetNDC(kTRUE);
    l.DrawLatex(.5, .02, TString::Format("opening: %3d - %3d", open_l, open_r));

    // write result to files
    std::string dirname   = FindHicResultForChip(chip)->GetName();
    TDirectory *rootdir   = rootfile_eye->GetDirectory(dirname.c_str());
    if (!rootdir) rootdir = rootfile_eye->mkdir(dirname.c_str());
    if (rootdir) rootdir->WriteTObject(&h_eye);
    c.SetLogz(kFALSE);
    c.Print(filename_eye.c_str());
    c.SetLogz(kTRUE);
    h_eye.SetMinimum(1.e-7);
    c.Print(filename_eye.c_str());
  }
  c.Print((filename_eye + "]").c_str());
  fclose(fp);
  rootfile_eye->Close();
  std::cout << "Done" << std::endl;
}
