#include <iostream>

#include "TColor.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLatex.h"
#include "TLine.h"
#include "TStyle.h"

/*Plot ThresholdMap for outer barrel*/

void set_plot_style()
{
  const Int_t NRGBs = 5;
  const Int_t NCont = 255;

  Double_t stops[NRGBs] = {0.00, 0.34, 0.61, 0.84, 1.00};
  Double_t red[NRGBs]   = {0.00, 0.00, 0.87, 1.00, 0.51};
  Double_t green[NRGBs] = {0.00, 0.81, 1.00, 0.20, 0.00};
  Double_t blue[NRGBs]  = {0.51, 1.00, 0.12, 0.00, 0.00};
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);
}


int AddressToColumn(int ARegion, int ADoubleCol, int AAddress)
{                                                // probably doesn't need position
  int Column    = ARegion * 32 + ADoubleCol * 2; // Double columns before ADoubleCol
  int LeftRight = ((AAddress % 4) < 2 ? 1 : 0);  // Left or right column within the double column

  Column += LeftRight;

  return Column;
}


int AddressToRow(int ARegion, int ADoubleCol, int AAddress)
{
  // Ok, this will get ugly
  int Row =
      AAddress / 2; // This is OK for the top-right and the bottom-left pixel within a group of 4
  if ((AAddress % 4) == 3) Row -= 1; // adjust the top-left pixel
  if ((AAddress % 4) == 0) Row += 1; // adjust the bottom-right pixel
  return Row;
}


void ReadFile(const char *fName, TH2F *hHitmap, int Chip)
{
  int   ninj, col, row; // position=new=upper or lower row
  FILE *fp = fopen(fName, "r");

  if (!fp) {
    std::cout << "Unable to open file " << fName << std::endl;
    return;
  }

  while (fscanf(fp, "%d %d %d", &col, &row, &ninj) == 3) {
    int Column =
        AddressToColumn(col / 16, col % 16, row); // probably doesn't need position...(below)
    int Row = AddressToRow(col / 16, col % 16, row);
    //        nLines++;

    if (Chip < 7) {
      hHitmap->Fill(1024 * Chip + (1024 - Column), Row, ninj);
    }
    else {
      hHitmap->Fill(1024 * (14 - Chip) + Column, Row + 512, ninj);
    }
  }
  fclose(fp);
}


int DigitalscanOB(const char *fName)
{
  // Meant to run on FitValue files!!

  std::string Prefix = fName;
  Prefix.erase(Prefix.rfind("_Mod"));
  std::cout << "Prefix: " << Prefix << std::endl;
  std::string directory = Prefix;
  directory.erase(directory.rfind("DigitalScan"));
  TFile *fl = new TFile(Form("%sDScanHisto.root", directory.c_str()), "RECREATE");
  std::cout << "open file" << Form("%sDscanHisto.root", directory.c_str()) << std::endl;

  char fNameChip[100];

  for (int imod = 1; imod < 8; imod++) {
    TH2F *hHitmap = new TH2F(Form("hHitmap_%d", imod), Form("DigitalScan map module %d", imod),
                             1024 * 7, -.5, 1024 * 7 - .5, 512 * 2, -.5, 512 * 2 - .5);

    for (int ichip = 0; ichip < 7; ichip++) {
      sprintf(fNameChip, "%s_Mod%d-Chip%d.dat", Prefix.c_str(), imod, ichip); // first (lower) row
      ReadFile(fNameChip, hHitmap, ichip);
      sprintf(fNameChip, "%s_Mod%d-Chip%d.dat", Prefix.c_str(), imod, ichip + 8); // second row
      ReadFile(fNameChip, hHitmap, ichip + 8);
    }
    /*
      gStyle->SetPalette(1);
      gStyle->SetOptStat(kFALSE);
      gStyle->SetOptTitle(kFALSE);

      hHitmap->GetXaxis()->SetTitle("Column");
      hHitmap->GetYaxis()->SetTitle("Row");
      if (noise) hHitmap->SetMaximum(20);*/
    hHitmap->SetOption("colz");
  }
  //  AddLabels();
  fl->Write();
  delete fl;
  return 1;
}
