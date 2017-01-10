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

Bool_t correlation_psd(
    const TString dirpath_data = "../../Data/na61/NoiseOccupancy_161210_193005",  // results dir
    const TString fname_psd = "../../Data/na61/run_027384_psd.txt"
    ) {
    
    cout << "correlation_psd() : Starting..." << endl;

    set_my_style();

    const Int_t expTriggers = 53564;
    
    const TString filename_out = "correlation_psd";
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_secs       = 1;   // number of sectors
    const Short_t n_chips      = 9;
    const TString dirpath_results = dirpath_data + "/results_cr3";

    Float_t totpixmult[100000] = {0.};
    Float_t totclumult[100000] = {0.};
    
    TFile* file_plots = new TFile(Form("%s/%s.root", dirpath_results.Data(), filename_out.Data()), "RECREATE");
    if(!file_plots->IsOpen()) {
        cerr << "correlation_psd() : ERROR: Cannot open output plots file! " << dirpath_results.Data() << "/" << filename_out.Data() << ".root" << endl;
        return kFALSE;
    }

    ifstream file_psd; file_psd.open(fname_psd.Data());
    if(!file_psd.is_open()) {
        cerr << "correlation_psd() : ERROR: Cannot open PSD data file! " << fname_psd.Data() << endl;
        return kFALSE;
    }    
    Int_t na61Trigger;
    Bool_t sT[4];
    Float_t ePSD;
    char line[500];
    file_psd.getline(line, 500);
    cout << line << endl;
    Int_t fpsdbeg = file_psd.tellg();

    TH1F *hEPSD = new TH1F("hEPSD", "PSD energy distribution;ePSD;a.u.", 5e2, 0., 5e4);
    TH2F *hNPixPSDAll = new TH2F("hNPixPSDAll", "Pixel hit multiplicity vs PSD energy - IB HIC;ePSD;# of hit pixels;a.u.", 3e2, 0, 3e4, 200, -0.5, 800-0.5);
    TH2F *hNPixPSD[n_chips];
    TH2F *hNCluPSDAll = new TH2F("hNCluPSDAll", "Cluster hit multiplicity vs PSD energy - IB HIC;ePSD;# of clusters;a.u.", 3e2, 0, 3e4, 300, -0.5, 300-0.5);
    TH2F *hNCluPSD[n_chips];
    
    Long_t trg_to_read = 0;
    
    for(Int_t ichip=0; ichip<n_chips; ++ichip) {

        file_plots->cd();
        hNPixPSD[ichip] = new TH2F(Form("hNPixPSD_Chip%i", ichip), Form("Pixel hitmultiplicity vs PSD energy - chip %i;ePSD;#o of pixels hit;a.u.", ichip), 3e2, 0, 3e4, 100, -0.5, 400-0.5);
        hNCluPSD[ichip] = new TH2F(Form("hNCluPSD_Chip%i", ichip), Form("Cluster hit multiplicity vs PSD energy - chip %i;ePSD;# of clusters hit;a.u.", ichip), 3e2, 0, 3e4, 200, -0.5, 200-0.5);
        
        TString filepath_tree = Form("%s/NoiseOccupancy_*_Chip%i_tree.root", dirpath_data.Data(), ichip);

        cout << filepath_tree.Data() << endl;
        TChain* chain = new TChain("event_tree", "event_tree");
        if(!chain->Add(filepath_tree.Data())) {
            cerr << "correlation_psd() : ERROR: Cannot find event tree in file! " << filepath_tree.Data() << endl;
            return kFALSE;
        }

        BinaryEvent* event = new BinaryEvent();
        chain->SetBranchAddress("event", &event);
        Long_t nentries = chain->GetEntries();

        if(nentries != expTriggers) {
            cerr << "correlation_psd() : ERROR: Number of expected triggers (" << expTriggers << ") and entries in tree (" << nentries << ") do not match!" << endl;
            //return kFALSE;
        }

        trg_to_read = nentries > expTriggers ? expTriggers : nentries;

        file_psd.seekg(fpsdbeg);
        
        for(Long_t ientry=0; ientry < trg_to_read; ++ientry) {
            file_psd >> na61Trigger >> sT[0] >> sT[1] >> sT[2] >> sT[3] >> ePSD;
            if(!file_psd.good()) {
                cerr << "correlation_psd() : ERROR: PSD data file error at trigger " << ientry << " of chip " << ichip <<  endl;
                return kFALSE;
            }
            hEPSD->Fill(ePSD);
            
            chain->GetEntry(ientry);
            if( (ientry+1)%10000 == 0 )
                cout << "Processed events: " << ientry+1 << " / " << nentries << endl;
            Float_t pixmult = 0., clumult = 0.;
            for(Short_t isec=0; isec < n_secs; ++isec) {
                clumult += event->GetPlane(isec)->GetNClustersSaved();
                for(Int_t iclu=0; iclu < event->GetPlane(isec)->GetNClustersSaved(); ++iclu) {
                    Int_t mult = event->GetPlane(isec)->GetCluster(iclu)->GetMultiplicity();
                    pixmult += mult;
                    BinaryCluster* cluster = event->GetPlane(isec)->GetCluster(iclu);
                    BinaryPixel* pixels = cluster->GetPixelArray();
                    
                } // FOR iclu
            } // FOR isec
            hNPixPSD[ichip]->Fill(ePSD, pixmult);
            hNCluPSD[ichip]->Fill(ePSD, clumult);
            totpixmult[ientry] += pixmult;
            totclumult[ientry] += clumult;
        } // FOR ientry
        
        delete chain;
        delete event;
        cout << endl;
    }

    file_psd.seekg(fpsdbeg);

    for(Long_t ientry=0; ientry < trg_to_read; ++ientry) {
        file_psd >> na61Trigger >> sT[0] >> sT[1] >> sT[2] >> sT[3] >> ePSD;
        if(!file_psd.good()) {
            cerr << "correlation_psd() : ERROR: PSD data file error at trigger " << ientry << " of all chips loop." <<  endl;
            return kFALSE;
        }
        //if(ePSD < 22222.)
        hNPixPSDAll->Fill(ePSD, totpixmult[ientry]);
        hNCluPSDAll->Fill(ePSD, totclumult[ientry]);
    }
 
    file_psd.close();
    
    TCanvas *c1 = new TCanvas("c1", "Canvas 1", 0, 0, 1800, 1000);
    c1->Divide(2,1);
    c1->cd(1);
    c1->GetPad(1)->SetRightMargin(0.15);
    hNCluPSDAll->DrawCopy("COLZ");
    c1->cd(2);
    c1->GetPad(2)->SetRightMargin(0.15);
    hNPixPSDAll->DrawCopy("COLZ");
    c1->Print(Form("%s/%s_c1.png", dirpath_results.Data(), filename_out.Data()));
    
    file_plots->cd();
    file_plots->Write();

    delete hEPSD;
    delete hNPixPSDAll;
    for(Int_t i=0; i<n_chips; ++i) delete hNPixPSD[i];
    delete hNCluPSDAll;
    for(Int_t i=0; i<n_chips; ++i) delete hNCluPSD[i];

    
    file_plots->Close();

    cout << "correlation_psd() : Done!" << endl;
    return kTRUE;
}


