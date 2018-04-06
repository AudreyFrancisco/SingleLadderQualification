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

/*Hitmap plotting for data without hottest pixels + file of hottest pixels + hits/pixel histo for
 * masked data*/


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

void ReadFile(const char *filepath, const char *fNameChip, TH2F *hHitmap,
              std::vector<TH2F *> &hlist, int Chip, int nInj, TDirectory *m_hist)
{
  int  mod, chip, col, row, nhits, hits;
  int  level = 20;
  char NameOfMask[100], fnamemask[100];

  FILE *fp = fopen(fNameChip, "r");
  if (!fp) {
    std::cout << "Unable to open file " << fNameChip << std::endl;
    return kFALSE;
  }
  std::cout << fNameChip << std::endl;

  int nLines = 0, nHot = 0, nIneff = 0;

  std::string histoname = fNameChip;
  histoname.erase(0, histoname.rfind("Chip"));
  histoname.erase(histoname.rfind(".dat"));

  std::vector<int> hitvector;

  std::vector<double> nhits_vect;

  while (fscanf(fp, "%d %d %d %d %d", &mod, &chip, &col, &row, &hits) == 5) {
    hitvector.push_back(hits);
  }
  rewind(fp);
  std::sort(hitvector.begin(), hitvector.end());
  std::cout << hitvector.size() << std::endl;
  if (hitvector.size() > level) {
    int maxlevel = hitvector.size() - level;
    std::cout << "mask value " << hitvector[maxlevel] << std::endl;
    sprintf(fnamemask, "%s/mask", filepath);
    mkdir(fnamemask, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    while (fscanf(fp, "%d %d %d %d %d", &mod, &chip, &col, &row, &nhits) == 5) {
      FILE *fp     = fopen(Form("%s/mask/Max_noise_%d_%d.dat", filepath, mod, chip), "a");
      int   Column = AddressToColumn(col / 16, col % 16, row);
      int   Row    = AddressToRow(col / 16, col % 16, row);
      nLines++;
      if (nInj > 0) {
        if (nhits < nInj) nIneff++;
        if (nhits > nInj) nHot++;
      }
      if (nhits > hitvector[maxlevel]) {
        fprintf(fp, "%d %d %d\n", nhits, Column, Row);
        std::cout << "bonne" << std::endl;
      }
      if (nhits <= hitvector[maxlevel]) {
        std::cout << "pas bonne" << std::endl;
        nhits_vect.push_back(nhits);
        hlist[chip]->Fill(Column, Row, nhits);
        if (Chip < 7) {
          hHitmap->Fill(1024 * Chip + Column, 512 - Row, nhits);
        }
        else {
          hHitmap->Fill(1024 * (14 - Chip) + (1024 - Column), Row + 512, nhits);
        }
      }
      fclose(fp);
    }

    int size = *std::max_element(nhits_vect.begin(), nhits_vect.end()) + 5;
    std::cout << size << std::endl;
    TH1D *h_histo =
        new TH1D(Form("%s_hits_histo", histoname.c_str()), "Hits histo", size, -1, size);
    for (int i = 0; i < nhits_vect.size(); i++) {
      h_histo->Fill(nhits_vect[i]);
    }
  }
  fclose(fp);
}

int HitmapOB_mask(TString directory, int nInj = -1)
{
  char filepath[100], fNameChip[100], fileName[100], fhrdir[100], fhroutput[100], ChipHistName[100];
  Int_t   x_max, y_max, z_max;
  Float_t int_hits  = 0;
  Float_t noise_occ = 0;

  set_plot_style();
  strncpy(filepath, directory, 32);

  sprintf(fileName, "%s/Histos_mask.root", filepath);
  TFile *     File   = new TFile(fileName, "RECREATE");
  TDirectory *m_hist = File->mkdir("masked histos");
  m_hist->cd();
  TDirectory *modhist[7];
  for (int imod = 1; imod < 8; imod++) {
    modhist[imod] = m_hist->mkdir(Form("histos for module %d", imod));
    modhist[imod]->cd();
    std::vector<TH2F *> hlist;
    for (int ich = 0; ich < 15; ich++) {
      if (ich != 7) {
        TH2F *ChipHitMap = new TH2F(Form("chip_%d_mod_%d", ich, imod), Form("chip hitmap %d", ich),
                                    1024, -0.5, 1024 - 0.5, 512, -0.5, 512 - 0.5);
        ChipHitMap->SetOption("colz");
        hlist.push_back(ChipHitMap);
      }
      else
        hlist.push_back(0);
    }

    auto  ModHistoName = "Module" + std::to_string(imod);
    TH2F *hHitmap = new TH2F(ModHistoName.c_str(), "Hit map", 7 * 1024, -.5, 7 * 1024 - .5, 512 * 2,
                             -.5, 512 * 2 - .5);

    for (int ichip = 0; ichip < 7; ichip++) {
      sprintf(fNameChip, "%s/Source_Chip%d_%d.dat", filepath, imod, ichip);
      ReadFile(filepath, fNameChip, hHitmap, hlist, ichip, nInj, m_hist);
      sprintf(fNameChip, "%s/Source_Chip%d_%d.dat", filepath, imod, ichip + 8); // second row
      ReadFile(filepath, fNameChip, hHitmap, hlist, ichip + 8, nInj, m_hist);
    }
    hHitmap->SetOption("colz");
  }
  m_hist->cd();
  File->Write();
  delete File;
  return 1;
}
