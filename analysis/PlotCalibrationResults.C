#include <iostream>
#include <string>
#include "TGraphErrors.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TArrow.h"
#include "TAxis.h"

void fillData(float *ithr, float *RMS, int Chips, char * filePrefix) { //Blatantly copied from test_ITHR
  int old_vcas; //only voltage+noise are used for now; the rest may be used later
  int old_ith;
  int goodPixels;
  float current;
  float currentRMS;
  float noise;
  float noiseRMS;
  char name[100];

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
      fclose(fp);
      std::cout << "Read " << name << std::endl;
    } else {
      std::cout << "Unable to open file " << name << std::endl;
      ithr[i]=0;
      RMS[i]=0;
    }
  }
}

//Must give PlotCal one of the summary file names AND the total number of chips
//  (including inactive ones)
//Missing chips are mapped to 0.
void PlotCalibrationResults(char * fileName, int Chips=0) {
  if(Chips==0) {
    std::cout << "Missing number of chips parameter" << std::endl;
    return;
  }
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
  filePrefix[length+1]=nul;

  fillData(ithr,RMS, Chips, filePrefix);
  for(int i=0; i<Chips; i++) { //xvals
    chipNums[i]=i;
  }

  TGraphErrors cal(Chips, chipNums, ithr, NULL, RMS);
  cal.SetTitle("Mean VCASN threshold value per chip, with errors");
  cal.SetMarkerStyle(21);
  cal.SetMarkerColor(4);
  cal.SetLineColor(4);
  auto axis = cal.GetXaxis();
  axis->SetLimits(-.5,9);
  auto can = new TCanvas();
  cal.DrawClone("APE");

  cal.Print("ITHRorVCASN.pdf");

}


