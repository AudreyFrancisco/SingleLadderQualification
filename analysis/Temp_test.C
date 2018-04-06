#include "helpers.h"
#include <Riostream.h>
#include <TAttMarker.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TPad.h>
#include <TString.h>
#include <TStyle.h>
#include <TVectorF.h>
#include <cstddef>
#include <vector>

void Temp_test(const char *fileName)
{
  int   chipnumber = 7;
  int   nmod       = 7;
  int   chipid;
  float temp, voltage;
  FILE *fp = fopen(fileName, "r");

  // creating vecors for data

  std::vector<float> nchip;  // list of chips in the first cooling contour (FCC)
  std::vector<float> nchip2; // list of chips in the second cooling contour (SCC)
  std::vector<float> Tchip;  // temperatures for the FCC
  std::vector<float> Tchip2; // temperatures for the SCC
  std::vector<float> volt;   // voltages for the FCC
  std::vector<float> volt2;  // voltages for the SCC
  std::vector<float> corr;   // T/V for the FCC
  std::vector<float> corr2;  // T/V for the SCC
  std::vector<float> TVar;   // T with step 1 deg for the FCC
  std::vector<float> TVar2;  // T with step 1 deg for SCC
  std::vector<float> VV;     // all V within 1 degree interval for the FCC
  std::vector<float> VV2;    // all V within 1 degree interval for the SCC
  std::vector<float> VarV;   // list of variations of V for the FCC
  std::vector<float> VarV2;  // list of variations of V for the SCC

  gStyle->SetOptStat(0);
  std::string fName = fileName;
  fName.erase(fName.rfind(".dat"));
  if (!fp) {
    std::cout << "Unable to open file " << fName << std::endl;
    return;
  }
  TFile *  out    = new TFile(Form("%s.root", fName.c_str()), "RECREATE");
  TCanvas *t_test = new TCanvas("Temperature vs chip", "Temperature plot", 0, 0, 1500, 1500);
  t_test->Divide(2, 2);
  gStyle->SetOptTitle(kFALSE);


  // reading input file

  while (fscanf(fp, "%d %f %f", &chipid, &temp, &voltage) == 3) {
    std::cout << chipid << "  " << temp << "  " << voltage << std::endl;
    float imod  = (chipid >> 4) & 0x7;
    float ichip = chipid & 0xf;
    if (ichip < 7) {
      nchip.push_back((ichip + 1) + 7 * (imod - 1));
      Tchip.push_back(temp);
      volt.push_back(voltage);
      corr.push_back(temp / voltage);
    }
    else {
      float id = 15 - ichip + 7 * (imod - 1);
      int   it = std::upper_bound(nchip2.cbegin(), nchip2.cend(), id) - nchip2.begin();
      nchip2.insert(nchip2.begin() + it, id);
      Tchip2.insert(Tchip2.begin() + it, temp);
      volt2.insert(volt2.begin() + it, voltage);
      corr2.insert(corr2.begin() + it, temp / voltage);
    }
  }

  // convert vectors to arrays for covinient plotting of TGraphs

  Float_t Nchip[100], Nchip2[100], tchip[100], tchip2[100], ChipVolt[100], ChipVolt2[100],
      Corr[100], Corr2[100];
  for (int i = 0; i < 100; i++) {
    Nchip[i]     = 0;
    Nchip2[i]    = 0;
    tchip[i]     = 0;
    tchip2[i]    = 0;
    ChipVolt[i]  = 0;
    ChipVolt2[i] = 0;
    Corr[i]      = 0;
    Corr2[i]     = 0;
  }
  for (int i = 0; i < nchip.size(); i++) {
    Nchip[i]    = nchip[i];
    tchip[i]    = Tchip[i];
    ChipVolt[i] = volt[i];
    Corr[i]     = corr[i];
  }

  for (int j = 0; j < nchip2.size(); j++) {
    Nchip2[j]    = nchip2[j];
    tchip2[j]    = Tchip2[j];
    ChipVolt2[j] = volt2[j];
    Corr2[j]     = corr2[j];
  }


  // finding average values for modules

  Float_t mod[7], mod2[6], modtemp[7], modtemp2[7], modvolt[7], modvolt2[7];

  for (int imod = 1; imod < 8; imod++) {
    int   nchip1 = chipnumber;
    int   nchip2 = chipnumber;
    float sum_t = 0, sum_t2 = 0, sum_v = 0, sum_v2 = 0;
    for (int ichip = 0; ichip < 7; ichip++) {
      int chip = ichip + (imod - 1) * 7;

      if (tchip[chip] != 0) {
        sum_t += tchip[chip];
        sum_v += ChipVolt[chip];
      }
      else {
        sum_t += 0;
        sum_v += 0;
        nchip1 -= 1;
      }

      if (tchip2[chip] != 0) {
        sum_t2 += tchip2[chip];
        sum_v2 += ChipVolt2[chip];
      }
      else {
        sum_t2 += 0;
        sum_v2 += 0;
        nchip2 -= 1;
      }
    }
    if (imod == 6) {
      mod[imod - 1] = imod + 1;
    }
    else
      mod[imod - 1] = imod;
    mod2[imod - 1]  = imod;

    modtemp[imod - 1]  = sum_t / nchip1;
    modtemp2[imod - 1] = sum_t2 / nchip2;
    modvolt[imod - 1]  = sum_v / nchip1;
    modvolt2[imod - 1] = sum_v2 / nchip2;
  }


  // filling vectors for variations of V


  for (int j = (int)*std::min_element(Tchip.begin(), Tchip.end());
       j < (int)*std::max_element(Tchip.begin(), Tchip.end()) + 1; j++) {
    for (int i = 0; i < Tchip.size(); i++) {
      if ((Tchip[i] > j) && (Tchip[i] < j + 1)) {
        VV.push_back(volt[i]);
      }
    }
    TVar.push_back(j);
    VarV.push_back(*std::max_element(VV.begin(), VV.end()) -
                   *std::min_element(VV.begin(), VV.end()));
    VV.clear();
  }
  for (int j = (int)*std::min_element(Tchip2.begin(), Tchip2.end());
       j < (int)*std::max_element(Tchip2.begin(), Tchip2.end()) + 1; j++) {
    for (int i = 0; i < Tchip2.size(); i++) {
      if ((Tchip2[i] > j) && (Tchip2[i] < j + 1)) {
        VV2.push_back(volt2[i]);
      }
    }
    TVar2.push_back(j);
    VarV2.push_back(*std::max_element(VV2.begin(), VV2.end()) -
                    *std::min_element(VV2.begin(), VV2.end()));
    VV2.clear();
  }

  Float_t tvar[TVar.size()], tvar2[TVar2.size()], varV[VarV.size()], varV2[VarV2.size()];
  for (int i = 0; i < TVar.size(); i++) {
    tvar[i] = TVar[i];
    varV[i] = VarV[i];
  }
  for (int j = 0; j < TVar2.size(); j++) {
    tvar2[j] = TVar2[j];
    varV2[j] = VarV2[j];
  }

  // filling histos for chips

  t_test->cd(1);

  TGraph *gr2 = new TGraph(nchip.size(), Nchip, tchip);
  gr2->SetMarkerStyle(21);
  gr2->SetMarkerColor(2);
  gr2->SetLineColor(2);
  gr2->GetXaxis()->SetTitle("Chip");
  gr2->GetYaxis()->SetTitle("Temperature, C");
  gr2->SetTitle("First temperature contour, left lineage of the stave");
  gr2->Draw("APLMS");
  gr2->Write();

  TGraph *gr1 = new TGraph(nchip2.size(), Nchip2, tchip2);
  gr1->SetMarkerStyle(20);
  gr1->SetMarkerStyle(21);
  gr1->SetLineColor(4);
  gr1->SetMarkerColor(4);
  gr1->SetTitle("Second temperature contour, right lineage of the stave");
  gr1->Draw("PLMS");
  gr1->Write();

  gPad->BuildLegend();

  t_test->cd(2);

  TGraph *gr3 = new TGraph(nchip.size(), Nchip, ChipVolt);
  gr3->SetMarkerStyle(21);
  gr3->SetMarkerColor(2);
  gr3->SetLineColor(2);
  gr3->GetXaxis()->SetTitle("Chip");
  gr3->GetYaxis()->SetTitle("Supply Voltage, V");
  gr3->SetTitle("First temperature contour, left lineage of the stave");
  gr3->Draw("APLMS");
  gr3->Write();

  TGraph *gr4 = new TGraph(nchip2.size(), Nchip2, ChipVolt2);
  gr4->SetMarkerStyle(20);
  gr4->SetMarkerStyle(21);
  gr4->SetLineColor(4);
  gr4->SetMarkerColor(4);
  gr4->SetTitle("Second temperature contour, right lineage of the stave");
  gr4->Draw("SAME PLMS");
  gr4->Write();

  gPad->BuildLegend();

  // filling histos for modules

  t_test->cd(3);

  TGraph *gr5 = new TGraph(nmod - 1, mod, modtemp);
  gr5->SetMarkerStyle(21);
  gr5->SetMarkerColor(2);
  gr5->SetLineColor(2);
  gr5->GetXaxis()->SetTitle("Module");
  gr5->GetYaxis()->SetTitle("Temperature, C");
  gr5->SetTitle("First temperature contour, left lineage of the stave");
  gr5->Draw("APLMS");
  gr5->Write();

  TGraph *gr6 = new TGraph(nmod, mod2, modtemp2);
  gr6->SetMarkerStyle(20);
  gr6->SetLineColor(4);
  gr6->SetMarkerColor(4);
  gr6->SetTitle("Second temperature contour, right lineage of the stave");
  gr6->Draw("SAME PLMS");
  gr6->Write();

  gPad->BuildLegend();

  t_test->cd(4);

  TGraph *gr8 = new TGraph(nmod, mod2, modvolt2);
  gr8->SetMarkerStyle(20);
  gr8->SetLineColor(4);
  gr8->GetXaxis()->SetTitle("Module");
  gr8->GetYaxis()->SetTitle("Supply Voltage, V");
  gr8->SetMarkerColor(4);
  gr8->SetTitle("Second temperature contour, right lineage of the stave");
  gr8->Draw("APLMS");
  gr8->Write();


  TGraph *gr7 = new TGraph(nmod - 1, mod, modvolt);
  gr7->SetMarkerStyle(21);
  gr7->SetMarkerColor(2);
  gr7->SetLineColor(2);
  gr7->SetTitle("First temperature contour, left lineage of the stave");
  gr7->Draw("SAME PLMS");
  gr7->Write();

  gPad->BuildLegend();

  // filling histos for correlations

  TCanvas *cp = new TCanvas("cp", "Correlation plots", 0, 0, 1500, 1500);
  cp->Divide(2, 2);
  cp->cd(1);

  TGraph *gr9 = new TGraph(nchip.size(), ChipVolt, tchip);
  gr9->SetMarkerStyle(21);
  gr9->SetMarkerColor(2);
  gr9->SetTitle("First temperature contour, right lineage of the stave");
  gr9->Draw("APMS");
  gr9->Fit("pol1");
  TF1 *chipfit1 = (TF1 *)gr9->GetFunction("pol1");
  chipfit1->SetLineColor(2);
  chipfit1->Draw("SAME");
  gr9->Write();
  float cf1 = gr9->GetCorrelationFactor();

  TGraph *gr10 = new TGraph(nchip2.size(), ChipVolt2, tchip2);
  gr10->SetMarkerStyle(20);
  gr10->SetMarkerColor(4);
  gr9->GetXaxis()->SetTitle("Voltage");
  gr9->GetYaxis()->SetTitle("Temperature");
  gr10->SetTitle("Second temperature contour, right lineage of the stave");
  gr10->Fit("pol1");
  TF1 *chipfit2 = (TF1 *)gr10->GetFunction("pol1");
  chipfit2->SetLineColor(4);
  gr10->Draw("SAME PMS");
  chipfit2->Draw("SAME");
  gr10->Write();
  float cf2 = gr10->GetCorrelationFactor();
  gPad->BuildLegend();
  std::cout << std::endl
            << "Correlation factor for the first contour = " << cf1 << std::endl
            << "Correlation factor for the second contour = " << cf2 << std::endl;

  cp->cd(2);

  TGraph *gr11 = new TGraph(nchip.size(), Nchip, Corr);
  gr11->SetMarkerStyle(21);
  gr11->SetMarkerColor(2);
  gr11->GetXaxis()->SetTitle("Chip");
  gr11->GetYaxis()->SetTitle("Temperature/Voltage");
  gr11->SetTitle("First temperature contour, right lineage of the stave");
  gr11->Draw("APLMS");
  gr11->Write();

  TGraph *gr12 = new TGraph(nchip2.size(), Nchip2, Corr2);
  gr12->SetMarkerStyle(20);
  gr12->SetMarkerColor(4);
  gr12->SetTitle("Second temperature contour, right lineage of the stave");
  gr12->Draw("SAME PLMS");
  gr12->Write();

  gPad->BuildLegend();

  cp->cd(3);

  TGraph *gr13 = new TGraph(nmod - 1, modvolt, modtemp);
  gr13->SetMarkerStyle(21);
  gr13->SetMarkerColor(2);
  gr13->GetXaxis()->SetTitle("Mean analogue voltage per module, V");
  gr13->GetYaxis()->SetTitle("Mean temperature per module, C");
  gr13->SetTitle("First temperature contour, right lineage of the stave");
  gr13->Draw("APMS");
  gr13->Fit("pol1");
  TF1 *modfit1 = (TF1 *)gr13->GetFunction("pol1");
  modfit1->SetLineColor(2);
  modfit1->Draw("SAME");
  gr13->Write();

  float cf3 = gr13->GetCorrelationFactor();
  gPad->BuildLegend();

  cp->cd(4);

  TGraph *gr14 = new TGraph(nmod, modvolt2, modtemp2);
  gr14->SetMarkerStyle(20);
  gr14->GetXaxis()->SetTitle("Mean analogue voltage per module, V");
  gr14->GetYaxis()->SetTitle("Mean temperature per module, C");
  gr14->SetMarkerColor(4);
  gr14->SetTitle("Second temperature contour, right lineage of the stave");
  gr14->Draw("APMS");
  gr14->Fit("pol1");
  TF1 *modfit2 = (TF1 *)gr14->GetFunction("pol1");
  modfit2->SetLineColor(4);
  modfit2->Draw("SAME");
  gr14->Write();

  float cf4 = gr14->GetCorrelationFactor();
  std::cout << std::endl
            << "Correlation factor for the first contour = " << cf3 << std::endl
            << "Correlation factor for the second contour = " << cf4 << std::endl;

  gPad->BuildLegend();

  // filling histo for variotions

  TCanvas *var =
      new TCanvas("var", "Variations of voltage within the 1 deg interval of T", 0, 0, 1700, 700);
  var->Divide(2, 1);

  var->cd(1);

  TGraph *grv1 = new TGraph(TVar.size(), tvar, varV);
  grv1->SetMarkerStyle(21);
  grv1->SetMarkerColor(2);
  grv1->SetLineColor(2);
  grv1->GetXaxis()->SetTitle("Temperature, C");
  grv1->GetYaxis()->SetTitle("Variations of V, (Vmax - Vmin) per one degree");
  grv1->SetTitle("First temperature contour, right lineage of the stave");
  grv1->Draw("ALPMS");
  grv1->Write();

  gPad->BuildLegend();

  var->cd(2);

  TGraph *grv2 = new TGraph(TVar2.size(), tvar2, varV2);
  grv2->SetMarkerStyle(20);
  grv2->SetMarkerColor(4);
  grv2->GetXaxis()->SetTitle("Temperature, C");
  grv2->GetYaxis()->SetTitle("Variations of V, (Vmax - Vmin) per one degree");
  grv2->SetLineColor(4);
  grv2->SetTitle("Second temperature contour, left lineage of the stave");
  grv2->Draw("ALPMS");
  grv2->Write();

  gPad->BuildLegend();

  TCanvas *cp2 = new TCanvas("cp2", "Correlation plot for chip", 0, 0, 1700, 1700);
  cp2->Divide(1, 2);
  cp2->cd(1);
  gStyle->SetPaintTextFormat("g");

  TH2F *corrplot[7];

  for (int j = 0; j < nmod - 1; j++) {
    corrplot[j] = new TH2F(Form("corrplot_%d", j), Form("Correlations T(V) module %d", j + 1), 200,
                           1.57, 1.78, 200, 15, 46);
    for (int i = 0; i < nchip.size(); i++) {
      int n = j * 7 + i;
      corrplot[j]->Fill(volt[n], Tchip[n], nchip[n]);
    }

    int col;
    if (j == 0) col = 12;
    if (j == 1) col = 40;
    if (j == 2) col = 9;
    if (j == 3) col = 41;
    if (j == 4) col = 8;
    if (j == 5) col = 46;
    if (j == 6) col = 28;
    corrplot[j]->SetMarkerStyle(20);
    corrplot[j]->SetMarkerColor(col);
    corrplot[j]->SetMarkerSize(1.8);
    corrplot[j]->Draw("SAME TEXT0");
  }
  gPad->BuildLegend();

  cp2->cd(2);
  TH2F *corrplot2[7];
  for (int j = 0; j < nmod; j++) {
    corrplot2[j] = new TH2F(Form("corrplot2_%d", j), Form("Correlations T(V) module %d", j + 1),
                            200, 1.57, 1.78, 200, 15, 46);
    for (int i = 0; i < nchip2.size(); i++) {
      int n = j * 7 + i;
      corrplot2[j]->Fill(volt2[n], Tchip2[n], Nchip2[n]);
      std::cout << Nchip2[n] << " nchip2 " << n << std::endl;
    }

    int col;
    if (j == 0) col = 12;
    if (j == 1) col = 40;
    if (j == 2) col = 9;
    if (j == 3) col = 41;
    if (j == 4) col = 8;
    if (j == 5) col = 46;
    if (j == 6) col = 28;
    corrplot2[j]->SetMarkerStyle(21);
    corrplot2[j]->SetMarkerColor(col);
    corrplot2[j]->SetMarkerSize(1.8);
    corrplot2[j]->Draw("SAME TEXT0");
  }
  gPad->BuildLegend();
  fclose(fp);
}
