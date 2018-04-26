#include <iostream>
#include <iomanip>
#include <fstream>

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TGraph.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TError.h"



void analyse_AVDD(TString inputFile) {
  TTree* t = 0x0;
  TFile* f_in = new TFile(inputFile.Data());

  TString inputFolder = inputFile;
  inputFolder.Remove(inputFolder.Last('/'));

  TString inputFileBaseName = inputFile;
  inputFileBaseName.Remove(inputFileBaseName.Last('.'));

  gDirectory->GetObject("AVDDsummary", t);
  if (!t) {
    std::cerr << "Could not find the TTree \"AVDDsummary\" in the input file "
              << inputFile.Data() << "!" << std::endl;
  }

  std::cout << TString::Format("%s_fit_results.txt", inputFileBaseName.Data()) << std::endl;
  TString f_out_fit_name = TString::Format("%s_fit_results.txt", inputFileBaseName.Data());
  f_out_fit_name.ReplaceAll(" ", "\\ ");
  std::ofstream f_out_fit;
  f_out_fit.open(f_out_fit_name.Data(), std::ofstream::out | std::ofstream::trunc);
  if (!f_out_fit.is_open()) {
    std::cerr << "Could not open the output file for the fit results: "
              << f_out_fit_name.Data() << "!" << std::endl;
  }

  TFile* f_out = new TFile(TString::Format("%s_output.root", inputFileBaseName.Data()), "RECREATE");

  TString draw_str[] = { "AVDDsetV:AVDDmeasV",               "AVDDsetV:VTEMPmeasV",              "AVDDmeasV:VTEMPmeasV",              "AVDDsetV:VTEMPmeasV/0.772+0.023",   "AVDDmeasV+0.023:VTEMPmeasV/0.772+0.023", "AVDDsetV:VTEMPmeasV/0.772+0.023-AVDDsetV", "AVDDmeasV+0.023:VTEMPmeasV/0.772+0.023-AVDDmeasV+0.023" };
  TString name_str[] = { "gAVDDmeasVvsAVDDsetV_Chip%d",      "gVTEMPmeasVvsAVDDsetV_Chip%d",     "gVTEMPmeasVvsAVDDmeasV_Chip%d",     "EstimatedAVDDvsAVDDsetV_Chip%d",    "EstimatedAVDDvsAVDDmeasV_Chip%d",        "EstDevAVDDvsAVDDsetV_Chip%d",              "EstDevAVDDvsAVDDmeasV_Chip%d"                           };
  TString axes_str[] = { ";AVDD_{set} (V); AVDD_{meas} (V)", ";AVDD_{set} (V);VTEMP_{meas} (V)", ";AVDD_{meas} (V);VTEMP_{meas} (V)", ";AVDD_{meas} (V);VTEMP_{meas} (V)", ";AVDD_{meas} (V);VTEMP_{meas} (V)",      ";AVDD_{set} (V);# Delta AVDD_{set} (V)",   ";AVDD_{meas} (V);#Delta AVDD_{meas} (V)"                };
  Float_t fit_low_lim[] = { 1.500,                           1.60,                               1.60,                                1.60,                                1.60,                                     1.60,                                       1.60                                                     };
  Float_t fit_up_lim[]  = { 1.721,                           2.05,                               1.701,                               2.01,                                1.701,                                    2.01,                                       1.701                                                    };
  TString func_str[] = { "[1]*x+[0]",                        "[0]*x",                            "[0]*x",                             "[1]*(x-1.8)+[0]+1.8",               "[1]*(x-1.8)+[0]+1.8",                    "[1]*(x-1.8)+[0]",                          "[1]*(x-1.8)+[0]",                                       };
  Bool_t  sub_Xoffset[] = { kTRUE,                           kTRUE,                              kFALSE,                              kTRUE,                               kFALSE,                                   kTRUE,                                      kFALSE                                                   };
  Bool_t  sub_Yoffset[] = { kFALSE,                          kFALSE,                             kFALSE,                              kFALSE,                              kFALSE,                                   kTRUE,                                      kFALSE                                                   };

  Float_t offsets[] = { 0., -4e-3, -7e-3, -9e-3, -11e-3, -12e-3, -14e-3, -14e-3, -14e-3 };

  gStyle->SetOptFit(111);

  TCanvas* c = new TCanvas("c", "c", 1920, 1080);

  //f_out_fit << "#Chip\tp0_


  for (unsigned int i_chip=0; i_chip<9; ++i_chip) {
    TString cut_str = TString::Format("(Chip==%d)", i_chip);
    for (unsigned int i_draw_str=0; i_draw_str<sizeof(draw_str)/sizeof(TString); ++i_draw_str) {
      ULong64_t n = t->Draw(draw_str[i_draw_str].Data(), cut_str.Data(), "goff");
      if (n==0) {
        std::cerr << "Did not find data for " << draw_str[i_draw_str].Data() << "!" << std::endl;
        continue;
      }
      if (i_draw_str==0) f_out_fit << i_chip;
      c->Clear();
      TGraph* g = new TGraph(n, t->GetV1(), t->GetV2());
      g->SetName(TString::Format(name_str[i_draw_str].Data(), i_chip));
      g->SetTitle(axes_str[i_draw_str].Data());

      for (ULong64_t i_value = 0; i_value < n; ++i_value) {
        if (sub_Xoffset[i_chip]) g->GetX()[i_value] += offsets[i_chip];
        if (sub_Yoffset[i_chip]) g->GetY()[i_value] -= offsets[i_chip];
      }

      TF1* func = new TF1("func_lin", func_str[i_draw_str].Data(), fit_low_lim[i_draw_str], fit_up_lim[i_draw_str]);
      TFitResultPtr rslt = g->Fit(func, "SQ", "", fit_low_lim[i_draw_str], fit_up_lim[i_draw_str]);
      for (Int_t i_par=0; i_par < func->GetNpar(); ++i_par) {
        f_out_fit << '\t' << rslt->Value(i_par) << '\t' << rslt->Error(i_par);
      }
      rslt->Write();
      g->Draw("AP");
      c->SetName(TString::Format("c_%s",g->GetName()));
      c->SaveAs(TString::Format("%s/c_%s.root", inputFolder.Data(), g->GetName()));
      c->SaveAs(TString::Format("%s/c_%s.pdf",  inputFolder.Data(), g->GetName()));
    }
    f_out_fit << std::endl;
  }
  f_out->Close();
  f_in->Close();


  f_out_fit.close();
  delete f_out;
  delete f_in;
}
