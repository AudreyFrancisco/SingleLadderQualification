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

Bool_t analysis_basic_hic(
    const TString filepath_tree,  // input tree path
    const TString dirpath_plots,  // output plots path
    const TString file_id,        // identifier (suffix) for output files
    const Short_t binred   = 1,   // bin reduction - useful in case of low statistics
    const Short_t max_mult = 500, // max drawn cluster size
    const Short_t max_spread = 50 // max drawn cluster spread
    ) {
    
    cout << "analysis_basic_hic() : Starting..." << endl;

    set_my_style();

    const TString filename_out = "analysis_basic_hic_" + file_id;
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_chips      = 9;   // number of chips
    
    TChain* chain = new TChain("event_tree", "event_tree");
    if(!chain->Add(filepath_tree.Data())) {
        cerr << "analysis_basic_hic() : ERROR: Cannot find event tree in file! " << filepath_tree.Data() << endl;
        return kFALSE;
    }

    BinaryEvent* event = new BinaryEvent();
    chain->SetBranchAddress("event", &event);
    Long_t nentries = chain->GetEntries();

    TFile* file_plots = new TFile(Form("%s/%s.root", dirpath_plots.Data(), filename_out.Data()), "RECREATE");
    if(!file_plots->IsOpen()) {
        cerr << "analysis_basic_hic() : ERROR: Cannot open output plots file! " << dirpath_plots.Data() << endl;
        return kFALSE;
    }

    TH2F *hPixHitsHIC = new TH2F("hPixHitsHIC", "Pixel hit map, HIC;Column;Row;a.u.",
                                 n_chips*scols, -0.5, n_chips*scols-0.5, srows, -0.5, srows-0.5);
    hPixHitsHIC->SetStats(0);
    TH2F *hCluHitsHIC = new TH2F("hCluHitsHIC", "Cluster hit map, HIC;Column;Row;a.u.",
                                 n_chips*scols, -0.5, n_chips*scols-0.5, srows, -0.5, srows-0.5);
    hCluHitsHIC->SetStats(0);
    TH1F *hCluXHIC = new TH1F("hCluXHIC", "Clusters X-position, HIC;Column;a.u.",
                              n_chips*scols, -0.5, n_chips*scols-0.5);
    //hCluXHIC->SetStats(0);
    TH1F *hCluYHIC = new TH1F("hCluYHIC", "Clusters Y-Position, HIC;Row;a.u.",
                              srows-0.5, -0.5, srows-0.5);
    //hCluYHIC->SetStats(0);

    TH1F *hNPixHIC = new TH1F("hNPixHIC", "Number of hit pixels per event, HIC;Number of hit pixels per event;a.u.",
                              1500, -0.5, 1500-0.5);
    TH1F *hNCluHIC = new TH1F("hNCluHIC", "Number of clusters per event, HIC;Number of clusters per event;a.u.",
                              750, -0.5, 750-0.5);
    TH1F *hMultHIC = new TH1F("hMultHIC", "Cluster size, HIC;Number of pixels in cluster;a.u.",
                              max_mult+1, -1.5, max_mult-0.5);
    
    TH2F *hPixHits[n_chips];
    TH2F *hCluHits[n_chips];
    TH1F *hCluX[n_chips];
    TH1F *hCluY[n_chips];
    
    TH1F *hNPix[n_chips];
    TH1F *hNClu[n_chips];
    TH1F *hMult[n_chips];
    TH1F *hXSpread[n_chips];
    TH1F *hYSpread[n_chips];
    TH1F *hMaxSpread[n_chips];
    for(Short_t i=0; i<n_chips; ++i) {
        file_plots->mkdir(Form("Chip_%i", i));
        file_plots->cd(Form("Chip_%i", i));
        
        hPixHits[i] = new TH2F(Form("hPixHits_%i", i), Form("Pixel hit map, chip %i;Column;Row;a.u.", i),
                               scols, -0.5, scols-0.5, srows, -0.5, srows-0.5);
        hPixHits[i]->SetStats(0);
        hCluHits[i] = new TH2F(Form("hCluHits_%i", i), Form("Cluster hit map, chip %i;Column;Row;a.u.", i),
                               scols, -0.5, scols-0.5, srows, -0.5, srows-0.5);
        hCluHits[i]->SetStats(0);
        hCluX[i] = new TH1F(Form("hCluX_%i", i), Form("Clusters X-position, chip %i;Column;a.u.", i),
                            scols, -0.5, scols-0.5);
        //hCluX[i]->SetStats(0);
        hCluY[i] = new TH1F(Form("hCluY_%i", i), Form("Clusters Y-Position, chip %i;Row;a.u.", i),
                            srows-0.5, -0.5, srows-0.5);
        //hCluY[i]->SetStats(0);
        
        hNPix[i] = new TH1F(Form("hNPix_%i", i), Form("Number of hit pixels per event, chip %i;Number of hit pixels per event;a.u.", i),
                            1500, -0.5, 1500-0.5);
        hNClu[i] = new TH1F(Form("hNClu_%i", i), Form("Number of clusters per event, chip %i;Number of clusters per event;a.u.", i),
                            750, -0.5, 750-0.5);
        hMult[i] = new TH1F(Form("hMult_%i", i), Form("Cluster size, Chip %i;Number of pixels in cluster;a.u.", i),
                            max_mult+1, -1.5, max_mult-0.5);
        //hMult[i]->SetStats(0);
        hXSpread[i] = new TH1F(Form("hXSpread_%i", i), Form("Cluster X Spread, Chip %i;Cluster X Spread (pixels);a.u.", i),
                            max_spread+1, -1.5, max_spread-0.5);
        //hXSpread[i]->SetStats(0);
        hYSpread[i] = new TH1F(Form("hYSpread_%i", i), Form("Cluster Y Spread, Chip %i;Cluster Y Spread (pixels);a.u.", i),
                            max_spread+1, -1.5, max_spread-0.5);
        //hYSpread[i]->SetStats(0);
        hMaxSpread[i] = new TH1F(Form("hMaxSpread_%i", i), Form("Cluster Max Spread, Chip %i;Cluster Max Spread (pixels);a.u.", i),
                            max_spread+1, -1.5, max_spread-0.5);
        //hMaxSpread[i]->SetStats(0);
    }
    file_plots->cd();
    
    cout << "Number of events found in tree: " << nentries << endl;

    //nentries = 22994;
    
    for(Long_t ientry=0; ientry < nentries; ++ientry) {
        chain->GetEntry(ientry);
        if( (ientry+1)%10000 == 0 )
            cout << "Processed events: " << ientry+1 << " / " << nentries << endl;
        Int_t totnpix = 0, totnclu = 0;
        for(Short_t ichip=0; ichip < n_chips; ++ichip) {
            hNPix[ichip]->Fill(event->GetPlane(ichip)->GetNHitPix());
            hNClu[ichip]->Fill(event->GetPlane(ichip)->GetNClustersSaved());
            totnpix += event->GetPlane(ichip)->GetNHitPix();
            totnclu += event->GetPlane(ichip)->GetNClustersSaved();
            for(Int_t iclu=0; iclu < event->GetPlane(ichip)->GetNClustersSaved(); ++iclu) {
                Int_t mult = event->GetPlane(ichip)->GetCluster(iclu)->GetMultiplicity();
                BinaryCluster* cluster = event->GetPlane(ichip)->GetCluster(iclu);
                BinaryPixel* pixels = cluster->GetPixelArray();
                //if(mult <= 1) continue;
                if(cluster->HasBorderPixels()) continue;
                if(cluster->HasExclDblcolPixels()) continue;
                //{
                    hMult[ichip]->Fill(mult);
                    hMultHIC->Fill(mult);
                    hCluHits[ichip]->Fill(cluster->GetX(),cluster->GetY());
                    hCluHitsHIC->Fill(ichip*scols+cluster->GetX(),cluster->GetY());
                    for(Int_t ipix=0; ipix < mult; ++ipix) {
                        hPixHits[ichip]->Fill(pixels[ipix].GetCol(), pixels[ipix].GetRow());
                        hPixHitsHIC->Fill(ichip*scols+pixels[ipix].GetCol(), pixels[ipix].GetRow());
                    }
                    hCluX[ichip]->Fill(cluster->GetX());
                    hCluXHIC->Fill(ichip*scols+cluster->GetX());
                    hCluY[ichip]->Fill(cluster->GetY());
                    hCluYHIC->Fill(cluster->GetY());
                    hXSpread[ichip]->Fill(cluster->GetXSpread());
                    hYSpread[ichip]->Fill(cluster->GetYSpread());
                    hMaxSpread[ichip]->Fill(cluster->GetMaxSpread());
                //}
                    //if(iclu > 0)
                    //  hPixHits->Fill(event->GetPlane(ichip)->GetCluster(iclu)->GetY(), event->GetPlane(ichip)->GetCluster(iclu-1)->GetY());
                    
            }
        } // END FOR chips
        hNPixHIC->Fill(totnpix);
        hNCluHIC->Fill(totnclu);
    } // END FOR entries

    TCanvas *c1 = new TCanvas("c1", "Canvas 1", 0, 0, 1800, 1000);
    c1->Divide(1,4);
    c1->cd(1);
    //c1->GetPad(1)->SetRightMargin(0.15);
    hPixHitsHIC->DrawCopy("COLZ");
    c1->cd(3);
    //c1->GetPad(3)->SetRightMargin(0.15);
    hCluHitsHIC->DrawCopy("COLZ");
    c1->cd(2);
    //c1->cd(3)->SetGrid();
    //c1->GetPad(3)->SetLogy();
    ((TH1F*)hPixHitsHIC->ProjectionX())->DrawCopy();
    c1->cd(4);
    //c1->cd(4)->SetGrid();
    //c1->GetPad(4)->SetLogy();
    ((TH1F*)hCluHitsHIC->ProjectionX())->DrawCopy();
    c1->Print(Form("%s/%s_c1.png", dirpath_plots.Data(), filename_out.Data()));

    Int_t np = TMath::CeilNint(TMath::Sqrt(n_chips));
    
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
    
    //TCanvas *c3 = new TCanvas("c3", "Canvas 3", 100, 100, 1400, 700);
    //c3->Divide(4,3);
    //for(Short_t i=0; i<n_chips; ++i) {
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
    delete hPixHitsHIC;
    delete hCluHitsHIC;
    delete hCluXHIC;
    delete hCluYHIC;
    for(Short_t i=0; i<n_chips; ++i) {
        delete hPixHits[i];
        delete hCluHits[i];
        delete hCluX[i];
        delete hCluY[i];
        delete hMult[i];
        delete hXSpread[i];
        delete hYSpread[i];
        delete hMaxSpread[i];
    }
    file_plots->Close();

    cout << "analysis_basic_hic() : Done!" << endl;
    return kTRUE;
}


