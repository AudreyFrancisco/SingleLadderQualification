#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <sstream>

#include "TCanvas.h"
#include "TH2F.h"
#include "TH2I.h"
#include "TStyle.h"
#include "TColor.h"
#include "TString.h"
#include "TFile.h"
#include "TLine.h"
#include "helpers.h"

using namespace std;

char fNameCfg [1024];
char fNameOut [1024];
char fPathOut [1024];
char fSuffix  [1024];


//____________________________________________________________
Int_t getFiredPixels(TH2I *map){
  Int_t firedPixels = 0;
  for (int icol = 0; icol < 1024; icol++) {
    for (int irow = 0; irow < 512; irow ++) {
      if(map->GetBinContent(icol, irow)!= 0) firedPixels += 1;
    }
  }
  return firedPixels;
}


//____________________________________________________________
void PrepareOutputFile (TString fName) {
  string buff1=fName.Data();
  unsigned pos=buff1.find_last_of("_");
  string buff2=buff1.substr(0,pos);
  pos=buff2.find_last_of("_");
  sprintf(fSuffix, "%s", buff1.substr(pos, 14).c_str());
  pos=buff1.find_last_of("/");
    
  // fPathOut
  sprintf(fPathOut, "%s", buff1.substr(0, pos+1).c_str());
  printf("Output path: %s\n", fPathOut);

  // fNameCfg
  sprintf(fNameCfg, "ScanConfig%s.cfg", fSuffix);
}



//---------------------------------------------------------------------------------------------------
//   main macro
//---------------------------------------------------------------------------------------------------

Bool_t PulseshapeRawToHisto(TString fName) {

  Float_t TimeDelay = 0;
  Float_t clk_dur   = 0.0125; // 80MHz clock
  Int_t   counter, rowStep, colStep;
  Int_t   Charge, Delay, Col, Row, NHits;

  cout << "\nPulseshape RAW to HISTO:" << endl;
  PrepareOutputFile(fName);


  // Get info from the scan config:
  //-----------------------------------------------------------------------------------------------
  ifstream cfg_file(Form("%s%s", fPathOut, fNameCfg));
  if(!cfg_file.good()) {
    cout << "Config file not found!" << endl;
    return kFALSE;
  }

  MeasConfig_t conf = read_config_file(Form("%s%s", fPathOut, fNameCfg));
  print_meas_config(conf);

  Int_t myMaskStages  = conf.MASKSTAGES;
  Int_t myPixPerRegion= conf.PIXPERREGION;
  Int_t n_inj         = conf.NTRIGGERS;
  Int_t start_charge  = conf.CHARGESTART;
  Int_t end_charge    = conf.CHARGESTOP;
  Int_t step_charge   = conf.CHARGESTEP;
  Int_t range_charge  = end_charge-start_charge+1;
  Int_t n_bins_charge = (range_charge-1)/step_charge+1;
  Int_t start_delay   = conf.PULSEDELAYSTART;
  Int_t end_delay     = conf.PULSEDELAYSTOP;
  Int_t step_delay    = conf.PULSEDELAYSTEP;
  Int_t range_delay   = end_delay-start_delay+1; 
  Int_t n_bins_delay  = (range_delay-1)/step_delay+1;


  // Condition for sweeping uniformly the whole range of mask stages:
  //-----------------------------------------------------------------------------------------------
  if (512%myMaskStages == 0) rowStep = 512/myMaskStages;
  else rowStep = 511/(myMaskStages-1);
  colStep = 32/myPixPerRegion;
 

  // Create histograms for all scanned pixels:
  //-----------------------------------------------------------------------------------------------
  TH2F ***hPulse;
  hPulse = new TH2F**[1024];

  for (Int_t icol = 0; icol < 1024; icol += colStep) {
    counter = 0;
    hPulse[icol] = new TH2F*[512];

    for (Int_t irow = 0; irow < 512; irow += rowStep) {
      hPulse[icol][irow] = new TH2F(Form("hPulse_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i_strbBlen%i__PIX_%i_%i_%i",
					 conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
					 conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
					 conf.IDB, conf.IBIAS, conf.VCASP, conf.STROBELENGTH,
					 icol/32, icol%32, irow),
				    Form("Pulse Shape, pixel %i/%i/%i, StrBlen %ins, VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i", 
					 icol/32, icol%32, irow, conf.STROBELENGTH,
					 conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
					 conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
					 conf.IDB, conf.IBIAS, conf.VCASP),
				    n_bins_delay, (start_delay+4)*clk_dur-clk_dur*step_delay*0.5, (end_delay+4)*clk_dur+clk_dur*step_delay*0.5, 
				    n_bins_charge, start_charge-step_charge*0.5, end_charge+step_charge*0.5);
      counter += 1;
      if (counter == myMaskStages) break;
    }
  }
       
  TH2I *scan_map = new TH2I("scan_map", "Map of the scanned pixels; X(pixels); Y(pixels)", 1024, 0, 1023, 512, 0, 511);


  // Read data file and fill histograms:
  //-----------------------------------------------------------------------------------------------
  ifstream infile(fName.Data());
  if (!infile) {
    cout << "Cannot open file!!!" << endl;
    cout << fName << "\t" << fName.Data() << endl;
    return kFALSE; 
  }

  while (infile >> Col >> Row >> Charge >> Delay >> NHits) {
    TimeDelay = Delay + 4; // add 50ns offset
    TimeDelay *= clk_dur;
    hPulse[Col][Row]->Fill(TimeDelay, Charge, NHits);
    scan_map->Fill(Col, Row, 1);
  }
  infile.close();


  // Count the number of fired pixels and fill stat histogram:
  //-----------------------------------------------------------------------------------------------
  Int_t   nFiredPixels = getFiredPixels(scan_map);
  Int_t   nScanedPixels = myMaskStages*myPixPerRegion*32;
  Float_t perctentage = (Float_t)(nFiredPixels/nScanedPixels)*100.0;

  const Int_t nLabels = 6;
  const char *stats[nLabels] = {"Vbb", "Vcasn", "Ithr", "Fired pixels", "Scanned pixels", "Efficiency %"};
  Float_t    values[nLabels] = {(Float_t)conf.VBB, (Float_t)conf.VCASN, (Float_t)conf.ITHR, 
				(Float_t)nFiredPixels, (Float_t)nScanedPixels, perctentage};

  TH1F *hStats = new TH1F("hStats", "Scan settings and efficiency", nLabels, 0, nLabels);
  for (Int_t i = 0; i < nLabels; i++){
    hStats->GetXaxis()->SetBinLabel(i+1, stats[i]);
    hStats->SetBinContent(i+1, values[i]);
  }


  // Normalizing bin contents for efficiency:
  //-----------------------------------------------------------------------------------------------
  for (Int_t icol = 0; icol < 1024; icol += colStep) {
    counter = 0;
    for (Int_t irow = 0; irow < 512; irow += rowStep) {
      for (Int_t i_col_hist = 1; i_col_hist <= n_bins_delay; i_col_hist++) {
	for (Int_t i_row_hist = 1; i_row_hist <= n_bins_charge; i_row_hist++) {
	  hPulse[icol][irow]->SetBinContent(i_col_hist, i_row_hist, 
					    hPulse[icol][irow]->GetBinContent(i_col_hist, i_row_hist)/n_inj);
	}
      }
      counter += 1;
      if (counter == myMaskStages) break;
    }
  }


  // Drawing plots:
  //-----------------------------------------------------------------------------------------------
  TCanvas *c = new TCanvas("c", "c", 1200, 700);
  c->cd();
  c->cd()->SetGrid();
  gStyle->SetOptStat(0);

  //Define new nice color palette:
  const Int_t NRGBs = 5;
  const Int_t NCont = 75;
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]     = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]    = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);

  TCanvas *map = new TCanvas("map", "map", 1200, 700);
  map->cd();
  scan_map->SetMarkerColor(8);
  scan_map->SetMarkerStyle(kFullSquare);
  scan_map->SetMarkerSize(0.5);
  scan_map->Draw("scat");
  TLine *line = new TLine(0, 0, 0, 511);
  line->SetLineColor(kGray);
  line->SetLineStyle(2);
  for(Int_t x=0; x<32; x++){line->DrawLine(32*x, 0, 32*x, 511);}

  TCanvas *s = new TCanvas("s","s", 600,600);
  hStats->Draw("hist");


  // Save results and delete objects:
  //-----------------------------------------------------------------------------------------------
  TFile *out_file = new TFile(Form("%s/PulselengthScan%s.root", fPathOut, fSuffix), "RECREATE");
  out_file->cd();
  scan_map->Write();
  hStats->Write();

  for(Int_t ireg = 0; ireg < 32; ireg++){
    TDirectory *subdir = out_file->mkdir(Form("Region_%i", ireg));
  }

  for (Int_t icol = 0; icol < 1024; icol += colStep) {
    counter = 0;
    out_file->cd(Form("Region_%i", icol/32));
    for (Int_t irow = 0; irow < 512; irow += rowStep) {
      c->Clear();
      hPulse[icol][irow]->Draw("COLZ");
      hPulse[icol][irow]->GetXaxis()->SetTitle("Delay [#mus]");
      hPulse[icol][irow]->GetYaxis()->SetTitle("Charge [DAC]");
      hPulse[icol][irow]->Write();
      delete hPulse[icol][irow]; hPulse[icol][irow] = 0x0;
      counter += 1;
      if (counter == myMaskStages) break;
    }
  }

  out_file->Close();

  delete hPulse; hPulse = 0x0;
  delete hStats; hStats = 0x0;
  delete scan_map; scan_map = 0x0;
  delete c; c = 0x0;
  delete s; s = 0x0;
  delete map; map = 0x0;
  delete line; line = 0x0;
  delete out_file; out_file = 0x0;

  return kTRUE;
}


