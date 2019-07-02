#include <iostream>
#include <stdio.h>

#include "TH2F.h"
#include "TProfile.h"
#include "TStyle.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TLegend.h"


int ITHRDistributionGUI(const char *fName, int hicid = 0, int chipid = 0, const char *datetime = "", float backbias = 0.0) {
  int col, row;
  float ithr, noise,smthg;
  TH1F *hITHR = new TH1F("hITHR", "hITHR", 125, 0., 150.);
  FILE *fp = fopen(fName, "r");
  if (!fp) {
    std::cout << "Unable to open file " << fName <<std::endl;
    return -1;
  }

  while (fscanf (fp,"%d %d %f %f %f", &col, &row, &ithr, &noise, &smthg) > 0) {
   //std::cout<<"VCASN "<<vcasn<<std::endl;
    hITHR->Fill(ithr);

  }

  // std::cout << "Pixels without hits: " << nNoHit << std::endl;
  // std::cout << "Pixels with <" << nInj << " hits: " << nIneff << std::endl;
  // std::cout << "Pixels with >" << nInj << " hits: " << nHot << std::endl;
  // std::cout << std::endl;


  TCanvas *cITHRDist = new TCanvas("cITHRDist", "ITHR distribution", 1000, 1000);


  std::string title = "ITHR distribution, HIC ";
  title += std::to_string( hicid );
  title += ", Chip ";
  title += std::to_string( chipid );
  const char *plotTitle = title.c_str();

  gStyle->SetOptStat(1111);


  gPad->SetTickx(1);
  gPad->SetBottomMargin(0.1);
  hITHR->SetTitle(plotTitle);
  hITHR->GetXaxis()->SetTitle("ITHR (DAC)");
  hITHR->GetYaxis()->SetTitle("# Pixels");
  // hThresh->SetOptStat(111);
  hITHR->Draw();



    int bbias = backbias;
  std::string filename = "Data/";
  filename += std::to_string( hicid );
  filename += "/ITHRDistribution_HIC";
  filename += std::to_string( hicid );
  filename += "_";
  filename += datetime;
  filename += "_Chip";
  filename += std::to_string( chipid );
  filename += "_BB";
  filename += std::to_string( bbias );
  filename += "V";
  filename += ".pdf";
  const char *finalname = filename.c_str();

  cITHRDist->SaveAs(finalname);

  fclose(fp);
  return 1;
}
