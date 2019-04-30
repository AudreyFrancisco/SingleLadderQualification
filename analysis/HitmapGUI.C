#include <iostream>

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

int HitmapGUI(const char *fName, int nInj = -1, int hicid = 0, int chipid = 0, const char *datetime = "") {
  int event, col, row, nhits;
  set_plot_style();
  // TH2F *hHitmap = new TH2F("hHitmap", "Hit map", 1024, -.5, 1023.5, 512, -.5, 511.5);
  TH2F *hDeadMap = new TH2F("hDeadMap", "hDeadMad", 1024, -.5, 1023.5, 512, -.5, 511.5);
  TH2F *hIneffMap = new TH2F("hIneffMap", "hIneffMap", 1024, -.5, 1023.5, 512, -.5, 511.5);
  TH2F *hHotMap = new TH2F("hHotMap", "hHotMap", 1024, -.5, 1023.5, 512, -.5, 511.5);
  FILE *fp = fopen(fName, "r");
  if (!fp) {
    std::cout << "Unable to open file " << fName <<std::endl;
    return -1;
  }

  int nLines = 0, nHot = 0, nIneff = 0, nDead = 0;
  int icol = 0, irow = 0;

  while (fscanf (fp,"%d %d %d", &col, &row, &nhits) == 3 && irow != 512) {
    nLines ++;
    if (icol != col || irow != row) {
      // printf("col = %d , icol = %d , row = %d , irow = %d\n", col, icol, row, irow);

        while (!(icol == col && irow == row)) {
            //printf("col = %d ,  row = %d , icol = %d ,irow = %d\n", col, row,  icol, irow);
            nDead ++;
            hDeadMap->Fill(icol, irow);
            icol ++;
            if (icol == 1024) {
                icol = 0;
                irow ++;
            }
        }
    }

    if (nInj > 0) {
      if (nhits < nInj) {
        nIneff ++;
        hIneffMap->Fill(col, row);
      }
      if (nhits > nInj) {
        nHot ++;
        hHotMap->Fill(col, row);
      }
    }
    // hHitmap->Fill(col, row, nhits);

    icol ++;
    if (icol == 1024) {
      icol = 0;
      irow ++;
    }
  }

  if (nInj > 0) {
    int nNoHit = 524288 - nLines;
    std::cout << std::endl;
    std::cout << "Pixels without hits: " << nNoHit << std::endl;
    std::cout << "Pixels with <" << nInj << " hits: " << nIneff << std::endl;
    std::cout << "Pixels with >" << nInj << " hits: " << nHot << std::endl;
    std::cout << std::endl;
  }

  TCanvas *cDefectivePixels = new TCanvas("cDefectivePixels", "Defective pixels map", 1600, 1000);

  std::string title = "Digital Scan, HIC ";
  title += std::to_string( hicid );
  title += ", Chip ";
  title += std::to_string( chipid );
  const char *mapTitle = title.c_str();

  // gStyle->SetOptStat("ne");

  hDeadMap->SetTitle(mapTitle);
  hDeadMap->SetMarkerStyle(20);
  hDeadMap->SetMarkerSize(1.5);
  hDeadMap->SetMarkerColor(1);
  hDeadMap->GetXaxis()->SetTitle("Column");
  hDeadMap->GetYaxis()->SetTitle("Row");
  hDeadMap->SetStats(0);
  hDeadMap->Draw("");

  hIneffMap->SetMarkerStyle(21);
  hIneffMap->SetMarkerColor(4);
  hIneffMap->SetMarkerSize(1.5);
  hIneffMap->SetStats(0);
  hIneffMap->Draw("SAME");

  hHotMap->SetMarkerStyle(22);
  hHotMap->SetMarkerColor(2);
  hHotMap->SetMarkerSize(1.5);
  hHotMap->SetStats(0);
  hHotMap->Draw("SAME");

  auto legend = new TLegend(0.901, 0.351, 0.998, 0.652);
  // legend->SetHeader("");
  legend->AddEntry(hDeadMap, "Dead pixels", "P");
  legend->AddEntry(hIneffMap, "Inefficient pixels", "P");
  legend->AddEntry(hHotMap, "Hot pixels", "P");
  legend->Draw();

  std::string filename = "Data/";
  filename += std::to_string( hicid );
  filename += "/DigitalScan_HIC";
  filename += std::to_string( hicid );
  filename += "_";
  filename += datetime;
  filename += "_Chip";
  filename += std::to_string( chipid );
  filename += ".pdf";
  const char *finalname = filename.c_str();

  cDefectivePixels->SaveAs(finalname);

  fclose(fp);
  return 1;
}
