/* Written by Miljenko Suljic, m.suljic@cern.ch */
#include "Riostream.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TString.h"
#include "TMath.h"
#include "TF1.h"
#include "TClonesArray.h"

#include "../classes/helpers.cpp"
#include "../classes/BinaryEvent.hpp"
#include "../classes/Alignment.hpp"

Bool_t prealignment_vd(
    const TString filepath_tree,  // input tree path
    const TString filepath_vd,    // file with vd tracks
    const TString dirpath_plots,  // output plots path
    const TString file_id         // identifier (suffix) for output files
    ) {
    
    cout << "prealignment_vd() : Starting..." << endl;

    set_my_style();

    const TString filename_out = "prealignment_vd_" + file_id;
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_chips      = 9;   // number of chips

    // ALPIDE event tree
    TChain* chain = new TChain("event_tree", "event_tree");
    if(!chain->Add(filepath_tree.Data())) {
        cerr << "prealignment_vd() : ERROR: Cannot find event tree in file! " << filepath_tree.Data() << endl;
        return kFALSE;
    }

    BinaryEvent* event = new BinaryEvent();
    chain->SetBranchAddress("event", &event);
    Long_t nentries = chain->GetEntries();

    // VD tracks tree
    TFile *file_vd = new TFile(filepath_vd.Data());
    if(!file_vd->IsOpen()) {
        cerr << "prealignment_vd() : ERROR: Cannot open VD file! " << filepath_vd.Data() << endl;
        return kFALSE;
    }

    TTree *tree_vd = (TTree*)file_vd->Get("tracks_tree");
    if(tree_vd == NULL) {
        cerr << "prealignment_vd() : ERROR: Cannot find tracks tree in VD file! " << filepath_vd.Data() << endl;
        return kFALSE;
    }
    Int_t vd_event_n;
    Int_t vd_n_tracks;
    Int_t dataset[10000];
    TClonesArray *vd_to = new TClonesArray("TVector3");
    TClonesArray *vd_td = new TClonesArray("TVector3");
    tree_vd->SetBranchAddress("event_n", &vd_event_n);
    tree_vd->SetBranchAddress("n_tracks", &vd_n_tracks);
    tree_vd->SetBranchAddress("dataset", dataset);
    tree_vd->SetBranchAddress("track_origin", &vd_to);
    tree_vd->SetBranchAddress("track_direction", &vd_td);

    // output plots file
    TFile* file_plots = new TFile(Form("%s/%s.root", dirpath_plots.Data(), filename_out.Data()), "RECREATE");
    if(!file_plots->IsOpen()) {
        cerr << "prealignment_vd() : ERROR: Cannot open output plots file! " << dirpath_plots.Data() << endl;
        return kFALSE;
    }

    // Alignment
    const Float_t xpos = 11.1;
    const Float_t ypos = 120.4;
    const Float_t zpos = 92.3;
    Alignment align_chip[n_chips];
    for(Short_t i=0; i<n_chips; ++i) {
        align_chip[i].SetPos(TVector3(xpos, ypos-30.15*i, zpos));
        TRotation rot;
        //rot.RotateY(-0.07*TMath::Pi());
        rot.RotateY(6.*TMath::DegToRad());
        //rot.RotateX(1.*TMath::DegToRad());
        align_chip[i].SetRotation(rot);
    }

    // histograms
    TH2F *hTrackHitsHIC = new TH2F("hTrackHitsHIC", "Track hit map, HIC;X [mm];Y [mm];a.u.",
                                 400, -10., 30., 4000, -200., 200.);
    hTrackHitsHIC->SetStats(0);
    
    TH2F *hTrackHits[n_chips];
    TH1F *hDX[n_chips];
    TH1F *hDY[n_chips];
    TH1F *hDXzoom[n_chips];
    TH1F *hDYzoom[n_chips];

    TH2F *hChipPosXY[n_chips];
    TH2F *hChipPosZY[n_chips];
    TH2F *hChipPosXZ[n_chips];
    
    for(Short_t i=0; i<n_chips; ++i) {
        file_plots->mkdir(Form("Chip_%i", i));
        file_plots->cd(Form("Chip_%i", i));
        hTrackHits[i] = new TH2F(Form("hTrackHits_%i", i), Form("Track hit map, chip %i;X [mm]; Y[mm];a.u.", i),
                               400, xpos-20., xpos+20., 400, ypos-30.15*i-20., ypos-30.15*i+20.);
        hTrackHits[i]->SetStats(0);
        hDX[i] = new TH1F(Form("hDX_%i", i), Form("Difference track hit - cluster pos X, chip %i;DeltaX [mm];a.u.", i),
                          400, xpos-20., xpos+20.);
        hDY[i] = new TH1F(Form("hDY_%i", i), Form("Difference track hit - cluster pos Y, chip %i;DeltaY [mm];a.u.", i),
                          400, ypos-30.15*i-40., ypos-30.15*i+40.);
        hDXzoom[i] = new TH1F(Form("hDXzoom_%i", i), Form("Difference track hit - cluster pos X, chip %i;DeltaX [mm];a.u.", i),
                              200, -0.25, 0.25);
        hDYzoom[i] = new TH1F(Form("hDYzoom_%i", i), Form("Difference track hit - cluster pos Y, chip %i;DeltaY [mm];a.u.", i),
                              200, -0.25, 0.25);
        
        hChipPosXY[i] = new TH2F(Form("hChipPosXY_%i", i), Form("Chip %i position in X-Y plane;X [mm]; Y[mm];a.u.", i),
                                 400, xpos-20., xpos+20., 400, ypos-30.15*i-20., ypos-30.15*i+20.);
        hChipPosZY[i] = new TH2F(Form("hChipPosZY_%i", i), Form("Chip %i position in Z-Y plane;Z [mm]; Y[mm];a.u.", i),
                                 400, zpos-20., zpos+20., 400, ypos-30.15*i-20., ypos-30.15*i+20.);
        hChipPosXZ[i] = new TH2F(Form("hChipPosXZ_%i", i), Form("Chip %i position in X-Z plane;X [mm]; Z[mm];a.u.", i),
                                 400, xpos-20., xpos+20., 400, zpos-20., zpos+20.);
    }
    file_plots->cd();
    
    cout << "prealignment_vd() : Number of events found in tree: " << nentries << endl;

    for(Int_t ichip=0; ichip < n_chips; ++ichip) {
        for(Int_t icol=0; icol < scols; ++icol) {
            for(Int_t irow=0; irow < srows; ++irow) {
                TVector3 pos = align_chip[ichip].PixToGlobal(icol, irow);
                hChipPosXY[ichip]->Fill(pos.X(), pos.Y(), 2*srows-irow);
                hChipPosZY[ichip]->Fill(pos.Z(), pos.Y(), 2*srows-irow);
                hChipPosXZ[ichip]->Fill(pos.X(), pos.Z(), 2*srows-irow);
            }
        }
    }
    
    for(Long_t ivd=0; ivd < tree_vd->GetEntriesFast(); ++ivd) {
        tree_vd->GetEntry(ivd);
        //if( ivd < 100) cout << vd_event_n << endl;
        vd_event_n -= 1;
        if(vd_event_n < nentries)
            chain->GetEntry(vd_event_n);
        else {
            cout << "prealignment_vd() : WARNING more tracks than saved triggers" << endl;
            continue;
        }
        if(event->GetIntTrigCnt() != vd_event_n) cout << "prealignment_vd() : ERROR sync" << endl;
        if( (ivd+1)%1000 == 0 )
            cout << "Processed events: " << ivd+1 << " / " << tree_vd->GetEntriesFast() << endl;
        
        for(Int_t itrack=0; itrack < vd_n_tracks; ++itrack) {
            TVector3 to(*(TVector3*)vd_to->At(itrack));
            TVector3 td(*(TVector3*)vd_td->At(itrack));

            //if(dataset[itrack] >= 3) continue;
            //if(td.X() / td.Z() > 1) continue;
            
            TVector3 p = align_chip[4].IntersectionPointWithLine(to, td);
            hTrackHitsHIC->Fill(p.X(), p.Y());
            
            for(Int_t ichip=0; ichip < n_chips; ++ichip) {
                p = align_chip[ichip].IntersectionPointWithLine(to, td);
                hTrackHits[ichip]->Fill(p.X(), p.Y());
                
                //if( event->GetPlane(ichip)->GetNClustersSaved() > 20 ) continue;
                
                for(Int_t iclu=0; iclu < event->GetPlane(ichip)->GetNClustersSaved(); ++iclu) {
                    Int_t mult = event->GetPlane(ichip)->GetCluster(iclu)->GetMultiplicity();
                    BinaryCluster* cluster = event->GetPlane(ichip)->GetCluster(iclu);
                    
                    if(mult > 4) continue;
                    if(cluster->HasHotPixels()) continue;
                    if(cluster->HasBorderPixels()) continue;
                    if(cluster->HasExclDblcolPixels()) continue;
                    
                    TVector3 clupos = align_chip[ichip].PixToGlobal(cluster->GetX(), cluster->GetY());
                    if( TMath::Abs(clupos.Y() - p.Y()) < 0.5 ) {
                        hDX[ichip]->Fill( clupos.X() - p.X() );
                        hDXzoom[ichip]->Fill( clupos.X() - p.X() );
                    }
                    if( TMath::Abs(clupos.X() - p.X()) < 0.5 ) {
                        hDY[ichip]->Fill( clupos.Y() - p.Y() );
                        hDYzoom[ichip]->Fill( clupos.Y() - p.Y() );
                    }
                } // END FOR clusters
            } // END FOR chips
        } // END FOR tracks
    } // END FOR events

    TF1 *fres = new TF1("fres", "[0]*TMath::Gaus(x, [1], [2])+[3]", -0.2, 0.2);
    fres->SetParLimits(2, 10., 10000.);
    fres->SetParLimits(2, 0.001, 1);
    
    for(Short_t i=0; i<n_chips; ++i) {
        fres->SetParameter(0, 200.);
        fres->SetParameter(2, 0.01);
        fres->SetParameter(3, 1);

        fres->SetParameter(1, hDXzoom[i]->GetMean());
        hDXzoom[i]->Fit(fres, "RL");
        fres->SetParameter(1, hDYzoom[i]->GetMean());
        hDYzoom[i]->Fit(fres, "RL");
    }

    
    Int_t ctd = 4;
    
    TCanvas *c1 = new TCanvas("c1", "Canvas 1", 0, 0, 1800, 1000);
    c1->Divide(3,1);
    c1->cd(1);
    //hTrackHitsHIC->DrawCopy("COLZ");
    hChipPosXY[ctd]->DrawCopy("COLZ");
    c1->cd(2);
    //((TH1F*)hTrackHitsHIC->ProjectionX())->DrawCopy();
    hChipPosZY[ctd]->DrawCopy("COLZ");
    c1->cd(3);
    //((TH1F*)hTrackHitsHIC->ProjectionY())->DrawCopy();
    hChipPosXZ[ctd]->DrawCopy("COLZ");
    c1->Print(Form("%s/%s_c1.png", dirpath_plots.Data(), filename_out.Data()));

    Int_t np = TMath::CeilNint(TMath::Sqrt(n_chips));

    TCanvas *c2 = new TCanvas("c2", "Canvas 2", 50, 50, 1700, 1000);
    c2->Divide(2,2);
    c2->cd(1);
    hDX[ctd]->DrawCopy();
    c2->cd(2);
    hDY[ctd]->DrawCopy();
    c2->cd(3);
    hDXzoom[ctd]->DrawCopy();
    c2->cd(4);
    hDYzoom[ctd]->DrawCopy();
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
    
    file_vd->Close();
    delete file_vd;
    
    delete hTrackHitsHIC;
    for(Short_t i=0; i<n_chips; ++i) {
        delete hTrackHits[i];
        delete hDX[i];
        delete hDY[i];
        delete hDXzoom[i];
        delete hDYzoom[i];
        delete hChipPosXY[i];
        delete hChipPosZY[i];
        delete hChipPosXZ[i];
        
    }
    file_plots->Close();

    cout << "prealignment_vd() : Done!" << endl;
    return kTRUE;
}


