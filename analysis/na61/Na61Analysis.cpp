#include <Riostream.h>
#include <TMath.h>
#include <TF1.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TAxis.h>

#include "Na61Analysis.hpp"
#include "../classes/helpers.h"

using namespace std;

ClassImp(Na61Analysis)

//______________________________________________________________________
Na61Analysis::Na61Analysis():
    TObject(),
    fVerboseLevel(5)
{   
    fDirPathPlots = "./";
    fSuffix = "";
    
    fEventTree = new TChain("event_tree", "event_tree");
    fEvent = new BinaryEvent();
    
    fVDTracksTree = new TChain("tracks_tree", "tracks_tree");
    fVD_to = new TClonesArray("TVector3");
    fVD_td = new TClonesArray("TVector3");
    fVD_hits1 = new TClonesArray("TVector3");
    fVD_hits2 = new TClonesArray("TVector3");
    fVD_hits4 = new TClonesArray("TVector3");
    
    fExTracksTree = NULL;
    fExHitsTree = NULL;
    
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

        hEffGood[i] = NULL;
        hEffRej[i] = NULL; 
        hEffIneff[i] = NULL;
        hEffEff[i] = NULL;
        hEffEffD[i] = NULL;
        hEffCluMult[i] = NULL;
        
        fEffNTracks[i] = 0;
        fEffNRej   [i] = 0;
        fEffNGood  [i] = 0;
        fEffNEff   [i] = 0;
        fEffNIneff [i] = 0;

    }
    
    fResMean[0] = -1.;
    fResMean[1] = -1.;
    fResSigma[0] = -1.;
    fResSigma[1] = -1.;
    
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
    delete fVD_hits1;
    delete fVD_hits2;
    delete fVD_hits4;

    if(fExTracksTree) fExTracksTree->ResetBranchAddresses();
    delete fExTracksTree;
    if(fExHitsTree) fExHitsTree->ResetBranchAddresses();
    delete fExHitsTree;
    
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

        delete hEffGood[i];
        delete hEffRej[i]; 
        delete hEffIneff[i];
        delete hEffEff[i]; 
        delete hEffEffD[i];
        delete hEffCluMult[i];
    }
}

//______________________________________________________________________
void Na61Analysis::InitHistograms(const Int_t binred) 
{
    hTrackHitsHIC = new TH2F("hTrackHitsHIC", "Track hit map, HIC;X [mm];Y [mm];a.u.",
                             400, -10., 30., 4000, -200., 200.);
    //hTrackHitsHIC->SetStats(0);
    hTrackHitsMiss = new TH2F("hTrackHitsMiss", "Track hit map, tracks not intersecting HIC;X [mm];Y [mm];a.u.",
                              400, -10., 30., 4000, -200., 200.);
    //hTrackHitsMiss->SetStats(0);
    
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
        // efficiency
        hEffGood[i] = new TH2F(Form("hEffGood_%i", i), Form("Accepted tracks for efficiency calc, chip %i;Column;Row;a.u.", i),
                               fNCols/binred, -0.5, fNCols-0.5, fNRows/binred, -0.5, fNRows-0.5);
        hEffIneff[i] = new TH2F(Form("hEffIneff_%i", i), Form("Inefficient tracks, chip %i;Column;Row;a.u.", i),
                                fNCols/binred, -0.5, fNCols-0.5, fNRows/binred, -0.5, fNRows-0.5);
        hEffEff[i] = new TH2F(Form("hEffEff_%i", i), Form("Efficient tracks, chip %i;Column;Row;a.u.", i),
                              fNCols/binred, -0.5, fNCols-0.5, fNRows/binred, -0.5, fNRows-0.5);
        hEffRej[i] = new TH2F(Form("hEffRej_%i", i), Form("Rejected tracks for efficiency calculation, chip %i;Column;Row;a.u.", i),
                              (fNCols+512)/binred, -256-0.5, 256+fNCols-0.5, (fNRows+512)/binred, -384-0.5, 128+fNRows-0.5);
        hEffEffD[i] = new TH2F(Form("hEffD_%i", i), Form("Efficient tracks relative to hit position, chip %i;X [mm];Y [mm];a.u.", i),
                               800, -0.4, 0.4, 800, -0.4, 0.4);
        hEffCluMult[i] = new TH1F(Form("hEffCluMult_%i", i), Form("Cluter multiplicity of efficient events, chip %i;# of clusters in event; a.u.", i),
                                  200, -0.5, 199.5);
    }
    
}

//__________________________________________________________
void Na61Analysis::DrawHistograms(TString set, Int_t ichip) {
    if(ichip < 0 || ichip >= fNChips) {
        Report(2, Form("Chip %i out of range 0-%i. Drawing chip 4.", ichip, fNChips));
        ichip = 4;
    }
    set.ToLower();
    if(set.BeginsWith("prealign")) {
        TCanvas *c = new TCanvas("c_"+set, set, 50, 50, 1700, 1000);
        c->Divide(2,2);
        c->cd(1);
        hDX[ichip]->DrawCopy();
        c->cd(2);
        hDY[ichip]->DrawCopy();
        c->cd(3);
        hDXzoom[ichip]->DrawCopy();
        c->cd(4);
        hDYzoom[ichip]->DrawCopy();
    }
    else if(set.BeginsWith("eff")) {
        TCanvas *c = new TCanvas("c_"+set, set, 50, 50, 1700, 1000);
        c->Divide(2,2);
        c->cd(1);
        hEffRej[ichip]->DrawCopy("COLZ");
        c->cd(2);
        hEffGood[ichip]->DrawCopy("COLZ");
        c->cd(3);
        hEffIneff[ichip]->SetMaximum(1);
        hEffIneff[ichip]->DrawCopy("COLZ");
        c->cd(4);
        hEffEff[ichip]->DrawCopy("COLZ");
        TCanvas *cc = new TCanvas("cc_"+set, set, 700, 300, 800, 800);
        cc->Divide(2,2);
        cc->cd(1);
        hEffEffD[ichip]->DrawCopy("COLZ");
        cc->cd(2);
        hEffCluMult[ichip]->DrawCopy();
        cc->cd(3);
        hEffEffD[ichip]->ProjectionX()->DrawCopy();
        cc->cd(4);
        hEffEffD[ichip]->ProjectionY()->DrawCopy();        
    }
    
}

//__________________________________________________________
Bool_t Na61Analysis::WriteHistograms(TString fname, TString opt) {
    fname = fDirPathPlots + fname;
    TFile* file_plots = new TFile(fname.Data(), opt.Data());
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
        
        hTrackHits  [i]->Write();
        hHits       [i]->Write();
        hHitsAligned[i]->Write();

        hDX    [i]->Write();
        hDY    [i]->Write();
        hDXzoom[i]->Write();
        hDYzoom[i]->Write();
        hDXY   [i]->Write();

        if(hChipPosXY[i]->GetEntries()) hChipPosXY[i]->Write();
        if(hChipPosZY[i]->GetEntries()) hChipPosZY[i]->Write();
        if(hChipPosXZ[i]->GetEntries()) hChipPosXZ[i]->Write();

        hMult[i]->Write();
        
        if(hEffGood[i]->GetEntries()) hEffGood   [i]->Write();
        if(hEffGood[i]->GetEntries()) hEffRej    [i]->Write();
        if(hEffGood[i]->GetEntries()) hEffIneff  [i]->Write();
        if(hEffGood[i]->GetEntries()) hEffEff    [i]->Write();
        if(hEffGood[i]->GetEntries()) hEffEffD   [i]->Write();
        if(hEffGood[i]->GetEntries()) hEffCluMult[i]->Write();
    }
    file_plots->Close();
    delete file_plots;
    Report(3, "Histograms written to file successfully.");
    return kTRUE;
}

//__________________________________________________________
Bool_t Na61Analysis::WriteTracksTree(TString fname, TString opt) {
    if(!fExTracksTree) {
        Report(1, "Extracted tracks tree does NOT exist!");
        return kFALSE;
    }
    fname = fDirPathPlots + "/" + fname;
    TFile* file_plots = new TFile(fname.Data(), opt.Data());
    if(!file_plots->IsOpen()) {
        Report(1, Form("ERROR: Cannot open output plots file! %s", fname.Data()));
        return kFALSE;
    } 
    file_plots->cd();
    fExTracksTree->Write();
    file_plots->Close();
    delete file_plots;
    Report(3, "Tracks tree written to file successfully.");
    return kTRUE;
}

//__________________________________________________________
Bool_t Na61Analysis::SetInputFileALPIDE(TString filepath_tree)
{
    Report(3, "Setting new ALPIDE file: " + filepath_tree);
    fEventTree->Reset();
    if(!fEventTree->Add(filepath_tree.Data())) {
        Report(1, "Cannot find event tree in file! " + filepath_tree);
        return kFALSE;
    }
    else if( !fEventTree->GetEntries() ) {
        Report(1, "Cannot find entries in file! " + filepath_tree);
        return kFALSE;
    }
    fEventTree->SetBranchAddress("event", &fEvent);
    return kTRUE;
}

//__________________________________________________________
Bool_t Na61Analysis::SetInputFileVDTracks(TString filepath_tree)
{
    Report(3, "Setting new VD tracks file: " + filepath_tree);
    fVDTracksTree->Reset();
    if(!fVDTracksTree->Add(filepath_tree.Data())) {
        Report(1, "Cannot find tracks tree in file! " + filepath_tree);
        return kFALSE;
    }
    else if( !fVDTracksTree->GetEntries() ) {
        Report(1, "Cannot find entries in file! " + filepath_tree);
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
        Alignment aln;
        aln.SetPos(TVector3(xpos, ypos-30.15*i, zpos));
        TRotation rot;
        rot.RotateY(angY);
        rot.RotateX(angX);
        rot.RotateZ(angZ);
        aln.SetRotation(rot);
        SetAlignment(i, aln);
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
Bool_t Na61Analysis::LoadHotPixMask(Int_t ichip, TString fname) {
    Report(4, "Loading hot pixel mask from file " + fname);
    ifstream file_mask(fname.Data()); // read hot pixels from mask file
    if(!file_mask.is_open()) {
        Report(1, "Unable to open hot pixel mask file: " + fname);
        return kFALSE;
    }
    Short_t col, row;
    file_mask >> col >> row;
    while(file_mask.good()) {
        fHotPixRows[ichip].push_back(row);
        fHotPixCols[ichip].push_back(col);
        file_mask >> col >> row;
    }
    file_mask.close();
    Report(3, Form("Succesfully loaded %i hot pixels for chip %i", (Int_t)fHotPixRows[ichip].size(), ichip));
    return kTRUE;
}


//__________________________________________________________
Bool_t Na61Analysis::LoadExclColMask(Int_t ichip, TString fname) {
    Report(4, "Loading excluded columns mask from file " + fname);
    ifstream file_mask(fname.Data()); // read hot pixels from mask file
    if(!file_mask.is_open()) {
        Report(1, "Unable to open excluded columns mask file: " + fname);
        return kFALSE;
    }
    Short_t col;
    file_mask >> col;
    while(file_mask.good()) {
        fExclCols[ichip].push_back(col);
        file_mask >> col;
    }
    file_mask.close();
    Report(3, Form("Succesfully loaded %i excluded columns for chip %i", (Int_t)fExclCols[ichip].size(), ichip));
    return kTRUE;
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
void Na61Analysis::PrealignmentVD(Float_t ex_sigma) {

    Float_t extract_tracks = ex_sigma > 0 ? kTRUE : kFALSE;
    TString mname = extract_tracks ? "ExtractTracksVD() : " : "PrealignmentVD() : ";
    
    if(extract_tracks && fResSigma[0] < 0.)
        Report(2, mname + "Extracting tracks but stdev < 1");
    
    Float_t exx1=0., exx2=0., exy1=0., exy2=0.; // extract intervals
    Float_t ex_clux, ex_cluy, ex_to[3], ex_td[3];
    
    if(extract_tracks) {
        if(fExTracksTree) {
            Report(2, "Extracted tracks tree already initialised.");
            fExTracksTree->SetBranchAddress("clu_x", &ex_clux, "clu_x/F");
            fExTracksTree->SetBranchAddress("clu_y", &ex_cluy, "clu_y/F");
            fExTracksTree->SetBranchAddress("track_origin", ex_to, "track_origin[3]/F");
            fExTracksTree->SetBranchAddress("track_direction", ex_td, "track_direction[3]/F");
        }
        else {
            fExTracksTree = new TTree("extracted_tracks", "extracted_tracks");
            fExTracksTree->Branch("clu_x", &ex_clux, "clu_x/F");
            fExTracksTree->Branch("clu_y", &ex_cluy, "clu_y/F");
            fExTracksTree->Branch("track_origin", ex_to, "track_origin[3]/F");
            fExTracksTree->Branch("track_direction", ex_td, "track_direction[3]/F");
            Report(3, "Extracted tracks tree intialised.");
        }
        // extratact tracks in alignment peak inside mean +- ex_sigma*stdev
        exx1 = fResMean[0] - ex_sigma*fResSigma[0];
        exx2 = fResMean[0] + ex_sigma*fResSigma[0];
        exy1 = fResMean[1] - ex_sigma*fResSigma[1];
        exy2 = fResMean[1] + ex_sigma*fResSigma[1];
    }
    
    Int_t nentries_al = fEventTree->GetEntries();
    Int_t nentries_vd = fVDTracksTree->GetEntries();
    
    if(extract_tracks) Report(3, mname + "Starting extraction");
    else               Report(3, mname + "Starting prealignment");
    for(Int_t ivd=0; ivd < nentries_vd; ++ivd) {
        // read events from vd tree and find equivalent in event tree
        fVDTracksTree->GetEntry(ivd);
        
        fVD_EventN -= 1; // correct for different trigger numbering between VD and ALPIDE
        
        if(fVD_EventN > nentries_al) {
            Report(2, mname + "WARNING more tracks than saved triggers");
            continue;
        }
        
        fEventTree->GetEntry(fVD_EventN);
            
        if(fEvent->GetIntTrigCnt() != fVD_EventN)
            Report(2, mname + "sync error");
        
        if( (ivd+1)%5000 == 0 )
            Report(3, Form(mname + "Processed events %i / %i", ivd+1, nentries_vd));
            
        // loop over all tracks in event
        for(Int_t itrack=0; itrack < fVD_NTracks; ++itrack) {
            TVector3 to(*(TVector3*)fVD_to->At(itrack));
            TVector3 td(*(TVector3*)fVD_td->At(itrack));

            //_cuts____
            //if(fVD_Dataset[itrack] < 7) continue; // {down1, down2, up1, up2, up1x, 3pt0, 3pt1, 3pt3}
            //if(td.X() / td.Z() > 1) continue;
            Float_t c4, r4;
            TVector3 p4 = fAlignChip[4].IntersectionPointWithLine(to, td);
            fAlignChip[4].GlobalToPix(p4, c4, r4);
            if(r4<0) continue;
            //if(c4<0 || c4>fNCols) continue; // only chip4 is interesting
            //_cuts____
                
            Bool_t flagHICHit = kFALSE;
            
            for(Int_t ichip=0; ichip < fNChips; ++ichip) {
                //if( fEvent->GetPlane(ichip)->GetNClustersSaved() > 20 ) continue;
                if(extract_tracks && ichip != 4) continue; // only interested in extracting tracks for chip for

                TVector3 p = fAlignChip[ichip].IntersectionPointWithLine(to, td);
                Float_t col, row; 
                if(!extract_tracks) {                    
                    fAlignChip[ichip].GlobalToPix(p, col, row);
                    if(col>=0 && row>=0 && col<fNCols && row<fNRows) {
                        hTrackHits[ichip]->Fill(p.X(), p.Y());
                        hTrackHitsHIC->Fill(p.X(), p.Y());
                        hHits[ichip]->Fill(col, row);
                        flagHICHit = kTRUE;
                    }
                }
                
                for(Int_t iclu=0; iclu < fEvent->GetPlane(ichip)->GetNClustersSaved(); ++iclu) {
                    Int_t mult = fEvent->GetPlane(ichip)->GetCluster(iclu)->GetMultiplicity();
                    BinaryCluster* cluster = fEvent->GetPlane(ichip)->GetCluster(iclu);

                    //_cuts____
                    //if(mult > 10) continue;
                    //if(cluster->HasHotPixels()) continue;
                    //if(cluster->HasBorderPixels()) continue;
                    //if(cluster->HasExclDblcolPixels()) continue;
                    //_cuts____

                    TVector3 d = fAlignChip[ichip].DistPixLine(cluster->GetX(), cluster->GetY(), to, td, kTRUE);
                    if(extract_tracks) {
                        if( exx1 < d.X() && d.X() < exx2
                            && exy1 < d.Y() && d.Y() < exy2 ) {
                            // fill extracted tree
                            ex_clux = cluster->GetX();
                            ex_cluy = cluster->GetY();
                            to.GetXYZ(ex_to);
                            td.GetXYZ(ex_td);
                            fExTracksTree->Fill();
                            // fill histograms
                            fAlignChip[ichip].GlobalToPix(p, col, row);
                            hHitsAligned[ichip]->Fill(col, row);
                            hMult[ichip]->Fill(mult);
                        }       
                    }
                    else {
                        if( TMath::Abs(d.Y()) < 0.5 ) { //_cuts____
                            hDX[ichip]->Fill( d.X() );
                            hDXzoom[ichip]->Fill( d.X() );
                        }
                        if( TMath::Abs(d.X()) < 0.5 ) { //_cuts____
                            hDY[ichip]->Fill( d.Y() );
                            hDYzoom[ichip]->Fill( d.Y() );
                        }
                        hDXY[ichip]->Fill( d.X(), d.Y() );
                    }
                } // END FOR clusters
            } // END FOR chips
                
            if(!extract_tracks && !flagHICHit) {
                TVector3 p = fAlignChip[4].IntersectionPointWithLine(to, td);
                Float_t col, row;
                fAlignChip[4].GlobalToPix(p, col, row); // chip 4 is just used as reference. any chip can be used
                hTrackHitsMiss->Fill(p.X(), p.Y());
            }
        } // END FOR tracks
    } // END FOR events

    if(extract_tracks) {
        Report(3, mname + Form("Extracted %i tracks.", (Int_t)fExTracksTree->GetEntriesFast() ));
    }
    else {
        TF1 *fres = new TF1("fres", "[0]*TMath::Gaus(x, [1], [2])+[3]", -0.2, 0.2);
        fres->SetParNames("Norm", "#mu", "#sigma", "Const (backg)");
        fres->SetNpx(1000);
        fres->SetParLimits(0, 10., 10000.);
        fres->SetParLimits(2, 0.001, 1);
        
        for(Short_t i=0; i<fNChips; ++i) {
            if(hDXzoom[i]->GetEntries() < 10 || hDYzoom[i]->GetEntries() < 10) {
                Report( (i==4 ? 3 : 4), mname + Form("Skipping fit - insufficent tracks in residual plots, chip %i", i));
                continue;
            }
            fres->SetParameter(0, 300.);
            fres->SetParameter(2, 0.008);
            fres->SetParameter(3, 1);
            
            fres->SetParameter(1, hDXzoom[i]->GetMean());
            hDXzoom[i]->Fit(fres, "QR0");
            if(i == 4) {
                fResMean[0] = fres->GetParameter(1);
                fResSigma[0] = fres->GetParameter(2);
                Report(4, Form(mname + "Residual mean = %f, sigma = %f", fResMean[0]*1e3, fResSigma[0]*1e3));
            }
            
            fres->SetParameter(1, hDYzoom[i]->GetMean());
            hDYzoom[i]->Fit(fres, "QR0");
            if(i == 4) {
                fResMean[1] = fres->GetParameter(1);
                fResSigma[1] = fres->GetParameter(2);
                Report(4, Form(mname + "Residual mean = %f, sigma = %f", fResMean[1]*1e3, fResSigma[1]*1e3));
            }
            // reset drawing bit
            hDXzoom[i]->GetFunction("fres")->ResetBit(1<<9);
            hDYzoom[i]->GetFunction("fres")->ResetBit(1<<9);
        }
        delete fres;
    }

    if(extract_tracks && fExTracksTree) fExTracksTree->ResetBranchAddresses();
    Report(3, mname + "Finished.");
}

//__________________________________________________________
void Na61Analysis::EfficiencyVD(Int_t ichip) {
    TString mname = "EfficiencyVD() : ";
    Report(3, mname + Form("Preliminary efficiency = %f", 100.*hHitsAligned[4]->GetEntries()/hHits[4]->GetEntries() ));

    Float_t wx = 0.1; // search radius in x [mm]
    Float_t wy = 0.1; // search rafius in y [mm]
    Int_t   wc = TMath::Ceil(wy/29.24e-3); // search radius in columns
    Int_t   wr = TMath::Ceil(wx/26.88e-3); // search radius in rows

    Int_t nentries_al = fEventTree->GetEntries();
    Int_t nentries_vd = fVDTracksTree->GetEntries();

    Int_t nn_tracks = 0;
    Int_t nn_good   = 0;
    Int_t nn_eff    = 0;
    Int_t nn_rej    = 0;
    Int_t nn_ineff  = 0;
    Int_t nn_mult   = 0;
    Int_t nn_discev = 0;
    
    for(Int_t ivd=0; ivd < nentries_vd; ++ivd) {
        // read events from vd tree and find equivalent in event tree
        fVDTracksTree->GetEntry(ivd);
        
        fVD_EventN -= 1; // correct for different trigger numbering between VD and ALPIDE
        
        if(fVD_EventN > nentries_al) {
            Report(2, mname + "WARNING more tracks than saved triggers");
            continue;
        }
        
        fEventTree->GetEntry(fVD_EventN);
            
        if(fEvent->GetIntTrigCnt() != fVD_EventN)
            Report(2, mname + "sync error");
        
        if( (ivd+1)%5000 == 0 )
            Report(3, Form(mname + "Processed events %i / %i", ivd+1, nentries_vd));

        Int_t nnt_good = 0;
        Int_t nnt_eff  = 0;
        
        // loop over all tracks in event
        for(Int_t itrack=0; itrack < fVD_NTracks; ++itrack) {
            TVector3 to(*(TVector3*)fVD_to->At(itrack));
            TVector3 td(*(TVector3*)fVD_td->At(itrack));
            nn_tracks++;
            
            //_cuts____
            //if(fVD_Dataset[itrack] < 7) continue; // {down1, down2, up1, up2, up1x, 3pt0, 3pt1, 3pt3}
            //_cuts____
                
            //if( fEvent->GetPlane(ichip)->GetNClustersSaved() > 20 ) continue;

            TVector3 p = fAlignChip[ichip].IntersectionPointWithLine(to, td);
            Float_t col, row;
            fAlignChip[ichip].GlobalToPix(p, col, row);

            Bool_t flagRej = kFALSE;
            if(col<=wc || row<=wr || col>fNCols-wc || row>fNRows-wr)
                flagRej = kTRUE;
            for(UInt_t i=0; i<fHotPixCols[ichip].size(); i++)
                if( TMath::Abs(col-fHotPixCols[ichip].at(i)) < wc && TMath::Abs(row-fHotPixRows[ichip].at(i)) < wr)
                    flagRej = kTRUE;
            for(UInt_t i=0; i<fExclCols[ichip].size(); i++)
                if( TMath::Abs(col-fExclCols[ichip].at(i)) < wc )
                    flagRej = kTRUE;
            
            if(flagRej) {
                nn_rej++;
                hEffRej[ichip]->Fill(col, row);
                continue;
            }
            
            nn_good++;
            hEffGood[ichip]->Fill(col, row);
            nnt_good++;

            Int_t nhits = 0;
            for(Int_t iclu=0; iclu < fEvent->GetPlane(ichip)->GetNClustersSaved(); ++iclu) {
                Int_t mult = fEvent->GetPlane(ichip)->GetCluster(iclu)->GetMultiplicity();
                BinaryCluster* cluster = fEvent->GetPlane(ichip)->GetCluster(iclu);

                //_cuts____
                //if(mult > 10) continue;
                //if(cluster->HasHotPixels()) continue;
                //if(cluster->HasBorderPixels()) continue;
                //if(cluster->HasExclDblcolPixels()) continue;
                //_cuts____

                TVector3 d = fAlignChip[ichip].DistPixLine(cluster->GetX(), cluster->GetY(), to, td, kTRUE); // search in localc coordinates

                if( d.X()*d.X()/wx/wx + d.Y()*d.Y()/wy/wy < 1. ) {
                    nhits++;
                    hEffEffD[ichip]->Fill(d.X(), d.Y());
                }
                
            } // END FOR clusters
            
            if( nhits >= 1 ) {
                nn_eff++;
                hEffEff[ichip]->Fill(col, row);
                nnt_eff++;
            }
            else {
                nn_ineff++;
                hEffIneff[ichip]->Fill(col, row);
            }
            
            if( nhits >  1 ) {
                nn_mult++;
            }

            //if( nhits >  1 ) Report(3, mname + "Double hit");
            
        } // END FOR tracks
        
        //if(100.*nnt_eff/nnt_good < 99.9)
        //    Report(3, mname + Form("Event = %i, Efficiency = %.2f    %i %i %i", fVD_EventN+1, 100.*nnt_eff/nnt_good, nnt_eff, nnt_good, fVD_NTracks));
        if(nnt_eff > 1) {
            hEffCluMult[ichip]->Fill( fEvent->GetPlane(ichip)->GetNClustersSaved() );
        }
        else if(nnt_good > 1 && nnt_eff == 0) {
            //hEffCluMult[ichip]->Fill( fEvent->GetPlane(ichip)->GetNClustersSaved() );
            nn_discev++;
            nn_good  -= nnt_good;
            nn_ineff -= nnt_good;
            nn_rej   += nnt_good;
        }
    } // END FOR events

    Report(3, mname + Form("Total tracks:  %i", nn_tracks));
    Report(3, mname + Form("Rej.  tracks:  %i", nn_rej));
    Report(3, mname + Form("Mult. tracks:  %i", nn_mult));
    Report(3, mname + Form("Good  tracks:  %i", nn_good));
    Report(3, mname + Form("Eff.  tracks:  %i", nn_eff));
    Report(3, mname + Form("Ineff tracks:  %i", nn_ineff));
    Report(3, mname + Form("Efficiency:    %f %%", 100.*nn_eff/nn_good));
    Report(3, mname + Form("Discarded events:  %i", nn_discev));

    fEffNTracks[ichip] += nn_tracks;
    fEffNEff   [ichip] += nn_eff;
    fEffNGood  [ichip] += nn_good;
    fEffNRej   [ichip] += nn_rej;
    fEffNIneff [ichip] += nn_ineff;

    Report(3, mname + "Finished.");
}

//__________________________________________________________
void Na61Analysis::PrintEfficiencyVD(Int_t ichip) {
    Float_t k = fEffNEff[ichip];
    Float_t n = fEffNGood[ichip];
    Float_t eff = k/n;
    Float_t mean = (k+1)/(n+2);
    Float_t err = TMath::Sqrt( (k+1)*(k+2)/(n+2)/(n+3) - (k+1)*(k+1)/(n+2)/(n+2) );
    Float_t errup = mean + err - eff;
    Float_t errdo = eff - mean + err;
    
    cout << endl << "Cumulative efficiency results: " << endl
         << "\t Total tracks:  " << fEffNTracks[ichip] << endl
         << "\t Rej.  tracks:  " << fEffNRej   [ichip] << endl
         << "\t Good  tracks:  " << fEffNGood  [ichip] << endl  
         << "\t Eff.  tracks:  " << fEffNEff   [ichip] << endl
         << "\t Ineff tracks:  " << fEffNIneff [ichip] << endl
         << "\t Efficiency:    " << 100.*eff  << " + " << 100.*errup << " - " << 100.*errdo << endl
         << endl;
}


//__________________________________________________________
void Na61Analysis::ExtractHitsVD(Int_t ichip) {
    TString mname = "ExtractHitsVD() : ";
    Report(3, mname + "Starting hits extraction" );

    Float_t wx = 0.1; // search radius in x [mm]
    Float_t wy = 0.1; // search rafius in y [mm]
    Int_t   wc = TMath::Ceil(wy/29.24e-3); // search radius in columns
    Int_t   wr = TMath::Ceil(wx/26.88e-3); // search radius in rows

    Int_t sba = 0;
    sba += fVDTracksTree->SetBranchAddress("track_hits_s1", &fVD_hits1);
    sba += fVDTracksTree->SetBranchAddress("track_hits_s2", &fVD_hits2);
    sba += fVDTracksTree->SetBranchAddress("track_hits_s4", &fVD_hits4);
    if(sba) {
        Report(0, mname + "Problem with setting branch address of hits in VD tree!");
        return;
    }

    TClonesArray *hits[4];
    for(Int_t i=0; i<4; ++i) hits[i] = new TClonesArray("TVector3");
    Int_t nhits = 0;
    Int_t nhitstot = 0;
    
    if(fExHitsTree) {
        Report(2, "Extracted hits tree already initialised.");
        fExHitsTree->SetBranchAddress("event_n", &fVD_EventN);
        fExHitsTree->SetBranchAddress("n_tracks", &nhits);
        for(Int_t i=0; i<4; ++i)
            fExHitsTree->SetBranchAddress(Form("hits_s%i", i+1), &hits[i]);
    }
    else {
        fExHitsTree = new TTree("extracted_hits", "extracted_hits");
        fExHitsTree->Branch("event_n", &fVD_EventN);
        fExHitsTree->Branch("n_tracks", &nhits);
        for(Int_t i=0; i<4; ++i)
            fExHitsTree->Branch(Form("hits_s%i", i+1), &hits[i]);
        Report(3, "Extracted tracks tree intialised.");
    }
    
    Int_t nentries_al = fEventTree->GetEntries();
    Int_t nentries_vd = fVDTracksTree->GetEntries();
    
    for(Int_t ivd=0; ivd < nentries_vd; ++ivd) {
        // read events from vd tree and find equivalent in event tree
        fVDTracksTree->GetEntry(ivd);
        
        fVD_EventN -= 1; // correct for different trigger numbering between VD and ALPIDE
        
        if(fVD_EventN > nentries_al) {
            Report(2, mname + "WARNING more tracks than saved triggers");
            continue;
        }
        
        fEventTree->GetEntry(fVD_EventN);
            
        if(fEvent->GetIntTrigCnt() != fVD_EventN)
            Report(2, mname + "sync error");
        
        if( (ivd+1)%5000 == 0 )
            Report(3, Form(mname + "Processed events %i / %i", ivd+1, nentries_vd));

        fVD_EventN += 1; // resync with vd event numbering for backward compatibility
        nhits = 0;
        for(Int_t i=0; i<4; ++i)
            hits[i]->Clear();
            
        // loop over all tracks in event
        for(Int_t itrack=0; itrack < fVD_NTracks; ++itrack) {
            TVector3 to(*(TVector3*)fVD_to->At(itrack));
            TVector3 td(*(TVector3*)fVD_td->At(itrack));

            //_cuts____
            //if(fVD_Dataset[itrack] < 7) continue; // {down1, down2, up1, up2, up1x, 3pt0, 3pt1, 3pt3}
            //if(td.X() / td.Z() > 1) continue;
            //if(c4<0 || c4>fNCols) continue; // only chip4 is interesting
            //_cuts____
                
            //if( fEvent->GetPlane(ichip)->GetNClustersSaved() > 20 ) continue;

            TVector3 p = fAlignChip[ichip].IntersectionPointWithLine(to, td);
            Float_t col, row;
            fAlignChip[ichip].GlobalToPix(p, col, row);
            
            if(col<=wc || row<=wr || col>fNCols-wc || row>fNRows-wr)
                continue;
            
            hTrackHits[ichip]->Fill(p.X(), p.Y());
            hHits[ichip]->Fill(col, row);
            
            for(Int_t iclu=0; iclu < fEvent->GetPlane(ichip)->GetNClustersSaved(); ++iclu) {
                Int_t mult = fEvent->GetPlane(ichip)->GetCluster(iclu)->GetMultiplicity();
                BinaryCluster* cluster = fEvent->GetPlane(ichip)->GetCluster(iclu);
                 
                //_cuts____
                //if(mult > 10) continue;
                //if(cluster->HasHotPixels()) continue;
                //if(cluster->HasBorderPixels()) continue;
                //if(cluster->HasExclDblcolPixels()) continue;
                //_cuts____
                 
                TVector3 d = fAlignChip[ichip].DistPixLine(cluster->GetX(), cluster->GetY(), to, td, kTRUE);
                if( d.X()*d.X()/wx/wx + d.Y()*d.Y()/wy/wy < 1. ) {
                    // add hit to clones array
                    TVector3 *hit[4]; for(Int_t i=0; i<4; ++i) hit[i] = (TVector3*)hits[i]->ConstructedAt(nhits);
                    TVector3 vdhit1(*(TVector3*)fVD_hits1->At(itrack));
                    TVector3 vdhit2(*(TVector3*)fVD_hits2->At(itrack));
                    TVector3 alphit = fAlignChip[ichip].PixToGlobal(cluster->GetX(), cluster->GetY());
                    TVector3 vdhit4(*(TVector3*)fVD_hits4->At(itrack));
                    hit[0]->SetXYZ( vdhit1.X(), vdhit1.Y(), vdhit1.Z() );
                    hit[1]->SetXYZ( vdhit2.X(), vdhit2.Y(), vdhit2.Z() );
                    hit[2]->SetXYZ( alphit.X(), alphit.Y(), alphit.Z() );
                    hit[3]->SetXYZ( vdhit4.X(), vdhit4.Y(), vdhit4.Z() );
                    nhits++;
                    nhitstot++;
                    // fill histograms
                    hHitsAligned[ichip]->Fill(col, row);
                    hMult[ichip]->Fill(mult);
                }       
             } // END FOR clusters
        } // END FOR tracks
        if(nhits) fExHitsTree->Fill();
    } // END FOR events
    
    Report(3, mname + Form("Extracted %i hits from %i events", nhitstot, (Int_t)fExHitsTree->GetEntriesFast() ));

    fVDTracksTree->ResetBranchAddress(fVDTracksTree->FindBranch("track_hits_s1"));
    fVDTracksTree->ResetBranchAddress(fVDTracksTree->FindBranch("track_hits_s2"));
    fVDTracksTree->ResetBranchAddress(fVDTracksTree->FindBranch("track_hits_s4"));
    if(fExHitsTree) {
        fExHitsTree->ResetBranchAddresses();
        for(Int_t i=0; i<4; ++i) {
            delete hits[i];
        }
    }
    Report(3, mname + "Finished.");
}

