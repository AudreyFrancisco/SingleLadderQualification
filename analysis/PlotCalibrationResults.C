#include "TArrow.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TROOT.h"
#include <iostream>
#include <math.h>
#include <string>

// WARNING:  CURRENTLY INCOMPLETE!!  Potential issue with scope+MakeGraphErrors result...

void fillData(float *ithr, float *RMS, float *actualRMS, int target, int Chips, char *filePrefix)
{                 // Blatantly copied from test_ITHR
  int   old_vcas; // only voltage+noise are used for now; the rest may be used later
  int   old_ith;
  int   goodPixels;
  float current;
  float currentRMS;
  float noise;
  float noiseRMS;
  char  name[100];
  float rmsSum    = 0;
  int   deadChips = 0;

  std::cout << "Filling Ithr" << std::endl;
  for (int i = 0; i < Chips; i++) {
    // get file; name of one of them is in summaryName.
    // Only use first 34 chars of summaryName; insert chip # right after.
    sprintf(name, "%s%i_0.dat", filePrefix, i);
    FILE *fp = fopen(name, "r");
    // load file into array
    if (fp) {
      fscanf(fp, "%i %i %i %f %f %f %f", &old_vcas, &old_ith, &goodPixels, &current, &currentRMS,
             &noise, &noiseRMS);
      ithr[i] = noise;    // current;
      RMS[i]  = noiseRMS; // currentRMS;
      rmsSum += (current - target) * (current - target);
      fclose(fp);
      std::cout << "Read " << name << std::endl;
    }
    else {
      std::cout << "Unable to open file " << name << std::endl;
      ithr[i] = 0;
      RMS[i]  = 0;
      deadChips++;
    }
  }
  rmsSum /= (Chips - deadChips);
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

// Must give PlotCal one of the summary file names AND the total number of chips
//  (including inactive ones)
// Missing chips are mapped to 0.
// Give 2 file names to plot ITHR and VCASN cal results.
// Chips=number of chips, target=ideal threshold value (in electrons)
void PlotCalibrationResults(int Chips, int target, char *fileName1, char *fileName2 = NULL)
{
  // NOTE:  first filename plotted in red, second in blue

  /*float *ithr = new float[Chips];
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
  cal.Print("ITHRorVCASN.pdf");*/

  if (!fileName2) { // VCASN or ITHR
    /*TGraphErrors * cal1=NULL;
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
    cal1->Print("ITHRorVCASN.pdf");*/

    TGraphErrors *cal1 = NULL;

    // cal1
    float *thr1      = new float[Chips];
    float *RMS1      = new float[Chips];
    float *chipNums1 = new float[Chips];
    char * filePrefix1;                           // must be null-terminated...
    int    length1 = strcspn(fileName1, "C") + 4; // add 4 to include "Chip"
    filePrefix1    = new char[length1 + 1];
    for (int i = 0; i < length1; i++) {
      filePrefix1[i] = fileName1[i];
    }
    char nul             = '\0';
    filePrefix1[length1] = nul;

    float *actualRMS = new float;
    int    targetRMS = 100;
    fillData(thr1, RMS1, actualRMS, targetRMS, Chips, filePrefix1);
    std::cout << "thr1 filled" << std::endl;
    for (int i = 0; i < Chips; i++) { // xvals
      chipNums1[i] = i;
    }
    cal1 = new TGraphErrors(Chips, chipNums1, thr1, NULL, RMS1);
    std::cout << "cal1 created" << std::endl;


    // Generating plot
    std::string title = "Mean threshold, target=" + std::to_string(targetRMS) +
                        ", RMS error=" + std::to_string(*actualRMS) + "; Chip number; Threshold";
    std::cout << title << std::endl;
    const char *ttl = title.c_str();
    // cal1->SetTitle(ttl);
    cal1->SetMarkerStyle(21);
    cal1->SetMarkerColor(2);
    cal1->SetLineColor(2);
    auto axis1 = cal1->GetXaxis();
    axis1->SetLimits(-.5, 8.5);

    TMultiGraph *mg = new TMultiGraph();
    mg->Add(cal1);

    TCanvas *c1 = new TCanvas("c1", "multigraph", 400, 300);
    c1->SetGrid();

    mg->Draw("apl");
    // mg->GetXaxis()->SetTitle("Chip number");
    // mg->GetYaxis()->SetTitle("Threshold");
    mg->SetTitle(ttl);

    gPad->Update();
    gPad->Modified();
  }
  else { // first filename in red, second in blue
    TGraphErrors *cal1 = NULL;
    TGraphErrors *cal2 = NULL;
    // MakeGraphErrors(Chips, cal1, fileName1);  //trouble passing cal back...
    // MakeGraphErrors(Chips, cal2, fileName2);

    // cal1
    float *thr1      = new float[Chips];
    float *RMS1      = new float[Chips];
    float *chipNums1 = new float[Chips];
    char * filePrefix1;                           // must be null-terminated...
    int    length1 = strcspn(fileName1, "C") + 4; // add 4 to include "Chip"
    filePrefix1    = new char[length1 + 1];
    for (int i = 0; i < length1; i++) {
      filePrefix1[i] = fileName1[i];
    }
    char nul             = '\0';
    filePrefix1[length1] = nul;

    float *actualRMS = new float;
    int    targetRMS = 150;
    fillData(thr1, RMS1, actualRMS, targetRMS, Chips, filePrefix1);
    std::cout << "thr1 filled" << std::endl;
    for (int i = 0; i < Chips; i++) { // xvals
      chipNums1[i] = i;
    }
    cal1 = new TGraphErrors(Chips, chipNums1, thr1, NULL, RMS1);
    std::cout << "cal1 created" << std::endl;

    // cal2
    float *thr2      = new float[Chips];
    float *RMS2      = new float[Chips];
    float *chipNums2 = new float[Chips];
    char * filePrefix2;                           // must be null-terminated...
    int    length2 = strcspn(fileName2, "C") + 4; // add 4 to include "Chip"
    filePrefix2    = new char[length2 + 1];
    for (int i = 0; i < length2; i++) {
      filePrefix2[i] = fileName2[i];
    }
    // char nul = '\0';
    filePrefix2[length2] = nul;

    fillData(thr2, RMS2, actualRMS, targetRMS, Chips, filePrefix2);
    std::cout << "thr2 filled" << std::endl;
    for (int i = 0; i < Chips; i++) { // xvals
      chipNums2[i] = i;
    }
    cal2 = new TGraphErrors(Chips, chipNums2, thr2, NULL, RMS2);
    std::cout << "cal2 created" << std::endl;


    // Generating plot
    cal1->SetTitle("Mean threshold value per chip");
    cal2->SetTitle("Mean threshold value per chip");
    cal1->SetMarkerStyle(21);
    cal2->SetMarkerStyle(21);
    cal1->SetMarkerColor(2);
    cal2->SetMarkerColor(4);
    cal1->SetLineColor(2);
    cal2->SetLineColor(4);
    auto axis1 = cal1->GetXaxis();
    axis1->SetLimits(-.5, 8.5);
    auto axis2 = cal2->GetXaxis();
    axis2->SetLimits(-.5, 8.5);

    TMultiGraph *mg = new TMultiGraph();
    mg->Add(cal1);
    mg->Add(cal2);

    TCanvas *c1 = new TCanvas("c1", "multigraph", 700, 700);
    c1->SetGrid();

    mg->Draw("apl");
    mg->GetXaxis()->SetTitle("Chip number");
    mg->GetYaxis()->SetTitle("Threshold");

    gPad->Update();
    gPad->Modified();
  }
}
