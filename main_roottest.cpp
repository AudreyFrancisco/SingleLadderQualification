#include <iostream>

#include "TFile.h"
#include "TH1F.h"

// This macro is a simple-test executable to check the linking against ROOT
int main() {
  TFile* f = new TFile("test.root", "RECREATE");
  TH1F* h = new TH1F("h", "h", 100, 0., 1.);
  h->Fill(0.2, 50);
  h->Fill(0.3, 30);
  h->Fill(0.4, 20);
  h->Write();
  f->Close();
  delete f;
  f = 0x0;
}
