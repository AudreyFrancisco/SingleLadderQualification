/* Written by Miljenko Suljic, m.suljic@cern.ch */
#include "Riostream.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TString.h"

#include "classes/helpers.cpp"
#include "classes/BinaryEvent.hpp"

Bool_t thresholds_ibhic(
    const TString dirpath_results = "/home/msuljic/work/na61/data/ThresholdScan_161216_combined/",  // results dir
    const Short_t binred   = 5   // bin reduction - useful in case of low statistics
    ) {
    
    cout << "thresholds_ibhic() : Starting..." << endl;

    set_my_style();

    const TString filename_out = "thresholds_ibhic";
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_secs       = 1;   // number of sectors
    const Short_t n_chips      = 9;
    
    TFile* file_plots = new TFile(Form("%s/%s.root", dirpath_results.Data(), filename_out.Data()), "RECREATE");
    if(!file_plots->IsOpen()) {
        cerr << "thresholds_ibhic() : ERROR: Cannot open output plots file! " << dirpath_results.Data() << "/" << filename_out.Data() << ".root" << endl;
        return kFALSE;
    }

    
    TH2F *hThreshold = new TH2F("hThreshold_hic", "Threshold map;Column;Row;Threshold",
                              n_chips*scols, -0.5, n_chips*scols-0.5, srows, -0.5, srows-0.5);
    hThreshold->SetStats(0);
    TH2F *hNoise = new TH2F("hNoise_hic", "Noise map;Column;Row;Noise",
                              n_chips*scols, -0.5, n_chips*scols-0.5, srows, -0.5, srows-0.5);
    hNoise->SetStats(0);

    TH1F *hThrshChip[n_chips];
    TH1F *hNoiseChip[n_chips];
    

    for(Int_t ichip=0; ichip<n_chips; ++ichip) {

        TString fname = Form("%s/ThresholdScan_161216_combined_Chip%i.root", dirpath_results.Data(), ichip);
        TFile* file = new TFile(fname.Data());
        if(!file_plots->IsOpen()) {
            cerr << "thresholds_ibhic() : ERROR: Cannot open results file for chip " << ichip << " !!" << endl;
            return kFALSE;
        }
        cout << fname.Data() << endl;

        TH2F *hThresholdChipPos = (TH2F*)file->Get("hThreshPos");
        if(hThresholdChipPos == NULL) { cerr << "thresholds_ibhic() : ERROR: missing histogram in " << fname << endl; return kFALSE; }
        TH2F *hNoiseChipPos = (TH2F*)file->Get("hNoisePos");
        if(hNoiseChipPos == NULL) { cerr << "thresholds_ibhic() : ERROR: missing histogram in " << fname << endl; return kFALSE; }

        hNoiseChip[ichip] = (TH1F*)file->Get("hNoise_0");
        if(hNoiseChip[ichip] == NULL) { cerr << "thresholds_ibhic() : ERROR: missing histogram in " << fname << endl; return kFALSE; }
        hNoiseChip[ichip]->SetDirectory(0);
        hNoiseChip[ichip]->SetNameTitle(Form("hNoiseChip_%i", ichip), Form("Noise, Chip %i", ichip));
        
        hThrshChip[ichip] = (TH1F*)file->Get("hThresh_0");
        if(hThrshChip[ichip] == NULL) { cerr << "thresholds_ibhic() : ERROR: missing histogram in " << fname << endl; return kFALSE; }
        hThrshChip[ichip]->SetDirectory(0);
        hThrshChip[ichip]->SetNameTitle(Form("hThrshChip_%i", ichip), Form("Threshold, Chip %i", ichip));
        
        for(Int_t icol=0; icol<scols; icol++) {
            for(Int_t irow=0; irow<srows; irow++) {
                if(ichip == 5 && icol == 905 && irow == 46) continue; // FIX = REMOVE
                hThreshold->SetBinContent(ichip*1024+icol+1, irow+1, hThresholdChipPos->GetBinContent(icol+1, irow+1));
                hNoise->SetBinContent(ichip*1024+icol+1, irow+1, hNoiseChipPos->GetBinContent(icol+1, irow+1));
            }
        }
        
        
        file->Close();
    }

    hThreshold->Scale(1./srows);
    hNoise->Scale(1./srows);

    TCanvas *c1 = new TCanvas("c1", "Canvas 1", 0, 0, 1800, 1000);
    c1->Divide(1,4);
    c1->cd(1);
    //c1->GetPad(1)->SetRightMargin(0.15);
    hThreshold->DrawCopy("COLZ");
    c1->cd(3);
    //c1->GetPad(3)->SetRightMargin(0.15);
    hNoise->DrawCopy("COLZ");
    c1->cd(2);
    //c1->cd(3)->SetGrid();
    //c1->GetPad(3)->SetLogy();
    ((TH1F*)hThreshold->ProjectionX())->DrawCopy();
    c1->cd(4);
    //c1->cd(4)->SetGrid();
    //c1->GetPad(4)->SetLogy();
    ((TH1F*)hNoise->ProjectionX())->DrawCopy();
    c1->Print(Form("%s/%s_c1.png", dirpath_results.Data(), filename_out.Data()));

/*
    TCanvas *c2 = new TCanvas("c2", "Canvas 2", 50, 50, 1800, 1000);
    c2->Divide(3,2);
    for(Short_t i=3; i<n_chips; ++i) {
        c2->GetPad(i+1-3)->SetLogy();
        c2->cd(i+1-3);
        //zoom_th1(hMult[i]->DrawCopy());
        hMult[i]->DrawCopy()->GetXaxis()->SetRangeUser(0, 150);
    }    
    c2->Print(Form("%s/%s_c2.png", dirpath_results.Data(), filename_out.Data()));
*/

    TCanvas *c3 = new TCanvas("c3", "Canvas 3", 50, 50, 1800, 1000);
    c3->Divide(3,3);
    for(Short_t i=0; i<n_chips; ++i) {
        //c3->GetPad(i+1)->SetLogy();
        c3->cd(i+1);
        //zoom_th1(hNoiseChip[i]->DrawCopy());
        hNoiseChip[i]->DrawCopy()->GetXaxis()->SetRangeUser(0, 15);
    }    
    c3->Print(Form("%s/%s_c3.png", dirpath_results.Data(), filename_out.Data()));

    TCanvas *c4 = new TCanvas("c4", "Canvas 4", 50, 50, 1800, 1000);
    c4->Divide(3,3);
    for(Short_t i=0; i<n_chips; ++i) {
        //c4->GetPad(i+1)->SetLogy();
        c4->cd(i+1);
        //zoom_th1(hThrshChip[i]->DrawCopy());
        hThrshChip[i]->DrawCopy()->GetXaxis()->SetRangeUser(0, 200);
    }    
    c4->Print(Form("%s/%s_c4.png", dirpath_results.Data(), filename_out.Data()));
    
    
    file_plots->cd();
    file_plots->Write();

    /*
    delete hThreshold;
    delete hNoise;
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

    cout << "thresholds_ibhic() : Done!" << endl;
    return kTRUE;
}


