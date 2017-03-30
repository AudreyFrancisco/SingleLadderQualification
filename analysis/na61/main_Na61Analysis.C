#include <TSystem.h>
#include <Riostream.h>
#include "Na61Analysis.hpp"

void main_Na61Analysis() {

    enum  AnaType { kPrealign, kEfficiency };
    const TString dir_al = "/home/msuljic/work/na61/data/by_run/";
    const TString dir_vd = "/home/msuljic/work/na61/data/vdal/";
    const TString runs_al[] = {"27487",
                               "27489",
                               "27490"};
    const TString runs_vd_aln[] = {"vd_all_183.root",
                                   "vd_all_185.root",
                                   "vd_all_186.root"};
    const TString runs_vd_eff[] = {"vd_all_183_pv.root",
                                   "vd_all_185_pv.root",
                                   "vd_all_186_pv.root"};
    const TString suffix_dir = "_cr2";
    const Int_t   n_runs     = 3;
    const Bool_t  write_separate = kFALSE;
    //const AnaType ana_type = kPrealign;
    const AnaType ana_type = kEfficiency;
    const TString cut_ds   = "pawelall";

    TString suffix_out;
    const TString *runs_vd;
    
    if(ana_type == kPrealign)        { runs_vd = runs_vd_aln; suffix_out += "_aln"; }
    else if(ana_type == kEfficiency) { runs_vd = runs_vd_eff; suffix_out += "_eff"; }
    else                             { runs_vd = runs_vd_eff; suffix_out += "_ukn"; }

    if(cut_ds != "") suffix_out += "_" + cut_ds;
    
    Na61Analysis *ana = new Na61Analysis();
    if( !ana->SetLogFile(Form("/home/msuljic/work/na61/data/alignment_vd/na61analysis%s.log", suffix_out.Data())) )
        gSystem->Exit(1);
    ana->SetVerboseLevel(3);

    ana->SetDefaultAlignment();
    ana->InitHistograms();
    //ana->FillChipPosHistos();
    ana->SetCutDataset(cut_ds);

    ana->LoadHotPixMask(4, "/home/msuljic/work/na61/data/masks/mask_chip4.dat");
    ana->LoadExclColMask(4, "/home/msuljic/work/na61/data/masks/excl_cols_chip4.dat");
    
    for(Int_t i=0; i<n_runs; ++i) {
        cout << endl;
        TString run_al = dir_al + runs_al[i] + "/event_tree.root";
        TString run_vd = dir_vd + runs_vd[i];
        TString dplots = dir_al + runs_al[i] + "/results" + suffix_dir + "/";
        
        if( !ana->SetInputFileALPIDE(run_al) )   gSystem->Exit(1);
        if( !ana->SetInputFileVDTracks(run_vd) ) gSystem->Exit(1);

        if(ana_type == kPrealign) {
            ana->PrealignmentVD();
            ana->ExtractTracksVD(4);

            //run_vd = dir_vd + runs_vd_eff[i];
            //if( !ana->SetInputFileVDTracks(run_vd) ) gSystem->Exit(1);
            //ana->PrealignmentVD();
            //ana->ExtractTracksVD(4);

            if(write_separate) {
                ana->SetOutputDirPlots(dplots);
                ana->WriteHistograms("prealignment_vd.root");
                ana->WriteTracksTree("prealignment_vd.root");
                ana->ResetHistograms();
                ana->ResetTracksTree();
            }
        }
        
        ana->EfficiencyVD();
        //ana->ExtractHitsVD();
        //ana->ExtractChipHits("/home/msuljic/work/na61/data/vdal/alp_hits_run_" + runs_al[i] + ".root");

    }
    cout << endl;
    
    ana->SetOutputDirPlots("/home/msuljic/work/na61/data/alignment_vd/");
    if(ana_type == kPrealign && !write_separate) {
        ana->WriteHistograms(Form("prealignment_vd%s.root", suffix_out.Data()));
        ana->WriteTracksTree(Form("prealignment_vd%s.root", suffix_out.Data()));
        ana->DrawHistograms("prealignment");
    }
    else if(ana_type == kEfficiency) {
        ana->WriteHistograms(Form("efficiency_vd%s.root", suffix_out.Data()));
        //ana->WriteHitsTree("efficiency_vd.root");
        ana->PrintEfficiencyVD();
        ana->DrawHistograms("efficiency");
    }
    
    delete ana;
    
}
