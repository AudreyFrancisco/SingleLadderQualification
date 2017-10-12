#include <iostream>
#include <iomanip>

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"


void analyse_DAC(TString inputFile, TString DAC, Int_t chip = -1) {
  TTree* t = 0x0;
  TFile* f_in = new TFile(inputFile.Data());
  gDirectory->GetObject("DACsummary", t);
  if (!t) {
    std::cerr << "Could not find the TTree \"DACsummary\" in the input file "
              << inputFile.Data() << "!" << std::endl;
  }
  //std::cout << DAC << std::endl;

  TString draw_str = "Voltage:MeasValue";
  TString cut_str  = "(SetValue==200)";
  cut_str += TString::Format("&&(DAC==\"%s\")", DAC.Data());

  float expected_value_base = 200./255.;

  std::cout << std::setprecision(3);

  std::cout << "#Voltage\tExpecteValue\tMeasMean\tMeasRMS\tDevMean\tDevMean (%)\tMeasured Voltage\tMeasured Deviation" << std::endl;

  for (float voltage=1.30; voltage<2.00; voltage+=0.02) {
    float mean = 0.;
    float rms  = 0.;
    float expected_value = expected_value_base * voltage;
    for (unsigned int i_chip=0; i_chip<9; ++i_chip) {
      TString cut_str_mod = cut_str;
      cut_str_mod += TString::Format("&&(Chip==%d)", i_chip);
      cut_str_mod += TString::Format("&&(Voltage>%0.3f)&&(Voltage<%0.3f)", voltage-0.001, voltage+0.001);
      ULong64_t n = t->Draw(draw_str.Data(), cut_str_mod.Data(), "goff");
      if (n==0) continue;
      float meas_value = t->GetV2()[0];
      float dev        = expected_value - meas_value;
      //std::cout << voltage << "\t" << i_chip << "\t" << expected_value << "\t" << meas_value << "\t" << dev << "\t" << dev/expected_value*100. << std::endl;
      mean += meas_value/9.;
      rms  += meas_value/9. * meas_value;
   }
   rms = TMath::Sqrt(rms-mean*mean);
   float measured_voltage = mean/expected_value_base;
   std::cout << voltage << "\t" << expected_value << "\t" << mean << "\t" << rms
             << "\t" << expected_value-mean << "\t" << (expected_value-mean)/expected_value*100. << "\t"
   << measured_voltage << "\t" << measured_voltage - voltage << std::endl;
 }
}
