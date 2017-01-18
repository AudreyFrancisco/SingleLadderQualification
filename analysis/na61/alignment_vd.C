/* Written by Miljenko Suljic, m.suljic@cern.ch */
#include "Riostream.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TString.h"
#include "TMath.h"

#include "../classes/helpers.cpp"
#include "../classes/BinaryEvent.hpp"
#include "../classes/Alignment.hpp"

Bool_t alignment_vd(
    const TString filepath_tree,  // input tree path
    const TString dirpath_plots,  // output plots path
    const TString file_id        // identifier (suffix) for output files
    ) {
    
    cout << "alignment_vd() : Starting..." << endl;

    set_my_style();

    const TString filename_out = "alignment_vd_" + file_id;
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_chips      = 9;   // number of chips
    
    TChain* chain = new TChain("event_tree", "event_tree");
    if(!chain->Add(filepath_tree.Data())) {
        cerr << "alignment_vd() : ERROR: Cannot find event tree in file! " << filepath_tree.Data() << endl;
        return kFALSE;
    }

    BinaryEvent* event = new BinaryEvent();
    chain->SetBranchAddress("event", &event);
    Long_t nentries = chain->GetEntries();

    TFile* file_plots = new TFile(Form("%s/%s.root", dirpath_plots.Data(), filename_out.Data()), "RECREATE");
    if(!file_plots->IsOpen()) {
        cerr << "alignment_vd() : ERROR: Cannot open output plots file! " << dirpath_plots.Data() << endl;
        return kFALSE;
    }

    TH2F *hCluHitsHIC = new TH2F("hCluHitsHIC", "Cluster hit map, HIC;Column;Row;a.u.",
                                 n_chips*scols, -0.5, n_chips*scols-0.5, srows, -0.5, srows-0.5);
    hCluHitsHIC->SetStats(0);

    TH2F *hCluHits[n_chips];
    for(Short_t i=0; i<n_chips; ++i) {
        file_plots->mkdir(Form("Chip_%i", i));
        file_plots->cd(Form("Chip_%i", i));
        hCluHits[i] = new TH2F(Form("hCluHits_%i", i), Form("Cluster hit map, chip %i;Column;Row;a.u.", i),
                               scols, -0.5, scols-0.5, srows, -0.5, srows-0.5);
        hCluHits[i]->SetStats(0);
    }
    file_plots->cd();
    
    cout << "Number of events found in tree: " << nentries << endl;

    for(Long_t ientry=0; ientry < nentries; ++ientry) {
        chain->GetEntry(ientry);
        if( (ientry+1)%5000 == 0 )
            cout << "Processed events: " << ientry+1 << " / " << nentries << endl;
        for(Short_t ichip=0; ichip < n_chips; ++ichip) {
            for(Int_t iclu=0; iclu < event->GetPlane(ichip)->GetNClustersSaved(); ++iclu) {
                Int_t mult = event->GetPlane(ichip)->GetCluster(iclu)->GetMultiplicity();
                BinaryCluster* cluster = event->GetPlane(ichip)->GetCluster(iclu);
                BinaryPixel* pixels = cluster->GetPixelArray();
                //if(mult <= 1) continue;
                //if(cluster->HasBorderPixels()) continue;
                //if(cluster->HasExclDblcolPixels()) continue;
                hCluHits[ichip]->Fill(cluster->GetX(),cluster->GetY());
                hCluHitsHIC->Fill(ichip*scols+cluster->GetX(),cluster->GetY());
            } // END FOR clusters
        } // END FOR chips
    } // END FOR entries

    TCanvas *c1 = new TCanvas("c1", "Canvas 1", 0, 0, 1800, 1000);
    c1->Divide(1,3);
    c1->cd(1);
    hCluHitsHIC->DrawCopy("COLZ");
    c1->cd(2);
    ((TH1F*)hCluHitsHIC->ProjectionX())->DrawCopy();
    c1->cd(3);
    ((TH1F*)hCluHitsHIC->ProjectionY())->DrawCopy();
    c1->Print(Form("%s/%s_c1.png", dirpath_plots.Data(), filename_out.Data()));

    Int_t np = TMath::CeilNint(TMath::Sqrt(n_chips));
/*    
    TCanvas *c2 = new TCanvas("c2", "Cluster size", 50, 50, 1800, 1000);
    c2->Divide(np,np);
    for(Short_t i=0; i<n_chips; ++i) {
        c2->GetPad(i+1)->SetLogy();
        c2->cd(i+1);
        zoom_th1(hMult[i]->DrawCopy());
    }
    c2->Print(Form("%s/%s_c2.png", dirpath_plots.Data(), filename_out.Data()));
    
    TCanvas *c3 = new TCanvas("c3", "Number of clusters per event", 100, 100, 1800, 1000);
    c3->Divide(np,np);
    for(Short_t i=0; i<n_chips; ++i) {
        c3->GetPad(i+1)->SetLogy();
        c3->cd(i+1);
        if(binred > 1) hNClu[i]->Rebin(binred);
        zoom_th1(hNClu[i]->DrawCopy());
    }
    c3->Print(Form("%s/%s_c3.png", dirpath_plots.Data(), filename_out.Data()));
    
    TCanvas *c4 = new TCanvas("c4", "Number of pixels per event", 150, 150, 1800, 1000);
    c4->Divide(np,np);
    for(Short_t i=0; i<n_chips; ++i) {
        c4->GetPad(i+1)->SetLogy();
        c4->cd(i+1);
        if(binred > 1) hNPix[i]->Rebin(binred);
        zoom_th1(hNPix[i]->DrawCopy());
    }
    c4->Print(Form("%s/%s_c4.png", dirpath_plots.Data(), filename_out.Data()));
*/
    file_plots->cd();
    file_plots->Write();

    delete chain;
    delete hCluHitsHIC;
    for(Short_t i=0; i<n_chips; ++i) {
        delete hCluHits[i];
    }
    file_plots->Close();

    cout << "alignment_vd() : Done!" << endl;
    return kTRUE;
}


