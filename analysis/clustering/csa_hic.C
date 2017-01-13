/* Written by Miljenko Suljic, m.suljic@cern.ch */
#include "Riostream.h"
#include <TTree.h>
#include <TFile.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TMath.h>

#include "../classes/AliALPIDEModuleStreamMS.hpp"
#include "../classes/BinaryEvent.hpp"
#include "../classes/helpers.h"

#define MAX_CS 5000 // maximum pixels in an event
//#define DEBUG

using namespace std;

// check if <pix1> and <pix2> are neighbours in the <crown> cro
//_______________________________________________________________________________________________
Bool_t IsNeighbour(Short_t crown, BinaryPixel pix1, BinaryPixel pix2) {
    Short_t dcol = pix1.GetCol() - pix2.GetCol();
    Short_t drow = pix1.GetRow() - pix2.GetRow();
    if(-crown <= dcol && dcol <= crown) {
        if(-crown <= drow && drow <= crown) { return kTRUE; }
        else { return kFALSE; }
    }
    else { return kFALSE; }
}

// check if pixel <pix> is border pixel. <scols> and <srows> is number of columns, rows in sector
//_______________________________________________________________________________________________
Bool_t IsBorderPixel(BinaryPixel pix, Short_t scols, Short_t srows) {
    if( !(pix.GetRow() % srows) || !((pix.GetRow()+1) % srows) )
        return kTRUE;
    else if( !(pix.GetCol() % scols) || !((pix.GetCol()+1) % scols) )
        return kTRUE;
    else
        return kFALSE;
}


//_______________________________________________________________________________________________
Bool_t csa_hic(
    const TString filepath_raw,                      // path to NoiseOccupancy_*_*_Chip%i.dat files
    const TString filepath_tree,                     // output tree path
    const TString dirpath_mask="",                   // path to mask file
    const Short_t crown = 1,                         // consider neighbours all pixels in <crown> crown
    const Bool_t  flagRmSingleHotPixClusters = kTRUE // do not save clusters with just one pixels which is hot
    ) {

    cout << "csa_hic() : Starting..." << endl;

    // HIC constants
    const Short_t n_chips = 9;  // number of chips
    const Short_t scols = 1024; // number of columns in a chip
    const Short_t srows = 512;  // number of rows in a chip

    // raw file reader
    AliALPIDEModuleStreamMS *hic = new AliALPIDEModuleStreamMS(n_chips);
    if( !hic->SetInputFiles(filepath_raw.Data()) ) {
        cerr << "csa_hic() : Cannot open raw file!" << endl
             << "\tFile path: " << filepath_raw.Data() << endl;
        return kFALSE;
    }
    hic->SetVerboseLevel(2);

    // Event tree variables
    TFile* file_tree = new TFile(filepath_tree.Data(), "RECREATE");
    BinaryEvent* event = new BinaryEvent();
    TTree* tree = new TTree("event_tree", "event_tree"); 
    tree->Branch("event", "BinaryEvent", &event);
    
    // CSA variables
    BinaryPlane* plane[n_chips];
    for(Short_t i=0; i<n_chips; ++i) plane[i] = new BinaryPlane(); 
    BinaryCluster* cluster = new BinaryCluster();
    vector<BinaryPixel> pix_vec; pix_vec.reserve(MAX_CS);
    BinaryPixel*  pix_arr = new BinaryPixel[MAX_CS];
    BinaryPixel   pix_tmp;

    // statistics histograms
    TH1F *hNPixAll = new TH1F("hNPixAll", "Number of hit pixels per event;Number of hit pixels per event;Frequency",
                            1000, 0, 10000);
    Int_t n_hitpix[n_chips];
    TH1F *hNPix[n_chips];
    TH1F *hNClu[n_chips];
    for(Short_t i=0; i<n_chips; ++i) {
        hNPix[i] = new TH1F(Form("hNPix_%i", i), Form("Number of hit pixels per event, chip %i;Number of hit pixels per event;Frequency", i),
                            1000, 0, 10000);
        hNClu[i] = new TH1F(Form("hNClu_%i", i), Form("Number of clusters per event, chip %i;Number of clusters per event;Frequency", i),
                            1000, 0, 10000);
    }

    // TODO: make the code nicer
    // hot pixels variables
    Bool_t flagHot = dirpath_mask.EqualTo("") ? kFALSE : kTRUE;
    Bool_t hot_map[n_chips][scols][srows]; 
    if(flagHot) {
        for(Int_t ichip=0; ichip < n_chips; ++ichip) {
            for(Int_t i=0; i<scols; ++i) // initialise hotmap
                for(Int_t j=0; j<srows; ++j)
                    hot_map[ichip][i][j] = kFALSE;
            
            TString filename_mask = dirpath_mask; filename_mask += "mask_chip"; filename_mask += ichip; filename_mask += ".dat";
            ifstream file_mask(filename_mask.Data()); // read hot pixels from mask file
            if(!file_mask.is_open()) {
                cout << "csa_hic() : ERROR: Unable to open mask file: "
                     << filename_mask.Data() << endl;
                return kFALSE;
            }
            Short_t col, row, cnt=0;
            file_mask >> col >> row;
            while(file_mask.good()) {
                hot_map[ichip][col][row] = kTRUE;
                ++cnt;
                file_mask >> col >> row;
            }
            file_mask.close();
            cout << "csa_hic() : Successfully loaded " << cnt << " hot pixels from mask file! " << filename_mask << endl;
        }
    }
        
    //-------------------------------------------
    cout << "csa_hic() : Started reading RAW file." << endl;
    Long_t evts = 0;
    while(!hic->IsLastEvent()) {
        
        if(!hic->ReadEvent()) {
            cerr << "csa_hic() : FATAL: Error reading event" << endl;
            return kFALSE;
        }
        if(!hic->ProcessEvent()) {
            cerr << "csa_hic() : ERROR: Error processing event" << endl;
            return kFALSE;
        }
        
#ifdef DEBUG
        cout << "csa_hic() : Processed events: " << evts+1 << " / Trigger: " << hic->GetEventCounter() << endl;
#else
        if( (evts+1)%5000 == 0 )
            cout << "csa_hic() : Processed events: " << evts+1 << endl;
#endif
        event->Reset();
        event->SetEventID(evts);
        event->SetIntTrigCnt(hic->GetEventCounter());
        event->SetExtTrigCnt(hic->GetEventCounter());
        
        for(Short_t ichip=0; ichip<n_chips; ++ichip) {
            plane[ichip]->Reset();
            plane[ichip]->SetPlaneID(ichip);
            n_hitpix[ichip] = 0;

            Int_t   nhits = hic->GetNumHits(ichip);
            Int_t   j=0;
            Short_t col, row;
            Bool_t  flagPixAdded = kTRUE;
            hNPixAll->Fill(hic->GetNumHitsTotal());
            
#ifdef DEBUG
            int d_n_clu = 0;
            int d_get_hit_cnt = 0;
#endif

            // read which pixels are hit and re-construct first cluster
            cluster->Reset();
            while(hic->GetNextHit(ichip, &col, &row)) {
#ifdef DEBUG
                cout << "csa_hic() : DEBUG: Reading hit " << d_get_hit_cnt << " of expected " << nhits << ", col: " << col << " row: " << row
                     << " NClu = " << d_n_clu << " nhits = " << nhits << " j = " << j << endl;
                d_get_hit_cnt++;
#endif
                //if(nhits > 100) cout << nhits << " " << j << endl;
                n_hitpix[ichip]++;
                pix_tmp.Reset();
                pix_tmp.Set(col, row);
                // check if there are conditions that require setting a flag for this pixel
                if(flagHot) if(hot_map[ichip][col][row])        pix_tmp.SetFlag(0, kTRUE);
                if(IsBorderPixel(pix_tmp, scols, srows)) pix_tmp.SetFlag(1, kTRUE);
                // 1st cluster reconstruction
                if(j==0) {
                    pix_arr[j] = pix_tmp;
                    ++j; --nhits;
                }
                else {
                    Bool_t flagNeigh = kFALSE;
                    for(Int_t k=0; k<j && !flagNeigh; ++k) {
                        if(IsNeighbour(crown, pix_tmp, pix_arr[k])) {
                            pix_arr[j] = pix_tmp;
                            ++j; --nhits;
                            if(j >= MAX_CS) cerr << "csa_hic() : FATAL: cluster size > MAX cluster size" << endl;
                            flagNeigh = kTRUE;
                        }
                    }
                    if(!flagNeigh) pix_vec.push_back(pix_tmp);
                }
            }
            // some pixels excluded from cluster at the beginning of previous loop
            // might be neighbours with pixels included at the end
            flagPixAdded = kTRUE;
            while(flagPixAdded && nhits) {
                flagPixAdded = kFALSE;
                for(Int_t k=0; k<j; ++k) {
                    for(Int_t i=0; i < nhits; ++i) {
                        if(IsNeighbour(crown, pix_vec[i], pix_arr[k])) {
                            pix_arr[j] = pix_vec[i];
                            pix_vec.erase(pix_vec.begin()+i);
                            ++j; --nhits; --i;
                            if(j >= MAX_CS) cerr << "csa_hic() : FATAL: cluster size > MAX cluster size" << endl;
                            flagPixAdded = kTRUE;
                            break;
                        }
                    }
                }
            }
#ifdef DEBUG
            d_n_clu++;
            cout << "csa_hic() : DEBUG: NClu = " << d_n_clu << " nhits = " << nhits << " j = " << j << endl;
#endif
            if(j > 0) cluster->SetPixelArray(j, pix_arr);

            if(flagHot && flagRmSingleHotPixClusters
               && cluster->GetNPixels()==1 && cluster->HasHotPixels() )
                cluster->Reset();
            else if(cluster->GetMultiplicity() > 0)
                plane[ichip]->AddCluster(cluster);

//#ifdef DEBUG
//        cout << "Dump 1" << endl;
//        for(Int_t i=0; i < pix_vec.size(); ++i)
//            cout << i << " col: " << pix_vec[i].GetCol() << " row: " << pix_vec[i].GetRow() << endl;
//#endif
        
        // reconstruct other clusters

            while(nhits) {
                cluster->Reset();
                pix_arr[0] = pix_vec[0];
                pix_vec.erase(pix_vec.begin());
                j=1; --nhits;
                for(Int_t k=0; k<j; ++k) {
                    for(Int_t i=0; i < nhits; ++i) {
                        if(IsNeighbour(crown, pix_vec[i], pix_arr[k])) {
                            pix_arr[j] = pix_vec[i];
                            pix_vec.erase(pix_vec.begin()+i);
                            if(j >= MAX_CS) cerr << "csa_hic() : FATAL: cluster size > MAX cluster size" << endl;
                            ++j; --nhits; --i;
                        }
                    }
                }
                
                // some pixels excluded from cluster at the beginning of previous loop
                // might be neighbours with pixels included at the end
                flagPixAdded = kTRUE;
                while(flagPixAdded && nhits) {
                    flagPixAdded = kFALSE;
                    for(Int_t k=0; k<j; ++k) {
                        for(Int_t i=0; i < nhits; ++i) {
                            if(IsNeighbour(crown, pix_vec[i], pix_arr[k])) {
                                pix_arr[j] = pix_vec[i];
                                pix_vec.erase(pix_vec.begin()+i);
                                ++j; --nhits; --i;
                                if(j >= MAX_CS) cerr << "csa_hic() : FATAL: cluster size > MAX cluster size" << endl;
                                flagPixAdded = kTRUE;
                                break;
                            }
                        }
                    }
                }
#ifdef DEBUG
                d_n_clu++;
                cout << "csa_hic() : DEBUG: NClu = " << d_n_clu << " nhits = " << nhits << " j = " << j << endl;
#endif
                if(j > 0) cluster->SetPixelArray(j, pix_arr);
                
                if(flagHot && flagRmSingleHotPixClusters
                   && cluster->GetNPixels()==1 && cluster->HasHotPixels() )
                    cluster->Reset();
                else if(cluster->GetMultiplicity() > 0)
                    plane[ichip]->AddCluster(cluster);
            }

        } // end for planes
        
        // fill the tree (if at least one plane is hit) // do not check. fill everything
        //Bool_t flagFill = kFALSE;
        Bool_t flagFill = kTRUE;
        for(Short_t i=0; i<n_chips; ++i) {
            // stats hists
            hNClu[i]->Fill(plane[i]->GetNClustersSaved());
            hNPix[i]->Fill(n_hitpix[i]);
            // tree
            if(flagRmSingleHotPixClusters) // if this flag is set n_hitpix counter is different than number of saved pixels
                n_hitpix[i] = plane[i]->GetNHitPix(kTRUE);
            else {  // check that n_hitpix information is correct
                plane[i]->SetNHitPix(n_hitpix[i]);
                if(n_hitpix[i] != plane[i]->GetNHitPix(kTRUE))
                    cout << "csa_hic() : WARNING: n_hitpix information incorrect. Corrected!" << endl;
            }
            event->AddPlane(plane[i]);
            if(plane[i]->GetNClustersSaved()) flagFill = kTRUE;
        }
        if(flagFill) tree->Fill();
        ++evts;    
    } // End while ReadEvent()
    cout << "csa_hic() : Finished reading RAW file." << endl;

    //tree->Print();

    file_tree->Write();
    tree->ResetBranchAddresses();
    file_tree->Close();

    delete file_tree;
    delete hic;
    delete event;
    for(Short_t i=0; i<n_chips; ++i) delete plane[i];
    delete cluster;
    delete[] pix_arr;

    cout << "csa_hic() : Done!" << endl;
    return kTRUE;
}
