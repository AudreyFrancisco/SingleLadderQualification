/* Written by Miljenko Suljic, m.suljic@cern.ch */
#include "Riostream.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TString.h"

#include "../classes/helpers.cpp"
#include "../classes/BinaryEvent.hpp"

Bool_t analysis_basic(
    const TString filepath_tree,  // input tree path
    const TString dirpath_plots,  // output plots path
    const TString file_id,        // identifier (suffix) for output files
    const Short_t binred   = 5,   // bin reduction - useful in case of low statistics
    const Short_t max_mult = 500, // max drawn cluster size
    const Short_t max_spread = 50 // max drawn cluster spread
    ) {
    
    cout << "analysis_basic() : Starting..." << endl;

    set_my_style();

    const TString filename_out = "analysis_basic_" + file_id;
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_secs       = 1;   // number of sectors
    
    TChain* chain = new TChain("event_tree", "event_tree");
    if(!chain->Add(filepath_tree.Data())) {
        cerr << "analysis_basic() : ERROR: Cannot find event tree in file! " << filepath_tree.Data() << endl;
        return kFALSE;
    }

    BinaryEvent* event = new BinaryEvent();
    chain->SetBranchAddress("event", &event);
    Long_t nentries = chain->GetEntries();

    TFile* file_plots = new TFile(Form("%s/%s.root", dirpath_plots.Data(), filename_out.Data()), "RECREATE");
    if(!file_plots->IsOpen()) {
        cerr << "analysis_basic() : ERROR: Cannot open output plots file! " << dirpath_plots.Data() << endl;
        return kFALSE;
    }

    TH2F *hPixHits = new TH2F("hPixHits", "Pixel hit map;Column;Row;Frequency",
                              n_secs*scols, -0.5, n_secs*scols-0.5, srows, -0.5, srows-0.5);
    hPixHits->SetStats(0);
    TH2F *hCluHits = new TH2F("hCluHits", "Cluster hit map;Column;Row;Frequency",
                              n_secs*scols, -0.5, n_secs*scols-0.5, srows, -0.5, srows-0.5);
    hCluHits->SetStats(0);
    TH1F *hCluX = new TH1F("hCluX", "Clusters X-position;Column;Frequency",
                           n_secs*scols, -0.5, n_secs*scols-0.5);
    hCluX->SetStats(0);
    TH1F *hCluY = new TH1F("hCluY", "Clusters Y-Position;Row;Frequency",
                           srows-0.5, -0.5, srows-0.5);
    hCluY->SetStats(0);

    TH1F *hNPix[n_secs];
    TH1F *hNClu[n_secs];
    TH1F *hMult[n_secs];
    TH1F *hXSpread[n_secs];
    TH1F *hYSpread[n_secs];
    TH1F *hMaxSpread[n_secs];
    for(Short_t i=0; i<n_secs; ++i) {
        hNPix[i] = new TH1F(Form("hNPix_%i", i), Form("Number of hit pixels per event, sector %i;Number of hit pixels per event;Frequency", i),
                            1000, 0, 3000);
        hNClu[i] = new TH1F(Form("hNClu_%i", i), Form("Number of clusters per event, sector %i;Number of clusters per event;Frequency", i),
                            1000, 0, 1500);
        hMult[i] = new TH1F(Form("hMult_%i", i), Form("Cluster size, Sector %i;Number of pixels in cluster;Frequency", i),
                            max_mult+1, -1.5, max_mult-0.5);
        //hMult[i]->SetStats(0);
        hXSpread[i] = new TH1F(Form("hXSpread_%i", i), Form("Cluster X Spread, Sector %i;Cluster X Spread (pixels);Frequency", i),
                            max_spread+1, -1.5, max_spread-0.5);
        hXSpread[i]->SetStats(0);
        hYSpread[i] = new TH1F(Form("hYSpread_%i", i), Form("Cluster Y Spread, Sector %i;Cluster Y Spread (pixels);Frequency", i),
                            max_spread+1, -1.5, max_spread-0.5);
        hYSpread[i]->SetStats(0);
        hMaxSpread[i] = new TH1F(Form("hMaxSpread_%i", i), Form("Cluster Max Spread, Sector %i;Cluster Max Spread (pixels);Frequency", i),
                            max_spread+1, -1.5, max_spread-0.5);
        hMaxSpread[i]->SetStats(0);
    }

    cout << "Number of events found in tree: " << nentries << endl;

    for(Long_t ientry=0; ientry < nentries; ++ientry) {
        chain->GetEntry(ientry);
        if( (ientry+1)%10 == 0 )
            cout << "Processed events: " << ientry+1 << " / " << nentries << endl;
        for(Short_t isec=0; isec < n_secs; ++isec) {
            hNPix[isec]->Fill(event->GetPlane(isec)->GetNHitPix());
            hNClu[isec]->Fill(event->GetPlane(isec)->GetNClustersSaved());
            for(Int_t iclu=0; iclu < event->GetPlane(isec)->GetNClustersSaved(); ++iclu) {
                Int_t mult = event->GetPlane(isec)->GetCluster(iclu)->GetMultiplicity();
                BinaryCluster* cluster = event->GetPlane(isec)->GetCluster(iclu);
                BinaryPixel* pixels = cluster->GetPixelArray();
//                if(mult > 1)
                //if(!cluster->HasBorderPixels())
                //{
                    hMult[isec]->Fill(mult);
                    hCluHits->Fill(cluster->GetX(),
                                   cluster->GetY());
                    for(Int_t ipix=0; ipix < mult; ++ipix)
                        hPixHits->Fill(pixels[ipix].GetCol(), pixels[ipix].GetRow());
                    hCluX->Fill(cluster->GetX());
                    hCluY->Fill(cluster->GetY());
                    hXSpread[isec]->Fill(cluster->GetXSpread());
                    hYSpread[isec]->Fill(cluster->GetYSpread());
                    hMaxSpread[isec]->Fill(cluster->GetMaxSpread());
                //}
            }
        } // END FOR sectors
    } // END FOR entries

    TCanvas *c1 = new TCanvas("c1", "Canvas 1", 0, 0, 1400, 700);
    c1->Divide(2,2);
    c1->cd(1);
    //c1->cd(1)->SetGrid();
    c1->GetPad(1)->SetRightMargin(0.15);
    hPixHits->DrawCopy("COLZ");
    c1->cd(2);
    //c1->cd(2)->SetGrid();
    c1->GetPad(2)->SetRightMargin(0.15);
    hCluHits->DrawCopy("COLZ");
    c1->cd(3);
    c1->cd(3)->SetGrid();
    c1->GetPad(3)->SetLogy();
    hCluX->DrawCopy();
    c1->cd(4);
    c1->cd(4)->SetGrid();
    c1->GetPad(4)->SetLogy();
    hCluY->DrawCopy();
    c1->Print(Form("%s/%s_c1.png", dirpath_plots.Data(), filename_out.Data()));

    TCanvas *c2 = new TCanvas("c2", "Canvas 2", 50, 50, 2100, 450);
    c2->Divide(3,1);
    //for(Short_t i=0; i<n_secs; ++i) {
    //    c2->GetPad(i+1)->SetLogy();
    //    c2->cd(i+1);
    //    zoom_th1(hMult[i]->DrawCopy());
    //}
    c2->cd(1);
    //c2->GetPad(1)->SetLogy();
    zoom_th1(hMult[0]->DrawCopy());
    //c2->GetPad(2)->SetLogy();
    c2->cd(2);
    //zoom_th1(hXSpread[0]->DrawCopy());
    //c2->GetPad(3)->SetLogy();
    hNPix[0]->Rebin(binred);
    hNPix[0]->DrawCopy();
    c2->cd(3);
    hNClu[0]->Rebin(binred);
    hNClu[0]->DrawCopy();
    //zoom_th1(hYSpread[0]->DrawCopy());
    //c2->GetPad(4)->SetLogy();
    //c2->cd(4);
    //zoom_th1(hMaxSpread[0]->DrawCopy());

    c2->Print(Form("%s/%s_c2.png", dirpath_plots.Data(), filename_out.Data()));


    //TCanvas *c3 = new TCanvas("c3", "Canvas 3", 100, 100, 1400, 700);
    //c3->Divide(4,3);
    //for(Short_t i=0; i<n_secs; ++i) {
    //    c3->GetPad(i+1)->SetLogy();
    //    c3->cd(i+1);
    //    zoom_th1(hXSpread[i]->DrawCopy());
    //    c3->GetPad(i+1+4)->SetLogy();
    //    c3->cd(i+1+4);
    //    zoom_th1(hYSpread[i]->DrawCopy());
    //    c3->GetPad(i+1+8)->SetLogy();
    //    c3->cd(i+1+8);
    //    zoom_th1(hMaxSpread[i]->DrawCopy());
    //}
    //c3->Print(Form("%s/%s_c3.png", dirpath_plots.Data(), filename_out.Data()));

    file_plots->cd();
    file_plots->Write();

    delete chain;
    delete hPixHits;
    delete hCluHits;
    delete hCluX;
    delete hCluY;
    for(Short_t i=0; i<n_secs; ++i) {
        delete hMult[i];
        delete hXSpread[i];
        delete hYSpread[i];
        delete hMaxSpread[i];
    }
    file_plots->Close();

    cout << "analysis_basic() : Done!" << endl;
    return kTRUE;
}


