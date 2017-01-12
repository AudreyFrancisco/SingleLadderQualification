#include "Riostream.h"
#include "TString.h"

#include "../classes/AliALPIDEModuleStreamMS.hpp"

Bool_t create_masks_from_noiseocc_hic(
    //const TString filepath_raw = "/home/msuljic/cernbox/na61/data/NoiseOccupancy_161211_204329/NoiseOccupancy_161211_204329_Chip%i.dat",   // path to NoiseOccupancy_*_*_Chip%i.dat files
    const TString filepath_raw = "/home/msuljic/cernbox/na61/data/NoiseOccupancy_161212_092321/NoiseOccupancy_161212_092321_Chip%i.dat",   // path to NoiseOccupancy_*_*_Chip%i.dat files
    const TString dirpath_out  = "/home/msuljic/cernbox/na61/data/masks/",    // output dir for mask files
    const Int_t   n_hits_hot = 3  // number of hits to consider a pixel hot
    ) {

    const Short_t n_chips = 9;  // number of chips

    AliALPIDEModuleStreamMS *hic = new AliALPIDEModuleStreamMS(n_chips);
    if( !hic->SetInputFiles(filepath_raw.Data()) ) {
        cerr << "Cannot open raw file!" << endl
             << "\tFile path: " << filepath_raw.Data() << endl;
        return kFALSE;
    }
    hic->SetVerboseLevel(2);

    vector<Short_t> pix_col[n_chips];
    vector<Short_t> pix_row[n_chips];
    vector<Int_t> pix_hit[n_chips];
    
    Long_t evts = 0;
    while(!hic->IsLastEvent()) {
        
        if(!hic->ReadEvent()) {
            cerr << "FATAL: Error reading event" << endl;
            return kFALSE;
        }
        if(!hic->ProcessEvent()) {
            cerr << "FATAL: Error processing event" << endl;
            return kFALSE;
        }
        
        if( (evts+1)%100000 == 0 )
            cout << "Processed events: " << evts+1 << endl;

        Short_t col, row;
        
        for(Short_t ichip=0; ichip<n_chips; ++ichip) {
            while(hic->GetNextHit(ichip, &col, &row)) {

                Bool_t flagPixInList = kFALSE;
                for(UInt_t i=0; i<pix_col[ichip].size(); ++i) {
                    if(pix_col[ichip][i] == col) {
                        if(pix_row[ichip][i] == row) {
                            pix_hit[ichip][i]++;
                            flagPixInList = kTRUE;
                            break;
                        }
                    }
                }
                if(!flagPixInList) {
                    pix_col[ichip].push_back(col);
                    pix_row[ichip].push_back(row);
                    pix_hit[ichip].push_back(1);
                }
            } // end while next hit
        } // end for chip
        

        ++evts;
    } // end while IsLastEvent

    

    for(Short_t ichip=0; ichip<n_chips; ++ichip) {
        cout << "In chip " << ichip << " found " << pix_hit[ichip].size() << " hits\t";
        for(UInt_t i=0; i < pix_hit[ichip].size(); ++i) {
            if(pix_hit[ichip][i] < n_hits_hot) {
                pix_col[ichip].erase(pix_col[ichip].begin()+i);
                pix_row[ichip].erase(pix_row[ichip].begin()+i);
                pix_hit[ichip].erase(pix_hit[ichip].begin()+i);
                --i;
            }
        }
        cout << "of which " << pix_hit[ichip].size() << " are hot.\t";

        TString fname = dirpath_out; fname += "mask_chip"; fname += ichip; fname += ".dat";
        ofstream file_mask;
        file_mask.open(fname.Data());
        if(!file_mask.is_open()) {
            cout << "ERROR : Cannot open mask file " << fname << endl;
            continue;
        }
        for(UInt_t i=0; i < pix_hit[ichip].size(); ++i)
            file_mask << pix_col[ichip][i] << " " << pix_row[ichip][i] << endl;
        
        file_mask.close();
        cout << "Hot pixels written to file " << fname << endl;
        
    }
    
    delete hic;
    
    return kTRUE;
}
