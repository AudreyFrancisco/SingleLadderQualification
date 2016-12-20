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

Bool_t analysis_ibhic(
    const TString dirpath_results,  // results dir
    const Short_t binred   = 5   // bin reduction - useful in case of low statistics
    ) {
    
    cout << "analysis_ibhic() : Starting..." << endl;

    set_my_style();

    const TString filename_out = "analysis_ibhic";
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_secs       = 1;   // number of sectors
    const Short_t n_chips      = 9;
    
    TFile* file_plots = new TFile(Form("%s/%s.root", dirpath_results.Data(), filename_out.Data()), "RECREATE");
    if(!file_plots->IsOpen()) {
        cerr << "analysis_ibhic() : ERROR: Cannot open output plots file! " << dirpath_results.Data() << "/" << filename_out.Data() << ".root" << endl;
        return kFALSE;
    }

    
    TH2F *hPixHits = new TH2F("hPixHits_hic", "Pixel hit map;Column;Row;Frequency",
                              n_chips*scols, -0.5, n_chips*scols-0.5, srows, -0.5, srows-0.5);
    hPixHits->SetStats(0);
    TH2F *hCluHits = new TH2F("hCluHits_hic", "Cluster hit map;Column;Row;Frequency",
                              n_chips*scols, -0.5, n_chips*scols-0.5, srows, -0.5, srows-0.5);
    hCluHits->SetStats(0);

    
    TH1F *hNPix[n_chips];
    TH1F *hNClu[n_chips];
    TH1F *hMult[n_chips];
    /*
    TH1F *hXSpread[n_secs];
    TH1F *hYSpread[n_secs];
    TH1F *hMaxSpread[n_secs];
    
    for(Short_t i=0; i<n_chips; ++i) {
        
        hXSpread[i] = new TH1F(Form("hXSpread_%i", i), Form("Cluster X Spread, Sector %i;Cluster X Spread (pixels);Frequency", i),
                            max_spread+1, -1.5, max_spread-0.5);
        hXSpread[i]->SetStats(0);
        hYSpread[i] = new TH1F(Form("hYSpread_%i", i), Form("Cluster Y Spread, Sector %i;Cluster Y Spread (pixels);Frequency", i),
                            max_spread+1, -1.5, max_spread-0.5);
        hYSpread[i]->SetStats(0);
        hMaxSpread[i] = new TH1F(Form("hMaxSpread_%i", i), Form("Cluster Max Spread, Sector %i;Cluster Max Spread (pixels);Frequency", i),
                            max_spread+1, -1.5, max_spread-0.5);
        hMaxSpread[i]->SetStats(0);
        */
    //  }
    

    for(Int_t ichip=0; ichip<n_chips; ++ichip) {

        TString fname = Form("%s/analysis_basic_Chip%i.root", dirpath_results.Data(), ichip);
        TFile* file = new TFile(fname.Data());
        if(!file_plots->IsOpen()) {
            cerr << "analysis_ibhic() : ERROR: Cannot open results file for chip " << ichip << " !!" << endl;
            return kFALSE;
        }
        cout << fname.Data() << endl;

        TH2F *hPixHitsChip = (TH2F*)file->Get("hPixHits");
        if(hPixHitsChip == NULL) { cerr << "analysis_ibhic() : ERROR: missing histogram in " << fname << endl; return kFALSE; }
        TH2F *hCluHitsChip = (TH2F*)file->Get("hCluHits");
        if(hCluHitsChip == NULL) { cerr << "analysis_ibhic() : ERROR: missing histogram in " << fname << endl; return kFALSE; }

        hMult[ichip] = (TH1F*)file->Get("hMult_0");
        if(hMult[ichip] == NULL) { cerr << "analysis_ibhic() : ERROR: missing histogram in " << fname << endl; return kFALSE; }
        hMult[ichip]->SetDirectory(0);
        hMult[ichip]->SetNameTitle(Form("hMult_%i", ichip), Form("Cluster multiplicity, Chip %i", ichip));

        hNClu[ichip] = (TH1F*)file->Get("hNClu_0");
        if(hNClu[ichip] == NULL) { cerr << "analysis_ibhic() : ERROR: missing histogram in " << fname << endl; return kFALSE; }
        hNClu[ichip]->SetDirectory(0);
        hNClu[ichip]->SetNameTitle(Form("hNClu_%i", ichip), Form("Number of clusters per event, Chip %i", ichip));
        
        hNPix[ichip] = (TH1F*)file->Get("hNPix_0");
        if(hNPix[ichip] == NULL) { cerr << "analysis_ibhic() : ERROR: missing histogram in " << fname << endl; return kFALSE; }
        hNPix[ichip]->SetDirectory(0);
        hNPix[ichip]->SetNameTitle(Form("hNPix_%i", ichip), Form("Number of hit pixels per event, Chip %i", ichip));
        
        for(Int_t icol=0; icol<scols; icol++) {
            for(Int_t irow=0; irow<srows; irow++) {
                if(ichip == 5 && icol == 905 && irow == 46) continue; // FIX = REMOVE
                hPixHits->SetBinContent(ichip*1024+icol+1, irow+1, hPixHitsChip->GetBinContent(icol+1, irow+1));
                hCluHits->SetBinContent(ichip*1024+icol+1, irow+1, hCluHitsChip->GetBinContent(icol+1, irow+1));
            }
        }
        
        
        file->Close();
    }


    TCanvas *c1 = new TCanvas("c1", "Canvas 1", 0, 0, 1800, 1000);
    c1->Divide(1,4);
    c1->cd(1);
    //c1->GetPad(1)->SetRightMargin(0.15);
    hPixHits->DrawCopy("COLZ");
    c1->cd(3);
    //c1->GetPad(3)->SetRightMargin(0.15);
    hCluHits->DrawCopy("COLZ");
    c1->cd(2);
    //c1->cd(3)->SetGrid();
    //c1->GetPad(3)->SetLogy();
    ((TH1F*)hPixHits->ProjectionX())->DrawCopy();
    c1->cd(4);
    //c1->cd(4)->SetGrid();
    //c1->GetPad(4)->SetLogy();
    ((TH1F*)hCluHits->ProjectionX())->DrawCopy();
    c1->Print(Form("%s/%s_c1.png", dirpath_results.Data(), filename_out.Data()));


    TCanvas *c2 = new TCanvas("c2", "Canvas 2", 50, 50, 1800, 1000);
    c2->Divide(3,2);
    for(Short_t i=3; i<n_chips; ++i) {
        c2->GetPad(i+1-3)->SetLogy();
        c2->cd(i+1-3);
        //zoom_th1(hMult[i]->DrawCopy());
        hMult[i]->DrawCopy()->GetXaxis()->SetRangeUser(0, 150);
    }    
    c2->Print(Form("%s/%s_c2.png", dirpath_results.Data(), filename_out.Data()));


    TCanvas *c3 = new TCanvas("c3", "Canvas 3", 50, 50, 1800, 1000);
    c3->Divide(3,2);
    for(Short_t i=3; i<n_chips; ++i) {
        c3->GetPad(i+1-3)->SetLogy();
        c3->cd(i+1-3);
        //zoom_th1(hNClu[i]->DrawCopy());
        hNClu[i]->DrawCopy()->GetXaxis()->SetRangeUser(0, 200);
    }    
    c3->Print(Form("%s/%s_c3.png", dirpath_results.Data(), filename_out.Data()));

    TCanvas *c4 = new TCanvas("c4", "Canvas 4", 50, 50, 1800, 1000);
    c4->Divide(3,2);
    for(Short_t i=3; i<n_chips; ++i) {
        c4->GetPad(i+1-3)->SetLogy();
        c4->cd(i+1-3);
        //zoom_th1(hNPix[i]->DrawCopy());
        hNPix[i]->DrawCopy()->GetXaxis()->SetRangeUser(0, 500);
    }    
    c4->Print(Form("%s/%s_c4.png", dirpath_results.Data(), filename_out.Data()));
    
    
    file_plots->cd();
    file_plots->Write();

    /*
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
    */
    file_plots->Close();

    cout << "analysis_ibhic() : Done!" << endl;
    return kTRUE;
}


