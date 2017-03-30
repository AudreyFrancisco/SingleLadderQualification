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
#include <TF1.h>

#include "../classes/BinaryEvent.hpp"
#include "../classes/Alignment.hpp"

//#include "/home/msuljic/work/na61/vdal/utils/UVdEvent.h"
//#include "/home/msuljic/work/na61/vdal/utils/UVdTrack.h"

class Na61Analysis: public TObject {

public:
    Na61Analysis();
    virtual ~Na61Analysis();

    void   InitHistograms(const Int_t binred = 2);
    void   DrawHistograms(TString set, Int_t ichip = 4);
    Bool_t WriteHistograms(TString fname, TString opt="RECREATE");
    Bool_t WriteTracksTree(TString fname, TString opt="UPDATE");
    Bool_t WriteHitsTree(TString fname, TString opt="UPDATE");

    void   ResetHistograms();
    void   ResetTracksTree();
    void   ResetHitsTree();
    
    void   SetVerboseLevel(Int_t level)             { fVerboseLevel = level; }
    Bool_t SetLogFile(TString filepath);
    Bool_t SetInputFileALPIDE(TString filepath_tree);
    Bool_t SetInputFileVDTracks(TString filepath_tree);
    void   SetOutputDirPlots(TString dirpath_plots) { fDirPathPlots = dirpath_plots; }
    void   SetIOSuffix(TString suffix)              { fSuffix = suffix; }
    void   SetDefaultAlignment();
    Bool_t SetAlignment(Int_t chip, Alignment a);
    // cuts
    Bool_t SetCutDataset(TString cut, Int_t set=-1);
    
    Bool_t LoadHotPixMask(Int_t ichip, TString fname);
    Bool_t LoadExclColMask(Int_t ichip, TString fname);
    
    // misc
    void   FillChipPosHistos();
    
    // analyses
    void   PrealignmentVD(Float_t ex_sigma=-1.);
    void   ExtractTracksVD(Float_t extract_sigma) { PrealignmentVD(extract_sigma); }
    void   EfficiencyVD(Int_t ichip=4);
    void   PrintEfficiencyVD(Int_t ichip=4);
    void   ExtractChipHits(TString fpath_out, Int_t ichip=4);
    void   ExtractHitsVD(Int_t ichip=4);
private:
    /*
    // methods
    void FitLine_w2(TVector3* hit1,TVector3* hit2,TVector3* hit3,TVector3* hit4,
                    Float_t& ax,Float_t& ay,Float_t& bx,Float_t& by,
                    Float_t& chi2x,Float_t& chi2y,Float_t& N, Float_t& zmin);
    bool FindPrimaryVertex(TObjArray* tracktab, Vector3D* fPrimaryVertex);
    TH1F *fhVx;
    TH1F *fhVy;
    TH1F *fhVz;
    
    TF1 *fFGauss;

    TH1F *fhVx3;
    TH1F *fhVy3;
    TH1F *fhVz3;
    
    TH1F *fhVx4;
    TH1F *fhVy4;
    TH1F *fhVz4;

    TH1F *fhVxD;
    TH1F *fhVyD;
    TH1F *fhVzD;

    TH2F *fhVx34;
    TH2F *fhVy34;
    TH2F *fhVz34;

    TH1F *fhchi2x;
    TH1F *fhchi2y;
    */
    
    // Na61Analysis constants
    static const Short_t fNChips = 9;   // number of chips in HIC
    static const Short_t fNCols = 1024; // number of columns 
    static const Short_t fNRows = 512;  // number of rows
    static const Float_t fPixX  = 0.02924e3;
    static const Float_t fPixY  = 0.02688e3;
    
    // Na61Analysis variables
    TChain   *fEventTree;
    TChain   *fVDTracksTree;
    TTree    *fExTracksTree;
    TTree    *fExHitsTree;
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
    TClonesArray *fVD_hits1;
    TClonesArray *fVD_hits2;
    TClonesArray *fVD_hits4;
    
    // efficiency variables
    Int_t fEffNTracks[fNChips];
    Int_t fEffNGood[fNChips];  
    Int_t fEffNEff[fNChips];   
    Int_t fEffNRej[fNChips];   
    Int_t fEffNIneff[fNChips];
    Int_t fEffNEvTot [fNChips];
    Int_t fEffNEvGood[fNChips];
    Int_t fEffNEvDisc[fNChips];

    // cut variables
    Bool_t  fCutDataset;
    TString fCutDatasetString;
    Int_t   fCutDatasetValue[4];
    
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

    static const Int_t fNMultPos = 5;
    TH2F *hMultPos[fNChips][fNMultPos];

    // efficiency
    TH2F *hEffGood[fNChips];
    TH2F *hEffRej[fNChips];
    TH2F *hEffIneff[fNChips];
    TH2F *hEffEff[fNChips];
    TH2F *hEffEffD[fNChips];
    TH1F *hEffCluMult[fNChips];
    TH1F *hEffCluMultDisc[fNChips];

    // cut methods
    Bool_t ApplyCutDataset(Int_t val);
    // error reporting method and log
    ofstream fFileLog;
    Int_t    fVerboseLevel;
    void     Report(Short_t level, const char * message);
    
    ClassDef(Na61Analysis,1);
};

#endif








