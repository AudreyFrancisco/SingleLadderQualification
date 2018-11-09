#include <iostream>

#include "TH2F.h"
#include "TProfile.h"
#include "TStyle.h"
#include "TColor.h"

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

int HitmapGUI(const char *fName, int nInj = -1) {
  int event, col, row, nhits;
  set_plot_style();
  TH2F *hHitmap = new TH2F("hHitmap", "Hit map", 1024, -.5, 1023.5, 512, -.5, 511.5);
  FILE *fp = fopen(fName, "r");
  if (!fp) {
    std::cout << "Unable to open file " << fName <<std::endl;
    return -1;
  }
  TProfile *hCs = new TProfile("hCs", "Cluster Size", 512, -.5, 511.5);

  int ClusterSize = -1;
  int nLines = 0, nHot = 0, nIneff = 0;

  while (fscanf (fp,"%d %d %d", &col, &row, &nhits) == 3) {

    nLines ++;
    if (nInj > 0) {
      if (nhits < nInj) nIneff ++;
      if (nhits > nInj) nHot   ++;
    }
    // hHitmap->Fill(Column, Row, nhits);
    hHitmap->Fill(col, row, nhits);
  }

  if (nInj > 0) {
    int nNoHit = 524288 - nLines;
    std::cout << std::endl;
    std::cout << "Pixels without hits: " << nNoHit << std::endl;
    std::cout << "Pixels with <" << nInj << " hits: " << nIneff << std::endl;
    std::cout << "Pixels with >" << nInj << " hits: " << nHot << std::endl;
    std::cout << std::endl;
  }


  gStyle->SetPalette(1);
  hHitmap->GetXaxis()->SetTitle("Column");
  hHitmap->GetYaxis()->SetTitle("Row");
  hHitmap->Draw("COLZ");
  //hCs->Draw();
  fclose(fp);
  return 1;
}
