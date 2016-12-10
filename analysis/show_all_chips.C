#include "Riostream.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TString.h"

void show_all_chips() {

    TString prefix = "/home/palpidefs/MOSAIC/new-alpide-software/Data/na61_161210_025127/results/analysis_basic_";


    TH2F *mapall = new TH2F("mapall", "IB HIC", 1024*9, -0.5, 1024*9-0.5, 512, -0.5, 512);
    //mapall->SetStats(0);

    for(Int_t i=0; i<9; ++i) {
        TString fname = prefix; fname += i; fname += ".root";

        TFile *file = new TFile(fname.Data());
        
        TH2F *h = (TH2F*)file->Get("hCluHits");

        for(Int_t icol=0; icol<1024; icol++) {
            for(Int_t irow=0; irow<512; irow++) {
                mapall->SetBinContent(i*1024+icol+1, irow+1, h->GetBinContent(icol+1, irow+1));
            }
        }

        file->Close();
    }

    TCanvas *c1 = new TCanvas("c1", "c1", 50, 50, 1900, 600);
    c1->Divide(1,2);
    c1->cd(1);
    mapall->DrawCopy("COLZ");
    c1->cd(2);
    mapall->ProjectionX()->DrawCopy();
}
