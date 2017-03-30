#include <Riostream.h>
#include <TMath.h>
#include <TF1.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <TStyle.h>

#include "Na61Analysis.hpp"
#include "../classes/helpers.h"

using namespace std;

ClassImp(Na61Analysis)

//______________________________________________________________________
Na61Analysis::Na61Analysis():
    TObject(),
    fCutDataset(kFALSE),
    fCutDatasetString(""),
    fCutDatasetValue(),
    fFileLog(),
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
        hEffCluMultDisc[i] = NULL;
        
        fEffNTracks[i] = 0;
        fEffNRej   [i] = 0;
        fEffNGood  [i] = 0;
        fEffNEff   [i] = 0;
        fEffNIneff [i] = 0;
        fEffNEvTot [i] = 0;
        fEffNEvGood[i] = 0;
        fEffNEvDisc[i] = 0;

    }
    
    fResMean[0] = -1.;
    fResMean[1] = -1.;
    fResSigma[0] = -1.;
    fResSigma[1] = -1.;
    
    set_my_style();
    gStyle->SetOptStat(1110);
/*
    fhVx = new TH1F("hVx","event 2 part vertexes",2000,-20,20);
    fhVy = new TH1F("hVy","event 2 part vertexes",2000,-20,20);
    fhVz = new TH1F("hVz","event 2 part vertexes",1000,-90,10);
    TString gaus = "[0]*exp(-0.5*((x-[1])/[2])^2)";
    fFGauss = new TF1("fFGauss",gaus.Data(),-500,100);

    fhVx3 = new TH1F("hVx3","event 2 part vertexes, 3pt",2000,-20,20);
    fhVy3 = new TH1F("hVy3","event 2 part vertexes, 3pt",2000,-20,20);
    fhVz3 = new TH1F("hVz3","event 2 part vertexes, 3pt",1000,-90,10);

    fhVx4 = new TH1F("hVx4","event 2 part vertexes, 4pt",2000,-20,20);
    fhVy4 = new TH1F("hVy4","event 2 part vertexes, 4pt",2000,-20,20);
    fhVz4 = new TH1F("hVz4","event 2 part vertexes, 4pt",1000,-90,10);

    fhVxD = new TH1F("hVxD","vert diff",200,-0.1,0.1);
    fhVyD = new TH1F("hVyD","vert diff",200,-0.1,0.1);
    fhVzD = new TH1F("hVzD","vert diff",100,-0.5,0.5);

    fhVx34 = new TH2F("hVx34","event 2 part vertexes, 3vs4pt",2000,-20,20,2000,-20,20);
    fhVy34 = new TH2F("hVy34","event 2 part vertexes, 3vs4pt",2000,-20,20,2000,-20,20);
    fhVz34 = new TH2F("hVz34","event 2 part vertexes, 3vs4pt",1000,-90,10,1000,-90,10);

    fhchi2x = new TH1F("hchi2x","chi2 x",200,0.,50.);
    fhchi2y = new TH1F("hchi2y","chi2 y",200,0.,50.);
*/
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

    if(fFileLog.is_open()) fFileLog.close();
    
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
        delete hEffCluMultDisc[i];
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
        hEffCluMult[i] = new TH1F(Form("hEffCluMult_%i", i), Form("Hit multiplicity of efficient events, chip %i ;# of clusters in event; a.u.", i),
                                  100, -0.5, 199.5);
        hEffCluMultDisc[i] = new TH1F(Form("hEffCluMultDisc_%i", i), Form("Hit multiplicity of discarded events, chip %i ;# of clusters in event; a.u.", i),
                                      100, -0.5, 199.5);
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
        hEffCluMultDisc[ichip]->SetLineColor(kRed);
        hEffCluMultDisc[ichip]->DrawCopy("SAME");
        cc->cd(3);
        hEffEffD[ichip]->ProjectionX()->DrawCopy();
        cc->cd(4);
        hEffEffD[ichip]->ProjectionY()->DrawCopy();        
    }
    /*
    else if(set.BeginsWith("vertex")) {
        TCanvas *c = new TCanvas("c_"+set, set, 50, 50, 1700, 1000);
        c->Divide(3,2);
        c->cd(1);
        fhVxD->DrawCopy();
        c->cd(2);
        fhVyD->DrawCopy();
        c->cd(3);
        fhVzD->DrawCopy();
        c->cd(4);
        fhchi2x->DrawCopy();
        //fhVx34->DrawCopy("COLZ");
        c->cd(5);
        fhchi2y->DrawCopy();
        //fhVy34->DrawCopy("COLZ");
        c->cd(6);
        //fhVz34->DrawCopy("COLZ");
    }
    */
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
        if(hEffGood[i]->GetEntries()) hEffCluMultDisc[i]->Write();
    }
    file_plots->Close();
    delete file_plots;
    Report(3, "Histograms written to file " + fname);
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
        Report(1, "ERROR: Cannot open output plots file! " + fname);
        return kFALSE;
    } 
    file_plots->cd();
    fExTracksTree->Write();
    file_plots->Close();
    delete file_plots;
    Report(3, "Tracks tree written to file " + fname);
    return kTRUE;
}

//__________________________________________________________
Bool_t Na61Analysis::WriteHitsTree(TString fname, TString opt) {
    if(!fExHitsTree) {
        Report(1, "Extracted tracks tree does NOT exist!");
        return kFALSE;
    }
    fname = fDirPathPlots + "/" + fname;
    TFile* file_plots = new TFile(fname.Data(), opt.Data());
    if(!file_plots->IsOpen()) {
        Report(1, "ERROR: Cannot open output tracks tree file! " + fname);
        return kFALSE;
    }
    file_plots->cd();
    fExHitsTree->Write();
    file_plots->Close();
    delete file_plots;
    Report(3, "Tracks tree written to file " + fname);
    return kTRUE;
}

//__________________________________________________________
void Na61Analysis::ResetHistograms() {
    hTrackHitsHIC->Reset();
    hTrackHitsMiss->Reset();

    for(Int_t i=0; i<fNChips; ++i) {        
        hTrackHits  [i]->Reset();
        hHits       [i]->Reset();
        hHitsAligned[i]->Reset();

        hDX    [i]->Reset();
        hDY    [i]->Reset();
        hDXzoom[i]->Reset();
        hDYzoom[i]->Reset();
        hDXY   [i]->Reset();

        hChipPosXY[i]->Reset();
        hChipPosZY[i]->Reset();
        hChipPosXZ[i]->Reset();

        hMult[i]->Reset();
        
        hEffGood   [i]->Reset();
        hEffRej    [i]->Reset();
        hEffIneff  [i]->Reset();
        hEffEff    [i]->Reset();
        hEffEffD   [i]->Reset();
        hEffCluMult[i]->Reset();
        hEffCluMultDisc[i]->Reset();
    }
    Report(3, "All histograms reset.");
}

//__________________________________________________________
void Na61Analysis::ResetTracksTree() {
    if(fExTracksTree) fExTracksTree->ResetBranchAddresses();
    delete fExTracksTree;
    fExTracksTree = NULL;
}

//__________________________________________________________
void Na61Analysis::ResetHitsTree() {
    if(fExHitsTree) fExHitsTree->ResetBranchAddresses();
    delete fExHitsTree;
    fExHitsTree = NULL;
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
Bool_t Na61Analysis::SetLogFile(TString filepath) {
    fFileLog.open(filepath.Data());
    if(!fFileLog.is_open()) {
        Report(1, "Unable to open log file " + filepath);
        return kFALSE;
    }
    else {
        Report(3, "Set log output to file " + filepath);
        return kTRUE;
    }
}

//__________________________________________________________
void Na61Analysis::SetDefaultAlignment() {
    const Float_t xpos = 11.049;
    const Float_t ypos = 120.6-0.115;
    const Float_t zpos = 92.265;
    const Float_t angY = 6.589*TMath::DegToRad();
    const Float_t angX = 0.;
    const Float_t angZ = 0.024*TMath::DegToRad();

    for(Short_t i=0; i<fNChips; ++i) {
        Alignment aln;
        //aln.SetPos(TVector3(11.1, 120.5-30.15*4, 92.3));
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
Bool_t Na61Analysis::SetCutDataset(TString cut, Int_t set) {
    // {down1, down2, up1, up2, up1x, up2x, 3pt0, 3pt1, 3pt3}
    //    0      1     2    3    4     5     6     7     8
    TString mname = "SetCutDataset() : ";
    cut.ToLower();
    fCutDataset = kTRUE;
    if( cut == "" || cut.Contains("off") || cut.Contains("nocut") || cut.Contains("disable") ) {
        fCutDataset       = kFALSE;
        Report(3, mname + "Cut disabled");
    } else if( cut.Contains("4pt") || cut.Contains("full") ) {
        fCutDatasetString    = "<";
        fCutDatasetValue[0]  = 4;
        Report(3, mname + "Analysing only full (4pt) tracks");
    } else if( cut == "pawel3pt" ) {
        fCutDatasetString    = "><";
        fCutDatasetValue[0]  = 3;
        fCutDatasetValue[1]  = 6;
        Report(3, mname + "Analysing only Pawel's 3pt tracks");
    } else if( cut == "pawelall" ) {
        fCutDatasetString    = "<";
        fCutDatasetValue[0]  = 6;
        Report(3, mname + "Analysing all Pawel's tracks");
    } else if( cut == "miko3pt" ) {
        fCutDatasetString    = ">";
        fCutDatasetValue[0]  = 5;
        Report(3, mname + "Analysing only Miko's 3pt tracks");
    } else if( cut == "set" ) {
        fCutDatasetString    = "==";
        fCutDatasetValue[0]  = set;
        Report(3, mname + Form("Analysing only set %i tracks", set));
    } else {
        fCutDataset       = kFALSE;
        Report(2, mname + "Uknown cut " + cut);
        return kFALSE;
    }   
    return kTRUE;
}

//__________________________________________________________
Bool_t Na61Analysis::ApplyCutDataset(Int_t val) {
    if(fCutDatasetString == "<")
        return (val < fCutDatasetValue[0]);
    if(fCutDatasetString == ">")
        return (val > fCutDatasetValue[0]);
    if(fCutDatasetString == "==")
        return (val == fCutDatasetValue[0]);
    if(fCutDatasetString == "><")
        return (val > fCutDatasetValue[0] && val < fCutDatasetValue[1]);
    else {
        Report(2, "ApplyCutDataset() : Uknown cut string");
        return kFALSE;
    }
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
    TString msg = "Na61Analysis : ";
    switch(level) {
    case 0 : msg += " FATAL  : "; break;
    case 1 : msg += " ERROR  : "; break;
    case 2 : msg += "WARNING : "; break;
    case 3 : msg += " INFO   : "; break;
    case 4 : msg += " DEBUG  : "; break;
    default: msg += " CUSTOM : ";
    }
    msg += message;
    cout << msg.Data() << endl;
    if(fFileLog.is_open())
        fFileLog << msg.Data() << endl;
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
            fExTracksTree->SetBranchAddress("clu_x", &ex_clux);
            fExTracksTree->SetBranchAddress("clu_y", &ex_cluy);
            fExTracksTree->SetBranchAddress("track_origin", ex_to);
            fExTracksTree->SetBranchAddress("track_direction", ex_td);
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
            if(fCutDataset) if(!ApplyCutDataset(fVD_Dataset[itrack])) continue;
            //if(td.X() / td.Z() > 1) continue;
            Float_t c4, r4;
            TVector3 p4 = fAlignChip[4].IntersectionPointWithLine(to, td);
            fAlignChip[4].GlobalToPix(p4, c4, r4);
            //if(r4<-100) continue;
            //if(c4<-100 || c4>fNCols+100) continue; // only chip4 is interesting
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
                        if( TMath::Abs(d.Y()) < 0.25 ) //_cuts____
                        {
                            hDX[ichip]->Fill( d.X() );
                            hDXzoom[ichip]->Fill( d.X() );
                        }
                        if( TMath::Abs(d.X()) < 0.25 ) //_cuts____
                        {
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
        Float_t range = 0.05, mean = 0.;
        TF1 *fres = new TF1("fres", "[0]*TMath::Gaus(x, [1], [2])+[3]", -range, range);
        fres->SetParNames("Norm", "#mu", "#sigma", "Const (backg)");
        fres->SetNpx(1000);
        fres->SetParLimits(0, 10., 10000.);
        fres->SetParLimits(2, 0.001, 1);
        
        for(Short_t i=0; i<fNChips; ++i) {
            if(hDXzoom[i]->Integral() < 10 || hDYzoom[i]->Integral() < 10) {
                Report( (i==4 ? 3 : 4), mname + Form("Skipping fit - insufficent tracks in residual plots, chip %i", i));
                continue;
            }
            fres->SetParameter(0, 300.);
            fres->SetParameter(2, 0.008);
            fres->SetParameter(3, 1);

            mean = hDXzoom[i]->GetMean();
            fres->SetRange(mean-range, mean+range);
            fres->SetParameter(1, mean);
            hDXzoom[i]->Fit(fres, "QR0");
            if(i == 4) {
                fResMean[0] = fres->GetParameter(1);
                fResSigma[0] = fres->GetParameter(2);
                Report(4, Form(mname + "Residual mean = %f, sigma = %f", fResMean[0]*1e3, fResSigma[0]*1e3));
            }
            
            mean = hDYzoom[i]->GetMean();
            fres->SetRange(mean-range, mean+range);
            fres->SetParameter(1, mean);
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

    Float_t wx = 0.072; // search radius in x [mm]
    Float_t wy = 0.088; // search rafius in y [mm]
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
    Int_t nn_goodev = 0;
    
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

        // do two loops, first to check if efficiency of the event is 0, and second to fill the histograms
        Bool_t flagDiscard = kFALSE;
        for(Int_t loop=0; loop < 2; loop++) {
            
            Int_t nnt_good = 0;
            Int_t nnt_eff  = 0;
            
            // loop over all tracks in event
            for(Int_t itrack=0; itrack < fVD_NTracks; ++itrack) {
                TVector3 to(*(TVector3*)fVD_to->At(itrack));
                TVector3 td(*(TVector3*)fVD_td->At(itrack));
                if(loop) nn_tracks++;
            
                //_cuts____
                if(fCutDataset) if(!ApplyCutDataset(fVD_Dataset[itrack])) continue;
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
            
                if(flagRej || flagDiscard) {
                    nn_rej++;
                    hEffRej[ichip]->Fill(col, row);
                    continue;
                }
            
                if(loop) nn_good++;
                if(loop) hEffGood[ichip]->Fill(col, row);
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

                    //if( d.X()*d.X()/wx/wx + d.Y()*d.Y()/wy/wy < 1. ) {
                    if( TMath::Abs(d.X()) < wx && TMath::Abs(d.Y()) < wy ) {
                        nhits++;
                        if(loop) hEffEffD[ichip]->Fill(d.X(), d.Y());
                    }
                
                } // END FOR clusters

                if(loop) {
                    if( nhits >= 1 ) {
                        nn_eff++;
                        hEffEff[ichip]->Fill(col, row);
                    }
                    else {
                        nn_ineff++;
                        hEffIneff[ichip]->Fill(col, row);
                    }
                    
                    if( nhits >  1 ) {
                        nn_mult++;
                    }
                }
                
                if( nhits >= 1 )
                    nnt_eff++;
                
                //if( nhits >  1 ) Report(3, mname + "Double hit");
            } // END FOR tracks

            if(loop == 0) {
                if(nnt_good > 0 && nnt_eff == 0) {
                    hEffCluMultDisc[ichip]->Fill( fEvent->GetPlane(ichip)->GetNClustersSaved() );
                    nn_discev++;
                    flagDiscard = kTRUE;
                    Report(4, mname + Form("Discarding event = %i, Efficiency = %.2f = %i/%i, ntracks = %i",
                                           fVD_EventN+1, 100.*nnt_eff/nnt_good, nnt_eff, nnt_good, fVD_NTracks));
                }
                else if(nnt_good > 0) nn_goodev++;
            }
            else {
                //if(!nnt_good)// || nnt_eff < nnt_good)
                if(nnt_eff && nnt_eff == nnt_good) {
                    hEffCluMult[ichip]->Fill( fEvent->GetPlane(ichip)->GetNClustersSaved() );
                }
            }
        } // END FOR loop
    } // END FOR events

    Report(3, mname + Form("Total tracks:  %i", nn_tracks));
    Report(3, mname + Form("Rej.  tracks:  %i", nn_rej));
    Report(3, mname + Form("Mult. tracks:  %i", nn_mult));
    Report(3, mname + Form("Good  tracks:  %i", nn_good));
    Report(3, mname + Form("Eff.  tracks:  %i", nn_eff));
    Report(3, mname + Form("Ineff tracks:  %i", nn_ineff));
    Report(3, mname + Form("Efficiency:    %f %%", 100.*nn_eff/nn_good));
    Report(3, mname + Form("Total     events:  %i", nentries_vd));
    Report(3, mname + Form("Accepted  events:  %i", nn_goodev));
    Report(3, mname + Form("Discarded events:  %i", nn_discev));

    fEffNTracks[ichip] += nn_tracks;
    fEffNEff   [ichip] += nn_eff;
    fEffNGood  [ichip] += nn_good;
    fEffNRej   [ichip] += nn_rej;
    fEffNIneff [ichip] += nn_ineff;
    fEffNEvTot [ichip] += nentries_vd;
    fEffNEvGood[ichip] += nn_goodev;
    fEffNEvDisc[ichip] += nn_discev;

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
         << "\t Total     events:  " << fEffNEvTot [ichip] << endl
         << "\t Accepted  events:  " << fEffNEvGood[ichip] << endl
         << "\t Discarded events:  " << fEffNEvDisc[ichip] << endl
         << "\t Total tracks:  " << fEffNTracks[ichip] << endl
         << "\t Rej.  tracks:  " << fEffNRej   [ichip] << endl
         << "\t Good  tracks:  " << fEffNGood  [ichip] << endl  
         << "\t Eff.  tracks:  " << fEffNEff   [ichip] << endl
         << "\t Ineff tracks:  " << fEffNIneff [ichip] << endl
         << "\t Efficiency:    " << 100.*eff  << " + " << 100.*errup << " - " << 100.*errdo << endl
         << endl;
    if(fFileLog.is_open())
        fFileLog << endl << "Cumulative efficiency results: " << endl
                 << "\t Total     events:  " << fEffNEvTot [ichip] << endl
                 << "\t Accepted  events:  " << fEffNEvGood[ichip] << endl
                 << "\t Discarded events:  " << fEffNEvDisc[ichip] << endl
                 << "\t Total tracks:  " << fEffNTracks[ichip] << endl
                 << "\t Rej.  tracks:  " << fEffNRej   [ichip] << endl
                 << "\t Good  tracks:  " << fEffNGood  [ichip] << endl  
                 << "\t Eff.  tracks:  " << fEffNEff   [ichip] << endl
                 << "\t Ineff tracks:  " << fEffNIneff [ichip] << endl
                 << "\t Efficiency:    " << 100.*eff  << " + " << 100.*errup << " - " << 100.*errdo << endl
                 << endl;
}


//__________________________________________________________
void Na61Analysis::ExtractChipHits(TString fpath_out, Int_t ichip) {
    TString mname = "ExtractChipHits() : ";
    
    TFile *file_out = new TFile(fpath_out.Data(), "recreate");
    if(!file_out->IsOpen()) {
        Report(1, mname + "ERROR: Cannot open output file! " + fpath_out);
        return;
    }

    TClonesArray *hits = new TClonesArray("TVector3");
    TTree *tree = new TTree("alpide_hits", "alpide_hits");
    tree->Branch("hits", &hits);

    Int_t nentries = fEventTree->GetEntries();
    for(Int_t ientry=0; ientry < nentries; ++ientry) {
        fEventTree->GetEntry(ientry);
        
        hits->Clear();
        Int_t ihit = 0;
        
        for(Int_t iclu=0; iclu < fEvent->GetPlane(ichip)->GetNClustersSaved(); ++iclu) {
            Int_t mult = fEvent->GetPlane(ichip)->GetCluster(iclu)->GetMultiplicity();
            BinaryCluster* cluster = fEvent->GetPlane(ichip)->GetCluster(iclu);
            TVector3 hh = fAlignChip[ichip].PixToGlobal(cluster->GetX(), cluster->GetY());

            if(mult > 6) continue;

            TVector3 *hit = (TVector3*)hits->ConstructedAt(ihit++);
            hit->SetXYZ( hh.X(), hh.Y(), hh.Z() );
        }
        tree->Fill();
    }

    file_out->Write();
    file_out->Close();
    
    //delete tree;
    delete hits;
    delete file_out;
    Report(3, mname + "Hits extracted to file " + fpath_out);
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
        Report(2, mname + "Extracted hits tree already initialised.");
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
        Report(3, mname + "Extracted tracks tree intialised.");
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
            if(fCutDataset) if(!ApplyCutDataset(fVD_Dataset[itrack])) continue;
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



/*

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

        TObjArray *reco_tracks_3pt = new TObjArray(1000);
        TObjArray *reco_tracks_4pt = new TObjArray(1000);
        
        // loop over all tracks in event
        for(Int_t itrack=0; itrack < fVD_NTracks; ++itrack) {
            TVector3 to(*(TVector3*)fVD_to->At(itrack));
            TVector3 td(*(TVector3*)fVD_td->At(itrack));

            //_cuts____
            if(fCutDataset) if(!ApplyCutDataset(fVD_Dataset[itrack])) continue; 
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
                    // reconstruct track
                    Float_t ax;
                    Float_t ay;
                    Float_t bx;
                    Float_t by;
                    Float_t chi2x;
                    Float_t chi2y;
                    Float_t N;
                    Float_t zmin;

                    FitLine_w2(hit[0],hit[1],hit[2],hit[3],ax,ay,bx,by,chi2x,chi2y,N,zmin);
                    fhchi2x->Fill(chi2x);
                    fhchi2y->Fill(chi2y);
                    Vector3D origin(bx,by,0.);
                    Vector3D direction(ax,ay,1.);
                    UVdTrack* track4 = new UVdTrack(origin,direction);
                    reco_tracks_4pt->Add(track4);

                    Vector3D origin3(to.X(),to.Y(),to.Z());
                    Vector3D direction3(td.X(),td.Y(),td.Z());
                    UVdTrack* track3 = new UVdTrack(origin3,direction3);
                    reco_tracks_3pt->Add(track3);
                }       
             } // END FOR clusters
        } // END FOR tracks
        if(nhits) fExHitsTree->Fill();
        Vector3D* pvert3 = new Vector3D();
        Vector3D* pvert4 = new Vector3D();
        if( FindPrimaryVertex(reco_tracks_3pt, pvert3) ) {
            fhVx3->Fill(pvert3->GetX());
            fhVy3->Fill(pvert3->GetY());
            fhVz3->Fill(pvert3->GetZ());
        }

        if( FindPrimaryVertex(reco_tracks_4pt, pvert4) ) {
            fhVx4->Fill(pvert4->GetX());
            fhVy4->Fill(pvert4->GetY());
            fhVz4->Fill(pvert4->GetZ());

            fhVxD->Fill(pvert4->GetX()-pvert3->GetX());
            fhVyD->Fill(pvert4->GetY()-pvert3->GetY());
            fhVzD->Fill(pvert4->GetZ()-pvert3->GetZ());

            fhVx34->Fill(pvert4->GetX(),pvert3->GetX());
            fhVy34->Fill(pvert4->GetY(),pvert3->GetY());
            fhVz34->Fill(pvert4->GetZ(),pvert3->GetZ());
        }

        
        
        delete reco_tracks_3pt;
        delete reco_tracks_4pt;
        delete pvert3;
        delete pvert4;
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
    delete fExHitsTree; fExHitsTree=NULL;
}

//___________________________________________________________________________________
void Na61Analysis::FitLine_w2(TVector3* hit1,TVector3* hit2,TVector3* hit3,TVector3* hit4,
                              Float_t& ax,Float_t& ay,Float_t& bx,Float_t& by,
                              Float_t& chi2x,Float_t& chi2y,Float_t& N, Float_t& zmin)
{
  // simple regration (assuming constant errors)
  // It is quite straight forward to use weighted regression

    Float_t hx,hy,hz;

    Float_t S=0;
    Float_t Sz=0;
    Float_t Szz=0;
 
    Float_t Sx=0;
    Float_t Szx=0;

    Float_t Sy=0;
    Float_t Szy=0;
 
    
    //Float_t sig[4] = {0.004, 0.004, 0.004, 0.004}; // errors in mm
    //Float_t sig[4] = {0.004, 0.0145, 0.032, 0.0416}; // errors in mm
    Float_t sig[4] = {0.004, 0.005, 0.006, 0.007}; // errors in mm

    TVector3* hits[4] = {hit1,hit2,hit3,hit4};
    
    //cout<<hit1->GetZ()<<" "<<hit2->GetZ()<<" "<<hit3->GetZ()<<" "<<hit4->GetZ()<<endl;

    zmin = 10000.;
    N = 0;    
    
    // perhaps we need waited regression
    
    for(Int_t i=0;i<4;i++){      
      TVector3* hit = hits[i];
      
      if(!hit)continue; // now we always have 4 hits
      
      N++;
      
      // smearing is done in dedicated method
      hx = hit->X(); 
      hy = hit->Y();
      hz = hit->Z(); 
      
      //Int_t   ivds = hit->GetStationID()-1;
      Float_t sig2 = sig[i]*sig[i];     

      S   += 1./sig2;
      Sz  += hz/sig2;
      Szz += (hz*hz)/sig2;

      Sx  += hx/sig2;
      Szx += (hz*hx)/sig2;

      Sy  += hy/sig2;
      Szy += (hz*hy)/sig2;
      
      if(hz<zmin)zmin=hz;
      
    }
    
    ax = (S*Szx - Sz*Sx)/(S*Szz-Sz*Sz);
    bx = (Szz*Sx - Sz*Szx)/(S*Szz-Sz*Sz);

    ay = (S*Szy - Sz*Sy)/(S*Szz-Sz*Sz);
    by = (Szz*Sy - Sz*Szy)/(S*Szz-Sz*Sz);
    

    chi2x=0;
    chi2y=0;
    for(Int_t i=0;i<4;i++){
      TVector3* hit = hits[i];
      
      if(!hit)continue;      

      //Int_t   ivds = hit->GetStationID();
      Float_t sig2 = sig[i]*sig[i];     

      hx = hit->X(); 
      hy = hit->Y();
      hz = hit->Z(); 

      chi2x += (hx - ax*hz - bx)*(hx - ax*hz - bx)/sig2;
      chi2y += (hy - ay*hz - by)*(hy - ay*hz - by)/sig2;
    }

    //cout<<"FitLine_w2: ax="<<ax<<" ay="<<ay<<" bx="<<bx<<" by="<<by<<" N="<<N<<"  chi2x="<<chi2x<<"  chi2y="<<chi2y<<endl;

}

//_____________________________________________________________
bool Na61Analysis::FindPrimaryVertex(TObjArray* tracktab, Vector3D* fPrimaryVertex)
{

    //Int_t fullTracks = tracktab[0]->GetEntries()+tracktab[1]->GetEntries();//+tracktab[3]->GetEntries();

    //fhFullTracks -> Fill(fullTracks);

    //TObjArray* vertexes = new  TObjArray();
    //vertexes->Clear();
    //vertexes->SetOwner(kTRUE);

    TObjArray tracks;
    tracks.Clear();
    for(Int_t i=0; i<tracktab->GetEntries(); i++){
        UVdTrack* track = (UVdTrack*)tracktab->At(i);
        //if(!track->IsMarkedForRemoval()) tracks.Add(track);
        tracks.Add(track);
    }

    Int_t nVertexes=0;
    
    fhVx->Reset();
    fhVy->Reset();
    fhVz->Reset();

    Float_t sum_up_x = 0;
    Float_t sum_do_x = 0;
    Float_t sum_up_y = 0;
    Float_t sum_do_y = 0;
    
    for(Int_t i=0;i<tracks.GetEntries();i++){
        UVdTrack* track1 = (UVdTrack*)tracks.At(i); 
        for(Int_t j=i+1;j<tracks.GetEntries();j++){
            UVdTrack* track2 = (UVdTrack*)tracks.At(j); 

            track1->Activate();
            track2->Activate();
            
            //Float_t x1 = track1->GetXatZ(150.);
            //Float_t x2 = track2->GetXatZ(150.);
            //Float_t y1 = track1->GetYatZ(150.);
            //Float_t y2 = track2->GetYatZ(150.);
            //if(TMath::Abs(x2-x1)<0.2)continue;
            //if(TMath::Abs(y2-y1)<0.2)continue;
      
            Line3D* line1 = track1->Getline();
            Line3D* line2 = track2->Getline();
      
            Vector3D vertex = line1->GetClosestProximityPoint(line2);
      
            Float_t x = vertex.X(); 
            Float_t y = vertex.Y(); 
            Float_t z = vertex.Z(); 
      
            fhVx->Fill(x);
            fhVy->Fill(y);
            fhVz->Fill(z);
            //fhVertexZ ->Fill(z); 
            //fhVertexXY ->Fill(x,y);
            
            //vertexes -> Add(new Vector3D(vertex));
      
            /////////////////// stuff for new method
            Float_t axi = track1->GetDX()/track1->GetDZ();
            Float_t axj = track2->GetDX()/track2->GetDZ();
            Float_t bxi = track1->GetX();
            Float_t bxj = track2->GetX();
      
            Float_t ayi = track1->GetDY()/track1->GetDZ();
            Float_t ayj = track2->GetDY()/track2->GetDZ();
            Float_t byi = track1->GetY();
            Float_t byj = track2->GetY();
      
            sum_up_x = sum_up_x + (axi-axj)*(bxi-bxj);
            sum_do_x = sum_do_x + (axi-axj)*(axi-axj);
      
            sum_up_y = sum_up_y + (ayi-ayj)*(byi-byj);
            sum_do_y = sum_do_y + (ayi-ayj)*(ayi-ayj);
            //cout<<"i="<<i<<" j="<<j<<"  "<<(axi-axj)*(axi-axj)<<" "<<axi<<" "<<axj<<endl;
      
            nVertexes++;
        }
    } 

    if(nVertexes<10) return false; //10
    if(fhVx->Integral() < 5) return false; //5
    if(fhVy->Integral() < 5) return false; //5
    if(fhVz->Integral() < 5) return false; //5

    //if(fhVx->GetRMS()>0.1) return false;
    //if(fhVy->GetRMS()>0.1) return false;
    //if(fhVz->GetRMS()>0.9) return false;
    
    ////////// new method //////////////////////////////////////////////////
    //Float_t zx_min = - sum_up_x/sum_do_x; 
    //Float_t zy_min = - sum_up_y/sum_do_y; 
    //
    ////fhDZmin ->Fill(zy_min - zx_min);
    //
    //Float_t z_prim =  (zx_min + zy_min)/2.;
    //
    //fhRecoVertexZ_new      -> Fill(z_prim);
    //fhRecoVertexZ_fine_new -> Fill(z_prim);
    /////////////////////////////////////////////////////////////////////////

    Float_t reco_pvx, reco_pvy, reco_pvz;
    Bool_t fitreco = kFALSE; // reconstract thorugh fit or calculation
    if(fitreco) {
        // fit 2 particle vertexes
        /////////////////// fhVx
        Int_t   ibin  = fhVx->GetMaximumBin();
        Float_t mean  = fhVx->GetBinCenter(ibin);
        Float_t max   = fhVx->GetBinContent(ibin); 
        Float_t sigma = fhVx->GetRMS();
        fFGauss->SetParameters(max,mean,sigma);
        Float_t rmin = mean - 2*sigma;
        Float_t rmax = mean + 2*sigma;
        fhVx->Fit(fFGauss,"Q0","0",rmin,rmax); 
        reco_pvx =  fFGauss->GetParameter(1);

        /////////////////// fhVy
        ibin  = fhVy->GetMaximumBin();
        mean  = fhVy->GetBinCenter(ibin);
        max   = fhVy->GetBinContent(ibin); 
        sigma = fhVy->GetRMS();
        fFGauss->SetParameters(max,mean,sigma);
        rmin = mean - 2*sigma;
        rmax = mean + 2*sigma;
        fhVy->Fit(fFGauss,"Q0","0",rmin,rmax); 
        reco_pvy =  fFGauss->GetParameter(1);

        /////////////////// fhVz
        ibin  = fhVz->GetMaximumBin();
        mean  = fhVz->GetBinCenter(ibin);
        max   = fhVz->GetBinContent(ibin); 
        sigma = fhVz->GetRMS();
        fFGauss->SetParameters(max,mean,sigma);
        rmin = mean - 2*sigma;
        rmax = mean + 2*sigma;
        fhVz->Fit(fFGauss,"Q0","0",rmin,rmax); 
        reco_pvz =  fFGauss->GetParameter(1);
        ///////////////////////////////////////////////////////////
    }
    else {
        Float_t nsigma = 5.;
        Float_t mean = fhVx->GetBinCenter(fhVx->GetMaximumBin());
        Float_t sigma = fhVx->GetRMS();
        fhVx->GetXaxis()->SetRangeUser(mean-nsigma*sigma, mean+nsigma*sigma);
        reco_pvx = fhVx->GetMean();
        fhVx->GetXaxis()->SetRange();

        mean = fhVy->GetBinCenter(fhVy->GetMaximumBin());
        sigma = fhVy->GetRMS();
        fhVy->GetXaxis()->SetRangeUser(mean-nsigma*sigma, mean+nsigma*sigma);
        reco_pvy = fhVy->GetMean();
        fhVy->GetXaxis()->SetRange();

        mean = fhVz->GetBinCenter(fhVz->GetMaximumBin());
        sigma = fhVz->GetRMS();
        fhVz->GetXaxis()->SetRangeUser(mean-nsigma*sigma, mean+nsigma*sigma);
        reco_pvz = fhVz->GetMean();
        fhVz->GetXaxis()->SetRange();
    }


    //Vector3D reco_pvertex(reco_pvx,reco_pvy,reco_pvz);
    //fPrimaryVertexDefined = kTRUE;
    fPrimaryVertex->SetX(reco_pvx);
    fPrimaryVertex->SetY(reco_pvy);
    fPrimaryVertex->SetZ(reco_pvz);
    
    return true;
    
    //fhRecoVertexZ      -> Fill(reco_pvertex.Z());
    //fhRecoVertexZ_fine -> Fill(reco_pvertex.Z());
    //fhRecoVertexXY     -> Fill(reco_pvertex.X(),reco_pvertex.Y());
    //fhRecoVertexZ_diff -> Fill(reco_pvertex.Z() - z_prim);

}

*/
