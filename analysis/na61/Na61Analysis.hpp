// Created by Miljenko Suljic, m.suljic@cern.ch

#ifndef NA61ANALYSIS_HPP
#define NA61ANALYSIS_HPP

#include <TObject.h>
#include <TVector3.h>
//#include <TMath.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TString.h>
#include <TClonesArray.h>

#include "../classes/BinaryEvent.hpp"
#include "../classes/Alignment.hpp"

class Na61Analysis: public TObject {

public:
    Na61Analysis();
    virtual ~Na61Analysis();

    void   InitHistograms();
    
    Bool_t SetInputFileALPIDE(TString filepath_tree);
    Bool_t SetInputFileVDTracks(TString filepath_tree);
    void   SetOutputDirPlots(TString dirpath_plots) { fDirPathPlots = dirpath_plots; }
    void   SetIOSuffix(TString suffix)              { fSuffix = suffix; }
    void   SetDefaultAlignment();
    Bool_t SetAlignment(Int_t chip, Alignment a);

    Bool_t WriteHistograms(TString fname);

    // misc
    void   FillChipPosHistos();
    
    // analyses
    void   PreAlignmentVD();
    
private:
    // Na61Analysis constants
    static const Short_t fNChips = 9;   // number of chips in HIC
    static const Short_t fNCols = 1024; // number of columns 
    static const Short_t fNRows = 512;  // number of rows

    // Na61Analysis variables
    TChain   *fEventTree;
    TChain   *fVDTracksTree;
    TTree    *fExTracksTree;
    Alignment fAlignChip[fNChips];
    TString   fDirPathPlots;
    TString   fSuffix;

    // event tree variables
    BinaryEvent *fEvent;
    // VD tracks tree variables
    Int_t fVD_EventN;
    Int_t fVD_NTracks;
    Int_t fVD_Dataset[10000];
    TClonesArray *fVD_to;
    TClonesArray *fVD_td;
    // extracted tacks tree variables
    Float_t fEx_clux;
    Float_t fEx_cluy;
    Float_t fEx_to[3];
    Float_t fEx_td[3];
    
    // histograms
    TH2F *hTrackHitsHIC;
    TH2F *hTrackHitsMiss;
    
    TH2F *hTrackHits[fNChips];
    TH2F *hHits[fNChips];
    TH2F *hHitsAligned[fNChips];
    
    TH1F *hDX[fNChips];
    TH1F *hDY[fNChips];
    TH1F *hDXzoom[fNChips];
    TH1F *hDYzoom[fNChips];
    TH2F *hDXY[fNChips];
    
    TH2F *hChipPosXY[fNChips];
    TH2F *hChipPosZY[fNChips];
    TH2F *hChipPosXZ[fNChips];

    TH1F *hMult[fNChips];

    // error reporting method
    Int_t fVerboseLevel;
    void  InitExtractedTracksTree();
    void  Report(Short_t level, const char * message);
    
    ClassDef(Na61Analysis,1);
};

#endif








