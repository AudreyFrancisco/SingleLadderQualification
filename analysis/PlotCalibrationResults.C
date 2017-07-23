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

void fillData(float *ithr, float *RMS, float *actualRMS, int target, int Chips, char * filePrefix) { //Blatantly copied from test_ITHR
  int old_vcas; //only voltage+noise are used for now; the rest may be used later
  int old_ith;
  int goodPixels;
  float current;
  float currentRMS;
  float noise;
  float noiseRMS;
  char name[100];
  float rmsSum=0;
  int deadChips=0;

  std::cout << "Filling Ithr" << std::endl;
  for(int i = 0; i < Chips; i++) {
    //get file; name of one of them is in summaryName.
    //Only use first 34 chars of summaryName; insert chip # right after.
    sprintf(name, "%s%i_0.dat", filePrefix, i);
    FILE *fp = fopen(name, "r");
    //load file into array
    if(fp) {
      fscanf(fp, "%i %i %i %f %f %f %f", &old_vcas, &old_ith, &goodPixels, &current,
              &currentRMS, &noise, &noiseRMS);
      ithr[i]=current;
      RMS[i]=currentRMS;
      rmsSum += (current-target)*(current-target);
      fclose(fp);
      std::cout << "Read " << name << std::endl;
    } else {
      std::cout << "Unable to open file " << name << std::endl;
      ithr[i]=0;
      RMS[i]=0;
      deadChips++;
    }
  }
  rmsSum /= (Chips-deadChips);
  *actualRMS = sqrt(rmsSum);
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

//Must give PlotCal one of the summary file names AND the total number of chips
//  (including inactive ones)
//Missing chips are mapped to 0.
//Give 2 file names to plot ITHR and VCASN cal results.
//Chips=number of chips, target=ideal threshold value (in electrons)
void PlotCalibrationResults(int Chips, int target, char * fileName1, char * fileName2=NULL) {
  //NOTE:  first filename plotted in red, second in blue
  
  float *ithr = new float[Chips];
  float *RMS = new float[Chips];
  float *chipNums = new float[Chips];
  float * actualRMS = new float;
  char * filePrefix; //must be null-terminated...
  int length = strcspn(fileName1, "C")+4; //add 4 to include "Chip"
  filePrefix = new char[length+1];
  for(int i=0; i<length; i++) {
    filePrefix[i]=fileName1[i];
  }
  char nul = '\0';
  filePrefix[length]=nul;
  *actualRMS=1000;
  fillData(ithr, RMS, actualRMS, target, Chips, filePrefix);
  std::cout << "Data filled" << std::endl;
  for(int i=0; i<Chips; i++) { //xvals
    chipNums[i]=i;
  }
  std::cout << "Making graph" << std::endl;
  TGraphErrors cal(Chips, chipNums, ithr, NULL, RMS);
  char graphName[100];
  std::cout << "Printing title, " << target << ", " << *actualRMS << std::endl;
  sprintf(graphName, "Mean threshold per chip, target=%i, RMS error=%f", target, *actualRMS);
  std::cout << "Title printed" << std::endl;
  cal.SetTitle(graphName);
  cal.SetMarkerStyle(21);
  cal.SetMarkerColor(4);
  cal.SetLineColor(4);
  auto axis = cal.GetXaxis();
  axis->SetLimits(-.5,8.5);
  auto can = new TCanvas();
  cal.DrawClone("APE");
  cal.Print("ITHRorVCASN.pdf");
  
  /*if(!fileName2) { //VCASN or ITHR
    TGraphErrors * cal1=NULL;
    MakeGraphErrors(Chips, cal1, fileName1);
    if(cal1) {
      std::cout << "Constructed" << std::endl;
    }
    cal1->SetTitle("Mean threshold value per chip");
    std::cout << "Title" << std::endl;
    cal1->SetMarkerStyle(21);
    cal1->SetMarkerColor(4);
    cal1->SetLineColor(4);
    auto axis = cal1->GetXaxis();
    axis->SetLimits(-.5,8.5);
    std::cout << "GraphErrors set" << std::endl;
    
    auto can = new TCanvas();
    cal1->DrawClone("APE");
    cal1->Print("ITHRorVCASN.pdf");

  } else {  //first filename in red, second in blue
    TGraphErrors * cal1=NULL;
    TGraphErrors * cal2=NULL;
    MakeGraphErrors(Chips, cal1, fileName1);
    MakeGraphErrors(Chips, cal2, fileName2);
    cal1->SetTitle("Mean threshold value per chip");
    cal2->SetTitle("Mean threshold value per chip");
    cal1->SetMarkerStyle(21);
    cal2->SetMarkerStyle(21);
    cal1->SetMarkerColor(2);
    cal2->SetMarkerColor(4);
    cal1->SetLineColor(2);
    cal2->SetLineColor(4);
    auto axis1 = cal1->GetXaxis();
    axis1->SetLimits(-.5,8.5);
    auto axis2 = cal2->GetXaxis();
    axis2->SetLimits(-.5,8.5);

    TMultiGraph *mg = new TMultiGraph();
    mg->Add(cal1);
    mg->Add(cal2);

    TCanvas *c1 = new TCanvas("c1","multigraph",400,300);
    c1->SetGrid();

    mg->Draw("apl");
    mg->GetXaxis()->SetTitle("Chip number");
    mg->GetYaxis()->SetTitle("Threshold");

    gPad->Update();
    gPad->Modified();
  }*/
}
