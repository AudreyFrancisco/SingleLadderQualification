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

using namespace std;

#define NSEC 1

int nInj       = 50;

double maxchi2 = 5;

float data[256];
float x   [256];
int NPoints;

char fNameOut [50];
FILE *fpOut;

//float ELECTRONS_PER_DAC = 7. *226/160; 

int NPixels;

int NNostart;
int NChisq;

TH1F *hThresh[NSEC]={0};
TH1F *hNoise [NSEC]={0};
TH1F *hChi2=0;

void PrepareHistos() {
  for (int isec=0;isec<NSEC;++isec) {
    delete hThresh[isec];
    hThresh[isec]=new TH1F(Form("hThresh%d",isec),Form("Threshold, sector %d",isec),125,0.,500.);
    delete hNoise[isec];
    hNoise [isec]=new TH1F(Form("hNoise%d" ,isec),Form("Noise, sector %d"    ,isec),60,0., 30.);
  }
  delete hChi2;hChi2=0;
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
   TF1    *fitfcn = new TF1("fitfcn",erf,0,1500,2);
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
  int isec= 0;

  if (!GetThreshold(&thresh,&noise,&chi2)) return;

  if (fpOut)
    fprintf(fpOut, "%d %d %.1f %.1f %.2f\n", col, row, thresh, noise, chi2);
  //std::cout << "Thresh: " << thresh << std::endl;
  if(chi2<maxchi2){
    hThresh[isec]->Fill(thresh);
    hNoise [isec]->Fill(noise );
  }
}



void ProcessFile (const char *fName) {
  FILE *fp = fopen (fName, "r");
  int col, address, ampl, hits;
  int lastcol = -1, lastaddress = -1;
  NPoints  = 0;
  NPixels  = 0;
  NNostart = 0;
  NChisq   = 0;

  //printf("strstr result: %s\n", strstr(
  sprintf(fNameOut, "FitValues%s", strstr(fName,"_"));
  //printf("Output file: %s\n", fNameOut);
  fpOut = fopen(fNameOut, "w");

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
    x    [NPoints] = (float)ampl;  // * ELECTRONS_PER_DAC;
    NPoints ++;
  }
  fclose(fp);
  if (fpOut) fclose(fpOut);
}


int FitThresholdTuneVCASNIB(const char *fName, bool WriteToFile, int ITH, int VCASN, bool saveCanvas) {
  PrepareHistos();
  ProcessFile(fName);

  std::cout << "Found " << NPixels << " pixels, i.e." << 524288 - NPixels << " pixels have no hits." << std::endl;
  std::cout << "No start point found: " << NNostart << std::endl;
  std::cout << "Chisq cut failed:     " << NChisq << std::endl;
  std::cout << "Chisq cut value:     " << maxchi2 << std::endl;

  int GoodPixels = NPixels - NNostart - NChisq;
  //  hThresh->Draw();
  for (int isec=0;isec<NSEC;++isec) {
    hThresh[isec]->SetLineColor(1+isec);
    hNoise [isec]->SetLineColor(1+isec);
  }

//  hThresh1->SetMaximum(150);
//  hNoise1->SetMaximum(500);
  if (!WriteToFile) {
    TCanvas* c = new TCanvas;
    hThresh[0]->Draw();
    for (int isec=1;isec<NSEC;++isec) {
      hThresh[isec]->Draw("SAME");
    }

    TCanvas* c2 = new TCanvas;
    hNoise [0]->Draw();
    for (int isec=1;isec<NSEC;++isec) {
      hNoise [isec]->Draw("SAME");
    }

    if (saveCanvas)
      c->SaveAs("threshold_scan.png");
  }
  for (int isec=0;isec<NSEC;++isec)
    std::cout << "Threshold sector "<<isec<<": " << hThresh[isec]->GetMean() << " +- " << hThresh[isec]->GetRMS() << " ( " << hThresh[isec]->GetEntries() << " entries)" << std::endl;
 
  for (int isec=0;isec<NSEC;++isec)
    std::cout << "Noise sector "<<isec<<":     " << hNoise [isec]->GetMean() << " +- " << hNoise [isec]->GetRMS() << std::endl;

  if (WriteToFile) {
      char summary[50];
      sprintf(summary, "ThresholdSummary%s", strstr(fName,"_"));
      FILE *fp = fopen(summary, "a");

      fprintf(fp, "%d %d %d %.1f %.1f %.1f %.1f\n", ITH, VCASN, GoodPixels, hThresh[0]->GetMean(), hThresh[0]->GetRMS(), 
	  hNoise[0]->GetMean(), hNoise[0]->GetRMS());
      fclose(fp);
  }


  //hChisq->Draw();
  return 0;
}
