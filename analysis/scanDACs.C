/* Written by Miljenko Suljic, m.suljic@cern.ch */

#include <iostream>

#include <Riostream.h>
#include <TString.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>

Bool_t scanDACs(TString directory = "../pALPIDEfs-software/Data/", Int_t n_bits = 256) {

    const Int_t   n_dacs = 14;
    //const TString filename[n_dacs] = {"VAUX", "VRESET", "VCASN", "VCASP", "VPULSEL", "VPULSEH", "IRESET", "IBIAS", "IDB", "ITHR"};
    const TString filename[n_dacs] = {"VRESETP","VRESETD", "VCASP", "VCASN", "VPULSEH", "VPULSEL", "VCASN2","VCLIP", "VTEMP", "IAUX2", "IRESET", "IDB","IBIAS", "ITHR"};

    TCanvas *c1 = new TCanvas("cdacs", "pALPIDEfs DACs", 0, 0, 1000, 600);
    c1->Divide(5, 2);
    TGraph *grdac[n_dacs];
    Float_t* dac = new Float_t[n_bits];
    Float_t* adc = new Float_t[n_bits];

    TFile* out = new TFile (directory+"graphs.root", "RECREATE");

    for(Int_t i=0; i<n_dacs; ++i) {
	TString filepath(directory);
        filepath += (filename[i].BeginsWith("V") ? "VDAC_" : "IDAC_") + filename[i] + ".dat";
        ifstream dacfile(filepath.Data());
        if(!dacfile.good()) { cout << "Cannot find " << filename[i] << "(" << filepath << ")" << endl; return kFALSE; }
        for(Int_t j=0; j<n_bits; ++j) {
            if(!dacfile.good()) { cout << "Problem in " << filename[i] << endl; return kFALSE; }
            dacfile >> dac[j] >> adc[j];
        }
        c1->cd(i+1);
        grdac[i] = new TGraph(n_bits, dac, adc);
        grdac[i]->SetTitle(filename[i] + (filename[i].BeginsWith("V") ? ";DAC;Voltage [V]" : ";DAC;Current [nA]"));
        grdac[i]->SetName(filename[i]);
//        grdac[i]->SetMarkerStyle(21);
        grdac[i]->SetMarkerColor(2);
        grdac[i]->SetLineColor(2);
        grdac[i]->Draw("APL");
        TFitResultPtr r = grdac[i]->Fit("pol1", "QS");
        grdac[i]->Write();

        std::cout << filename[i] << '\t' << r->Value(0) << '\t' << r->Value(1) << std::endl;
    }

    c1->SaveAs("dacscan.png");
    c1->Write();
    out->Close();

    cout << "Done!" << endl;
    return kTRUE;
}
