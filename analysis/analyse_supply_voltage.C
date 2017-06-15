#include <iostream>
#include <vector>
#include <algorithm>

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TMultiGraph.h"
#include "TLegend.h"

//void analyse_supply_voltage(TString filename = "threshold_summary_608520W07R07_supplyVoltage_20170608_112250.root") {
void analyse_supply_voltage(TString filename = "threshold_summary_608520W07R07_supplyVoltage_20170609_190610.root") {
  TFile* f = new TFile(filename.Data());
  TTree* t = 0x0;
  gDirectory->GetObject("voltageDependence", t);

  t->Draw("Vbb", "", "goff");
  std::vector<float> vbb_list(t->GetV1(), t->GetV1()+t->GetSelectedRows());
  std::sort(vbb_list.begin(), vbb_list.end());
  vbb_list.erase(std::unique(vbb_list.begin(), vbb_list.end()), vbb_list.end());

  t->Draw("Ithr", "", "goff");
  std::vector<short> ithr_list(t->GetV1(), t->GetV1()+t->GetSelectedRows());
  std::sort(ithr_list.begin(), ithr_list.end());
  ithr_list.erase(std::unique(ithr_list.begin(), ithr_list.end()), ithr_list.end());

  t->Draw("Vcasn", "", "goff");
  std::vector<short> vcasn_list(t->GetV1(), t->GetV1()+t->GetSelectedRows());
  std::sort(vcasn_list.begin(), vcasn_list.end());
  vcasn_list.erase(std::unique(vcasn_list.begin(), vcasn_list.end()), vcasn_list.end());

  TFile* f_out = new TFile("output.root", "RECREATE");
  TF1* func = new TF1("func", "[0]*x+[1]");

  TTree* t_out = new TTree("coefficients","coefficients");
  int t_ithr    = 0;
  int t_vcasn   = 0;
  float t_vbb   = 0.;
  float t_thr   = 0.;
  float t_coeff = 0.;

  t_out->Branch("Ithr",  &t_ithr,  "Ithr/I");
  t_out->Branch("Vcasn", &t_vcasn, "Vcasn/I");
  t_out->Branch("Vbb",   &t_vbb,   "Vbb/F");
  t_out->Branch("Thr",   &t_thr,   "Thr/F");
  t_out->Branch("Coeff", &t_coeff, "Coeff/F");

  TCanvas* c = new TCanvas("c", "c", 1920, 1080);

  std::vector<float> thr;
  std::vector<float> coefficient;

  for (float vbb : vbb_list) {
    for (int ithr : ithr_list) {
      for (int vcasn : vcasn_list) {

        TString cutStr = TString::Format("(Vbb>%0.3f)&&(Vbb<%0.3f)&&(Ithr==%d)&&(Vcasn==%d)", vbb-0.01, vbb+0.01, ithr, vcasn);
        t->Draw("V*1000:THR*1.8/V", cutStr.Data());
        TGraph* g = new TGraph(t->GetSelectedRows(), t->GetV1(), t->GetV2());
        if (t->GetSelectedRows() > 0) {
          g->SetName(TString::Format("THR_Vbb%0.3f_Ithr%d_Vcasn%d", vbb, ithr, vcasn));
          g->SetTitle(";V (mV); Threshold (#it{e}^{-})");
          g->Write();
          g->Fit(func, "Q");

          cutStr += TString::Format("&&(V<1.8001)&&(V>1.7999)");
          t->Draw("THR", cutStr.Data(), "goff");

          t_vbb   = vbb;
          t_ithr  = ithr;
          t_vcasn = vcasn;
          t_coeff = func->GetParameter(0);
          t_thr   = t->GetV1()[0];

          std::cout << vbb << '\t' << ithr << '\t' << vcasn << '\t' << t_coeff << '\t' << t_thr << '\t' << func->GetChisquare()/func->GetNDF() << std::endl;
          if (fabs(t_coeff)>0.001 && t_thr > 0. && func->GetChisquare()/func->GetNDF()>0.1 && func->GetChisquare()/func->GetNDF()<2000) {
            thr.push_back(t->GetV1()[0]);
            coefficient.push_back(func->GetParameter(0));
            t_out->Fill();
          }
        }
      }
    }
  }

  //===============================================================================================

  TGraph* g_all = new TGraph(thr.size(), &thr[0], &coefficient[0]);
  g_all->SetTitle(";Threshold (#it{e}^{-});Supply Voltage Sensitivity (#it{e}^{-}/mV)");
  g_all->SetName("g_all");
  g_all->Sort();
  g_all->Draw("AP");
  g_all->Write();
  c->SaveAs("g_all.pdf");
  c->SaveAs("g_all.root");

  t_out->Write();

  t_out->Draw("Thr:Coeff", "Vbb==0", "goff");
  TGraph* g0V = new TGraph(t_out->GetSelectedRows(), t_out->GetV1(), t_out->GetV2());
  g0V->SetTitle(";Threshold (#it{e}^{-});Supply Voltage Sensitivity (#it{e}^{-}/mV)");
  g0V->SetName("g0V");
  g0V->Write();
  g0V->Draw("AP");
  c->SaveAs("g0V.pdf");
  c->SaveAs("g0V.root");


  t_out->Draw("Thr:Coeff", "Vbb<0", "goff");
  TGraph* g3V = new TGraph(t_out->GetSelectedRows(), t_out->GetV1(), t_out->GetV2());
  g3V->SetTitle(";Threshold (#it{e}^{-});Supply Voltage Sensitivity (#it{e}^{-}/mV)");
  g3V->SetName("g3V");
  g3V->Write();
  g3V->Draw("AP");
  c->SaveAs("g3V.pdf");
  c->SaveAs("g3V.root");

  std::vector<TMultiGraph*> mg_vcasn;
  std::vector<TMultiGraph*> mg_ithr;

  Int_t colours[] = { kBlack, kRed, kAzure, kOrange, kViolet, kGreen+3 };
  Int_t markers[] = { 20,     21,   24,     25,      47,      46       };

  for (float vbb : vbb_list) {
    TMultiGraph* mg = new TMultiGraph();
    TLegend* l = new TLegend(0.7, 0.7, 0.9, 0.9);
    mg->SetName(TString::Format("mg_Vbb%0.3f_vcasn", vbb));
    mg->SetTitle(";Threshold (#it{e}^{-});Supply Voltage Sensitivity (#it{e}^{-}/mV)");
    mg_vcasn.push_back(mg);
    Int_t i_graph = 0;
    for (int ithr : ithr_list) {
      TString cutStr = TString::Format("(Vbb>%0.3f)&&(Vbb<%0.3f)&&(Ithr==%d)",  vbb-0.01, vbb+0.01, ithr);
      t_out->Draw("Thr:Coeff", cutStr.Data(), "goff");
      if (t_out->GetSelectedRows()>0) {
        TGraph* g = new TGraph(t_out->GetSelectedRows(), t_out->GetV1(), t_out->GetV2());
        g->SetLineColor(colours[i_graph]);
        g->SetMarkerColor(colours[i_graph]);
        g->SetMarkerStyle(markers[i_graph]);
        g->SetName(TString::Format("g_Vbb%0.3f_Ithr%d", vbb, ithr));
        mg->Add(g, "lp");
        l->AddEntry(g, TString::Format("Ithr=%dDAC", ithr), "lp");
        ++i_graph;
      }
    }
    mg->Write();
    mg->Draw("ALP");
    l->Draw();
    c->SaveAs(TString::Format("%s.pdf", mg->GetName()));
    c->SaveAs(TString::Format("%s.root", mg->GetName()));

    mg = new TMultiGraph();
    mg->SetName(TString::Format("mg_Vbb%0.3f_ithr", vbb));
    mg->SetTitle(";Threshold (#it{e}^{-});Supply Voltage Sensitivity (#it{e}^{-}/mV)");
    mg_ithr.push_back(mg);
    l->Clear();
    i_graph = 0;
    for (int vcasn : vcasn_list) {
      TString cutStr = TString::Format("(Vbb>%0.3f)&&(Vbb<%0.3f)&&(Vcasn==%d)", vbb-0.01, vbb+0.01, vcasn);
      t_out->Draw("Thr:Coeff", cutStr.Data(), "goff");
      if (t_out->GetSelectedRows()>0) {
        TGraph* g = new TGraph(t_out->GetSelectedRows(), t_out->GetV1(), t_out->GetV2());
        g->SetLineColor(colours[i_graph]);
        g->SetMarkerColor(colours[i_graph]);
        g->SetMarkerStyle(markers[i_graph]);
        g->SetName(TString::Format("g_Vbb%0.3f_Vcasn%d", vbb, vcasn));
        mg->Add(g);
        mg->Add(g, "lp");
        l->AddEntry(g, TString::Format("Vcasn=%dDAC", vcasn), "lp");
        ++i_graph;
      }
    }
    mg->Write();
    mg->Draw("ALP");
    l->Draw();
    c->SaveAs(TString::Format("%s.pdf", mg->GetName()));
    c->SaveAs(TString::Format("%s.root", mg->GetName()));
  }

  f_out->Close();
  delete f_out;
  f_out = 0x0;

  f->Close();
  delete f;
  f = 0x0;
}
