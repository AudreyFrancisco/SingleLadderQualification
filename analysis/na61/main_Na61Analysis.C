#include <TSystem.h>
#include <Riostream.h>
#include "Na61Analysis.hpp"

void main_Na61Analysis() {

    const TString dir_al = "/home/msuljic/work/na61/data/by_run/";
    const TString dir_vd = "/home/msuljic/work/na61/data/vdal/";
    const TString runs_al[] = {"27487",
                               "27489",
                               "27490"};
    const TString runs_vd_aln[] = {"vd_183_new.root",
                                   "vd_185_new.root",
                                   "vd_186_new.root"};
    const TString runs_vd_eff[] = {"vd_3pt_183_eff.root",
                                   "vd_3pt_185_eff.root",
                                   "vd_3pt_186_eff.root"};
    enum  AnaType { kPrealign, kEfficiency };
    const TString suffix   = "_cr2";
    const Int_t   n_runs   = 3;
    const AnaType ana_type = kPrealign;
    //const AnaType ana_type = kEfficiency;
    
    const TString *runs_vd;
    if(ana_type == kPrealign)        runs_vd = runs_vd_aln;
    else if(ana_type == kEfficiency) runs_vd = runs_vd_eff;
    else                             runs_vd = runs_vd_eff;
        
    
    Na61Analysis *ana = new Na61Analysis();
    ana->SetVerboseLevel(3);

    ana->SetDefaultAlignment();
    ana->InitHistograms();
    //ana->FillChipPosHistos();

    ana->LoadHotPixMask(4, "/home/msuljic/work/na61/data/masks/mask_chip4.dat");
    ana->LoadExclColMask(4, "/home/msuljic/work/na61/data/masks/excl_cols_chip4.dat");
    
    for(Int_t i=0; i<n_runs; ++i) {
        cout << endl;
        TString run_al = dir_al + runs_al[i] + "/event_tree.root";
        TString run_vd = dir_vd + runs_vd[i];
        TString dplots = dir_al + runs_al[i] + "/results" + suffix + "/";
        
        if( !ana->SetInputFileALPIDE(run_al) )   gSystem->Exit(1);
        if( !ana->SetInputFileVDTracks(run_vd) ) gSystem->Exit(1);

        if(ana_type == kPrealign) {
            ana->PrealignmentVD();
            ana->ExtractTracksVD(4);

            //run_vd = dir_vd + runs_vd_eff[i];
            //if( !ana->SetInputFileVDTracks(run_vd) ) gSystem->Exit(1);
            //ana->PrealignmentVD();
            //ana->ExtractTracksVD(4);
            
            ana->SetOutputDirPlots(dplots);
            ana->WriteHistograms("prealignment_vd.root");
            ana->WriteTracksTree("prealignment_vd.root");
            
            ana->ResetHistograms();
            ana->ResetTracksTree();
        }
        
        ana->EfficiencyVD();
        //ana->ExtractHitsVD();

    }
    cout << endl;
    
    ana->SetOutputDirPlots("~/work/na61/data/alignment_vd/");
    if(ana_type == kPrealign) {
        //ana->WriteHistograms("prealignment_vd.root");
        //ana->WriteTracksTree("prealignment_vd.root");
        //ana->DrawHistograms("prealignment");
    }
    else if(ana_type == kEfficiency) {
        ana->WriteHistograms("efficiency_vd.root");
        //ana->WriteHitsTree("efficiency_vd.root");
        ana->PrintEfficiencyVD();
        ana->DrawHistograms("efficiency");
    }
    
    delete ana;
    
}
