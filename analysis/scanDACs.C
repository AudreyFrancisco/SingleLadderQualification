/* Written by Miljenko Suljic, m.suljic@cern.ch */

#include <iostream>

#include <Riostream.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TGraph.h>
#include <TString.h>

Bool_t scanDACs(const char *fName, Int_t n_bits = 256)
{

  std::string directory = fName;
  directory.erase(directory.rfind("DAC") - 1);
  std::string time = fName;
  time.erase(time.rfind(".dat"));
  //  std::string chipid = time;
  //  int         pos    = chipid.rfind("p") + 1;
  //  chipid.erase(0, pos);
  //  chipid.erase(chipid.rfind("_0"));
  time.erase(time.begin(), time.end() - 13);
  //  const Int_t   ChipId            = stoi(chipid, nullptr, 16);
  const Int_t   n_dacs           = 14;
  const TString filename[n_dacs] = {"VRESETP", "VRESETD", "VCASP", "VCASN", "VPULSEH",
                                    "VPULSEL", "VCASN2",  "VCLIP", "VTEMP", "IAUX2",
                                    "IRESET",  "IDB",     "IBIAS", "ITHR"};

  TFile *fl = new TFile(Form("%sScanDAC_%s.root", directory.c_str(), time.c_str()), "RECREATE");
  std::vector<std::vector<TGraph *>> hlist(128);
  for (int ich = 0; ich < 128; ich++) {
    hlist[ich] = std::vector<TGraph *>(n_dacs);
    for (int idac = 0; idac < n_dacs; idac++) {
      hlist[ich][idac] = new TGraph();
    }
  }

  for (Int_t ChipId = 16; ChipId < 128; ChipId++) {
    for (Int_t i = 0; i < n_dacs; ++i) {
      Float_t *dac = new Float_t[n_bits];
      Float_t *adc = new Float_t[n_bits];
      TString  filepath;
      filepath += directory + (filename[i].BeginsWith("V") ? "VDAC_" : "IDAC_") + filename[i] +
                  "_Chip" + std::to_string(ChipId) + "_0_" + time + ".dat";
      std::cout << "open file " << filepath << std::endl;
      TF1 *    f = new TF1("f", "pol1", 2., 150.); // filename[i].BeginsWith("V") ? 150. : 14.);
      ifstream dacfile(filepath.Data());
      if (!dacfile.good()) {
        cout << "Cannot find " << filename[i] << "(" << filepath << ")" << endl;
      }
      if (dacfile.good()) {
        for (Int_t j = 0; j < n_bits; ++j) {
          dacfile >> dac[j] >> adc[j];
        }
        hlist[ChipId][i] = new TGraph(n_bits, dac, adc);
        hlist[ChipId][i]->SetTitle(
            "chip_" + std::to_string(ChipId) + " " + filename[i] +
            (filename[i].BeginsWith("V") ? ";DAC;Voltage [V]" : ";DAC;Current [nA]"));
        hlist[ChipId][i]->SetName("Chip " + std::to_string(ChipId) + " " + filename[i]);
        hlist[ChipId][i]->Write();
        TFitResultPtr r = hlist[ChipId][i]->Fit("f", "QSR");
        std::cout << filename[i] << '\t' << r->Value(0) << '\t' << r->Value(1) << std::endl;
      }
      else
        hlist[ChipId][i] = 0;
    }
  }
  fl->Write();
  delete fl;

  cout << "Done!" << endl;
  return kTRUE;
}
