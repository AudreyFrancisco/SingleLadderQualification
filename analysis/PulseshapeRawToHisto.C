#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <sstream>

#include "TCanvas.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TColor.h"
#include "TString.h"
#include "TFile.h"

#include "helpers.h"

using namespace std;


char fNameCfg [1024];
char fNameOut [1024];
char fPathOut [1024];
char fSuffix  [1024]; // date_time suffix


//----------------------------------------------------------
void PrepareOutputFile (TString fName) {
    string buff1=fName.Data();
    unsigned pos=buff1.find_last_of("_");
    string buff2=buff1.substr(0,pos);
    pos=buff2.find_last_of("_");
    sprintf(fSuffix, "%s", buff1.substr(pos, 14).c_str());
    
    //// fNameOut
    //sprintf(fNameOut, "NoiseOccupancy%s.root", fSuffix);

    //printf("Output file: %s\n", fNameOut);
    pos=buff1.find_last_of("/");
    
    // fPathOut
    sprintf(fPathOut, "%s", buff1.substr(0, pos+1).c_str());
    printf("Output path: %s\n", fPathOut);

    // fNameCfg
    sprintf(fNameCfg, "ScanConfig%s.cfg", fSuffix);
}


// main macro
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//Bool_t Pulseshape(Int_t Run_num=0, Bool_t Do_plot=kFALSE, Int_t Reg_sel=5) {
Bool_t PulseshapeRawToHisto(TString fName, Bool_t Do_plot=kFALSE, Int_t Reg_sel_plot=5) {
    cout << "Pulseshape" << endl;
    PrepareOutputFile(fName);

    ifstream cfg_file(Form("%s%s", fPathOut, fNameCfg));
    if(!cfg_file.good()) {
        cout << "Config file not found!" << endl;
        return kFALSE;
    }
 
    Float_t clk_dur = 0.0125; // 80MHz clock
    Int_t Charge, Delay, Col, Row, NHits;
    // get info
    MeasConfig_t conf = read_config_file(Form("%s%s", fPathOut, fNameCfg));
    print_meas_config(conf);

    Int_t Col_sel = conf.COL;
    Int_t Row_sel = conf.ROW;
    //Int_t VCasn   = 50;
    //Int_t Ithr    = 51;
    //Int_t Strobe_B_length = 250; // ns
    //Int_t Vbias   = 0;
    //Int_t IDB     = 64;
    Int_t n_inj = conf.NTRIGGERS;
    Int_t start_charge  = conf.CHARGESTART;
    Int_t end_charge    = conf.CHARGESTOP;
    Int_t step_charge   = conf.CHARGESTEP;
    Int_t range_charge  = end_charge-start_charge+1;
    Int_t n_bins_charge = (range_charge-1)/step_charge+1;
    Int_t start_delay   = conf.PULSEDELAYSTART;
    Int_t end_delay     = conf.PULSEDELAYSTOP;
    Int_t step_delay    = conf.PULSEDELAYSTEP;
    Int_t range_delay   = end_delay-start_delay+1; 
    Int_t n_bins_delay  = (range_delay-1)/step_delay+1;

    ifstream infile(fName.Data());
    if (infile) {
        cout << "start charge value: " << start_charge << endl;
        cout << "end charge value: " << end_charge << endl;
        cout << "step charge: " << step_charge << endl;

        cout << "start delay value: " << start_delay << endl;
        cout << "end delay value: " << end_delay << endl;
        cout << "step delay: " << step_delay << endl;
    }
    else { 
        cout << "cannot open file!!!" << endl;
        return kFALSE; 
    }


    // histograms for all 32 regions
    TH2F *hPulse[32];
    Float_t TimeDelay = 0;
    for (Int_t i_reg=0; i_reg<32; i_reg++) {
        //hPulse[i_reg] = new TH2F(Form("hPulse_%i", i_reg), 
        //        //Form("Pulse Shape, pixel %i/%i/%i, ITHR %i, VCASN %i, VBB -%i.0V, StrBlen %ins", i_reg, Col_sel, Row_sel, Ithr, VCasn, Vbias, Strobe_B_length),
        //        Form("Pulse Shape, pixel %i/%i/%i, VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i", 
        //          i_reg, Col_sel, Row_sel,
        //          conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
        //          conf.VCLIP, conf.VRESETD, 
        //          conf.IDB, conf.IBIAS, conf.VCASP),
        //        n_bins_delay, (start_delay+4)*clk_dur-clk_dur*step_delay*0.5, (end_delay+4)*clk_dur+clk_dur*step_delay*0.5, 
        //        n_bins_charge, start_charge-step_charge*0.5, end_charge+step_charge*0.5);
        hPulse[i_reg] = new TH2F(Form("hPulse_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i__reg_%i_dcol%i_addr%i_strbBlen%i",
                  conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
                  conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
                  conf.IDB, conf.IBIAS, conf.VCASP,
                  i_reg, Col_sel, Row_sel, conf.STROBELENGTH),
                Form("Pulse Shape, pixel %i/%i/%i, StrBlen %ins, VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i", 
                  i_reg, Col_sel, Row_sel, conf.STROBELENGTH,
                  conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
                  conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
                  conf.IDB, conf.IBIAS, conf.VCASP),
                //Form("Pulse Shape, pixel %i/%i/%i, IThr %i, VCasn %i, Vbias -%i.0V, StrBlen %ins", i_reg, DCol_sel, Addr_sel, Ithr, VCasn, Vbias, Strobe_B_length),
                n_bins_delay, (start_delay+4)*clk_dur-clk_dur*step_delay*0.5, (end_delay+4)*clk_dur+clk_dur*step_delay*0.5, 
                n_bins_charge, start_charge-step_charge*0.5, end_charge+step_charge*0.5);
    }
    
    
    //TAxis *ax = (TAxis*)hPulse[Reg_sel_plot]->GetXaxis();
    //cout << ax->GetBinLowEdge(1) << endl;
    //cout << ax->GetBinLowEdge(10) << endl;
    //TAxis *ax = (TAxis*)hPulse[Reg_sel_plot]->GetYaxis();
    //cout << ax->GetBinLowEdge(1) << endl;
    //cout << ax->GetBinLowEdge(10) << endl;
        
    // read file and fill histogram
    while (infile >> Col >> Row >> Charge >> Delay >> NHits) {
        TimeDelay = Delay + 4; // add 50ns offset
        TimeDelay *= clk_dur;
        hPulse[Col/32]->Fill(TimeDelay, Charge, NHits);
    }
    infile.close();
    // normalizing bin contents for efficiency
    for (Int_t i_reg=0; i_reg<32; i_reg++) {
        for (Int_t i_col=1; i_col<=n_bins_delay; i_col++) {
            for (Int_t i_row=1; i_row<=n_bins_charge; i_row++) {
                 hPulse[i_reg]->SetBinContent(i_col, i_row, hPulse[i_reg]->GetBinContent(i_col, i_row)/n_inj);
            }
        }
    }
    
    // drawing
    TCanvas *c = new TCanvas("c", "c", 1200, 700);
    c->cd();
    c->cd()->SetGrid();
    gStyle->SetOptStat(0);
    //define new nice color palette:
    const Int_t NRGBs = 5;
    const Int_t NCont = 75;
    Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    Double_t red[NRGBs]     = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    Double_t blue[NRGBs]    = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);

    // write result histograms to file
    //TFile *out_file = new TFile(Form("%s/PulseShape%s_pix_%i_%i_ITH%i_VCASN%i_VBIAS-%i.0V_StrBLen%ins.root", 
    //        fPathOut, fSuffix, DCol_sel, Addr_sel, Ithr, VCasn, Vbias, Strobe_B_length), "RECREATE");
    //TFile *out_file = new TFile(Form("%s/PulseShape%s.root", fPathOut, fSuffix), "RECREATE");
    TFile *out_file = new TFile(Form("%s/PulselengthScan%s.root", fPathOut, fSuffix), "RECREATE");

    out_file->cd();

    for (Int_t i_reg=0; i_reg<32; i_reg++) {
        c->Clear();
        hPulse[i_reg]->Draw("COLZ");
        hPulse[i_reg]->GetXaxis()->SetTitle("Delay [#mus]");
        hPulse[i_reg]->GetYaxis()->SetTitle("Charge [DAC]");
        if (Reg_sel_plot==i_reg && Do_plot) {
            c->Print(Form("%s/Plots/PulseShape%s_pix_%i_%i_%i_ITH%i_VCASN%i_VBIAS-%i.0V_StrBLen%ins.pdf", 
                    fPathOut, fSuffix, Reg_sel_plot*32+Col_sel, Row_sel, conf.ITHR, conf.VCASN, conf.VBB, conf.STROBELENGTH*25));
            c->Print(Form("%s/Plots/PulseShape%s_pix_%i_%i_%i_ITH%i_VCASN%i_VBIAS-%i.0V_StrBLen%ins.png", 
                    fPathOut, fSuffix, Reg_sel_plot*32+Col_sel, Row_sel, conf.ITHR, conf.VCASN, conf.VBB, conf.STROBELENGTH*25));
        }
        hPulse[i_reg]->Write();
    }
    out_file->Close();


    return kTRUE;
}


