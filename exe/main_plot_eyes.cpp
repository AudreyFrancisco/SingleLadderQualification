#include <iostream>
#include <map>
#include <regex>

#include "TCanvas.h"
#include "TFile.h"
#include "TH2.h"
#include "TList.h"
#include "TSystemDirectory.h"

#include "TEyeAnalysis.h"

void print_usage()
{
  std::cout << "Usage:" << std::endl;
  std::cout << "  plot_eyes <HIC directory>" << std::endl;
  std::cout << "  No trailing slash!" << std::endl;
}

int main(int argc, char **argv)
{
  if (argc != 2) {
    print_usage();
    exit(-1);
  }

  std::string directory = argv[1];
  std::string hicName   = directory.substr(directory.find_last_of('/') + 1);
  hicName               = hicName.substr(0, hicName.find('_'));

  TSystemDirectory hicDir("hicdir", directory.c_str());
  std::regex       eyeRegex(R"~(eye_(D[0-9]+_P[0-9]+)__?([0-9]+_[0-9]+)\.root)~");
  std::smatch      m;
  std::map<std::string, std::pair<std::string, std::string>> eyeFileNames;

  for (auto file : *hicDir.GetListOfFiles()) {
    std::string filename = file->GetName();
    std::regex_search(filename, m, eyeRegex);

    if (m.size() == 3) {
      if (eyeFileNames.count(m.str(1)) == 0 || eyeFileNames[m.str(1)].first < m.str(2))
        eyeFileNames[m.str(1)] = std::make_pair(m.str(2), m.str(0));
    }
  }

  std::map<std::string, TFile *> eyeFiles;
  for (auto kv : eyeFileNames) {
    eyeFiles[kv.first] = TFile::Open((directory + "/" + kv.second.second).c_str(), "READ");
    std::cout << kv.first << ": " << kv.second.second << std::endl;
  }

  std::string fileNameSummary = directory + "/eye_summary.pdf";
  TCanvas     c;
  c.Print((fileNameSummary + "[").c_str());
  c.Divide(9, eyeFiles.size());
  int j = -1;
  for (auto kv : eyeFiles) {
    ++j;
    for (int ichip = 0; ichip < 9; ++ichip) {
      c.cd(j * 9 + ichip + 1);
      std::string hname = kv.first;
      std::transform(hname.begin(), hname.end(), hname.begin(),
                     [](unsigned char c) { return std::tolower(c); });
      hname = hicName + "/h_eye_" + std::to_string(ichip) + "_" + hname;
      // std::cout << hname << std::endl;
      TH2 *h = (TH2 *)kv.second->Get(hname.c_str());
      if (!h) continue;
      h->Draw("colz");
      gPad->SetLogz();
    }
  }

  c.Print(fileNameSummary.c_str());

  c.cd();
  for (auto kv : eyeFiles) {
    for (int ichip = 0; ichip < 9; ++ichip) {
      std::string hname = kv.first;
      std::transform(hname.begin(), hname.end(), hname.begin(),
                     [](unsigned char c) { return std::tolower(c); });
      hname  = hicName + "/h_eye_" + std::to_string(ichip) + "_" + hname;
      TH2 *h = (TH2 *)kv.second->Get(hname.c_str());
      if (!h) continue;
      TEyeAnalysis::PlotHisto(*gPad, *h, fileNameSummary);
    }
  }

  c.Print((fileNameSummary + "]").c_str());

  for (auto kv : eyeFiles) {
    delete kv.second;
  }

  return 0;
}
