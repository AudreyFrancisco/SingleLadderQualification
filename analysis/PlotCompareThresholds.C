#include <iostream>
#include <string>
#include <math.h>
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TArrow.h"
#include "TAxis.h"

//WARNING:  CURRENTLY INCOMPLETE!!  Potential issue with scope+MakeGraphErrors result...

void fillData(int nPix, float * thr1, float * thr2, char * fileName1, char * fileName2, bool diff) { //Blatantly copied from test_ITHR
  int col1, row1, dc, adr, col2, row2, X; //only voltage+noise are used for now; the rest may be used later
  float current1, current2;
  float noise1, noise2;
  float dump1, dump2;
  //float ** th1 = new float[3][nPix]; //stores addr1, addr2, thr
  //float ** th2 = new float[3][nPix];
  for(int i=0; i<nPix; i++) {
    thr1[i]=thr2[i]=0;
  }

  FILE *fp1 = fopen(fileName1, "r");
  FILE *fp2 = fopen(fileName2, "r");
  if(!fp1) {
    std::cout << "FILE " << fileName1 << " not found!" << std::endl;
  } else if(!fp2) {
    std::cout << "FILE " << fileName2 << " not found!" << std::endl;
  } else {
    std::cout << "Filling arrays" << std::endl;
    if(diff) {
      for(int i = 0; i < nPix-1; i++) {
        fscanf(fp1, "%i %i %f %f %f", &col1, &row1, &current1, &noise1, &dump1);
        fscanf(fp2, "%i %i %f %f %f", &dc, &adr, &current2, &noise2, &dump2);
        thr1[i]=current1; //i=col1+1024*row1
        row2=adr/2;
        X=0;
        if(adr%4==1 || adr%4==2) X=1;
        col2=dc*2+X;
        std::cout << col2 << ", " << row2 << std::endl;
        thr2[col2+1024*row2]=current2;  //i=col+1024*row
        if(row2==99) {
          thr1[i]=299;
          thr2[col2+1024*row2]=299;
        }
      }
    } else {
      for(int i = 0; i < nPix-1; i++) {
        fscanf(fp1, "%i %i %f %f %f", &col1, &row1, &current1, &noise1, &dump1);
        fscanf(fp2, "%i %i %f %f %f", &dc, &adr, &current2, &noise2, &dump2);
        std::cout << current1 << ", " << noise2 << std::endl;
        thr1[i]=current1; //i=col1+1024*row1
        thr2[i]=current2;
      }
    }
  }
  std::cout << "finished" << std::endl;
  fclose(fp1);
  fclose(fp2);
}

/*void MakeGraphErrors(int Chips, TGraphErrors * cal_, char * fileName) {
  TGraphErrors * cal;
  float *ithr = new float[Chips];
  float *RMS = new float[Chips];
  float *chipNums = new float[Chips];
  char * filePrefix; //must be null-terminated...
  int length = strcspn(fileName, "C")+4; //add 4 to include "Chip"
  filePrefix = new char[length+1];
  for(int i=0; i<length; i++) {
    filePrefix[i]=fileName[i];
  }
  char nul = '\0';
  filePrefix[length]=nul;

  fillData(ithr,RMS, Chips, filePrefix);
  for(int i=0; i<Chips; i++) { //xvals
    chipNums[i]=i;
  }
  cal = new TGraphErrors(Chips, chipNums, ithr, NULL, RMS);
  cal_ = (TGraphErrors*)cal->Clone();
  if(cal_) { std::cout << "Cloned" << std::endl; }
}*/

/*void setFilePrefix(char * fileName, char * filePrefix, ) {
  int len = strcspn(fileName, ".");
  char * filePrefix;
  if(len<50) { //ThresholdSummary
    int length1 = strcspn(fileName, "C")+4; //add 4 to include "Chip"
    filePrefix = new char[length1+1];
    for(int i=0; i<length1; i++) {
      filePrefix[i]=fileName[i];
    }
    char nul = '\0';
    filePrefix[length1]=nul;
  } else { //TThresholdAnalysis
    int length1 = strcspn(fileName, "x");
  }
}*/

//Must give PlotCal one of the summary file names AND the total number of chips
//  (including inactive ones)
//Missing chips are mapped to 0.
//Give 2 file names to plot ITHR and VCASN cal results.
//Chips=number of chips, target=ideal threshold value (in electrons)
void PlotCompareThresholds(char * fileName1, char * fileName2) {
  //Only accepts results from one chip at a time.
  //First file must be from test_scantest; the second must be from fitThreshold/etc.
  
  TGraphErrors * cal1 = NULL;

  int nPix = 10*1024; //99*1024;
  float *thr1 = new float[nPix]; //10 mask stages, 1024 pixels each
  float *thr2 = new float[nPix];

  fillData(nPix, thr1, thr2, fileName1, fileName2, true);
  //fillData(nPix, thr2, fileName2);

  if(!thr1 || !thr2) std::cout << "thr MISSING" << std::endl;
  std::cout << "Making graphErrors, nPix=" << nPix << std::endl;
  cal1 = new TGraphErrors(nPix, thr1, thr2, NULL, NULL); //no error bars for now
  //cal1 = new TGraphErrors(Chips, chipNums1, thr1, NULL, RMS1);
  std::cout << "cal1 created" << std::endl;

  //Generating plot
  cal1->SetTitle("Threshold of 1 vs threshold of 2");
  cal1->SetMarkerStyle(21);
  cal1->SetMarkerColor(4);
  cal1->SetMarkerSize(1);
  //cal1->SetLineWidth(0.1);
  //cal1->SetLineColor(2);
  TCanvas *c1 = new TCanvas("c1","grapherrors",700,700);
  c1->SetGrid();

  //cal1->Draw("apl");
  cal1->GetXaxis()->SetTitle("Threshold 1");
  cal1->GetYaxis()->SetTitle("Threshold 2");
  auto axis1 = cal1->GetYaxis();
  axis1->SetLimits(-10,300); //was -10 to 300
  cal1->SetMinimum(-10);
  cal1->SetMaximum(300);
  gPad->Update();
  gPad->Modified();
  auto axis2 = cal1->GetXaxis();
  axis2->SetLimits(-10,300);
  cal1->Draw("apl");
  gPad->Update();
  gPad->Modified();
}


