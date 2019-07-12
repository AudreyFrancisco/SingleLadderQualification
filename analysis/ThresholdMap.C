#include <iostream>
#include <stdio.h>

#include "TH2F.h"
#include "TProfile.h"
#include "TStyle.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TLegend.h"

void set_plot_style()
{
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;

    Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);
}


int AddressToColumn      (int ARegion, int ADoubleCol, int AAddress)
{
    int Column    = ARegion * 32 + ADoubleCol * 2;    // Double columns before ADoubleCol
    int LeftRight = ((((AAddress % 4) == 1) || ((AAddress % 4) == 2))? 1:0);       // Left or right column within the double column

    Column += LeftRight;

    return Column;
}


int AddressToRow         (int ARegion, int ADoubleCol, int AAddress)
{
    int Row = AAddress / 2;
    return Row;
}


void ThresholdMap (const char *fName, int hicid = 0, int chipid = 0, const char *datetime = "", float backbias = 0.0, bool tuned=kFALSE) {
  int scale = 2;
  FILE *fp = fopen (fName, "r");
  set_plot_style();
  TH2F *hThresh = new TH2F ("hThresh", "Threshold Map", 1024/scale, -0.5, 1023.5, 512/scale, -0.5, 511.5);
  // TProfile *hNoiseProf = new TProfile("hNoiseProf", "Noise Profile", 1024, -.5, 1023.5);
  TH2F *hNoise = new TH2F ("hNoise", "Noise Map", 1024/scale, -0.5, 1023.5, 512/scale, -0.5, 511.5);

  int col, row;
  float thresh, noise, chisq;

  while (fscanf(fp, "%d %d %f %f %f", &col, &row, &thresh, &noise, &chisq) == 5) {
    int pixel = col * 1024 + row;
    // if (!(pixel %10000)) cout << "processing pixel " << pixel << endl;

 //   if (thresh > 0) {
      // int Column = AddressToColumn(col / 16, col % 16, row);
      // int Row    = AddressToRow   (col / 16, col % 16, row);
      hThresh->Fill(col, row, thresh/scale/scale);
      hNoise->Fill(col, row, noise/scale/scale);

      // hThresh1->Fill(thresh);
      // Fill Histogram;
   // }
  }
  TCanvas *cThreshDist = new TCanvas("cThreshDist", "Threshold Map", 1000, 1000);
  cThreshDist->Divide(1,2);

  cThreshDist->cd(1);

  std::string title = "Threshold Map, HIC ";
  title += std::to_string( hicid );
  title += ", Chip ";
  title += std::to_string( chipid );
  const char *plotTitle = title.c_str();

  gStyle->SetOptStat (kFALSE);
  gStyle->SetOptTitle(kTRUE);
  hThresh->SetTitle(plotTitle);
  hThresh->GetXaxis()->SetTitle("Column");
  hThresh->GetYaxis()->SetTitle("Row");
  hThresh->GetZaxis()->SetTitle("Threshold [e]");
  hThresh->GetZaxis()->SetTitleOffset(1.1);
  hThresh->SetMaximum(450);
  hThresh->SetMinimum(0);

  hThresh->Draw("COLZ");

  cThreshDist->cd(2);

  std::string  title2 = "Noise Map, HIC ";
  title2 += std::to_string( hicid );
  title2 += ", Chip ";
  title2 += std::to_string( chipid );
  const char *plotTitle2 = title2.c_str();

  hNoise->SetTitle(plotTitle2);
  hNoise->GetXaxis()->SetTitle("Column");
  hNoise->GetYaxis()->SetTitle("Row");
  hNoise->GetZaxis()->SetTitle("Noise [e]");
  hNoise->GetZaxis()->SetTitleOffset(1.1);
  // hNoise->SetMaximum(450);
  // hNoise->SetMinimum(0);
  hNoise->Draw("COLZ");

  int bbias = backbias;

  std::string filename = "Data/";
  filename += std::to_string( hicid );
  if(tuned) filename += "/ThresholdMap_Tuned_HIC";
  else filename += "/ThresholdMap_HIC";
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

  cThreshDist->SaveAs(finalname);


  //hNoiseProf->Draw();
}
