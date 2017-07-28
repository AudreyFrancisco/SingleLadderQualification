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

void fillData(int nPix, float * thr, char * fileName) { //Blatantly copied from test_ITHR
  int addr1; //only voltage+noise are used for now; the rest may be used later
  int addr2;
  float current;
  float noise;
  float dump;

  FILE *fp = fopen(fileName, "r");
  if(!fp) {
    std::cout << "FILE " << fileName << " not found!" << std::endl;
  } else {
    std::cout << "Filling from " << fileName << std::endl;
    for(int i = 0; i < nPix; i++) {
      fscanf(fp, "%i %i %f %f %f", &addr1, &addr2, &current, &noise, &dump);
      thr[i]=current;
      //std::cout << current << std::endl;
      //RMS[i]=currentRMS;
    }
    fclose(fp);
  }
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
  
  TGraphErrors * cal1 = NULL;

  int nPix = 10*1024;
  float *thr1 = new float[nPix]; //10 mask stages, 1024 pixels each
  float *thr2 = new float[nPix];

  fillData(nPix, thr1, fileName1);
  fillData(nPix, thr2, fileName2);

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
  TCanvas *c1 = new TCanvas("c1","grapherrors",400,400);
  c1->SetGrid();

  cal1->Draw("apl");
  cal1->GetXaxis()->SetTitle("Threshold 1");
  cal1->GetYaxis()->SetTitle("Threshold 2");
  auto axis1 = cal1->GetYaxis();
  axis1->SetLimits(-20,300);
  auto axis2 = cal1->GetXaxis();
  axis2->SetLimits(-20,300);

  gPad->Update();
  gPad->Modified();
}


