#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <sstream>
#include <iomanip>

#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TColor.h"
#include "TString.h"
#include "TFile.h"
#include "TF1.h"
#include "TColor.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TLegend.h"
#include "helpers.h"

#define NSEC 32

using namespace std;

Int_t col[32] = {kBlack, kGray+2, kGray, kViolet+1, kViolet+10,  kBlue+2, kBlue, kAzure+1, kAzure+10, kCyan+2, kCyan, kTeal, kTeal+1,  kGreen+1, kGreen+3, kSpring+9, kSpring+2, kYellow-3, kYellow-7, kYellow, kOrange, kOrange+7, kOrange+10, kPink, kPink+1, kMagenta-10, kPink+7, kMagenta, kRed-7, kRed, kRed+2, kOrange-7};

//Int_t col[16] = {kBlack, kGray, kViolet+1, kBlue, kAzure+1, kCyan, kTeal, kTeal+1,  kGreen+1, kYellow-3, kYellow-7, kOrange, kOrange+7, kPink+1, kMagenta, kRed};
Int_t colpt[8] = {kViolet+1, kBlue, kAzure+10, kTeal+1, kGreen+1, kPink+1, kRed-7, kMagenta};

char fNameCfg   [1024];
char fNameOut   [1024];
char fPathOut   [1024];
char fSuffix    [1024]; 


struct MaxPulseLengthBin_t {
  Float_t Time;
  Float_t Charge;
  Int_t BinX;
  Int_t BinY;
};

struct MinThresholdBin_t {
  Float_t Time;
  Float_t Charge;
  Int_t BinX;
  Int_t BinY;
};

struct MinThreshold_t {
  Float_t Time;
  Float_t Threshold;
  Float_t Noise;
};

struct FitRange_t {
  Int_t BinLow;
  Int_t BinHigh;
};

//____________________________________________________________
// Get fast extimate for maximum pulse length location: 
MaxPulseLengthBin_t getMaxPulseLengthFast(TH2F *h2) {
  MaxPulseLengthBin_t maxpl;
  maxpl.Time   = -1.;
  maxpl.Charge = -1.;
  maxpl.BinX   = -1;
  maxpl.BinY   = -1;
  Int_t n_bins_x = h2->GetXaxis()->GetNbins();
  Int_t n_bins_y = h2->GetYaxis()->GetNbins();
  // search for bin at highest delay with content==1 -> max pulse length
  for (Int_t i_col=n_bins_x-1; i_col>=0; i_col--) {
    for (Int_t i_row=0; i_row<n_bins_y-15; i_row++) {
      if (h2->GetBinContent(i_col+1, i_row+1)>0.999) {
	maxpl.Time   = h2->GetXaxis()->GetBinCenter(i_col+1); 
	maxpl.Charge = h2->GetYaxis()->GetBinCenter(i_row+1);
	maxpl.BinX   = i_col+1;
	maxpl.BinY   = i_row+1;
	return maxpl;
      }
    }
  }
  // no bin with content==1 found
  return maxpl; 
}

//____________________________________________________________
// Get fast extimate for minimum threshold location: 
MinThresholdBin_t getMinThresholdFast(TH2F *h2) {
  MinThresholdBin_t minth;
  minth.Time   = -1.;
  minth.Charge = -1.;
  minth.BinX   = -1;
  minth.BinY   = -1;
  Int_t n_bins_x = h2->GetXaxis()->GetNbins();
  Int_t n_bins_y = h2->GetYaxis()->GetNbins();
  // search for bin at highest delay with content==1 -> min threshold
  for (Int_t i_row=0; i_row<n_bins_y; i_row++) {
    for (Int_t i_col=0; i_col<n_bins_x; i_col++) {
      if (h2->GetBinContent(i_col+1, i_row+1)>0.999) {
	minth.Time   = h2->GetXaxis()->GetBinCenter(i_col+1); 
	minth.Charge = h2->GetYaxis()->GetBinCenter(i_row+1);
	minth.BinX   = i_col+1;
	minth.BinY   = i_row+1;
	return minth;
      }
    }
  }
  // no bin with content==1 found
  return minth; 
}

//____________________________________________________________
// Get range for fitting hists for finding min threshold:
FitRange_t getFitRange(TH2F *h2) {
  FitRange_t fr;
  fr.BinLow = -1;
  fr.BinHigh = -1;
  Int_t n_bins_x = h2->GetXaxis()->GetNbins();

  // fit range should be columns where row < [row(max_pulse_length) - row(min_threshold)]/2 + row(min_threshold)
  Int_t row_maxpl = getMaxPulseLengthFast(h2).BinY;
  Int_t row_minth = getMinThresholdFast(h2).BinY;
  // row in center of row_minth and row_maxpl
  Int_t row_center = row_minth + (row_maxpl - row_minth)/2+1;
  // low edge
  for (Int_t i_col=0; i_col<n_bins_x; i_col++) {
    if (h2->GetBinContent(i_col+1, row_center)>0.999) {
      fr.BinLow = i_col+1;
      break;
    }
  }
  // high edge
  for (Int_t i_col=n_bins_x-1; i_col>=0; i_col--) {
    if (h2->GetBinContent(i_col+1, row_center)>0.999) {
      fr.BinHigh = i_col+1;
      break;
    }
  }
  return fr;
}

//____________________________________________________________
// Get minimum threshold:
MinThreshold_t getMinThreshold(TH2F *h2) {
  MinThreshold_t minth;
  minth.Time   = -1.;
  minth.Threshold = -1.;
  minth.Noise = -1.;
  FitRange_t fr = getFitRange(h2);
  MaxPulseLengthBin_t maxpl = getMaxPulseLengthFast(h2);

  gStyle->SetOptStat(111111);
  TH1F *h_th = new TH1F(Form("%s_th", h2->GetName()), Form("%s_th", h2->GetTitle()), 100, 0, maxpl.Charge);
  TH1F *h_th_noise = new TH1F(Form("%s_th_noise", h2->GetName()), Form("%s_th_noise", h2->GetTitle()), 100, 0, 10);
  TH1F *h_th_chi2 = new TH1F(Form("%s_th_chi2", h2->GetName()), Form("%s_th_chi2", h2->GetTitle()), 100, 0, 0.10);
  // histograms for thresholds and noise
  TH1D  *h1_threshold = (TH1D*)h2->ProjectionX("h1_threshold");
  h1_threshold->SetTitle("");
  h1_threshold->SetStats(0);
  h1_threshold->SetLineColor(kBlue);
  h1_threshold->SetMarkerColor(kBlue); 
  h1_threshold->SetMarkerStyle(20); 
  h1_threshold->SetMarkerSize(0.8); 
  h1_threshold->GetYaxis()->SetTitle("Threshold [DAC]");
  TH1D  *h1_noise     = (TH1D*)h2->ProjectionX("h1_noise");
  h1_noise->SetTitle("");
  h1_noise->SetStats(0);
  h1_noise->SetLineColor(kBlue);
  h1_noise->SetMarkerColor(kBlue); 
  h1_noise->SetMarkerStyle(20); 
  h1_noise->SetMarkerSize(0.8); 
  h1_noise->GetYaxis()->SetTitle("Threshold Noise [DAC]");

  // create histograms to fit with erf
  const Int_t n_hists = fr.BinHigh - fr.BinLow + 1;
  TH1D  *h1[n_hists];
  TF1 *fit_erf[n_hists];

  for (Int_t i_bin=1; i_bin<=h1_threshold->GetNbinsX(); i_bin++) {
    h1_threshold->SetBinContent(i_bin, 0);
    h1_noise->SetBinContent(i_bin, 0);
  }

  for (Int_t i_hist=0; i_hist<n_hists; i_hist++) {
    h1[i_hist] = (TH1D*)h2->ProjectionY(Form("h1_yslice_at_xbin_%i", i_hist+fr.BinLow), i_hist+fr.BinLow, i_hist+fr.BinLow);
    h1[i_hist]->SetTitle(Form("yslice at xbin=%i, t=%f", i_hist+fr.BinLow, h2->GetXaxis()->GetBinCenter(i_hist+fr.BinLow)));
    h1[i_hist]->SetStats(0);
    h1[i_hist]->Draw("hist");

    fit_erf[i_hist] = new TF1(Form("fit_erf_%i", i_hist+fr.BinLow),
			      "0.5*(1+TMath::Erf((x-[0])/(sqrt(2)*[1])))", 
			      h1[i_hist]->GetXaxis()->GetXmin(), maxpl.Charge);

    fit_erf[i_hist]->SetParNames("Threshold", "Noise"); // noise -> sigma of gaussian 
    Float_t thr_est = h1[i_hist]->GetXaxis()->GetBinCenter(h1[i_hist]->FindFirstBinAbove(0.3)); // estimate for threshold position
    fit_erf[i_hist]->SetParameters(thr_est, 1);
    fit_erf[i_hist]->SetParLimits(0, 0, 160);
    fit_erf[i_hist]->SetParLimits(1, 0, 2);

    h1[i_hist]->Fit(Form("fit_erf_%i", i_hist+fr.BinLow), "RQ");
    fit_erf[i_hist]->Draw("same");

    h_th->Fill(fit_erf[i_hist]->GetParameter(0));
    h_th_noise->Fill(fit_erf[i_hist]->GetParameter(1));
    h_th_chi2->Fill(fit_erf[i_hist]->GetChisquare());

    h1_threshold->SetBinContent(i_hist+fr.BinLow, fit_erf[i_hist]->GetParameter(0));
    h1_noise->SetBinContent(i_hist+fr.BinLow, fit_erf[i_hist]->GetParameter(1));
  }

  h1_threshold->GetXaxis()->SetRange(fr.BinLow, fr.BinLow+n_hists-1);
  Int_t minbin    = h1_threshold->GetMinimumBin();
  minth.Time      = h1_threshold->GetBinCenter(minbin);
  minth.Threshold = h1_threshold->GetBinContent(minbin);
  minth.Noise     = h1_noise->GetBinContent(minbin);
  h1_threshold->GetXaxis()->SetRange(0, h1_threshold->GetNbinsX());

  return minth; 
}

//____________________________________________________________
void getTimeAboveThreshold(TH2F *h2, TGraph *g, Int_t n_bins_y) {
  Int_t n_bins_x = h2->GetXaxis()->GetNbins();
  g->Set(n_bins_y);
  Float_t charge[n_bins_y];
  Float_t time_above_thr[n_bins_y];
  // search for bin at highest delay with content==1 -> min threshold
  for (Int_t i_row=0; i_row<n_bins_y; i_row++) {
    charge[i_row] = h2->GetYaxis()->GetBinCenter(i_row+1);
    time_above_thr[i_row] = 0;
    for (Int_t i_col=0; i_col<n_bins_x; i_col++) {
      if (h2->GetBinContent(i_col+1, i_row+1)>0.999) {
	time_above_thr[i_row]++;
      }
    }
    time_above_thr[i_row] *= h2->GetXaxis()->GetBinWidth(1);
    g->SetPoint(i_row, charge[i_row], time_above_thr[i_row]);
  }
}

//____________________________________________________________
void getTimeWalk(TH2F *h2, TGraph *g, TGraph *g_abovethr, Int_t n_bins_y, Float_t max_time_walk) {
  Int_t n_bins_x = h2->GetXaxis()->GetNbins();

  g->Set(n_bins_y);
  g_abovethr->Set(n_bins_y);

  Float_t charge[n_bins_y];
  Float_t time_walk[n_bins_y];
  Float_t time_above_thr[n_bins_y];
  // search for bin at highest delay with content==1 -> min threshold
  Bool_t row_min_thr_set=kFALSE;
  Int_t i_bin_abovethr=0;
  for (Int_t i_row=0; i_row<n_bins_y; i_row++) {
    charge[i_row] = h2->GetYaxis()->GetBinCenter(i_row+1);
    time_walk[i_row] = 0;
    time_above_thr[i_row] = 0;
    for (Int_t i_col=0; i_col<n_bins_x; i_col++) {
      if (h2->GetBinContent(i_col+1, i_row+1)>0.999) {
	time_above_thr[i_row]++;
      }
    }
    if (time_above_thr[i_row]==0) {
      time_walk[i_row]=0;
    }
    else {
      for (Int_t i_col=0; i_col<n_bins_x; i_col++) {
	if (h2->GetBinContent(i_col+1, i_row+1)>0.999) {
	  break;
	}
	time_walk[i_row]++;
      }
    }
    time_walk[i_row] *= h2->GetXaxis()->GetBinWidth(1);
    if (!row_min_thr_set && time_walk[i_row]!=0) {
      g->SetPoint(i_row-1, charge[i_row], max_time_walk);
      g->SetPoint(i_row, charge[i_row], time_walk[i_row]);
      g_abovethr->SetPoint(i_bin_abovethr, charge[i_bin_abovethr], max_time_walk);
      i_bin_abovethr++;
      g_abovethr->SetPoint(i_bin_abovethr, charge[i_bin_abovethr], time_walk[i_row]);
      i_bin_abovethr++;
      row_min_thr_set=kTRUE;
    }
    else if (row_min_thr_set){
      g->SetPoint(i_row, charge[i_row], time_walk[i_row]);
      g_abovethr->SetPoint(i_bin_abovethr, charge[i_bin_abovethr], time_walk[i_row]);
      i_bin_abovethr++;
    }
    else {
      g->SetPoint(i_row, charge[i_row], time_walk[i_row]);
    }
  }
  for (Int_t i=i_bin_abovethr;i<n_bins_y;i++) {
    g_abovethr->SetPoint(i, charge[i], 0);
  }
}

//____________________________________________________________
void PrepareOutputFile (TString fName) {
  string buff1=fName.Data();
  unsigned pos=buff1.find_last_of("_");
  string buff2=buff1.substr(0,pos);
  pos=buff2.find_last_of("_");
  sprintf(fSuffix, "%s", buff1.substr(pos, 14).c_str());

  // fNameOut
  sprintf(fNameOut, "PulselengthResults%s.root", fSuffix);
  printf("Output file: %s\n", fNameOut);
  pos=buff1.find_last_of("/");

  // fPathOut
  sprintf(fPathOut, "%s", buff1.substr(0, pos+1).c_str());
  printf("Output path: %s\n", fPathOut);

  // fNameCfg
  sprintf(fNameCfg, "ScanConfig%s.cfg", fSuffix);
}


//---------------------------------------------------------------------------------------------------
// main macro
//---------------------------------------------------------------------------------------------------

Bool_t PulseshapeAnalysis(TString file_path) {

  Int_t counter, rowEnd, rowStep, colStep;
  cout << "\nPulseshapeAnalysis:" << endl;

  // Load input files:
  //-----------------------------------------------------------------------------------------------
  TFile *in_file = new TFile(file_path, "READ");
  if (!in_file->IsOpen()) {
    cout << "Input file not found, please check!!!" << endl;
    cout << file_path << endl;
    return kFALSE;
  }

  PrepareOutputFile(file_path);
  ifstream cfg_file(Form("%s%s", fPathOut, fNameCfg));
  if(!cfg_file.good()) {
    cout << "Config file not found!" << endl;
    return kFALSE;
  }


  // Get info from the scan config:
  //-----------------------------------------------------------------------------------------------
  MeasConfig_t conf = read_config_file(Form("%s%s", fPathOut, fNameCfg));
  print_meas_config(conf);
  Int_t myMaskStages    = conf.MASKSTAGES;
  Int_t sweepStages   = conf.SWEEPSTAGES;
  Int_t myPixPerRegion  = conf.PIXPERREGION;
  Int_t myDCside        = conf.DCSIDE;
  Int_t Strobe_B_length = conf.STROBELENGTH;


  // Sweeping mask stages:
  //-----------------------------------------------------------------------------------------------
  if (sweepStages != 0) {    // condition for sweeping uniformly the whole range of mask stages:
    if (512%myMaskStages == 0) rowStep = 512/myMaskStages;
    else rowStep = 511/(myMaskStages-1);
    rowEnd = 512;
  }
  else {   // scan stages consecutively:
    rowEnd  = myMaskStages;
    rowStep = 1; 
  }
  colStep = 32/myPixPerRegion;


  // Load histograms for all scanned pixels:
  //-----------------------------------------------------------------------------------------------  
  TH2F *map = (TH2F*)in_file->Get("scan_map");

  TH2F ***hPulse;
  hPulse = new TH2F**[1024];

  for (Int_t icol = myDCside; icol < 1024; icol += colStep) {

    in_file->cd(Form("Region_%i", icol/32));
    hPulse[icol] = new TH2F*[512];
    counter = 0;

    for (Int_t irow = 0; irow < rowEnd; irow += rowStep) {
      hPulse[icol][irow] = (TH2F*)gDirectory->Get(Form("hPulse_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i_strbBlen%i__PIX_%i_%i_%i",
					      conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
					      conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
					      conf.IDB, conf.IBIAS, conf.VCASP, Strobe_B_length,
					      icol/32, icol%32, irow));

      if (hPulse[icol][irow] == NULL) {
	cout << "Histogram of the pixel: " << icol/32 << "/" << icol%32 << "/" << irow 
	     << " not found, please check!!!" << endl;
      }
      counter += 1;
      if (counter == myMaskStages) break;
    } 
  }


  // File names:
  //-----------------------------------------------------------------------------------------------
  TString f_out_name_base = Form("%sPulselengthResults", fPathOut);

  TString run_id_short = Form("_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i_strbBlen%i", 
			      conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
			      conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
			      conf.IDB, conf.IBIAS, conf.VCASP,
			      Strobe_B_length);

  TString run_id_full = Form("_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i_strbBlen%i__nMASKSTAGES_%i_nPIXPERREGION_%i", 
			     conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
			     conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
			     conf.IDB, conf.IBIAS, conf.VCASP,
			     Strobe_B_length, myMaskStages, myPixPerRegion);


  // Output ROOT file & subdirectories for each region:
  //-----------------------------------------------------------------------------------------------
  //TString f_out_name_root = f_out_name_base + run_id_full + "_mean.root";
  TString f_out_name_root = f_out_name_base + run_id_full + ".root";

  TFile *f_out_root = new TFile(f_out_name_root, "RECREATE");
  f_out_root->cd();

  for(Int_t ireg = 0; ireg < 32; ireg++){
    TDirectory *subdir = f_out_root->mkdir(Form("Region_%i", ireg));
  }


  // Output DATA file:
  //-----------------------------------------------------------------------------------------------
  TString f_out_name_dat = f_out_name_base + run_id_full + ".dat";
  ofstream f_out_dat(f_out_name_dat.Data());
  if(!f_out_dat.good()) {
    cout << "out.dat file not created!" << endl;
    return kFALSE;
  }
  f_out_dat << "Reg" 
	    << "\t Col"
	    << "\t Row"
	    << "\t MinThr"
	    << "\t MinThrDelay"
	    << "\t MinThrNoise"
	    << "\t MaxPulseLength"
	    << "\t MaxPulseLengthCharge"
	    << endl;


  // Declare and initialize objects:
  //-----------------------------------------------------------------------------------------------
  Int_t ireg;
  Double_t x=0;
  Double_t y=0;
  MinThreshold_t minThr;
  MaxPulseLengthBin_t maxPL;
  const Int_t n_bins_y = hPulse[myDCside][0]->GetYaxis()->GetNbins();

  //Arrays:
  Float_t x_err             [n_bins_y];
  Float_t charge            [n_bins_y];
  Float_t av_tot            [NSEC][n_bins_y];
  Float_t av_tot_err        [NSEC][n_bins_y];
  Float_t av_tw             [NSEC][n_bins_y];
  Float_t av_tw_err         [NSEC][n_bins_y];
  Float_t av_tw_abovethr    [NSEC][n_bins_y];
  Float_t av_tw_abovethr_err[NSEC][n_bins_y];

  Float_t av_tot_stages            [myMaskStages][n_bins_y];
  Float_t av_tot_stages_err        [myMaskStages][n_bins_y];
  Float_t av_tw_stages             [myMaskStages][n_bins_y];
  Float_t av_tw_stages_err         [myMaskStages][n_bins_y];
  Float_t av_tw_abovethr_stages    [myMaskStages][n_bins_y];
  Float_t av_tw_abovethr_stages_err[myMaskStages][n_bins_y];

  // Initialization of arrays (IMPORTANT!):
  for (Int_t i_sec=0; i_sec<NSEC; i_sec++) { 
    for (Int_t i_point=0; i_point<n_bins_y; i_point++) { 
      av_tot            [i_sec][i_point] = 0;
      av_tot_err        [i_sec][i_point] = 0;
      av_tw             [i_sec][i_point] = 0;
      av_tw_err         [i_sec][i_point] = 0;
      av_tw_abovethr    [i_sec][i_point] = 0;
      av_tw_abovethr_err[i_sec][i_point] = 0;
    }
  }

  for (Int_t irow = 0; irow < myMaskStages; irow++) { 
    for (Int_t i_point = 0; i_point < n_bins_y; i_point++) { 
      av_tot_stages             [irow][i_point] = 0;
      av_tot_stages_err         [irow][i_point] = 0;
      av_tw_stages              [irow][i_point] = 0;
      av_tw_stages_err          [irow][i_point] = 0;
      av_tw_abovethr_stages     [irow][i_point] = 0;
      av_tw_abovethr_stages_err [irow][i_point] = 0;
    }
  }
   
  // Graphs:
  TGraph ***gToT; 
  TGraph ***gTimeWalk; 
  TGraph ***gTimeWalkAboveThr; 
  gToT              = new TGraph**[1024];
  gTimeWalk         = new TGraph**[1024];
  gTimeWalkAboveThr = new TGraph**[1024];  

  TGraphErrors *gAvToT                   [NSEC];
  TGraphErrors *gAvTimeWalk              [NSEC];
  TGraphErrors *gAvTimeWalkAboveThr      [NSEC];
  TGraphErrors *gAvToTstages             [myMaskStages];
  TGraphErrors *gAvTimeWalkstages        [myMaskStages];
  TGraphErrors *gAvTimeWalkAboveThrstages[myMaskStages];

  /*TGraph *gAvToT                   [NSEC];
  TGraph *gAvTimeWalk              [NSEC];
  TGraph *gAvTimeWalkAboveThr      [NSEC];
  TGraph *gAvToTstages             [myMaskStages];
  TGraph *gAvTimeWalkstages        [myMaskStages];
  TGraph *gAvTimeWalkAboveThrstages[myMaskStages];*/

  TMultiGraph  *mgToT                 = new TMultiGraph();
  TMultiGraph  *mgTimeWalk            = new TMultiGraph();
  TMultiGraph  *mgTimeWalkAboveThr    = new TMultiGraph();
  TMultiGraph  *mgAvToT               = new TMultiGraph();
  TMultiGraph  *mgAvTimeWalk          = new TMultiGraph();
  TMultiGraph  *mgAvTimeWalkAboveThr  = new TMultiGraph();
  TMultiGraph  *mgAvToTS              = new TMultiGraph();
  TMultiGraph  *mgAvTimeWalkS         = new TMultiGraph();
  TMultiGraph  *mgAvTimeWalkAboveThrS = new TMultiGraph();

  // Histograms:
  // hist for saving the min Thr of the pixels: 
  TH2F *hMinThr           = new TH2F(Form("hMinThr%s", run_id_full.Data()),"Min Threshold [DAC]; Columns; Rows", 
				     32*myPixPerRegion, 0, 32*myPixPerRegion, myMaskStages, 0, myMaskStages);

  // hist for saving the min Thr delay value of the pixels: 
  TH2F *hMinThrDel        = new TH2F(Form("hMinThrDel%s", run_id_full.Data()),"Delay at Min Threshold [#mus]; Columns; Rows", 
				     32*myPixPerRegion, 0, 32*myPixPerRegion, myMaskStages, 0, myMaskStages); 

  // hist for saving the min Thr noise value of the pixels: 
  TH2F *hMinThrNoise      = new TH2F(Form("hMinThrNoise%s", run_id_full.Data()),"Noise at Min Threshold [DAC]; Columns; Rows", 
				     32*myPixPerRegion, 0, 32*myPixPerRegion, myMaskStages, 0, myMaskStages); 

  // hist for saving the max pulse length value of the pixels: 
  TH2F *hMaxPulseLen      = new TH2F(Form("hMaxPulseLen%s", run_id_full.Data()),"Max Pulse Length [#mus]; Columns; Rows", 
				     32*myPixPerRegion, 0, 32*myPixPerRegion, myMaskStages, 0, myMaskStages); 

  // hist for saving the charge value for max pulse length of the pixels:
  TH2F *hMaxPulseLenCharge= new TH2F(Form("hMaxPulseLenCharge%s", run_id_full.Data()),"Charge at Max Pulse Length [DAC]; Columns; Rows", 
				     32*myPixPerRegion, 0, 32*myPixPerRegion, myMaskStages, 0, myMaskStages); 

  // hist for the slice distribution of the ToT for all scanned pixels:
  Int_t nSlices = 4;
  Int_t slicesAT[4] = {300, 500, 800, 1200};   // 4 slices at: 300, 500, 800, 1200 e- 
  TH1F **hSliceAll;
  hSliceAll = new TH1F*[nSlices];


  // slice histograms for each region (or sector):
  TH1F ***hSlice;
  hSlice = new TH1F**[nSlices];
  for (Int_t iSlice = 0; iSlice < nSlices; iSlice++) {
    hSliceAll[iSlice] = new TH1F(Form("hSliceAll_charge%de-", slicesAT[iSlice]),"Slice histogram ToT for all scanned pixels;Time(#mus)",50, 0, 10);
    hSlice   [iSlice] = new TH1F*[NSEC];
    for (Int_t iSector = 0; iSector < NSEC; iSector++) {
      hSlice[iSlice][iSector] = new TH1F(Form("hSlice_reg%d_charge%de-", iSector, slicesAT[iSlice]),"Slice histogram ToT;Time(#mus)",50, 0, 10);
    }
  }


  TCanvas *c_test = new TCanvas("c_test", "c_test", 800, 600);
  c_test->cd();


  // Filling graphs & histograms:
  //-----------------------------------------------------------------------------------------------
  for (Int_t icol = myDCside; icol < 1024; icol += colStep) {
    gToT[icol]              = new TGraph*[512];
    gTimeWalk[icol]         = new TGraph*[512];
    gTimeWalkAboveThr[icol] = new TGraph*[512]; 

    ireg = icol/32;
    f_out_root->cd(Form("Region_%i", ireg));
    counter = 0;

    for (Int_t irow = 0; irow < rowEnd; irow += rowStep) {
      // Filling 2D histograms:
      minThr = getMinThreshold(hPulse[icol][irow]);
      cout << setiosflags(ios::fixed) << setprecision(2) 
	   << ireg << "\t" << icol%32 << "\t" << irow << "\t" 
	   << minThr.Threshold << "\t" << minThr.Time << "\t" << minThr.Noise << endl;

      hMinThr->SetBinContent(icol/colStep+1, irow/rowStep+1, minThr.Threshold);
      hMinThrDel->SetBinContent(icol/colStep+1, irow/rowStep+1, minThr.Time);
      hMinThrNoise->SetBinContent(icol/colStep+1, irow/rowStep+1,  minThr.Noise);

      maxPL = getMaxPulseLengthFast(hPulse[icol][irow]);
      hMaxPulseLen->SetBinContent(icol/colStep+1, irow/rowStep+1, maxPL.Time);
      hMaxPulseLenCharge->SetBinContent(icol/colStep+1, irow/rowStep+1, maxPL.Charge);

      f_out_dat << ireg 
		<< "\t" << icol%32
		<< "\t" << irow
		<< "\t" << minThr.Threshold
		<< "\t" << minThr.Time
		<< "\t" << minThr.Noise
		<< "\t" << maxPL.Time
		<< "\t" << maxPL.Charge
		<< endl;


      // Individual graphs for each pixel & multigraphs:
      gToT[icol][irow] = new TGraph();
      gTimeWalk[icol][irow] = new TGraph();
      gTimeWalkAboveThr[icol][irow] = new TGraph();

      getTimeAboveThreshold(hPulse[icol][irow], gToT[icol][irow], n_bins_y);
      getTimeWalk(hPulse[icol][irow], gTimeWalk[icol][irow], gTimeWalkAboveThr[icol][irow], n_bins_y, minThr.Time);

      gToT[icol][irow]->SetName(Form("gToT%s_reg%i_col%i_row%i", run_id_short.Data(), ireg, icol%32, irow)); 
      gToT[icol][irow]->SetTitle(Form("gToT_%i/%i/%i;Charge (DAC);Time (#mus)", ireg, icol%32, irow));
      gToT[icol][irow]->Write();
      mgToT->Add(gToT[icol][irow], "pl");

      gTimeWalk[icol][irow]->SetName(Form("gTimeWalk%s_reg%i_col%i_row%i", run_id_short.Data(), ireg, icol%32, irow)); 
      gTimeWalk[icol][irow]->SetTitle(Form("gTimeWalk_%i/%i/%i;Charge (DAC);Time (#mus)", ireg, icol%32, irow));
      gTimeWalk[icol][irow]->Write();
      mgTimeWalk->Add(gTimeWalk[icol][irow], "pl");

      gTimeWalkAboveThr[icol][irow]->SetName(Form("gTimeWalkAboveThr%s_reg%i_col%i_row%i", run_id_short.Data(), ireg, icol%32, irow)); 
      gTimeWalkAboveThr[icol][irow]->SetTitle(Form("gTimeWalkAboveThr_%i/%i/%i;Charge (DAC);Time (#mus)", ireg, icol%32, irow));
      gTimeWalkAboveThr[icol][irow]->Write();
      mgTimeWalkAboveThr->Add(gTimeWalkAboveThr[icol][irow], "pl");

      // Sums for means:
      for (Int_t i_point=0; i_point<n_bins_y; i_point++) {
	gToT[icol][irow]->GetPoint(i_point, x, y);
	av_tot[ireg/(32/NSEC)][i_point] += y;
	av_tot_stages[irow/rowStep][i_point] += y;

	// Filling of the slice histos (i.e. one slice of the ToT graph):
	for (Int_t iSlice = 0; iSlice < nSlices; iSlice++) {
	  if (i_point==(slicesAT[iSlice]/20)) {   
	    hSlice[iSlice][ireg/(32/NSEC)]->Fill(y);
	    hSliceAll[iSlice]->Fill(y);
	  }
	}

	gTimeWalk[icol][irow]->GetPoint(i_point, x, y);
	av_tw[ireg/(32/NSEC)][i_point] += y;
	av_tw_stages[irow/rowStep][i_point] += y;

	gTimeWalkAboveThr[icol][irow]->GetPoint(i_point, x, y);
	av_tw_abovethr[ireg/(32/NSEC)][i_point] += y;
	av_tw_abovethr_stages[irow/rowStep][i_point] += y;

	charge[i_point] = hPulse[icol][irow]->GetYaxis()->GetBinCenter(i_point+1);
      }
      counter += 1;
      if (counter == myMaskStages) break;
    }
  }

  f_out_dat.close();
  f_out_root->cd();

  hMinThr->Write();
  hMinThrDel->Write();
  hMinThrNoise->Write();
  hMaxPulseLen->Write();
  hMaxPulseLenCharge->Write();

  mgToT->Write("mgToT");
  mgTimeWalk->Write("mgTimeWalk");
  mgTimeWalkAboveThr->Write("mgTimeWalkAboveThr");


  // Calculation of the Mean:
  //-----------------------------------------------------------------------------------------------
  for (Int_t i_sec=0; i_sec<NSEC; i_sec++) { 
    for (Int_t i_point=0; i_point<n_bins_y; i_point++) { 
      av_tot[i_sec][i_point] /= myMaskStages*(32/NSEC);
      av_tw[i_sec][i_point] /= myMaskStages*(32/NSEC);
      av_tw_abovethr[i_sec][i_point] /= myMaskStages*(32/NSEC);
    }
  }

  for (Int_t irow = 0; irow < myMaskStages; irow++) { 
    for (Int_t i_point=0; i_point < n_bins_y; i_point++) { 
      av_tot_stages[irow][i_point] /= 32;  
      av_tw_stages[irow][i_point] /= 32;
      av_tw_abovethr_stages[irow][i_point] /= 32;
    }
  }


  // Calculation of the RMS for error bars:
  //-----------------------------------------------------------------------------------------------
  for (Int_t icol = myDCside; icol < 1024; icol += colStep) {
    ireg = icol/32;
    counter = 0;
    for (Int_t irow = 0; irow < rowEnd; irow += rowStep) {
      for (Int_t i_point = 0; i_point < n_bins_y; i_point++) { 
	gToT[icol][irow]->GetPoint(i_point, x, y);
	av_tot_err[ireg/(32/NSEC)][i_point] += (y-av_tot[ireg/(32/NSEC)][i_point])*(y-av_tot[ireg/(32/NSEC)][i_point]);
	av_tot_stages_err[irow/rowStep][i_point] += (y-av_tot_stages[irow/rowStep][i_point])*(y-av_tot_stages[irow/rowStep][i_point]);

	gTimeWalk[icol][irow]->GetPoint(i_point, x, y);
	av_tw_err[ireg/(32/NSEC)][i_point] += (y-av_tw[ireg/(32/NSEC)][i_point])*(y-av_tw[ireg/(32/NSEC)][i_point]);
	av_tw_stages_err[irow/rowStep][i_point] += (y-av_tw_stages[irow/rowStep][i_point])*(y-av_tw_stages[irow/rowStep][i_point]);

	gTimeWalkAboveThr[icol][irow]->GetPoint(i_point, x, y);
	av_tw_abovethr_err[ireg/(32/NSEC)][i_point] += (y-av_tw_abovethr[ireg/(32/NSEC)][i_point])*(y-av_tw_abovethr[ireg/(32/NSEC)][i_point]);
	av_tw_abovethr_stages_err[irow/rowStep][i_point] += (y-av_tw_abovethr_stages[irow/rowStep][i_point])*(y-av_tw_abovethr_stages[irow/rowStep][i_point]);
	x_err[i_point]=(charge[1]-charge[0])/2;
      }
      counter += 1;
      if (counter == myMaskStages) break;
    }
  }


  // Comparison of different SECTORS:
  //=================================================================================================
  TLegend *leg1 = new TLegend(0.85, 0, 0.98, 1);
  TDirectory *subdir_sectors = f_out_root->mkdir("SectorBySector");
  f_out_root->cd("SectorBySector");
  for (Int_t i_sec=0; i_sec<NSEC; i_sec++) { 
    // Finish the calculation of RMS:
    for (Int_t i_point=0; i_point<n_bins_y; i_point++) { 
      av_tot_err[i_sec][i_point] = sqrt(av_tot_err[i_sec][i_point]/((myMaskStages*32/NSEC)-1));
      av_tw_err[i_sec][i_point] = sqrt(av_tw_err[i_sec][i_point]/((myMaskStages*32/NSEC)-1));
      av_tw_abovethr_err[i_sec][i_point] = sqrt(av_tw_abovethr_err[i_sec][i_point]/((myMaskStages*32/NSEC)-1));
    }
    gAvToT[i_sec] = new TGraphErrors(n_bins_y, charge, av_tot[i_sec], 0, av_tot_err[i_sec]); 
    //gAvToT[i_sec] = new TGraph(n_bins_y, charge, av_tot[i_sec]); 
    gAvToT[i_sec]->SetLineColor(col[i_sec]);
    gAvToT[i_sec]->SetMarkerColor(col[i_sec]);
    gAvToT[i_sec]->SetMarkerStyle(20);
    gAvToT[i_sec]->SetMarkerSize(0.6);
    gAvToT[i_sec]->SetName(Form("gAvToT_sec%i", i_sec)); 
    gAvToT[i_sec]->SetTitle(Form("gAvToT_sec%i;Charge (DAC);Time (#mus)", i_sec));
    gAvToT[i_sec]->Write();
    mgAvToT->Add(gAvToT[i_sec], "pl");
    leg1->AddEntry(gAvToT[i_sec], Form("Sector %i", i_sec), "lp");

    gAvTimeWalk[i_sec] = new TGraphErrors(n_bins_y, charge, av_tw[i_sec], 0, av_tw_err[i_sec]); 
    //gAvTimeWalk[i_sec] = new TGraph(n_bins_y, charge, av_tw[i_sec]);
    gAvTimeWalk[i_sec]->SetLineColor(col[i_sec]);
    gAvTimeWalk[i_sec]->SetMarkerColor(col[i_sec]);
    gAvTimeWalk[i_sec]->SetMarkerStyle(20);
    gAvTimeWalk[i_sec]->SetMarkerSize(0.6);
    gAvTimeWalk[i_sec]->SetName(Form("gAvTimeWalk_sec%i", i_sec)); 
    gAvTimeWalk[i_sec]->SetTitle(Form("gAvTimeWalk_sec%i;Charge (DAC);Time (#mus)", i_sec));
    gAvTimeWalk[i_sec]->Write();
    mgAvTimeWalk->Add(gAvTimeWalk[i_sec], "pl");

    gAvTimeWalkAboveThr[i_sec] = new TGraphErrors(n_bins_y, charge, av_tw_abovethr[i_sec], 0, av_tw_abovethr_err[i_sec]); 
    //gAvTimeWalkAboveThr[i_sec] = new TGraph(n_bins_y, charge, av_tw_abovethr[i_sec]) ;
    gAvTimeWalkAboveThr[i_sec]->SetLineColor(col[i_sec]);
    gAvTimeWalkAboveThr[i_sec]->SetMarkerColor(col[i_sec]);
    gAvTimeWalkAboveThr[i_sec]->SetMarkerStyle(20);
    gAvTimeWalkAboveThr[i_sec]->SetMarkerSize(0.6);
    gAvTimeWalkAboveThr[i_sec]->SetName(Form("gAvTimeWalkAboveThr_sec%i", i_sec)); 
    gAvTimeWalkAboveThr[i_sec]->SetTitle(Form("gAvTimeWalkAboveThr_sec%i;Charge (DAC);Time (#mus)", i_sec));
    gAvTimeWalkAboveThr[i_sec]->Write();
    mgAvTimeWalkAboveThr->Add(gAvTimeWalkAboveThr[i_sec], "pl");
  }
  mgAvToT->Write("mgAvToT");
  mgAvTimeWalk->Write("mgAvTimeWalk");
  mgAvTimeWalkAboveThr->Write("mgAvTimeWalkAboveThr");



  // Comparison of different STAGES:
  //=================================================================================================
  TLegend *leg2 = new TLegend(0.8, 0.3, 0.95, 0.7);
  TDirectory *subdir_stages = f_out_root->mkdir("StageByStage");
  f_out_root->cd("StageByStage");
  for (Int_t irow = 0; irow < myMaskStages; irow++) { 
    // Finish the calculation of RMS:
    for (Int_t i_point=0; i_point < n_bins_y; i_point++) { 
      av_tot_stages_err        [irow][i_point] = sqrt(av_tot_stages_err[irow][i_point]/(32-1));    
      av_tw_stages_err         [irow][i_point] = sqrt(av_tw_stages_err[irow][i_point]/(32-1));
      av_tw_abovethr_stages_err[irow][i_point] = sqrt(av_tw_abovethr_stages_err[irow][i_point]/(32-1));
    }
    gAvToTstages[irow] = new TGraphErrors(n_bins_y, charge, av_tot_stages[irow], 0, av_tot_stages_err[irow]);
    //gAvToTstages[irow] = new TGraph(n_bins_y, charge, av_tot_stages[irow]); 
    gAvToTstages[irow]->SetLineColor(col[irow%32]);
    gAvToTstages[irow]->SetMarkerColor(col[irow%32]);
    gAvToTstages[irow]->SetMarkerStyle(20);
    gAvToTstages[irow]->SetMarkerSize(0.6);
    gAvToTstages[irow]->SetName(Form("gAvToT_stage%i", irow)); 
    gAvToTstages[irow]->SetTitle(Form("gAvToT_stage%i;Charge (DAC);Time (#mus)", irow));
    gAvToTstages[irow]->Write();
    mgAvToTS->Add(gAvToTstages[irow], "pl");
    leg2->AddEntry(gAvToTstages[irow], Form("Stage %i", irow), "lp");

    gAvTimeWalkstages[irow] = new TGraphErrors(n_bins_y, charge, av_tw_stages[irow], 0, av_tot_stages_err[irow]);
    //gAvTimeWalkstages[irow] = new TGraph(n_bins_y, charge, av_tw_stages[irow]); 
    gAvTimeWalkstages[irow]->SetLineColor(col[irow%32]);
    gAvTimeWalkstages[irow]->SetMarkerColor(col[irow%32]);
    gAvTimeWalkstages[irow]->SetMarkerStyle(20);
    gAvTimeWalkstages[irow]->SetMarkerSize(0.6);
    gAvTimeWalkstages[irow]->SetName(Form("gAvTimeWalk_stage%i", irow)); 
    gAvTimeWalkstages[irow]->SetTitle(Form("gAvTimeWalk_stage%i;Charge (DAC);Time (#mus)", irow));
    gAvTimeWalkstages[irow]->Write();
    mgAvTimeWalkS->Add(gAvTimeWalkstages[irow], "pl");

    gAvTimeWalkAboveThrstages[irow] = new TGraphErrors(n_bins_y, charge, av_tw_abovethr_stages[irow], 0, av_tw_abovethr_stages_err[irow]);
    //gAvTimeWalkAboveThrstages[irow] = new TGraph(n_bins_y, charge, av_tw_abovethr_stages[irow]); 
    gAvTimeWalkAboveThrstages[irow]->SetLineColor(col[irow%32]);
    gAvTimeWalkAboveThrstages[irow]->SetMarkerColor(col[irow%32]);
    gAvTimeWalkAboveThrstages[irow]->SetMarkerStyle(20);
    gAvTimeWalkAboveThrstages[irow]->SetMarkerSize(0.6);
    gAvTimeWalkAboveThrstages[irow]->SetName(Form("gAvTimeWalkAbove_stage%i", irow)); 
    gAvTimeWalkAboveThrstages[irow]->SetTitle(Form("gAvTimeWalkAbove_stage%i;Charge (DAC);Time (#mus)", irow));
    gAvTimeWalkAboveThrstages[irow]->Write();
    mgAvTimeWalkAboveThrS->Add(gAvTimeWalkAboveThrstages[irow], "pl");
  }

  f_out_root->cd();

  mgAvToT->Write("mgAvToT");
  mgAvTimeWalk->Write("mgAvTimeWalk");
  mgAvTimeWalkAboveThr->Write("mgAvTimeWalkAboveThr");

  mgAvToTS->Write("mgAvToTstages");
  mgAvTimeWalkS->Write("mgAvTimeWalkS");
  mgAvTimeWalkAboveThrS->Write("mgAvTimeWalkAboveThrS");




  // Comparison of different Slices:
  //=================================================================================================
  TDirectory **subdir_slices;
  subdir_slices = new TDirectory*[nSlices];
  for (Int_t iSlice = 0; iSlice < nSlices; iSlice++) {
    subdir_slices[iSlice] = f_out_root->mkdir(Form("SliceAT_%d", slicesAT[iSlice]));
    f_out_root->cd(Form("SliceAT_%d", slicesAT[iSlice]));
    for (Int_t i_sec=0; i_sec<NSEC; i_sec++) {
      hSlice[iSlice][i_sec]->Write();
    }
    hSliceAll[iSlice]->Write(); 
  }

  f_out_root->Close();
  cout << "Analysis done: root file written." << endl;
  return kTRUE;
}
