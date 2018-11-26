#include <iostream>

#include "TH2F.h"
#include "TProfile.h"
#include "TStyle.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TLegend.h"


int ThresholdDistributionGUI(const char *fName, int hicid = 0, int chipid = 0, const char *datetime = "") {
  int col, row;
  float threshold, noise, smthg;
  TH1F *hThresh = new TH1F("hThresh", "hThresh", 125, 0., 500.);
  TH1F *hNoise = new TH1F("hNoise", "hNoise", 60, 0., 30.);
  FILE *fp = fopen(fName, "r");
  if (!fp) {
    std::cout << "Unable to open file " << fName <<std::endl;
    return -1;
  }

  while (fscanf (fp,"%d %d %f %f %f", &col, &row, &threshold, &noise, &smthg) > 0) {
    hThresh->Fill(threshold);
    hNoise->Fill(noise);
  }

  // std::cout << "Pixels without hits: " << nNoHit << std::endl;
  // std::cout << "Pixels with <" << nInj << " hits: " << nIneff << std::endl;
  // std::cout << "Pixels with >" << nInj << " hits: " << nHot << std::endl;
  // std::cout << std::endl;


  TCanvas *cThreshDist = new TCanvas("cThreshDist", "Threshold and Noise distributions", 1000, 1000);
  cThreshDist->Divide(1,2,0,0);

  std::string title = "Threshold distribution, HIC ";
  title += std::to_string( hicid );
  title += ", Chip ";
  title += std::to_string( chipid );
  const char *plotTitle = title.c_str();

  gStyle->SetOptStat(1111);

  cThreshDist->cd(1);
  gPad->SetTickx(1);
  gPad->SetBottomMargin(0.1);
  hThresh->SetTitle(plotTitle);
  hThresh->GetXaxis()->SetTitle("Threshold (#e)");
  hThresh->GetYaxis()->SetTitle("# Pixels");
  // hThresh->SetOptStat(111);
  hThresh->Draw();


  std::string title2 = "Noise distribution, HIC ";
  title2 += std::to_string( hicid );
  title2 += ", Chip ";
  title2 += std::to_string( chipid );
  const char *plotTitle2 = title2.c_str();

  cThreshDist->cd(2);
  gPad->SetTicky(1);
  hNoise->SetTitle(plotTitle2);
  hNoise->GetXaxis()->SetTitle("Noise (#e)");
  hNoise->GetYaxis()->SetTitle("# Pixels");
  // hNoise->SetOptStat(111);
  hNoise->Draw();

  // auto legend = new TLegend(0.901, 0.351, 0.998, 0.652);
  // // legend->SetHeader("");
  // legend->AddEntry(hDeadMap, "Dead pixels", "P");
  // legend->AddEntry(hIneffMap, "Inefficient pixels", "P");
  // legend->AddEntry(hHotMap, "Hot pixels", "P");
  // legend->Draw();

  std::string filename = "Data/";
  filename += std::to_string( hicid );
  filename += "/ThresholdDistribution_HIC";
  filename += std::to_string( hicid );
  filename += "_";
  filename += datetime;
  filename += "_Chip";
  filename += std::to_string( chipid );
  filename += ".pdf";
  const char *finalname = filename.c_str();

  cThreshDist->SaveAs(finalname);

  fclose(fp);
  return 1;
}
