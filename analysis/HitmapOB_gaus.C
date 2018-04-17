#include <iostream>

#include "TBrowser.h"
#include "TColor.h"
#include "TFile.h"
#include "TH2F.h"
#include "TLatex.h"
#include "TLine.h"
#include "TStyle.h"
#include "TTree.h"
#include "sys/stat.h"
#include <algorithm>
#include <vector>

//#include <string.h>
//#include <stdio.h>

/*Hitmap plotting for outer barrel*/


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
{
  int Column    = ARegion * 32 + ADoubleCol * 2; // Double columns before ADoubleCol
  int LeftRight = ((AAddress % 4) < 2 ? 1 : 0);  // Left or right column within the double column

  Column += LeftRight;

  return Column;
}


int AddressToRow(int ARegion, int ADoubleCol, int AAddress)
{
  int Row =
      AAddress / 2; // This is OK for the top-right and the bottom-left pixel within a group of 4
  if ((AAddress % 4) == 3) Row -= 1; // adjust the top-left pixel
  if ((AAddress % 4) == 0) Row += 1; // adjust the bottom-right pixel
  return Row;
}

void ReadFile(const char *filepath, const char *fNameChip, std::vector<double> &hit_mod, int Chip,
              int nInj)
{
  int  mod, chip, col, row, nhits, hits;
  char NameOfMask[100];

  FILE *fp = fopen(fNameChip, "r");
  if (!fp) {
    std::cout << "Unable to open file " << fNameChip << std::endl;
    return kFALSE;
  }
  std::cout << fNameChip << std::endl;

  std::string histoname = fNameChip;
  histoname.erase(0, histoname.rfind("Chip"));
  histoname.erase(histoname.rfind(".dat"));

  std::vector<int> hitvector;

  while (fscanf(fp, "%d %d %d %d %d", &mod, &chip, &col, &row, &hits) == 5) {
    hitvector.push_back(hits);
    hit_mod.push_back(hits);
  }
  int   size    = *std::max_element(hitvector.begin(), hitvector.end()) + 5;
  TH1D *h_histo = new TH1D(Form("%s_hits_histo", histoname.c_str()), "Hits histo", size, -1, size);
  for (int i = 0; i < hitvector.size(); i++) {
    h_histo->Fill(hitvector[i]);
  }
  h_histo->Fit("gaus");
  TF1 *  fit   = (TF1 *)h_histo->GetFunction("gaus");
  double max   = fit->GetParameter(1);
  double sigma = fit->GetParameter(2);
  sigma        = sigma * 3 / 2;
  double left  = (int)(max - sigma);
  double right = (int)(max + sigma);

  rewind(fp);
  while (fscanf(fp, "%d %d %d %d %d", &mod, &chip, &col, &row, &nhits) == 5) {

    if ((double)nhits > right) {
      FILE *fp = fopen(Form("%s/susp/Noise_out_of_gaus_right_%d_%d.dat", filepath, mod, chip), "a");
      fprintf(fp, "%d %d %d %d %d\n", mod, chip, col, row, nhits);

      fclose(fp);
    }
  }

  rewind(fp);

  while (fscanf(fp, "%d %d %d %d %d", &mod, &chip, &col, &row, &nhits) == 5) {
    if ((double)nhits < left) {
      FILE *fp = fopen(Form("%s/susp/Noise_out_of_gaus_left_%d_%d.dat", filepath, mod, chip), "a");
      fprintf(fp, "%d %d %d %d %d\n", mod, chip, col, row, nhits);
      fclose(fp);
    }
  }

  fclose(fp);
}

int HitmapOB_gaus(TString directory, int nInj = -1)
{
  char    filepath[100], fNameChip[100], fileName[100], suspdir[100];
  Int_t   x_max, y_max, z_max;
  Float_t int_hits  = 0;
  Float_t noise_occ = 0;


  set_plot_style();
  strncpy(filepath, directory, 32);

  sprintf(suspdir, "%s/susp", filepath);
  mkdir(suspdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  sprintf(fileName, "%s/Histos_gaus.root", filepath);
  TFile *File = new TFile(fileName, "RECREATE");

  TDirectory *gausfits = File->mkdir("gausfits");
  gausfits->cd();
  TDirectory *moddir[7];
  for (int imod = 1; imod < 8; imod++) {
    moddir[imod] = gausfits->mkdir(Form("histos for module_%d", imod));
    moddir[imod]->cd();
    std::vector<double> hit_mod;
    for (int ichip = 0; ichip < 7; ichip++) {
      sprintf(fNameChip, "%s/Source_Chip%d_%d.dat", filepath, imod, ichip);
      ReadFile(filepath, fNameChip, hit_mod, ichip, nInj);
      sprintf(fNameChip, "%s/Source_Chip%d_%d.dat", filepath, imod, ichip + 8); // second row
      ReadFile(filepath, fNameChip, hit_mod, ichip + 8, nInj);
    }
    int   size    = *std::max_element(hit_mod.begin(), hit_mod.end()) + 5;
    TH1D *h_histo = new TH1D(Form("module_%d_hits_histo", imod), "Hits histo", size, -1, size);
    for (int i = 0; i < hit_mod.size(); i++) {
      h_histo->Fill(hit_mod[i]);
    }
    h_histo->Fit("gaus");
    TF1 *  fit   = (TF1 *)h_histo->GetFunction("gaus");
    double max   = fit->GetParameter(1);
    double sigma = fit->GetParameter(2);
    sigma        = sigma * 3 / 2;
    double left  = (int)(max - sigma);
    double right = (int)(max + sigma);
  }
  gausfits->cd();
  File->Write();
  delete File;
  return 1;
}
