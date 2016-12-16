/* Written by Miljenko Suljic, m.suljic@cern.ch */

#include <Riostream.h>
#include <TH2.h>
#include <TMath.h>
#include <TGraphErrors.h>
#include <TString.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <THStack.h>

#include "classes/helpers.h"

//#define DUMP

//_______________________________________________________________________________________________
Double_t erf(Double_t *x, Double_t *par){
    return 0.5*par[0]*(TMath::Erf((x[0] - par[1]) / (TMath::Sqrt2()*par[2])) + 1.);
}

//_______________________________________________________________________________________________
void findStartMeanSigma (Int_t npt, Float_t ninj, Float_t *x, Float_t *y, Float_t *mean, Float_t *sigma) {
    Float_t Upper = -1;
    Float_t Lower = -1;

    for (int i = 0; i < npt; i++) {
        if (y[i] > 0.99*ninj) {
            Upper = x[i];
            break;
        }
    }
    for (int i = npt-1; i > 0; i--) {
        if (y[i] < 0.01) {
            Lower = x[i];
            break;
        }
    }

    if ((Upper == -1) || (Lower == -1) || (Upper < Lower)) {
        *mean = -1;
        *sigma = -1.;
    }
    else {
        *mean =  (Upper + Lower)/2.;
        *sigma = (Upper - Lower)/6.;
    }
}

//_______________________________________________________________________________________________
void addErrors(Int_t npt, Int_t n, Float_t *k, Float_t *kerr) {
    for(Int_t i=0; i<npt; ++i) {
        kerr[i] = TMath::Sqrt((k[i]+1.)*(k[i]+2.)/(n+2.)/(n+3.)-(k[i]+1.)*(k[i]+1.)/(n+2.)/(n+2.));
        k[i] = (k[i]+1.)/(n+2.);
    }
}

//_______________________________________________________________________________________________
Int_t getNMidPoints(Int_t npt, Float_t ninj, Float_t *y) {
    Int_t nmp = 0;
    for (Int_t i=0; i<npt; ++i)
        if ( y[i] > 0. && y[i] < ninj )
            nmp++;
    return nmp;
}

//_______________________________________________________________________________________________
Bool_t pALPIDEfsThresholdPlots(
    const TString filepath,            // input raw file
    const Short_t n_secs = 8,          // number of sectors
    const Float_t chisq_cut = -5.,      // dont plot pixels with chisq higher than this value
    const Bool_t  WriteToFile = false, // write means to a text file
    const Int_t   ITH = 0,
    const Int_t   VCASN = 0,
    const Bool_t  saveCanvas = false  // create pdf of canvases and save plots to root file
    ) {
    
    set_my_style();
    
    // constants
    const Float_t n_inj = 50;
    const Float_t pALPIDE1calibration[4] = {7., 7., 7., 7.}; // electrons per dac [sector]
    const Float_t pALPIDE3calibration[8] = {9.93, 9.93, 9.93, 9.93, 9.93, 9.99, 10.22, 10.22}; // electrons per dac [sector]
    const Float_t *eledac = n_secs <= 4 ? pALPIDE1calibration : pALPIDE3calibration;

    // pALPIDEfs constants
    const Short_t scols = 1024/n_secs; // number of columns in a sector
    const Short_t srows = 512; // number of rows in a sector

    // macro constants
    const TString macro_name = "pALPIDEfsThresholdPlots";
    const Int_t   max_npt = 1024;
    TString fileplots_prefix = filepath; fileplots_prefix.Remove(fileplots_prefix.Last('.'), 4);
    TString fpath_out;

    // histograms
    TH1F *hThresh[n_secs];
    for(Int_t i=0; i<n_secs; ++i) {
        hThresh[i] = new TH1F (Form("hThresh_%i", i), Form("Sector %i;Threshold (electrons);Num. of pixels", i), 500, 0., 1000.);
        //hThresh[i]->SetLineColor(nice_color_for_index(i));
    }
    TH1F *hNoise[n_secs];
    for(Int_t i=0; i<n_secs; ++i) {
        hNoise[i] = new TH1F (Form("hNoise_%i", i), Form("Sector %i;Noise (electrons);Num. of pixels", i), 360, 0., 90.);
        //hNoise[i]->SetLineColor(nice_color_for_index(i));
    }
    TH1F *hTNR[n_secs];
    for(Int_t i=0; i<n_secs; ++i) {
        hTNR[i] = new TH1F (Form("hTNR_%i", i), Form("Sector %i, Threshold-to-Noise Ratio;Threshold-to-Noise Ratio;Num. of pixels", i), 200, 0., 200.);
        //hTNR[i]->SetLineColor(nice_color_for_index(i));
    }

    TH1F *hChisq = new TH1F ("hChisq", "Chi square distribution;Chi. sq;Num. of pixels", 1000, 0., 50.);
    TH1F *hProb = new TH1F ("hProb", "Probability distribution;Probability;Num. of pixels", 102, -0.01, 1.01);

    TH2F *hThreshPos = new TH2F("hThreshPos", "Threshold spatial distribution;Column;Row;Threshold (electrons)", 1024, -0.5, 1023.5, 512, -0.5, 511.5);
    hThreshPos->SetStats(0);
    TH2F *hNoisePos = new TH2F("hNoisePos", "Noise spatial distribution;Column;Row;Noise (electrons)", 1024, -0.5, 1023.5, 512, -0.5, 511.5);
    hNoisePos->SetStats(0);
    TH2F *hTNRPos = new TH2F("hTNRPos", "Threshold-to-Noise Ratio spatial distribution;Column;Row;TNR", 1024, -0.5, 1023.5, 512, -0.5, 511.5);
    hTNRPos->SetStats(0);
    TH2F *hChisqPos = new TH2F("hChisqPos", "Chi Square / NDF distribution;Column;Row;Chisq/NDF", 1024, -0.5, 1023.5, 512, -0.5, 511.5);
    hChisqPos->SetStats(0);

    TH2F *hThreshChisq = new TH2F("hThreshChisq", "Chi Square - Threshold Correlation;Chisq;Threshold (electrons);Num. of pixels", 1000, 0., 50., 200, 0., 400.);
    hThreshChisq->SetStats(0);
    TH2F *hNoiseChisq = new TH2F("hNoiseChisq", "Chi Square - Noise Correlation;Chisq;Noise (electrons);Num. of pixels", 1000, 0., 50., 120, 0., 30.);
    hNoiseChisq->SetStats(0);

    // process file
    ifstream file_raw; file_raw.open(filepath.Data());
    if(!file_raw.good()) {
        cerr << macro_name << " : ERROR: Cannot open file! " << filepath << endl;
        return kFALSE;
    }

    Short_t dblcol, addr;
    Float_t ampl, hits;
    Short_t lastdblcol = -1, lastaddr = -1;
    Int_t npt=0, npx=0, nnostart=0, nchisq=0, nmidpt=0;
    Float_t x[max_npt], y[max_npt];

    TF1 *ferf = new TF1("ferf", erf, 0., 1500., 3);
    ferf->FixParameter(0, n_inj);
    ferf->SetParLimits(1, 0.01, 1000);
    ferf->SetParLimits(2, 0.1, 100);
    ferf->SetParName(0, "Constant");
    ferf->SetParName(1, "Threshold");
    ferf->SetParName(2, "Noise");

#ifdef DUMP
    fpath_out = fileplots_prefix + "_dump.root";
    TFile *f_dump = new TFile(fpath_out.Data(), "RECREATE");
#endif

    //while ((fscanf (fp, "%d %d %d %f %f", &col, &address, &ampl, &hits, &err) == 5)) {
    while (file_raw >> dblcol >> addr >> ampl >> hits && file_raw.good()) {
        if ( dblcol % 16 == 0 && addr % 1024 == 0 && npt == 1)
            cout << macro_name << " : processing pixels in region " << dblcol/16 << endl;
        
        if ( ((lastdblcol != dblcol) || (lastaddr != addr)) && (npt != 0) ) {
            // create graph and get fit parameters
            Bool_t flagDraw = kTRUE;
            Float_t thrsh, noise, chisq=-1., prob;
            UShort_t col, row;
            dblcol_adr_to_col_row(dblcol, addr, col, row);
            Int_t sector = TMath::FloorNint(col / scols);

            findStartMeanSigma(npt, n_inj, x, y, &thrsh, &noise);

            if ( getNMidPoints(npt, n_inj, y) < 2 ) {
                nmidpt++;
                flagDraw = kFALSE;
                //cout << macro_name << " : WARNING: a pixel has insufficient points for good fit! SKIPPING!" << endl;
            }
            else if ( thrsh < 0 || noise < 0 ) {
                nnostart++;
                flagDraw = kFALSE;
                cout << macro_name << " : WARNING: a pixel has no starting point! SKIPPING!" << endl;
            }
            else {
                //addErrors(npt, n_inj, y, yerr);
                TGraphErrors *gr = new TGraphErrors(npt, x, y, 0, 0);
                ferf->SetParameter(1, thrsh);
                ferf->SetParameter(2, noise);
                gr->Fit(ferf, "Q");
                if(chisq_cut > 0 && chisq > chisq_cut) {
                    nchisq++;
                    flagDraw = kFALSE;
                    cout << macro_name << " : WARNING: a pixel has bad chi square value! SKIPPING!" << endl;
                }
                thrsh = ferf->GetParameter(1);
                noise = ferf->GetParameter(2);
                chisq = ferf->GetChisquare()/ferf->GetNDF();
                prob  = ferf->GetProb();
#ifdef DUMP
                if(noise > 2) flagDraw = kFALSE;
                else {
                    gr->SetNameTitle(Form("gr_%i-%i", col, row), Form("gr_%i-%i;Injected charge (electrons);Hits", col, row));
                    gr->Write();
                }
#endif
                delete gr;
            }

            // fill histograms
            if (flagDraw) {
                hThresh[sector]->Fill(thrsh);
                hNoise[sector]->Fill(noise);
                hTNR[sector]->Fill(thrsh/noise);
                hChisq->Fill(chisq);
                hProb->Fill(prob);
                hThreshPos->SetBinContent(col+1, row+1, thrsh);
                hNoisePos->SetBinContent(col+1, row+1, noise);
                hTNRPos->SetBinContent(col+1, row+1, thrsh/noise);
                hChisqPos->SetBinContent(col+1, row+1, chisq);
                hThreshChisq->Fill(chisq, thrsh);
                hNoiseChisq->Fill(chisq, noise);
            }

            npx++;
            npt=0;
        }
        
        Int_t sec = dblcol * 2 / scols;
        lastdblcol = dblcol;
        lastaddr = addr;
        x[npt] = ampl * eledac[sec];
        y[npt] = hits;
        npt++;
    }
    file_raw.close();
#ifdef DUMP
    f_dump->Close();
#endif

    // drawing
    cout << endl;
    cout << "Found " << npx << " pixel." << endl;
    cout << "No start point found: " << nnostart << endl;
    cout << "Insufficent points for fit: " << nmidpt << endl;
    cout << "Chisq cut failed:     " << nchisq << endl;
    cout << endl;
 
    TCanvas* c_thres = new TCanvas("c_thres", "Threshold", 0, 0, 1000, 700);
    c_thres->Divide(n_secs,1);
    for(Int_t i=0; i<n_secs; ++i) {
        c_thres->cd(i+1);
        hThresh[i]->SetTitle(Form("Sector %i, Threshold = %.1f", i, hThresh[i]->GetMean()));
        zoom_th1(hThresh[i]->DrawCopy());
    }

    TCanvas* c_noise = new TCanvas("c_noise", "Noise", 50, 50, 1000, 700);
    c_noise->Divide(n_secs,1);
    for(Int_t i=0; i<n_secs; ++i) {
        c_noise->cd(i+1);
        hNoise[i]->SetTitle(Form("Sector %i, Noise = %.1f", i, hNoise[i]->GetMean()));
        zoom_th1(hNoise[i]->DrawCopy());
    }

    TCanvas* c_sum = new TCanvas("c_sum", "Summary", 100, 100, 1000, 700);
    c_sum->Divide(2,2);
    c_sum->cd(1);
    THStack *hsThresh = new THStack("hsThresh", "Threshold distribution;Threshold (electrons);Num. of pixels");
    for(Int_t i=0; i<n_secs; ++i) hsThresh->Add(hThresh[i]);
    hsThresh->DrawClone("nostack");
    c_sum->GetPad(1)->BuildLegend();
    c_sum->cd(2);
    THStack *hsNoise = new THStack("hsNoise", "Noise distribution;Noise (electrons);Num. of pixels");
    for(Int_t i=0; i<n_secs; ++i) hsNoise->Add(hNoise[i]);
    hsNoise->DrawClone("nostack");
    c_sum->GetPad(2)->BuildLegend();
    c_sum->cd(3);
    c_sum->GetPad(3)->SetRightMargin(0.15);
    hThreshPos->DrawCopy("COLZ");
    c_sum->cd(4);
    c_sum->GetPad(4)->SetRightMargin(0.15);
    hNoisePos->DrawCopy("COLZ");

    TCanvas* c_ctrl = new TCanvas("c_ctrl", "Fit Control", 150, 150, 1000, 700);
    c_ctrl->Divide(2,2);
    c_ctrl->cd(1);
    zoom_th1(hChisq->DrawCopy());
    c_ctrl->cd(2);
    c_ctrl->GetPad(2)->SetRightMargin(0.15);
    hChisqPos->DrawCopy("COLZ");
    //hProb->DrawCopy();
    c_ctrl->cd(3);
    c_ctrl->GetPad(3)->SetRightMargin(0.15);
    hThreshChisq->DrawCopy("COLZ");
    c_ctrl->cd(4);
    c_ctrl->GetPad(4)->SetRightMargin(0.15);
    hNoiseChisq->DrawCopy("COLZ");

    if (saveCanvas) {
        fpath_out = fileplots_prefix + "_thresholds.pdf";
        c_thres->SaveAs(fpath_out.Data());
        fpath_out = fileplots_prefix + "_noise.pdf";
        c_noise->SaveAs(fpath_out.Data());
        fpath_out = fileplots_prefix + "_thrsh_summary.pdf";
        c_sum->SaveAs(fpath_out.Data());

        // save histograms also in root file
        fpath_out = fileplots_prefix + ".root";
        TFile *f_out = new TFile(fpath_out.Data(), "RECREATE");
        f_out->cd();

        // read run config file to create unique histogram names
        char fCfg[1024];
        string fNameCfg = filepath.Data();
        sprintf(fCfg, "%s.cfg", fNameCfg.substr(0,fNameCfg.length()-4).c_str());
        ifstream cfg_file(fCfg);
        if(!cfg_file.good() || !ITH || !VCASN) {
            std::cout << "Config file not found! Histogram naming not unique!" << std::endl;
        }
        else {
            // get info
            MeasConfig_t conf = read_config_file(fCfg);
            for(Int_t i=0; i<n_secs; ++i) {
                hThresh[i]->SetName(Form("h_thresholds_TEMP%i.0_VBB%2.1f_VCASN%i_ITHR%i_sec%i", conf.TEMP_SET, conf.VBB, conf.VCASN, conf.ITHR, i));
                hNoise[i]->SetName(Form("h_noise_TEMP%i.0_VBB%2.1f_VCASN%i_ITHR%i_sec%i", conf.TEMP_SET, conf.VBB, conf.VCASN, conf.ITHR, i));
            }
        }
        
        for(Int_t i=0; i<n_secs; ++i) hThresh[i]->Write();
        for(Int_t i=0; i<n_secs; ++i) hNoise[i]->Write();
        for(Int_t i=0; i<n_secs; ++i) hTNR[i]->Write();
        hThreshPos->Write();
        hNoisePos->Write();
        hTNRPos->Write();
        hChisq->Write();
        hProb->Write();
        hChisqPos->Write();
        hThreshChisq->Write();
        hNoiseChisq->Write();
        f_out->Close();
    }

    for(Int_t i=0; i<n_secs; ++i)
        cout << "Threshold sector " << i << ": " << hThresh[i]->GetMean() << " +- " << hThresh[i]->GetRMS() << endl;
    for(Int_t i=0; i<n_secs; ++i)
        cout << "Noise sector " << i << ": " << hNoise[i]->GetMean() << " +- " << hNoise[i]->GetRMS() << endl;
    

    if (WriteToFile) {
        TString fOutSum = fileplots_prefix + "_Summary.dat";
        FILE *fp = fopen(fOutSum.Data(), "w");
        for(Int_t i=0; i<n_secs; ++i)
            fprintf(fp, "0 %d %d %.1f %.1f %.1f %.1f\n", ITH, VCASN, hThresh[i]->GetMean(), hThresh[i]->GetRMS(), 
                    hNoise[i]->GetMean(), hNoise[i]->GetRMS());
        fclose(fp);
    }

    // cleanup
    for(Int_t i=0; i<n_secs; ++i) delete hThresh[i];
    for(Int_t i=0; i<n_secs; ++i) delete hNoise[i];
    for(Int_t i=0; i<n_secs; ++i) delete hTNR[i];
    delete hThreshPos;
    delete hNoisePos;
    delete hTNRPos;
    delete hChisq;
    delete hProb;
    delete hChisqPos;
    delete hThreshChisq;
    delete hNoiseChisq;
    delete ferf;
    
    return kTRUE;
}
