#include <Riostream.h>
#include <TMath.h>
#include <TF1.h>
#include <TFile.h>
#include "Na61Analysis.hpp"
#include "../classes/helpers.h"

using namespace std;

ClassImp(Na61Analysis)

//______________________________________________________________________
Na61Analysis::Na61Analysis():
    TObject(),
    fVerboseLevel(5)
{   
    fDirPathPlots = ".";
    fSuffix = "";
    
    fEventTree = new TChain("event_tree", "event_tree");
    fEvent = new BinaryEvent();
    
    fVDTracksTree = new TChain("tracks_tree", "tracks_tree");
    fVD_to = new TClonesArray("TVector3");
    fVD_td = new TClonesArray("TVector3");

    fExTracksTree = NULL;
    
    hTrackHitsHIC = NULL;
    hTrackHitsMiss = NULL;

    for(Int_t i=0; i<fNChips; ++i) {
        hTrackHits[i] = NULL;
        hHits[i] = NULL;
        hHitsAligned[i] = NULL;

        hDX[i] = NULL;
        hDY[i] = NULL;
        hDXzoom[i] = NULL;
        hDYzoom[i] = NULL;
        hDXY[i] = NULL;

        hChipPosXY[i] = NULL;
        hChipPosZY[i] = NULL;
        hChipPosXZ[i] = NULL;

        hMult[i] = NULL;
    }
    
    set_my_style();    
} 

//______________________________________________________________________
Na61Analysis::~Na61Analysis()
{
    fEventTree->ResetBranchAddresses();
    delete fEvent;
    delete fEventTree;
    
    fVDTracksTree->ResetBranchAddresses();
    delete fVDTracksTree;
    delete fVD_to;
    delete fVD_td;

    if(fExTracksTree) fExTracksTree->ResetBranchAddresses();
    delete fExTracksTree;
    
    // histograms
    delete hTrackHitsHIC;
    delete hTrackHitsMiss;

    for(Int_t i=0; i<fNChips; ++i) {
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

}

//______________________________________________________________________
void Na61Analysis::InitHistograms() 
{
    hTrackHitsHIC = new TH2F("hTrackHitsHIC", "Track hit map, HIC;X [mm];Y [mm];a.u.",
                             400, -10., 30., 4000, -200., 200.);
    //hTrackHitsHIC->SetStats(0);
    hTrackHitsMiss = new TH2F("hTrackHitsMiss", "Track hit map, tracks not intersecting HIC;X [mm];Y [mm];a.u.",
                              400, -10., 30., 4000, -200., 200.);
    //hTrackHitsMiss->SetStats(0);
    
    const Int_t binred = 2;
    for(Short_t i=0; i<fNChips; ++i) {
        Float_t xpos = fAlignChip[i].GetPosX();
        Float_t ypos = fAlignChip[i].GetPosY();
        Float_t zpos = fAlignChip[i].GetPosZ();
        
        hTrackHits[i] = new TH2F(Form("hTrackHits_%i", i), Form("Track hit map, chip %i;X [mm]; Y[mm];a.u.", i),
                               400, xpos-20., xpos+20., 400, ypos-20., ypos+20.);
        //hTrackHits[i]->SetStats(0);    
        hHits[i] = new TH2F(Form("hHits_%i", i), Form("Hit map all, chip %i;Column;Row;a.u.", i),
                            fNCols/binred, -0.5, fNCols-0.5, fNRows/binred, -0.5, fNRows-0.5);
        hHitsAligned[i] = new TH2F(Form("hHitsAlign_%i", i), Form("Hit map only aligned tracks, chip %i;Column;Row;a.u.", i),
                                   fNCols/binred, -0.5, fNCols-0.5, fNRows/binred, -0.5, fNRows-0.5);
        
        hDX[i] = new TH1F(Form("hDX_%i", i), Form("Difference track hit - cluster pos X, chip %i;#DeltaX [mm];a.u.", i),
                          400, xpos-20., xpos+20.);
        hDY[i] = new TH1F(Form("hDY_%i", i), Form("Difference track hit - cluster pos Y, chip %i;#DeltaY [mm];a.u.", i),
                          400, ypos-40., ypos+40.);
        hDXzoom[i] = new TH1F(Form("hDXzoom_%i", i), Form("Difference track hit - cluster pos X, chip %i;#DeltaX [mm];a.u.", i),
                              200, -0.25, 0.25);
        hDYzoom[i] = new TH1F(Form("hDYzoom_%i", i), Form("Difference track hit - cluster pos Y, chip %i;#DeltaY [mm];a.u.", i),
                              200, -0.25, 0.25);
        hDXY[i] = new TH2F(Form("hDXY_%i", i), Form("#DeltaX vs #DeltaY, chip %i;#DeltaX [mm];#DeltaY [mm];a.u.", i),
                           200, -0.25, 0.25, 200, -0.25, 0.25);
        //400, xpos-20., xpos+20., 400, ypos-40., ypos+40.);
        
        hChipPosXY[i] = new TH2F(Form("hChipPosXY_%i", i), Form("Chip %i position in X-Y plane;X [mm]; Y[mm];a.u.", i),
                                 400, xpos-20., xpos+20., 400, ypos-20., ypos+20.);
        hChipPosZY[i] = new TH2F(Form("hChipPosZY_%i", i), Form("Chip %i position in Z-Y plane;Z [mm]; Y[mm];a.u.", i),
                                 400, zpos-20., zpos+20., 400, ypos-20., ypos+20.);
        hChipPosXZ[i] = new TH2F(Form("hChipPosXZ_%i", i), Form("Chip %i position in X-Z plane;X [mm]; Z[mm];a.u.", i),
                                 400, xpos-20., xpos+20., 400, zpos-20., zpos+20.);

        hMult[i] = new TH1F(Form("hMult_%i", i), Form("Cluster size aligned, Chip %i;Number of pixels in cluster;a.u.", i),
                            101, -1.5, 100-0.5);
    }
    
}

//__________________________________________________________
void Na61Analysis::InitExtractedTracksTree() {
    fExTracksTree = new TTree("extracted_tracks", "extracted_tracks");
    fExTracksTree->Branch("clu_x", &fEx_clux, "clu_x/F");
    fExTracksTree->Branch("clu_y", &fEx_cluy, "clu_y/F");
    fExTracksTree->Branch("track_origin", fEx_to, "track_origin[3]/F");
    fExTracksTree->Branch("track_direction", fEx_td, "track_direction[3]/F");
}

//__________________________________________________________
Bool_t Na61Analysis::SetInputFileALPIDE(TString filepath_tree)
{
    fEventTree->Reset();
    if(!fEventTree->Add(filepath_tree.Data())) {
        TString err = "Cannot find event tree in file! "; err += filepath_tree;
        Report(1, err);
        return kFALSE;
    }
    fEventTree->SetBranchAddress("event", &fEvent);
    return kTRUE;
}

//__________________________________________________________
Bool_t Na61Analysis::SetInputFileVDTracks(TString filepath_tree)
{
    fVDTracksTree->Reset();
    if(!fVDTracksTree->Add(filepath_tree.Data())) {
        TString err = "Cannot find event tree in file! "; err += filepath_tree;
        Report(1, err);
        return kFALSE;
    }
    fVDTracksTree->SetBranchAddress("event_n", &fVD_EventN);
    fVDTracksTree->SetBranchAddress("n_tracks", &fVD_NTracks);
    fVDTracksTree->SetBranchAddress("dataset", fVD_Dataset);
    fVDTracksTree->SetBranchAddress("track_origin", &fVD_to);
    fVDTracksTree->SetBranchAddress("track_direction", &fVD_td);
    return kTRUE;
}

//__________________________________________________________
void Na61Analysis::SetDefaultAlignment() {
    const Float_t xpos = 11.044;
    const Float_t ypos = 120.6-0.117;
    const Float_t zpos = 92.243;
    const Float_t angY = 6.935*TMath::DegToRad();
    const Float_t angX = 0.;
    const Float_t angZ = 0.;

    for(Short_t i=0; i<fNChips; ++i) {
        Alignment a;
        a.SetPos(TVector3(xpos, ypos-30.15*i, zpos));
        TRotation rot;
        rot.RotateY(angY);
        rot.RotateX(angX);
        rot.RotateZ(angZ);
        a.SetRotation(rot);
        SetAlignment(i, a);
    }
}

//__________________________________________________________
Bool_t Na61Analysis::SetAlignment(Int_t chip, Alignment a) {
    if(chip < 0 || chip >= fNChips) {
        Report(1, Form("SetAlignment - chip %i out of range", chip));
        return kFALSE;
    }
    fAlignChip[chip] = a;
    return kTRUE;
}

//__________________________________________________________
Bool_t Na61Analysis::WriteHistograms(TString fname) {
    fname = fDirPathPlots + "/" + fname;
    TFile* file_plots = new TFile(fname.Data(), "RECREATE");
    if(!file_plots->IsOpen()) {
        Report(1, Form("ERROR: Cannot open output plots file! %s", fname.Data()));
        return kFALSE;
    }
    
    file_plots->cd();
    
    hTrackHitsHIC->Write();
    hTrackHitsMiss->Write();

    for(Int_t i=0; i<fNChips; ++i) {
        file_plots->mkdir(Form("Chip_%i", i));
        file_plots->cd(Form("Chip_%i", i));
        
        hTrackHits[i]->Write();
        hHits[i]->Write();
        hHitsAligned[i]->Write();

        hDX[i]->Write();
        hDY[i]->Write();
        hDXzoom[i]->Write();
        hDYzoom[i]->Write();
        hDXY[i]->Write();

        if(hChipPosXY[i]->GetEntries()) hChipPosXY[i]->Write();
        if(hChipPosZY[i]->GetEntries()) hChipPosZY[i]->Write();
        if(hChipPosXZ[i]->GetEntries()) hChipPosXZ[i]->Write();

        hMult[i]->Write();
    }
    file_plots->Close();
    delete file_plots;
}

// private
//__________________________________________________________
void Na61Analysis::Report(Short_t level, const char * message)
{
    // report a message
    // FATAL   = level 0
    // ERROR   = level 1
    // WARNING = level 2
    // INFO    = level 3
    // DEBUG   = level 4
    if(level > fVerboseLevel) return;
    cout << "Na61Analysis : ";
    switch(level) {
    case 0 : cout << " FATAL  : "; break;
    case 1 : cout << " ERROR  : "; break;
    case 2 : cout << "WARNING : "; break;
    case 3 : cout << " INFO   : "; break;
    case 4 : cout << " DEBUG  : "; break;
    default: cout << " CUSTOM : ";
    }
    cout << message << endl;
}

//__________________________________________________________
void Na61Analysis::FillChipPosHistos()
{   
    // draw chip position
    for(Int_t ichip=0; ichip < fNChips; ++ichip) {
        for(Int_t icol=0; icol < fNCols; ++icol) {
            for(Int_t irow=0; irow < fNRows; ++irow) {
                TVector3 pos = fAlignChip[ichip].PixToGlobal(icol, irow);
                hChipPosXY[ichip]->Fill(pos.X(), pos.Y(), 2*fNRows-irow);
                hChipPosZY[ichip]->Fill(pos.Z(), pos.Y(), 2*fNRows-irow);
                hChipPosXZ[ichip]->Fill(pos.X(), pos.Z(), 2*fNRows-irow);
            }
        }
    }
}

//__________________________________________________________
void Na61Analysis::PreAlignmentVD() {

    Report(3, "PreAlignmentVD() : Starting");

    const Float_t ex_sigma = 4.; // extratact tracks in alignment peak inside mean +- ex_sigma*stdev

    Bool_t extract_tracks = kFALSE+1;

    InitExtractedTracksTree();
    
    Float_t exx1 = 0., exx2=0., exy1=0., exy2=0.; // extract intervals

    Int_t nentries_al = fEventTree->GetEntries();
    Int_t nentries_vd = fVDTracksTree->GetEntries();
    for(Int_t loop=0; loop < 1+extract_tracks; ++loop) {
        // loop 0 drawing, loop 1 extracting and drawing extracted
        if(loop==0) Report(3, "PreAlignmentVD() : Starting prealignment");
        if(loop==1) Report(3, "PreAlignmentVD() : Extracting tracks");
        
        for(Int_t ivd=0; ivd < nentries_vd; ++ivd) {
            // read events from vd tree and find equivalent in event tree
            fVDTracksTree->GetEntry(ivd);
            
            fVD_EventN -= 1; // correct for different trigger numbering between VD and ALPIDE
            
            if(fVD_EventN > nentries_al) {
                Report(2, "PreAlignmentVD() : WARNING more tracks than saved triggers");
                continue;
            }
            
            fEventTree->GetEntry(fVD_EventN);
            
            if(fEvent->GetIntTrigCnt() != fVD_EventN)
                Report(2, "PreAlignmentVD() : sync error");
            
            if( (ivd+1)%1000 == 0 )
                Report(3, Form("PreAlignmentVD() : Processed events %i / %i", ivd+1, nentries_vd));
            
            // loop over all tracks in event
            for(Int_t itrack=0; itrack < fVD_NTracks; ++itrack) {
                TVector3 to(*(TVector3*)fVD_to->At(itrack));
                TVector3 td(*(TVector3*)fVD_td->At(itrack));

                //_cuts____
                //if(dataset[itrack] < 7) continue; // {down1, down2, up1, up2, up1x, 3pt0, 3pt1, 3pt3}
                //if(td.X() / td.Z() > 1) continue;
                Float_t c4, r4;
                TVector3 p4 = fAlignChip[4].IntersectionPointWithLine(to, td);
                fAlignChip[4].GlobalToPix(p4, c4, r4);
                if(r4<0) continue;
                //if(c4<0 || c4>fNCols) continue; // only chip4 is interesting
                //_cuts____
                
                Bool_t flagHICHit = kFALSE;
                
                for(Int_t ichip=0; ichip < fNChips; ++ichip) {
                    if(loop==1 && ichip != 4) continue;
                    TVector3 p = fAlignChip[ichip].IntersectionPointWithLine(to, td);
                    if(loop==0) {
                        Float_t col, row;
                        fAlignChip[ichip].GlobalToPix(p, col, row);
                        if(col>=0 && row>=0 && col<fNCols && row<fNRows) {
                            hTrackHits[ichip]->Fill(p.X(), p.Y());
                            hTrackHitsHIC->Fill(p.X(), p.Y());
                            hHits[ichip]->Fill(col, row);
                            flagHICHit = kTRUE;
                        }
                    }
                
                    //if( fEvent->GetPlane(ichip)->GetNClustersSaved() > 20 ) continue;
                
                    for(Int_t iclu=0; iclu < fEvent->GetPlane(ichip)->GetNClustersSaved(); ++iclu) {
                        Int_t mult = fEvent->GetPlane(ichip)->GetCluster(iclu)->GetMultiplicity();
                        BinaryCluster* cluster = fEvent->GetPlane(ichip)->GetCluster(iclu);

                        //_cuts____
                        //if(mult > 10) continue;
                        //if(cluster->HasHotPixels()) continue;
                        //if(cluster->HasBorderPixels()) continue;
                        //if(cluster->HasExclDblcolPixels()) continue;
                        //_cuts____

                        TVector3 d = fAlignChip[ichip].DistPixLine(cluster->GetX(), cluster->GetY(), to, td, kFALSE);
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
                                    fEx_clux = cluster->GetX();
                                    fEx_cluy = cluster->GetY();
                                    to.GetXYZ(fEx_to);
                                    td.GetXYZ(fEx_td);
                                    fExTracksTree->Fill();
                                    Float_t col, row;
                                    fAlignChip[ichip].GlobalToPix(p, col, row);
                                    hHitsAligned[ichip]->Fill(col, row);
                                    hMult[ichip]->Fill(mult);
                                    //hHitsAligned[ichip]->Fill(fEx_clux, fEx_cluy);
                                }
                            }       
                        }
                        
                    } // END FOR clusters
                } // END FOR chips
                
                if(!flagHICHit && loop==0) {
                    TVector3 p = fAlignChip[4].IntersectionPointWithLine(to, td);
                    Float_t col, row;
                    fAlignChip[4].GlobalToPix(p, col, row);
                    hTrackHitsMiss->Fill(p.X(), p.Y());
                }
            } // END FOR tracks
        } // END FOR events

        if(loop > 0) {
            cout << "prealignment_vd() : Extracted " << fExTracksTree->GetEntriesFast() << " tracks." << endl;
            continue; // no need to redo fitting
        }
        
        TF1 *fres = new TF1("fres", "[0]*TMath::Gaus(x, [1], [2])+[3]", -0.2, 0.2);
        fres->SetParNames("Norm", "#mu", "#sigma", "Const (backg)");
        fres->SetNpx(1000);
        fres->SetParLimits(0, 10., 10000.);
        fres->SetParLimits(2, 0.001, 1);
    
        for(Short_t i=0; i<fNChips; ++i) {
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
    
}
