#include "TH1F.h"
#include "TF1.h"
#include "TMath.h"
#include "TGraph.h"
#include "TString.h"
#include "TCanvas.h"
#include "FitThresholds.h"

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;


int nInj       = 50;

double maxchi2 = 5;

float data[256];
float x   [256];
int NPoints;

FILE *fpOut;

float ELECTRONS_PER_DAC = 10.;

int NPixels;

int NNostart;
int NChisq;

TH1F *hThresh=0;
TH1F *hNoise =0;
TH1F *hChi2=0;

void PrepareHistos() {
  hThresh=new TH1F("hThresh","Threshold",125,0.,500.);
  hNoise =new TH1F("hNoise" ,"Noise"    ,60, 0., 30.);
  hChi2=new TH1F("hChi2","Chi square distribution",1000,0.,100.);
}

void ResetData() {
  for (int i=0; i <= 256; i++) {
    data[i] = 0;
  }
}


Double_t erf( Double_t *xx, Double_t *par){
  return (nInj / 2) *TMath::Erf((xx[0] - par[0]) / (sqrt(2) *par[1])) +(nInj / 2);
}


float FindStart () {
  float Upper = -1;
  float Lower = -1;

  for (int i = 0; i < NPoints; i ++) {
    if (data[i] == nInj) {
      Upper = (float) x[i];
      break;
    }
  }
  if (Upper == -1) return -1;
  for (int i = NPoints-1; i > 0; i--) {
    if (data[i] == 0) {
      Lower = (float)x[i];
      break;
    }
  }
  if ((Lower == -1) || (Upper < Lower)) return -1;
  return (Upper + Lower)/2;
}


bool GetThreshold(double *thresh,double *noise,double *chi2) {
  TGraph *g      = new TGraph(NPoints, x, data);
  TF1    *fitfcn = new TF1("fitfcn", erf,0,1500,2);
  double Start  = FindStart();

  if (Start < 0) {
    NNostart ++;
    return false;
  }

  fitfcn->SetParameter(0,Start);

  fitfcn->SetParameter(1,8);

  fitfcn->SetParName(0, "Threshold");
  fitfcn->SetParName(1, "Noise");

  //g->SetMarkerStyle(20);
  //g->Draw("AP");
  g->Fit("fitfcn","Q");

  *noise =fitfcn->GetParameter(1);
  *thresh=fitfcn->GetParameter(0);
  *chi2  =fitfcn->GetChisquare()/fitfcn->GetNDF();

  hChi2->Fill(*chi2);

  g->Delete();
  fitfcn->Delete();
  return true;
}


void ProcessPixel (int col, int row) {
  double thresh,noise,chi2;

  if (!GetThreshold(&thresh,&noise,&chi2)) return;

  if (fpOut)
    fprintf(fpOut, "%d %d %.1f %.1f %.2f\n", col, row, thresh, noise, chi2);

  if(chi2<maxchi2){
    hThresh->Fill(thresh);
    hNoise ->Fill(noise );
  }
}



void ProcessFile (const char *fName) {
  std::cout << fName << std::endl;
  FILE *fp = fopen (fName, "r");
  int col, address, ampl, hits;
  int lastcol = -1, lastaddress = -1;
  NPoints  = 0;
  NPixels  = 0;
  NNostart = 0;
  NChisq   = 0;

  std::string fNameOut = fName;
  fNameOut.insert(fNameOut.rfind("ThresholdScan_"), "FitValues_");
  fNameOut.erase(fNameOut.rfind("ThresholdScan_"), 14);
  fpOut = fopen(fNameOut.c_str(), "w");

  ResetData();
  while ((fscanf (fp, "%d %d %d %d", &col, &address, &ampl, &hits) == 4)) {

    //if ((col < 255) || ((col == 255) && (address < 280))) continue;

    if (((lastcol != col) || (address != lastaddress)) && (NPoints!= 0)) {
      ProcessPixel(lastcol, lastaddress);
      NPixels ++;
      ResetData   ();
      NPoints  = 0;
    }

    lastcol = col;
    lastaddress = address;
    data [NPoints] = (float)hits;
    x    [NPoints] = (float)ampl * ELECTRONS_PER_DAC;
    NPoints ++;
  }
  fclose(fp);
  if (fpOut) fclose(fpOut);
}


int FitThresholds(const char *fName, bool WriteToFile, int ITH, int VCASN, bool saveCanvas) {
  PrepareHistos();
  std::cout << "Histos prepared" << std::endl;
  ProcessFile(fName);

  std::cout << "Found " << NPixels << " pixels, i.e." << 524288 - NPixels << " pixels have no hits." << std::endl;
  std::cout << "No start point found: " << NNostart << std::endl;
  std::cout << "Chisq cut failed:     " << NChisq << std::endl;
  std::cout << "Chisq cut value:     " << maxchi2 << std::endl;

  int GoodPixels = NPixels - NNostart - NChisq;
  //  hThresh->Draw();
  hThresh->SetLineColor(1);
  hNoise ->SetLineColor(1);

  if (!WriteToFile) {
    TCanvas* c = new TCanvas;
    hThresh->Draw();
    TCanvas* c2 = new TCanvas;
    hNoise->Draw();

    if (saveCanvas)
      c->SaveAs("threshold_scan.png");
  }
  std::cout << "Threshold: " << hThresh->GetMean() << " +- " << hThresh->GetRMS() << " ( " << hThresh->GetEntries() << " entries)" << std::endl;

  std::cout << "Noise:     " << hNoise ->GetMean() << " +- " << hNoise ->GetRMS() << std::endl;

  if (true) { //if WriteToFile
    std::string fSummary = fName;
    fSummary.insert(fSummary.rfind("ThresholdScan_"), "ThresholdSummary_");
    fSummary.erase(fSummary.rfind("ThresholdScan_"), 14);
    std::cout << "Summary file " << fSummary << std::endl;
    FILE *fp = fopen(fSummary.c_str(), "a");
    fprintf(fp, "%d %d %d %.1f %.1f %.1f %.1f\n", ITH, VCASN, GoodPixels, hThresh->GetMean(), hThresh->GetRMS(),
            hNoise->GetMean(), hNoise->GetRMS());
    fclose(fp);
  }


  //hChisq->Draw();
  return 0;
}
