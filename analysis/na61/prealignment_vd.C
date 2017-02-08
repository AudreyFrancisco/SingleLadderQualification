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
    const TString file_id,        // identifier (suffix) for output files
    const Bool_t  extract_tracks = kTRUE
    ) {
    
    cout << "prealignment_vd() : Starting..." << endl;

    set_my_style();

    // general constants
    const TString filename_out = "prealignment_vd_" + file_id;
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_chips      = 9;   // number of chips

    // extraction variables
    const Float_t ex_sigma = 4.; // extratact tracaks in alignment peak inside mean +- ex_sigma*stdev
    
    // Alignment
    /*
    const Float_t xpos = 11.1;
    const Float_t ypos = 120.4;
    const Float_t zpos = 92.3;
    */
    
    const Float_t xpos = 11.044;
    const Float_t ypos = 120.6-0.117;
    const Float_t zpos = 92.243;
    const Float_t angY = 6.935*TMath::DegToRad();
    const Float_t angX = 0.;
    const Float_t angZ = 0.;

    Alignment align_chip[n_chips];
    for(Short_t i=0; i<n_chips; ++i) {
        align_chip[i].SetPos(TVector3(xpos, ypos-30.15*i, zpos));
        TRotation rot;
        rot.RotateY(angY);
        rot.RotateX(angX);
        rot.RotateZ(angZ);
        align_chip[i].SetRotation(rot);
    }
    
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

    // histograms
    TH2F *hTrackHitsHIC = new TH2F("hTrackHitsHIC", "Track hit map, HIC;X [mm];Y [mm];a.u.",
                                 400, -10., 30., 4000, -200., 200.);
    //hTrackHitsHIC->SetStats(0);
    TH2F *hTrackHitsMiss = new TH2F("hTrackHitsMiss", "Track hit map, tracks not intersecting HIC;X [mm];Y [mm];a.u.",
                                    400, -10., 30., 4000, -200., 200.);
    //hTrackHitsMiss->SetStats(0);
    
    TH2F *hTrackHits[n_chips];
    TH2F *hHits[n_chips];
    TH2F *hHitsAligned[n_chips];
    
    TH1F *hDX[n_chips];
    TH1F *hDY[n_chips];
    TH1F *hDXzoom[n_chips];
    TH1F *hDYzoom[n_chips];
    TH2F *hDXY[n_chips];
    
    TH2F *hChipPosXY[n_chips];
    TH2F *hChipPosZY[n_chips];
    TH2F *hChipPosXZ[n_chips];

    TH1F *hMult[n_chips];
    
    const Int_t binred = 2;
    for(Short_t i=0; i<n_chips; ++i) {
        file_plots->mkdir(Form("Chip_%i", i));
        file_plots->cd(Form("Chip_%i", i));
        
        hTrackHits[i] = new TH2F(Form("hTrackHits_%i", i), Form("Track hit map, chip %i;X [mm]; Y[mm];a.u.", i),
                               400, xpos-20., xpos+20., 400, ypos-30.15*i-20., ypos-30.15*i+20.);
        //hTrackHits[i]->SetStats(0);    
        hHits[i] = new TH2F(Form("hHits_%i", i), Form("Hit map all, chip %i;Column;Row;a.u.", i),
                            scols/binred, -0.5, scols-0.5, srows/binred, -0.5, srows-0.5);
        hHitsAligned[i] = new TH2F(Form("hHitsAlign_%i", i), Form("Hit map only aligned tracks, chip %i;Column;Row;a.u.", i),
                                   scols/binred, -0.5, scols-0.5, srows/binred, -0.5, srows-0.5);
        
        hDX[i] = new TH1F(Form("hDX_%i", i), Form("Difference track hit - cluster pos X, chip %i;#DeltaX [mm];a.u.", i),
                          400, xpos-20., xpos+20.);
        hDY[i] = new TH1F(Form("hDY_%i", i), Form("Difference track hit - cluster pos Y, chip %i;#DeltaY [mm];a.u.", i),
                          400, ypos-30.15*i-40., ypos-30.15*i+40.);
        hDXzoom[i] = new TH1F(Form("hDXzoom_%i", i), Form("Difference track hit - cluster pos X, chip %i;#DeltaX [mm];a.u.", i),
                              200, -0.25, 0.25);
        hDYzoom[i] = new TH1F(Form("hDYzoom_%i", i), Form("Difference track hit - cluster pos Y, chip %i;#DeltaY [mm];a.u.", i),
                              200, -0.25, 0.25);
        hDXY[i] = new TH2F(Form("hDXY_%i", i), Form("#DeltaX vs #DeltaY, chip %i;#DeltaX [mm];#DeltaY [mm];a.u.", i),
                           200, -0.25, 0.25, 200, -0.25, 0.25);
        //400, xpos-20., xpos+20., 400, ypos-30.15*i-40., ypos-30.15*i+40.);
        
        hChipPosXY[i] = new TH2F(Form("hChipPosXY_%i", i), Form("Chip %i position in X-Y plane;X [mm]; Y[mm];a.u.", i),
                                 400, xpos-20., xpos+20., 400, ypos-30.15*i-20., ypos-30.15*i+20.);
        hChipPosZY[i] = new TH2F(Form("hChipPosZY_%i", i), Form("Chip %i position in Z-Y plane;Z [mm]; Y[mm];a.u.", i),
                                 400, zpos-20., zpos+20., 400, ypos-30.15*i-20., ypos-30.15*i+20.);
        hChipPosXZ[i] = new TH2F(Form("hChipPosXZ_%i", i), Form("Chip %i position in X-Z plane;X [mm]; Z[mm];a.u.", i),
                                 400, xpos-20., xpos+20., 400, zpos-20., zpos+20.);

        hMult[i] = new TH1F(Form("hMult_%i", i), Form("Cluster size aligned, Chip %i;Number of pixels in cluster;a.u.", i),
                            101, -1.5, 100-0.5);
    }
    file_plots->cd();

    // extracted tracks tree
    TTree *tree_ex = NULL;
    Float_t ex_clux, ex_cluy;
    Float_t ex_to[3], ex_td[3];
    if(extract_tracks) {
        tree_ex = new TTree("extracted_tracks", "extracted_tracks");
        tree_ex->Branch("clu_x", &ex_clux, "clu_x/F");
        tree_ex->Branch("clu_y", &ex_cluy, "clu_y/F");
        tree_ex->Branch("track_origin", ex_to, "track_origin[3]/F");
        tree_ex->Branch("track_direction", ex_td, "track_direction[3]/F");
    }
    Float_t exx1 = 0., exx2=0., exy1=0., exy2=0.; // extract intervals    
    
    cout << "prealignment_vd() : Number of events found in tree: " << nentries << endl;

    // draw chip position
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
    
    for(Int_t loop=0; loop < 1+extract_tracks; ++loop) {
        // loop 0 drawing, loop 1 extracting and drawing extracted
        if(loop==0) cout << "prealignment_vd() : Starting prealignment..." << endl;
        if(loop==1) cout << "prealignment_vd() : Extracting tracks..." << endl;
        
        for(Long_t ivd=0; ivd < tree_vd->GetEntriesFast(); ++ivd) {
            // read events from vd tree and find equivalent in event tree
            tree_vd->GetEntry(ivd);
            vd_event_n -= 1;
            
            if(vd_event_n < nentries)
                chain->GetEntry(vd_event_n);
            else {
                cout << "prealignment_vd() : WARNING more tracks than saved triggers" << endl;
                continue;
            }
            
            if(event->GetIntTrigCnt() != vd_event_n)
                cout << "prealignment_vd() : ERROR sync" << endl;
            if( (ivd+1)%1000 == 0 )
                cout << "Processed events: " << ivd+1 << " / " << tree_vd->GetEntriesFast() << " in loop " << loop << endl;

            // loop over all tracks in event
            for(Int_t itrack=0; itrack < vd_n_tracks; ++itrack) {
                TVector3 to(*(TVector3*)vd_to->At(itrack));
                TVector3 td(*(TVector3*)vd_td->At(itrack));

                //_cuts____
                //if(dataset[itrack] != 4) continue; // {down1, down2, up1, up2, up1x}
                //if(td.X() / td.Z() > 1) continue;
                //_cuts____

                Bool_t flagHICHit = kFALSE;
                
                for(Int_t ichip=0; ichip < n_chips; ++ichip) {
                    if(loop==1 && ichip != 4) continue;
                    TVector3 p = align_chip[ichip].IntersectionPointWithLine(to, td);
                    if(loop==0) {
                        Float_t col, row;
                        align_chip[ichip].GlobalToPix(p, col, row);
                        if(col>=0 && row>=0 && col<scols && row<srows) {
                            hTrackHits[ichip]->Fill(p.X(), p.Y());
                            hTrackHitsHIC->Fill(p.X(), p.Y());
                            hHits[ichip]->Fill(col, row);
                            flagHICHit = kTRUE;
                        }
                    }
                
                    //if( event->GetPlane(ichip)->GetNClustersSaved() > 20 ) continue;
                
                    for(Int_t iclu=0; iclu < event->GetPlane(ichip)->GetNClustersSaved(); ++iclu) {
                        Int_t mult = event->GetPlane(ichip)->GetCluster(iclu)->GetMultiplicity();
                        BinaryCluster* cluster = event->GetPlane(ichip)->GetCluster(iclu);

                        //_cuts____
                        //if(mult > 10) continue;
                        //if(cluster->HasHotPixels()) continue;
                        //if(cluster->HasBorderPixels()) continue;
                        //if(cluster->HasExclDblcolPixels()) continue;
                        //_cuts____

                        TVector3 d = align_chip[ichip].DistPixLine(cluster->GetX(), cluster->GetY(), to, td, kFALSE);
                        if(loop==0) {
                            if( TMath::Abs(d.Y()) < 0.5 ) //_cuts____
                            {
                                hDX[ichip]->Fill( d.X() );
                                hDXzoom[ichip]->Fill( d.X() );
                            }
                            if( TMath::Abs(d.X()) < 0.5 ) //_cuts____
                            {
                                hDY[ichip]->Fill( d.Y() );
                                hDYzoom[ichip]->Fill( d.Y() );
                            }
                            hDXY[ichip]->Fill( d.X(), d.Y() );
                        }
                        else if(loop==1) {
                            if( exx1 < d.X() && d.X() < exx2 ) {
                                if( exy1 < d.Y() && d.Y() < exy2 ) {
                                    ex_clux = cluster->GetX();
                                    ex_cluy = cluster->GetY();
                                    to.GetXYZ(ex_to);
                                    td.GetXYZ(ex_td);
                                    tree_ex->Fill();
                                    Float_t col, row;
                                    align_chip[ichip].GlobalToPix(p, col, row);
                                    hHitsAligned[ichip]->Fill(col, row);
                                    hMult[ichip]->Fill(mult);
                                    //hHitsAligned[ichip]->Fill(ex_clux, ex_cluy);
                                }
                            }       
                        }
                        
                    } // END FOR clusters
                } // END FOR chips
                
                if(!flagHICHit && loop==0) {
                    TVector3 p = align_chip[4].IntersectionPointWithLine(to, td);
                    Float_t col, row;
                    align_chip[4].GlobalToPix(p, col, row);
                    hTrackHitsMiss->Fill(p.X(), p.Y());
                }
            } // END FOR tracks
        } // END FOR events

        if(loop > 0) {
            cout << "prealignment_vd() : Extracted " << tree_ex->GetEntriesFast() << " tracks." << endl;
            continue; // no need to redo fitting
        }
        TF1 *fres = new TF1("fres", "[0]*TMath::Gaus(x, [1], [2])+[3]", -0.2, 0.2);
        fres->SetParNames("Norm", "#mu", "#sigma", "Const (backg)");
        fres->SetNpx(1000);
        fres->SetParLimits(0, 10., 10000.);
        fres->SetParLimits(2, 0.001, 1);
    
        for(Short_t i=0; i<n_chips; ++i) {
            fres->SetParameter(0, 300.);
            fres->SetParameter(2, 0.008);
            fres->SetParameter(3, 1);
            
            fres->SetParameter(1, hDXzoom[i]->GetMean());
            hDXzoom[i]->Fit(fres, "QR");
            if(i == 4) {
                exx1 = fres->GetParameter(1) - ex_sigma*fres->GetParameter(2);
                exx2 = fres->GetParameter(1) + ex_sigma*fres->GetParameter(2);
            }
            
            fres->SetParameter(1, hDYzoom[i]->GetMean());
            hDYzoom[i]->Fit(fres, "QR");
            if(i == 4) {
                exy1 = fres->GetParameter(1) - ex_sigma*fres->GetParameter(2);
                exy2 = fres->GetParameter(1) + ex_sigma*fres->GetParameter(2);
            }
        }
    } // END FOR LOOP

    Int_t ctd = 4; // chip to draw
    
    TCanvas *c1 = new TCanvas("c1", "Canvas 1", 0, 0, 1800, 1000);
    if(0) {
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
    }
    else {
        c1->Divide(1,2);
        c1->cd(1);
        hHits[4]->DrawCopy("COLZ");
        c1->cd(2);
        hHitsAligned[4]->DrawCopy("COLZ");
    }
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

    file_plots->cd();
    file_plots->Write();

    delete chain;
    
    file_vd->Close();
    delete file_vd;
    
    delete hTrackHitsHIC;
    for(Short_t i=0; i<n_chips; ++i) {
        delete hTrackHits[i];
        delete hHits[i];
        delete hHitsAligned[i];
        delete hDX[i];
        delete hDY[i];
        delete hDXzoom[i];
        delete hDYzoom[i];
        delete hDXY[i];
        delete hChipPosXY[i];
        delete hChipPosZY[i];
        delete hChipPosXZ[i];
        delete hMult[i];
    }
    if(tree_ex) delete tree_ex;
    file_plots->Close();

    cout << "prealignment_vd() : Done!" << endl;
    
    return kTRUE;
}


