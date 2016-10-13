#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2.h"
#include "TMath.h"
#include "TGraph.h"
#include "TString.h"
#include "TCanvas.h"

#include <stdio.h>
#include <fstream>
#include <iostream>

#include "helpers.h"

using namespace std;

#define NSEC 1

int nInj       = 50;

double maxchi2 = 5;

int data[1024];
int x   [1024];
int NPoints;

char fNameCfg [1024];
char fNameOut [1024];
char fPathOut [1024];
char fSuffix  [1024]; // date_time suffix
FILE *fpOut;

int ELECTRONS_PER_DAC = 10; 

int NPixels;

int NNostart;
int NChisq;

TH1F *hThresh[NSEC]={0};
TH1F *hNoise [NSEC]={0};
TH1F *hChi2=0;

void PrepareHistos() {
    for (int isec=0;isec<NSEC;++isec) {
        delete hThresh[isec];
        hThresh[isec]=new TH1F(Form("hThresh%d",isec),Form("Threshold, sector %d",isec),125,0.,750.);
        delete hNoise[isec];
        hNoise [isec]=new TH1F(Form("hNoise%d" ,isec),Form("Noise, sector %d"    ,isec),60,0., 60.);
    }
    delete hChi2;hChi2=0;
    hChi2=new TH1F("hChi2","Chi square distribution",1000,0.,100.);
}

void ResetData() {
    for (int i=0; i <= 256; i++) {
        data[i] = 0;
    }
}


Double_t erf( Double_t *xx, Double_t *par){
    return (nInj / 2) *TMath::Erf((xx[0] - par[0]) / (sqrt(2) *par[1])) +(nInj / 2);
}


float FindStart () {
    float Upper = -1;
    float Lower = -1;

    for (int i = 0; i < NPoints; i ++) {
        if (data[i] == nInj) {
            Upper = (float) x[i];
            break;
        }
    }
    if (Upper == -1) return -1;
    for (int i = NPoints-1; i > 0; i--) {
        if (data[i] == 0) {
            Lower = (float)x[i];
            break;
        }
    }
    if ((Lower == -1) || (Upper < Lower)) return -1;
    return (Upper + Lower)/2;
}


bool GetThreshold(double *thresh,double *noise,double *chi2) {
    TGraph *g      = new TGraph(NPoints, x, data);
    TF1    *fitfcn = new TF1("fitfcn", erf, 0, 1500, 2);
    double Start   = FindStart();

    if (Start < 0) {
        NNostart ++;
        return false;
    }

    fitfcn->SetParameter(0,Start);

    fitfcn->SetParameter(1,8);

    fitfcn->SetParName(0, "Threshold");
    fitfcn->SetParName(1, "Noise");

    //g->SetMarkerStyle(20);
    //g->Draw("AP");
    g->Fit("fitfcn","Q");

    *noise  = fitfcn->GetParameter(1);
    *thresh = fitfcn->GetParameter(0);
    *chi2   = fitfcn->GetChisquare()/fitfcn->GetNDF();

    hChi2->Fill(*chi2);

    g->Delete();
    fitfcn->Delete();
    return true;
}


void ProcessPixel (int dcol, int address) {
    double thresh, noise, chi2;
    int isec = dcol/(1024/(NSEC*2));

    if (!GetThreshold(&thresh, &noise, &chi2)) return;

    if (fpOut)
        fprintf(fpOut, "%d %d %.1f %.1f %.2f\n", dcol, address, thresh, noise, chi2);

    if(chi2<maxchi2){
        hThresh[isec]->Fill(thresh);
        hNoise [isec]->Fill(noise );
    }
}



void PrepareOutputFile (const char *fName, bool suffixIsTstmp) {
    string buff1=fName;
    unsigned pos1=buff1.find_last_of("_");
    string buff2=buff1.substr(0,pos1);
    unsigned pos2=buff2.find_last_of("_");
    if (!suffixIsTstmp) {
        sprintf(fSuffix, "%s", buff1.substr(pos2, 14).c_str()); // for std measurements
    } 
    else {
        sprintf(fSuffix, "%s", buff1.substr(pos1, 11).c_str()); // with unix tstmp
    }
    // fNameOut
    sprintf(fNameOut, "FitValues%s.dat", fSuffix);

    printf("Output file: %s\n", fNameOut);
    pos1=buff1.find_last_of("/");

    // fPathOut
    sprintf(fPathOut, "%s", buff1.substr(0, pos1+1).c_str());
    printf("Output path: %s\n", fPathOut);

    char fOut[1024];
    sprintf(fOut, "%s%s", fPathOut, fNameOut);
    // fpOut
    fpOut = fopen(fOut, "w");

    // fNameCfg
    sprintf(fNameCfg, "ScanConfig%s.cfg", fSuffix);
    printf("Cfg file: %s\n", fNameCfg);

}


int ProcessFile (const char *fName) {
    FILE *fp = fopen (fName, "r");
    if (fp==0) {
      std::cout << "threshold data file could not be opened, please check!" << std::endl;
      return -1;
    }
    int dcol, address, ampl, hits;
    int lastdcol = -1, lastaddress = -1;
    NPoints  = 0;
    NPixels  = 0;
    NNostart = 0;
    NChisq   = 0;

    //printf("strstr result: %s\n", strstr(
    //sprintf(fNameOut, "FitValues%s", strstr(fName,"_"));
    //printf("Output file: %s\n", fNameOut);
    //fpOut = fopen(fNameOut, "w");

    ResetData();
    while ((fscanf (fp, "%d %d %d %d", &dcol, &address, &ampl, &hits) == 4)) {

        //if ((dcol < 255) || ((dcol == 255) && (address < 280))) continue;

        if (((lastdcol != dcol) || (address != lastaddress)) && (NPoints!= 0)) {
            ProcessPixel(lastdcol, lastaddress);
            NPixels ++;
            ResetData   ();
            NPoints  = 0;
        }

        lastdcol = dcol;
        lastaddress = address;
        data [NPoints] = hits;
        x    [NPoints] = ampl * ELECTRONS_PER_DAC;
        NPoints ++;
    }
    fclose(fp);
    if (fpOut) fclose(fpOut);
    return 0;
}


int ThresholdRawToHisto(const char *fName, bool WriteToFile=false, bool saveCanvas=false, bool suffixIsTstmp=false) {
    PrepareHistos();
    PrepareOutputFile(fName, suffixIsTstmp);

    // read run config file for settings information
    MeasConfig_t conf; reset_meas_config(&conf);
    ifstream cfg_file(Form("%s%s", fPathOut, fNameCfg));
    if(!cfg_file.good()) {
        std::cout << "Config file not found!" << std::endl;
        return -1;
    } 
    conf = read_config_file(Form("%s%s", fPathOut, fNameCfg));
    print_meas_config(conf);

    nInj=conf.NTRIGGERS;
    // process threshold data
    if (ProcessFile(fName)==-1) {
        return -1;
    }

    std::cout << "Found " << NPixels << " pixel." << std::endl;
    std::cout << "No start point found: " << NNostart << std::endl;
    std::cout << "Chisq cut failed:     " << NChisq << std::endl;
    std::cout << "Chisq cut value:     " << maxchi2 << std::endl;

    for (int isec=0;isec<NSEC;++isec) {
        hThresh[isec]->SetLineColor(1+isec);
        hNoise [isec]->SetLineColor(1+isec);
    }

    //  hThresh[0]>SetMaximum(150);
    //  hNoise[0]->SetMaximum(500);

    TCanvas* c_thresh = new TCanvas;
    hThresh[0]->Draw();
    for (int isec=1;isec<NSEC;++isec) {
        hThresh[isec]->Draw("SAME");
    }

    TCanvas* c_noise = new TCanvas;
    hNoise [0]->Draw();
    for (int isec=1;isec<NSEC;++isec) {
        hNoise [isec]->Draw("SAME");
    }

    if (saveCanvas) {
        c_thresh->SaveAs(Form("%sthresholds%s.png", fPathOut, fSuffix));
        c_noise ->SaveAs(Form("%snoise%s.png"     , fPathOut, fSuffix));
    }

    if (WriteToFile) {
        // file with result summary
        FILE *fp = fopen(Form("%sThresholdSummary%s.dat", fPathOut, fSuffix), "w");

        for (int isec=0;isec<NSEC;++isec) {
            fprintf(fp, "%i %.1f %.1f %.1f %.1f %.0f %i\n", isec,
                    hThresh[isec]->GetMean(), hThresh[isec]->GetRMS(), 
                    hNoise[isec]->GetMean(), hNoise[isec]->GetRMS(),
                    hThresh[isec]->GetEntries(), conf.MASKSTAGES*32*4); // number of pixels with successful fit, number of pixels scanned
        }
        fclose(fp);

        // root file with histograms
        TFile *f_out = new TFile(Form("%sThresholds%s.root", fPathOut, fSuffix), "RECREATE");
        f_out->cd();

        for (int isec=0;isec<NSEC;++isec) {
            hThresh[isec]->SetName(Form("h_thresholds_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i_sec%i", 
                        conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
                        conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
                        conf.IDB, conf.IBIAS, conf.VCASP, isec));
            hThresh[isec]->Write();
            hNoise[isec]->SetName(Form("h_noise_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i_sec%i", 
                        conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
                        conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
                        conf.IDB, conf.IBIAS, conf.VCASP, isec));
            hNoise[isec]->Write();
        }

        // additional file with result summary, with file name containing settings info
        fp = fopen(Form("%sThresholdSummary_VBB%2.1f_ITHR%i_VCASN%i_VCASN2%i_VCLIP%i_IRESET%i_VRESETP%i_VRESETD%i_IDB%i_IBIAS%i_VCASP%i.dat", 
                    fPathOut,
                    conf.VBB, conf.ITHR, conf.VCASN, conf.VCASN2, 
                    conf.VCLIP, conf.IRESET, conf.VRESETP, conf.VRESETD, 
                    conf.IDB, conf.IBIAS, conf.VCASP), "w");

        for (int isec=0;isec<NSEC;++isec) {
            fprintf(fp, "%i %.1f %.1f %.1f %.1f %.0f %i\n", isec,
                    hThresh[isec]->GetMean(), hThresh[isec]->GetRMS(), 
                    hNoise[isec]->GetMean(), hNoise[isec]->GetRMS(),
                    hThresh[isec]->GetEntries(), conf.MASKSTAGES*32*4); // number of pixels with successful fit, number of pixels scanned
        }
        fclose(fp);

        f_out->Close();
    }

    for (int isec=0;isec<NSEC;++isec)
        std::cout << "Threshold sector "<<isec<<": " << hThresh[isec]->GetMean() << " +- " << hThresh[isec]->GetRMS() << " ( " << hThresh[isec]->GetEntries() << " entries / " << conf.MASKSTAGES*32*4 << " )" << std::endl;

    for (int isec=0;isec<NSEC;++isec)
        std::cout << "Noise sector "<<isec<<":     " << hNoise [isec]->GetMean() << " +- " << hNoise [isec]->GetRMS() << std::endl;


    //hChisq->Draw();

    return 0;
}
