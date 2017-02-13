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

    void   InitHistograms(const Int_t binred = 2);
    void   DrawHistograms(TString set);
    Bool_t WriteHistograms(TString fname, TString opt="RECREATE");
    Bool_t WriteTracksTree(TString fname, TString opt="UPDATE");
    
    void   SetVerboseLevel(Int_t level)             { fVerboseLevel = level; }
    Bool_t SetInputFileALPIDE(TString filepath_tree);
    Bool_t SetInputFileVDTracks(TString filepath_tree);
    void   SetOutputDirPlots(TString dirpath_plots) { fDirPathPlots = dirpath_plots; }
    void   SetIOSuffix(TString suffix)              { fSuffix = suffix; }
    void   SetDefaultAlignment();
    Bool_t SetAlignment(Int_t chip, Alignment a);

    Bool_t LoadHotPixMask(Int_t ichip, TString fname);
    Bool_t LoadExclColMask(Int_t ichip, TString fname);
    
    // misc
    void   FillChipPosHistos();
    
    // analyses
    void   PrealignmentVD(Float_t ex_sigma=-1.);
    void   ExtractTracksVD(Float_t extract_sigma) { PrealignmentVD(extract_sigma); }
    void   EfficiencyVD(Int_t ichip=4);
    void   PrintEfficiencyVD(Int_t ichip=4);
    
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

    Float_t   fResMean[2];  // x/y residual mean
    Float_t   fResSigma[2]; // x/y residual rms or std dev

    vector<Short_t> fHotPixRows[fNChips];
    vector<Short_t> fHotPixCols[fNChips];
    vector<Short_t> fExclCols[fNChips];
    
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
    // efficiency variables
    Int_t fEffNTracks[fNChips];
    Int_t fEffNGood[fNChips];  
    Int_t fEffNEff[fNChips];   
    Int_t fEffNRej[fNChips];   
    Int_t fEffNIneff[fNChips];   
    
    
    // histograms
    // prealignment and extraction
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
    // efficiency
    TH2F *hEffGood[fNChips];
    TH2F *hEffRej[fNChips];
    TH2F *hEffIneff[fNChips];
    TH2F *hEffEff[fNChips];
    TH2F *hEffEffD[fNChips];
    

    // error reporting method
    Int_t fVerboseLevel;
    void  InitExtractedTracksTree();
    void  Report(Short_t level, const char * message);
    
    ClassDef(Na61Analysis,1);
};

#endif








