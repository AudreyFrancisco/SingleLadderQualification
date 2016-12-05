/* Written by Jacobus van Hoorne, jvanhoor@cern.ch */

#include <Riostream.h>
#include <TString.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>

#include "helpers.h"

#define NSEC 1

Float_t n_trg=1e6;

char fNameCfg [1024];
char fNameOut [1024];
char fPathOut [1024];
char fSuffix  [1024]; // date_time suffix

//----------------------------------------------------------
void dblcol_adr_to_col_row(UShort_t doublecol, UShort_t address, UShort_t &col, UShort_t &row) {
    // palpide 1/2
    //col = doublecol*2 + (address%4 < 2 ? 1 : 0);
    //row = 2*(address/4) + 1-(address%2);
    // palpide 3/ alpide
    col = doublecol*2;
    col += ((((address%4)==1) || ((address%4)==2)) ? 1:0);
    row = address/2;
}


void PrepareOutputFile (TString fName) {
    string buff1=fName.Data();
    unsigned pos=buff1.find_last_of("_");
    string buff2=buff1.substr(0,pos);
    pos=buff2.find_last_of("_");
    sprintf(fSuffix, "%s", buff1.substr(pos, 14).c_str());
    
    // fNameOut
    sprintf(fNameOut, "NoiseOccupancy%s.root", fSuffix);

    printf("Output file: %s\n", fNameOut);
    pos=buff1.find_last_of("/");
    
    // fPathOut
    sprintf(fPathOut, "%s", buff1.substr(0, pos+1).c_str());
    printf("Output path: %s\n", fPathOut);

    // fNameCfg
    sprintf(fNameCfg, "ScanConfig%s.cfg", fSuffix);
}



// main
//-------------------------------------------------------------------------------------------
Bool_t NoiseOccupancyRawToHisto(TString file_path) {

    ifstream raw_file(file_path.Data());
    if(!raw_file.good()) {
        cout << "RAW file not found!" << endl;
        return kFALSE;
    }

    PrepareOutputFile(file_path);
    // output root file
    TString f_out_name = Form("%s%s", fPathOut, fNameOut);
    TFile *f_out = new TFile(f_out_name, "RECREATE");
    f_out->cd();

    // read config from file
    //pos = file_path.Last('.');
    //TString f_cfg_name = TString(file_path(0, pos) + ".cfg");
    //ifstream cfg_file(Form("%s%s", fPathOut, fNameCfg));
    //if(!cfg_file.good()) {
    //    cout << "Config file not found!" << endl;
    //    return kFALSE;
    //}
 
    // get info
    MeasConfig_t conf; //= read_config_file(Form("%s%s", fPathOut, fNameCfg));
    print_meas_config(conf);


    //Float_t n_trg=conf.NTRIGGERS;


    // read in noise file and fill hit map
    TH2F *h2_hitmap;
    h2_hitmap = new TH2F(Form("h2_hitmap_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i", 
        conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
        conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
        conf.IDB, conf.IBIAS, conf.VCASP),
      Form("Noise Hit Map, VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i; Column; Row", 
        conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
        conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
        conf.IDB, conf.IBIAS, conf.VCASP),
      1024, -0.5, 1024.-0.5, 512, -0.5, 512.-0.5);
    h2_hitmap->SetStats(0);
  
    UShort_t dblcol, adr, col, row;
    UInt_t nhits;

    while(raw_file >> dblcol >> adr >> nhits && raw_file.good()) {
        dblcol_adr_to_col_row(dblcol, adr, col, row);
        cout << dblcol << " " << adr << " " << col << " " << row << " " << nhits << endl;
        h2_hitmap->Fill(col, row, nhits);
    }
    h2_hitmap->Write();
    // draw hit map before exluding noisiest pixels
    TCanvas *c1 = new TCanvas("c1", "Canvas 1", 0, 0, 1024, 512);
    c1->cd();
    h2_hitmap->DrawCopy("COLZ");

    // noise occupancy sector by sector, excluding hottest pixels 
    TH1F *h_noise_occ[NSEC];
    Int_t x_max, y_max, z_max; 
    Float_t int_hits = 0;
    Float_t noise_occ = 0;
    for (Int_t i_sec=0; i_sec<NSEC; i_sec++) {
        h_noise_occ[i_sec] = new TH1F(Form("h_noiseocc_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i_sec%i", 
            conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
            conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
            conf.IDB, conf.IBIAS, conf.VCASP, i_sec),
          Form("Noise Occupancy, Sector %i, VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i; # excluded pixels; noise occupancy [/event/pixel]", 
            i_sec, 
            conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
            conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
            conf.IDB, conf.IBIAS, conf.VCASP),
          1000, 0, 1000);

        h_noise_occ[i_sec]->SetLineColor(i_sec+1);
        h_noise_occ[i_sec]->SetStats(0);
        
        // exclude hottest pixels        
        cout << "----------------" << endl;
        cout << "sector " << i_sec << endl; 
        h2_hitmap->GetXaxis()->SetRange(i_sec*(1024/NSEC)+1, (i_sec+1)*(1024/NSEC)+1);
        int_hits = h2_hitmap->Integral();
        for (Int_t i_pix=0; i_pix<1000; i_pix++) {
            noise_occ = int_hits/n_trg/(1024./NSEC*512.);
            if (i_pix == 0) std::cout << "occ (0 masked) = " << noise_occ << std::endl;
            if (i_pix == 5) std::cout << "occ (5 masked) = " << noise_occ << std::endl;
            if (i_pix == 50) std::cout << "occ (50 masked) = " << noise_occ << std::endl;
            h_noise_occ[i_sec]->SetBinContent(i_pix+1, noise_occ);
    
            h2_hitmap->GetMaximumBin(x_max, y_max, z_max);
            int_hits-=h2_hitmap->GetBinContent(x_max, y_max);
            
            //cout << i_pix << "\t" << x_max << "\t" << y_max << "\t" << h2_hitmap->GetBinContent(x_max, y_max) << endl;
            h2_hitmap->SetBinContent(x_max, y_max, 0);
        }
        h_noise_occ[i_sec]->Write();
    }
 
    // plotting
    TCanvas *c2 = new TCanvas("c2", "Canvas 2", 0, 0, 1024, 512);
    c2->cd();
    c2->SetLogy();
    h_noise_occ[0]->SetMinimum(1e-12);
    h_noise_occ[0]->SetMaximum(1e-3);
    h_noise_occ[0]->DrawCopy("");
    for (Int_t i_sec=1; i_sec<NSEC; i_sec++) {
        h_noise_occ[i_sec]->DrawCopy("same");
    }

    cout << "Done!" << endl;

    c1->Print(Form("%s/hitmap%s.png", fPathOut, fSuffix));
    c1->Print(Form("%s/hitmap%s.pdf", fPathOut, fSuffix));
    c2->Print(Form("%s/noise_occ%s.png", fPathOut, fSuffix));
    c2->Print(Form("%s/noise_occ%s.pdf", fPathOut, fSuffix));

    ofstream fhr_file(Form("%s/fhr.txt",fPathOut));
    for (int iExcl=0; iExcl<1000; ++iExcl) {
        fhr_file << iExcl << "\t" << h_noise_occ[0]->GetBinContent(iExcl+1) << endl;
    }
    fhr_file.close();

    //delete h2_hitmap;
    f_out->Close();

    return kTRUE;
}
