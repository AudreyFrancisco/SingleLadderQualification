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
#include "TGraph.h"
#include "TSystem.h"
#include "TAxis.h"
#include "TStyle.h"

#include "../classes/helpers.cpp"
#include "../classes/Alignment.hpp"

#define MAXNPT 10000

Int_t   ntracks;
Float_t clux[MAXNPT];
Float_t cluy[MAXNPT];
Float_t to_arr[MAXNPT][3];
Float_t td_arr[MAXNPT][3];

const Int_t npars = 6;

void fcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag ) {
    Double_t x0 = par[0];
    Double_t y0 = par[1];
    Double_t z0 = par[2];
    Double_t ang1 = par[3];
    Double_t ang2 = par[4];
    Double_t ang3 = par[5];
    
    Alignment align;
    align.SetPos(TVector3(x0, y0, z0));
    TRotation rot;
    rot.RotateY(ang1);
    rot.RotateX(ang2);
    rot.RotateZ(ang3);
    align.SetRotation(rot);
    
    Double_t sum=0;

    for (Int_t i=0; i<ntracks; ++i) {
        TVector3 d = align.DistPixLine(clux[i], cluy[i], TVector3(to_arr[i]), TVector3(td_arr[i]), 0);
        sum += (d.X()*d.X() + d.Y()*d.Y());
    }
    
    f=sum;
}

Bool_t alignment_vd(
    const TString dirpath_data = "/home/msuljic/cernbox/na61/data/by_run/",
    const TString dirpath_out  = "/home/msuljic/cernbox/na61/data/alignment_vd/",
    const TString suffix      = "_cr2"    // identifier (suffix)
    ) {

    const Int_t n_runs = 3;
    const TString run[] = {"27487", "27489", "27490"};
    
    cout << "alignment_vd() : Starting..." << endl;

    //set_my_style();
    gStyle->SetOptStat(1110);
    gStyle->SetOptFit(111);
    
    const TString filename_out = "alignment_vd_" + suffix;
    const Short_t scols        = 1024;
    const Short_t srows        = 512;
    const Short_t n_chips      = 9;   // number of chips
    
    
    TChain* chain = new TChain("extracted_tracks", "extracted_tracks");
    for(Int_t i=0; i<n_runs; ++i) {
        TString fname = dirpath_data + run[i];
        fname += "/results" + suffix;
        fname += "/prealignment_vd";// + suffix;
        fname += ".root";
        cout << "alignment_vd() : Loading file " << fname << endl;
        if(!chain->Add(fname.Data())) {
            cerr << "alignment_vd() : ERROR: Cannot find extracted tracks tree in file! " << fname << endl;
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

    // alignment before
    Alignment align_before;
    align_before.SetPos(TVector3(11.1, 120.5-30.15*4, 92.3));
    TRotation rot;
    rot.RotateY(6.*TMath::DegToRad());
    align_before.SetRotation(rot);
    if(!align_before.WriteFile(Form("%s/%s_before.aln", dirpath_out.Data(), filename_out.Data()))) {
        cerr << "alignment_vd() : ERROR: Cannot write alignment parameters to file! " << endl;
        return kFALSE;
    }
                
    
    TString fname_out = dirpath_out + "/alignment_vd_combined.root";
    TFile* file_plots = new TFile(fname_out, "RECREATE");
    if(!file_plots->IsOpen()) {
        cerr << "alignment_vd() : ERROR: Cannot open output plots file! " << fname_out << endl;
        return kFALSE;
    }
    
    TH1F *hDXbefore = new TH1F("hDXbefore", "Difference track hit - cluster pos X, BEFORE alignment;#DeltaX [mm];a.u.",
                               200, -0.25, 0.25);
    TH1F *hDYbefore = new TH1F("hDYbefore", "Difference track hit - cluster pos Y, BEFORE alignment;#DeltaY [mm];a.u.",
                               200, -0.25, 0.25);
    TH1F *hDXafter = new TH1F("hDXafter", "Difference track hit - cluster pos X, AFTER alignment;#DeltaX [mm];a.u.",
                               200, -0.25, 0.25);
    TH1F *hDYafter = new TH1F("hDYafter", "Difference track hit - cluster pos Y, AFTER alignment;#DeltaY [mm];a.u.",
                               200, -0.25, 0.25);
    
    ntracks = nentries;    
    for(Int_t i=0; i < nentries; ++i) {
        chain->GetEntry(i);
        clux[i] = ex_clux;
        cluy[i] = ex_cluy;
        for(Int_t j=0; j<3; ++j) {
            to_arr[i][j] = ex_to[j];
            td_arr[i][j] = ex_td[j];
        }
        TVector3 d = align_before.DistPixLine(clux[i], cluy[i], TVector3(to_arr[i]), TVector3(td_arr[i]));
        hDXbefore->Fill(d.X());
        hDYbefore->Fill(d.Y());
    }

    //__________MINUIT________________________________________________________
    const Float_t xpos = 11.1;
    const Float_t ypos = 120.4;//-30.15*4;
    const Float_t zpos = 92.3;

    Bool_t   fix_par[]          = { 0, 0, 0, 0, 0, 0};
    Char_t   PARM_NAMES[6][255] = {"x0","y0","z0", "ang1", "ang2", "ang3"};
    Double_t PARM_START[6]      = {11.05, -0.115, 92.24, 0.115,  0.0,  0.};
    //Double_t PARM_START[6]      = {900., 900., 100, 0.10,  0.0,  0.0};
    Double_t PARM_STEP[6]       = { 0.1, 0.1, 0.2, 0.01,  0.1, 0.1};
    Double_t PARM_LOWER[6]      = {-1e3, -1e3, -1e3, -1.,  -1., -1.};
    Double_t PARM_UPPER[6]      = { 1e3,  1e3,  1e3,  1.,   1.,  1.}; 
    Double_t PARM_END[6];
    
    TMinuit *minuit = new TMinuit(npars);
    minuit->SetFCN( fcn );
    
    /* set starting values and steps */
    for (Int_t i=0; i < npars; i++) {
        //if(fix_par[i]) PARM_START[i] = 0.;
        if( minuit->DefineParameter(i, PARM_NAMES[i], PARM_START[i], PARM_STEP[i], PARM_LOWER[i], PARM_UPPER[i]) ) {
            cout << "alignment_vd() : MINUIT ERROR : Unable to define parameter " << i << endl;
            return kFALSE;
        }
        if(fix_par[i]) minuit->FixParameter(i);
    }
    
    Double_t arglist[10];
    Int_t ivarbl,ierflg;
    Double_t bnd1, bnd2;
    arglist[0] = 1.0;
    if (arglist[0]) minuit->mnexcm("SET ERR", arglist, 1, ierflg);
    arglist[0] = 10000;   // do at least 1000 function calls
    arglist[1] = 0.1;     // tolerance = 0.1
    minuit->mnexcm("MIGRAD", arglist, 2, ierflg );
    //minuit->mnexcm("HESSE", arglist, 2, ierflg );
    //minuit->mnexcm("MINOS", arglist, 2, ierflg );
    cout << endl << "alignment_vd() : MIGRAD exited with status " << ierflg << endl;
    if(ierflg) gSystem->Exit(1);//return kFALSE;
    //minuit->mnexcm("IMPROVE", arglist, 1, ierflg );
    
    cout << endl;
    for (Int_t i=0; i<npars; i++) {
        Double_t val, err;
        TString tag;
        minuit->GetParameter(i, val, err);
        PARM_END[i] = val;
        //minuit->mnpout( i, tag, val, err, bnd1, bnd2, ivarbl);
        if(i >= 3)
            printf("%s: %f +/- %f\n",PARM_NAMES[i],val*TMath::RadToDeg(),err*TMath::RadToDeg());
        else
            printf("%s: %f +/- %f\n",PARM_NAMES[i],val,err);
    }

/*    //Get error matrix 
    Double_t err_mat[npars][npars];
    minuit->mnemat(&err_mat[0][0], npars);
    printf("Error matrix:\n");
    for (Int_t i=0;i<npars;i++){
        printf("%f %f\n",err_mat[0][i],err_mat[1][i]);
    }
*/    
    //__________MINUIT________________________________________________________    

    Alignment align_after;
    align_after.SetPos(TVector3(PARM_END[0], PARM_END[1], PARM_END[2]));
    TRotation rota;
    rota.RotateY(PARM_END[3]);
    rota.RotateX(PARM_END[4]);
    rota.RotateZ(PARM_END[5]);
    align_after.SetRotation(rota);
    if(!align_after.WriteFile(Form("%s/%s_after.aln", dirpath_out.Data(), filename_out.Data()))) {
        cerr << "alignment_vd() : ERROR: Cannot write alignment parameters to file! " << endl;
        return kFALSE;
    }
    
    for(Int_t i=0; i < ntracks; ++i) {
        TVector3 d = align_after.DistPixLine(clux[i], cluy[i], TVector3(to_arr[i]), TVector3(td_arr[i]));
        hDXafter->Fill(d.X());
        hDYafter->Fill(d.Y());
    }

    Float_t rang = 0.1;
    TCanvas *c1 = new TCanvas("c1", "Alignment Canvas 1", 50, 50, 1600, 1000);
    c1->Divide(2,1);
    c1->cd(1);
    hDXbefore->Fit("gaus", "Q");
    hDXbefore->GetXaxis()->SetRangeUser(-rang,rang);
    hDXbefore->DrawCopy();
    c1->cd(2);
    hDYbefore->Fit("gaus", "Q");
    hDYbefore->GetXaxis()->SetRangeUser(-rang,rang);
    hDYbefore->DrawCopy();
    c1->cd(3);
    hDXafter->Fit("gaus", "Q");
    hDXafter->GetXaxis()->SetRangeUser(-rang,rang);
    hDXafter->DrawCopy();
    c1->cd(4);
    hDYafter->Fit("gaus", "Q");
    hDYafter->GetXaxis()->SetRangeUser(-rang,rang);
    hDYafter->DrawCopy();
    c1->Print(Form("%s/alignment_vd_c1.pdf", dirpath_out.Data()));

    file_plots->Write();
    
    delete hDXbefore;
    delete hDYbefore;
    delete hDXafter;
    delete hDYafter;
    
    file_plots->Close();

    cout << "alignment_vd() : Done!" << endl;
    
    return kTRUE;
}
