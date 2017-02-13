#include <TSystem.h>
#include <Riostream.h>
#include "Na61Analysis.hpp"

void main_Na61Analysis() {

    const TString dir_al = "/home/msuljic/work/na61/data/by_run/";
    const TString dir_vd = "/home/msuljic/work/na61/data/vdal/";
    const TString runs_al[] = {"27487",
                               "27489"};
    //const TString runs_vd[] = {"vd_183.root",
    //                           "vd_185.root"};
    const TString runs_vd[] = {"vd_3pt_183_eff.root",
                               "vd_3pt_185_eff.root"};
    
    Na61Analysis *ana = new Na61Analysis();
    ana->SetVerboseLevel(3);

    ana->SetDefaultAlignment();
    ana->InitHistograms();
    //ana->FillChipPosHistos();

    ana->LoadHotPixMask(4, "/home/msuljic/work/na61/data/masks/mask_chip4.dat");
    ana->LoadExclColMask(4, "/home/msuljic/work/na61/data/masks/excl_cols_chip4.dat");
    
    for(Int_t i=0; i<2; ++i) {
        TString run_al = dir_al + runs_al[i] + "/event_tree.root";
        TString run_vd = dir_vd + runs_vd[i];
        if( !ana->SetInputFileALPIDE(run_al) )   gSystem->Exit(1);
        if( !ana->SetInputFileVDTracks(run_vd) ) gSystem->Exit(1);
        //ana->PrealignmentVD();
        //ana->ExtractTracksVD(4);
        ana->EfficiencyVD();
    }

    ana->SetOutputDirPlots("~/work/na61/data/alignment_vd/");
    ana->WriteHistograms("efficiency_vd.root");
    //ana->WriteTracksTree("prealignment_vd.root");

    ana->PrintEfficiencyVD();
    ana->DrawHistograms("efficiency");
    
    delete ana;
    
}
