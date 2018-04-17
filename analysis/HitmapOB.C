#include <iostream>

#include "TBrowser.h"
#include "TColor.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH2F.h"
#include "TLatex.h"
#include "TLine.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "sys/stat.h"


//#include <string.h>
//#include <stdio.h>

/*Hitmap plotting for outer barrel + FHR analysis + hits/pixel histo for unmasked data*/


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

void ReadFile(const char *fNameChip, TH2F *hHitmap, TH1F *modprox, TH1F *modproy,
              std::vector<TH2F *> &hlist, std::vector<TH1F *> &chipprox,
              std::vector<TH1F *> &chipproy, int Chip, int nInj)
{
  int mod, chip, col, row, nhits;

  FILE *fp = fopen(fNameChip, "r");
  if (!fp) {
    std::cout << "Unable to open file " << fNameChip << std::endl;
    return kFALSE;
  }
  std::cout << fNameChip << std::endl;
  /*
    std::string histoname = fNameChip;
    histoname.erase(0, histoname.rfind("Chip"));
    histoname.erase(histoname.rfind(".dat"));
    std::vector<double> nhits_vect;
  */
  int nLines = 0, nHot = 0, nIneff = 0;

  while (fscanf(fp, "%d %d %d %d %d", &mod, &chip, &col, &row, &nhits) == 5) {
    int Column = AddressToColumn(col / 16, col % 16, row);
    int Row    = AddressToRow(col / 16, col % 16, row);
    nLines++;
    if (nInj > 0) {
      if (nhits < nInj) nIneff++;
      if (nhits > nInj) nHot++;
    }
    hlist[chip]->Fill(Column, Row, nhits);
    chipprox[chip]->Fill(Column);
    chipproy[chip]->Fill(Row);

    //    nhits_vect.push_back(nhits);

    if (Chip < 7) {
      hHitmap->Fill(1024 * Chip + Column, Row, nhits);
      modprox->Fill(1024 * Chip + Column);
      modproy->Fill(Row);
    }
    else {
      hHitmap->Fill(1024 * (14 - Chip) + (1024 - Column), Row + 512, nhits);
      modprox->Fill(1024 * (14 - Chip) + (1024 - Column));
      modproy->Fill(Row + 512);
    }
  }
  /*
      int size =*std::max_element(nhits_vect.begin(), nhits_vect.end());
      std::cout << size << std::endl;
      TH1D *h_histo= new TH1D(Form("%s_hits_histo", histoname.c_str()), "Hits histo",size, -1,
     size);
      for (int i=0; i<nhits_vect.size(); i++) {
        h_histo->Fill(nhits_vect[i]);
      }
  */
  /*
    if (nInj > 0) {
      int nNoHit = 1024 * 512 - nLines; // May be incorrect //524288 - nLines;
      std::cout << std::endl;
      std::cout << std::endl << "Chip " << Chip << ":" << std::endl;
      std::cout << "  Pixels without hits: " << nNoHit << std::endl;
      std::cout << "  Pixels with <" << nInj << " hits: " << nIneff << std::endl;
      std::cout << "  Pixels with >" << nInj << " hits: " << nHot << std::endl;
      std::cout << std::endl;
    }*/
  fclose(fp);
}

int HitmapOB(TString directory, int nInj = -1)
{
  char filepath[100], fNameChip[100], fileName[100], fhrdir[100], fhroutput[100], fhroutput2[100],
      ChipHistName[100];
  Int_t   x_max, y_max, z_max, x_max2, y_max2, z_max2;
  Float_t int_hits  = 0;
  Float_t noise_occ = 0;
  Int_t   n_trg     = 1000000;

  set_plot_style();
  strncpy(filepath, directory, 32);
  /*
    sprintf(fhrdir, "%s/Fhr", filepath);
    mkdir(fhrdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  */
  sprintf(fileName, "%s/Histos.root", filepath);
  TFile *File = new TFile(fileName, "RECREATE");


  TDirectory *h_dir = File->mkdir("histos");
  h_dir->cd();

  TDirectory *moddir[7];
  for (int imod = 1; imod < 8; imod++) {
    moddir[imod] = h_dir->mkdir(Form("histos for module_%d", imod));
    moddir[imod]->cd();
    std::vector<TH2F *> hlist;
    std::vector<TH1F *> chipprox;
    std::vector<TH1F *> chipproy;
    for (int ich = 0; ich < 15; ich++) {
      if (ich != 7) {
        TH2F *ChipHitMap = new TH2F(Form("chip_%d_mod_%d", ich, imod), Form("chip hitmap %d", ich),
                                    1024, -0.5, 1024 - 0.5, 512, -0.5, 512 - 0.5);
        ChipHitMap->SetOption("colz");
        TH1F *pro_plot_x = new TH1F(Form("x_projection_chip_%d_mod%d", ich, imod),
                                    Form("X projection %d", ich), 1026, -1, 1025);
        chipprox.push_back(pro_plot_x);
        TH1F *pro_plot_y = new TH1F(Form("y_projection_chip_%d_mod%d", ich, imod),
                                    Form("Y projection %d", ich), 514, -1, 513);
        chipproy.push_back(pro_plot_y);
        hlist.push_back(ChipHitMap);
      }
      else {
        hlist.push_back(0);
        chipprox.push_back(0);
        chipproy.push_back(0);
      }
    }


    // making histos for fhr analysis
    /*
        TDirectory *fhr_dir = moddir[imod]->mkdir("fhr");
        fhr_dir->cd();
        std::vector<TH1F *> fhrlist;
        for (int ich = 0; ich < 15; ich++) {
          if (ich != 7) {
            TH1F *FHR_plot = new TH1F(Form("chip_%d_mod%d", ich, imod), Form("Noise occupancy %d",
       ich),
                                      600, -0.5, 599.5);
            fhrlist.push_back(FHR_plot);
          }
          else
            fhrlist.push_back(0);
        }
        moddir[imod]->cd();
    */

    auto ModHistoName  = "Module" + std::to_string(imod);
    auto ModHistoNameX = "X_projection_mod_" + std::to_string(imod);
    auto ModHistoNameY = "Y_projection_mod_" + std::to_string(imod);

    TH2F *hHitmap = new TH2F(ModHistoName.c_str(), "Hit map", 7 * 1024, -.5, 7 * 1024 - .5, 512 * 2,
                             -.5, 512 * 2 - .5);
    TH1F *modprox = new TH1F(ModHistoNameX.c_str(), "Projection X", 7 * 1024, -.5, 7 * 1024);
    TH1F *modproy = new TH1F(ModHistoNameY.c_str(), "Projection Y", 512 * 2, -.5, 512 * 2);

    for (int ichip = 0; ichip < 7; ichip++) {
      sprintf(fNameChip, "%s/Source_Chip%d_%d.dat", filepath, imod, ichip);
      ReadFile(fNameChip, hHitmap, modprox, modproy, hlist, chipprox, chipproy, ichip, nInj);
      sprintf(fNameChip, "%s/Source_Chip%d_%d.dat", filepath, imod, ichip + 8); // second row
      ReadFile(fNameChip, hHitmap, modprox, modproy, hlist, chipprox, chipproy, ichip + 8, nInj);
    }


    // filling histos for fhr analysis
    /*
        for (int ich = 0; ich < 15; ich++) {
          if (ich != 7) {
            TH2F *ch_h = (TH2F *)hlist[ich]->Clone();
            int_hits   = ch_h->Integral();

            for (Int_t i_pix = 0; i_pix < 600; i_pix++) {
              noise_occ = int_hits / n_trg / (1024 * 512);
              fhrlist[ich]->SetBinContent(i_pix + 1, noise_occ);
              ch_h->GetMaximumBin(x_max, y_max, z_max);
              int_hits -= ch_h->GetBinContent(x_max, y_max);
              ch_h->SetBinContent(x_max, y_max, 0);
            }

            sprintf(fhroutput, "%s/Fhr_%d_%d", fhrdir, imod, ich);
            ofstream outfile(fhroutput);

            for (Int_t ibin = 0; ibin < 600; ibin++) {
              Double_t bincont = fhrlist[ich]->GetBinContent(ibin + 1, ibin + 1);
              outfile << ibin << "  " << bincont << endl;
            }
            delete ch_h;
          }
        }

        TH2F * mod_h      = (TH2F *)hHitmap->Clone();
        auto   fhrmodname = ModHistoName + "_fhr";
        TH1F * FHR_mod    = new TH1F(fhrmodname.c_str(), "Fhr_module", 600, -0.5, 599.5);
        double int2_hits  = mod_h->Integral();
        for (Int_t i_pix = 0; i_pix < 600; i_pix++) {
          double noise_occ2 = int2_hits / n_trg / (7 * 1024 * 512 * 2);
          FHR_mod->SetBinContent(i_pix + 1, noise_occ2);
          mod_h->GetMaximumBin(x_max2, y_max2, z_max2);
          int2_hits -= mod_h->GetBinContent(x_max2, y_max2);
          mod_h->SetBinContent(x_max2, y_max2, 0);
        }

        sprintf(fhroutput2, "%s/Fhr_mod_%d", fhrdir, imod);
        ofstream outfile(fhroutput2);

        for (Int_t ibin = 0; ibin < 600; ibin++) {
          Double_t bincont2 = FHR_mod->GetBinContent(ibin + 1, ibin + 1);
          outfile << ibin << "  " << bincont2 << endl;
        }
        delete mod_h;
    */
    hHitmap->SetOption("colz");
  }
  h_dir->cd();
  File->Write();
  delete File;
  return 1;
}
