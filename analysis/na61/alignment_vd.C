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
#include "TMinuit.h"

#include "../classes/helpers.cpp"
#include "../classes/Alignment.hpp"

#define MAXNPT 10000

Int_t   ntracks;
Float_t clux[MAXNPT];
Float_t cluy[MAXNPT];
Float_t to[MAXNPT][3];
Float_t td[MAXNPT][3];

const Int_t npars = 5;

TString  Tag[npars];
Double_t var[npars], verr[npars];
Double_t err_mat[npars][npars];

void fcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag ) {
    Double_t x0 = par[0];
    Double_t y0 = par[1];
    Double_t z0 = par[2];
    Double_t ang1 = par[3];
    Double_t ang2 = par[4];
    //Double_t ang3 = par[5];

    Alignment align;
    align.SetPos(TVector3(x0, y0, z0));
    TRotation rot;
    rot.RotateY(ang1);
    rot.RotateZ(ang2);
    align.SetRotation(rot);
    
    Double_t sum=0;

    for (Int_t itrack=0; itrack<ntracks; ++itrack) {
        TVector3 p = align.IntersectionPointWithLine(TVector3(to[itrack]), TVector3(td[itrack]));
        TVector3 c = align.PixToGlobal(clux[itrack], cluy[itrack]);
        TVector3 d = c - p;
        sum += d.X()*d.X() + d.Y()*d.Y();
    }
    
    f=sum;
}

Bool_t alignment_vd(
    const TString dirpath_data = "/home/msuljic/cernbox/na61/data/by_run/",
    const TString dirpath_out  = "/home/msuljic/cernbox/na61/data/alignment_vd/",
    const TString suffix      = "cr2"    // identifier (suffix)
    ) {

    const Int_t n_runs = 2;
    const TString run[n_runs] = {"27487", "27489"};
    
    cout << "alignment_vd() : Starting..." << endl;

    set_my_style();

    const TString filename_out = "alignment_vd_" + suffix;
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_chips      = 9;   // number of chips


    /*
    Alignment align_chip[n_chips];
    for(Short_t i=0; i<n_chips; ++i) {
        align_chip[i].SetPos(TVector3(xpos, ypos-30.15*i, zpos));
        TRotation rot;
        //rot.RotateY(-0.07*TMath::Pi());
        rot.RotateY(6.*TMath::DegToRad());
        //rot.RotateX(1.*TMath::DegToRad());
        align_chip[i].SetRotation(rot);
    }
    */

    TChain* chain = new TChain("extracted_tracks", "extracted_tracks");
    for(Int_t i=0; i<n_runs; ++i) {
        TString fname = dirpath_data + run[i];
        fname += "/results_" + suffix;
        fname += "/prealignment_vd_" + suffix;
        fname += ".root";
        cout << "alignment_vd() : Loading file " << fname << endl;
        if(!chain->Add(fname.Data())) {
            cerr << "prealignment_vd() : ERROR: Cannot find extracted tracks tree in file! " << fname << endl;
            return kFALSE;
        }
    }
    Float_t ex_clux, ex_cluy;
    Float_t ex_to[3], ex_td[3];
    chain->SetBranchAddress("clu_x", &ex_clux);
    chain->SetBranchAddress("clu_y", &ex_cluy);
    chain->SetBranchAddress("track_origin", ex_to);
    chain->SetBranchAddress("track_direction", ex_td);
    
    Int_t nentries = chain->GetEntries();
    
    cout << "alignment_vd() : Number of events found in chain: " << nentries << endl;

    ntracks = nentries;
    
    for(Int_t i=0; i < nentries; ++i) {
        chain->GetEntry(i);
        clux[i] = ex_clux;
        cluy[i] = ex_cluy;
        for(Int_t j=0; j<3; ++j) {
            to[i][j] = ex_to[j];
            td[i][j] = ex_td[j];
        }
    }

    // Alignment
    const Float_t xpos = 11.1;
    const Float_t ypos = 120.4-30.15*4;
    const Float_t zpos = 92.3;
    
    Double_t pi  = TMath::Pi();
    Double_t pi2 = 0.5*TMath::Pi();
    Char_t PARM_NAMES[6][255]={"x0","y0","z0", "ang1", "ang2", "ang3"};
    Double_t PARM_START[6] =  {xpos, ypos, zpos, 0.1,   -0.1, 0.};
    Double_t PARM_STEP[6]  =  {0.01, 0.01, 0.01,  0.01,  0.01, 0.01};
    Double_t PARM_LOWER[6] =  {-1e3, -1e3, -1e3, -0.5, -0.2,   0.};
    Double_t PARM_UPPER[6] =  { 1e3,  1e3,  1e3,  0.5,  0.2,  pi2}; 

    TMinuit *gMinuit = new TMinuit(npars);
    gMinuit->SetFCN( fcn );
    
    Double_t arglist[10];
    Int_t ivarbl,ierflg;
    Double_t bnd1, bnd2;

    /* set starting values and steps */
    for (Int_t i=0; i<npars; i++) {
        gMinuit->mnparm( i, PARM_NAMES[i], PARM_START[i], PARM_STEP[i], 
                         PARM_LOWER[i], PARM_UPPER[i], ierflg );
    }

    //This is where the delta chi2 is defined
    //This is critical to getting the correct error estimates
    arglist[0] = 1.0;
    if (arglist[0]) gMinuit->mnexcm("SET ERR", arglist, 1, ierflg);
    arglist[0] = 10000;            // do at least 1000 function calls
    arglist[1] = 0.05;             // tolerance = 0.1

    gMinuit->mnexcm("MIGRAD", arglist, 2, ierflg );

    /*
    for (Int_t i=0; i<npars; i++) {
        Double_t val, err;
        gMinuit->GetParameter(i, val, err);
        printf("%s: %f +/- %f\n",PARM_NAMES[i],val,err);
        //gMinuit->mnpout( i, Tag[i], var[i], verr[i], bnd1, bnd2, ivarbl);
    }

    
    //Get error matrix
    gMinuit->mnemat(&err_mat[0][0],npars);

    printf("Error matrix:\n");
    for (Int_t i=0;i<npars;i++){
        printf("%f %f\n",err_mat[0][i],err_mat[1][i]);
    }
    */
    
    
    
    return kTRUE;
}
