#include "AliALPIDEModuleStreamMS.hpp"

//__________________________________________________________
AliALPIDEModuleStreamMS::AliALPIDEModuleStreamMS(Int_t nchips) :
    fVerboseLevel(5),
    fNChips(nchips),
    fNChipsInitialised(0),
    fFirstEvent(1),
    fLastEvent(0),
    fCurrentEvent(0)
{
    // Construct
    fChip = new AliPALPIDEFSRawStreamMS[nchips];
    fChipID = new Short_t[nchips];
    fFinishedReading = new Bool_t[nchips];
    fNHits = new Int_t[nchips];
    for(Int_t i=0; i<fNChips; ++i) {
        fFinishedReading[i] = kFALSE;
        fNHits[i] = 0;
    }
    
}

//__________________________________________________________
AliALPIDEModuleStreamMS::~AliALPIDEModuleStreamMS()
{
    delete[] fChip;
    delete[] fChipID;
    delete[] fFinishedReading;
    delete[] fNHits;
}

//__________________________________________________________
void AliALPIDEModuleStreamMS::Report(Short_t level, const char * message)
{
    // report a message
    // FATAL   = level 0
    // ERROR   = level 1
    // WARNING = level 2
    // INFO    = level 3
    // DEBUG   = level 4
    if(level > fVerboseLevel) return;
    cout << "AliALPIDEModuleStreamMS : ";
    switch(level) {
    case 0 : cout << " FATAL  : "; break;
    case 1 : cout << " ERROR  : "; break;
    case 2 : cout << "WARNING : "; break;
    case 3 : cout << " INFO   : "; break;
    case 4 : cout << " DEBUG  : "; break;
    default: cout << " CUSTOM : ";
    }
    cout << message << endl;
}
    
//__________________________________________________________
Bool_t AliALPIDEModuleStreamMS::SetInputFile(const char *filename, Int_t chipID)
{
    // set input file for chipID
    if( fNChipsInitialised < fNChips ) {
        if ( fChip[fNChipsInitialised].SetInputFile(filename) ) {
            fChip[fNChipsInitialised].SetSkipErrorEvents(kFALSE);
            fChipID[fNChipsInitialised] = chipID;
            fNChipsInitialised++;
            Report(3, Form("%s set as chip %i", filename, chipID));
            return kTRUE;
        }
        else {
            Report(1, Form("Setting input file failed! %s", filename));
            return kFALSE;
        }
    }
    else {
        Report(1, "All chips already initalised!");
        return kFALSE;
    }
}

//__________________________________________________________
Bool_t AliALPIDEModuleStreamMS::SetInputFiles(const char *genfname)
{
    // if filenames for each chipId is e.g. RawData_1234_1212_Chip0..N.dat
    // you can use this method to set all files by passing "RawData_1234_1212_Chip%i.dat"
    Bool_t retval = kTRUE;
    for(Int_t i=0; i<fNChips; ++i)
        retval &= SetInputFile(Form(genfname, i), i);
    return retval;
}


//__________________________________________________________
Bool_t AliALPIDEModuleStreamMS::IsLastEvent()
{
    Bool_t retval = kTRUE;
    for(Int_t i=0; i<fNChips; ++i)
        retval &= fChip[i].IsLastEvent();
    return retval;
}

//__________________________________________________________
Bool_t AliALPIDEModuleStreamMS::ReadEvent()
{   
    if(fNChipsInitialised < fNChips) {
        Report(1, Form("Not all chips (%i/%i) are initialised! Not reading event!", fNChipsInitialised, fNChips));
        return kFALSE;
    }

    if(fFirstEvent) { // first time read all chips
        for(Int_t i=0; i<fNChips; ++i) {
            if(!fChip[i].ReadEvent()) {
                Report(0, Form("Problem reading event for chip %i", fChipID[i]));
                return kFALSE;
            }
        }
        Report(4, "First event read successfully.");
        fFirstEvent = kFALSE;
    }
    
    else { // for all the following events read only chips thaht have been processed 
        for(Int_t i=0; i<fNChips; ++i) {
            fNHits[i] = 0;
            if(fFinishedReading[i]) continue;
            if(fChip[i].IsLastEvent()) {
                fFinishedReading[i] = kTRUE;
                Report(4, Form("Finished reading chip %i", fChipID[i]));
            }
            else if(fChip[i].GetEventCounter() == fCurrentEvent) {
                if(!fChip[i].ReadEvent()) {
                    Report(0, Form("Problem reading event for chip %i", fChipID[i]));
                    return kFALSE;
                }
                Report(4, Form("Read event for chip %i", fChipID[i]));
            }
        }
    }

    // determine current event
    fCurrentEvent = 1e9; // this could be nicer
    for(Int_t i=0; i<fNChips; ++i)
        if(!fFinishedReading[i] && fChip[i].GetEventCounter() < fCurrentEvent)
            fCurrentEvent = fChip[i].GetEventCounter();
    
    Report(4, Form("Current event = %i", fCurrentEvent));

    if( fCurrentEvent < 1e9 ) {
        return kTRUE;
    }
    else {
        Report(3, "Finished reading all chips");
        return kFALSE;
    }
}

//__________________________________________________________
Bool_t AliALPIDEModuleStreamMS::ProcessEvent() {
    // check hit consistency and count number of hits
    Short_t col, row, bunch, refbunch = -999;
    for(Int_t i=0; i < fNChips; ++i) {
        if(fChip[i].GetEventCounter() != fCurrentEvent) continue; // check only this event

        if(fChip[i].CheckDoubleHits(kTRUE))
            Report(2, Form("Found twice hit pixel(s) and removed multiple(s), event %i, chipID %i", fCurrentEvent, fChipID[i]));
        
        while(fChip[i].GetNextHit(&col, &row, &bunch)) {
            if(bunch >=0) {
                if(refbunch == -999) refbunch = bunch;
                if(bunch != refbunch) {
                    Report(2, Form("Bunch counter changed within an event (%i), chipID %i", fCurrentEvent, fChipID[i]));
                    return kFALSE;
                }
                fNHits[i]++;
            }
            else if(bunch == -1)
                Report(2, Form("Found missing event (%i), chipID %i", fCurrentEvent, fChipID[i]));
            else if(bunch == -2)
                Report(2, Form("Found decoding problem (%i), chipID %i", fCurrentEvent, fChipID[i]));
        }
        fChip[i].ResetHitIter();
    }
    return kTRUE;
}

//__________________________________________________________
Bool_t AliALPIDEModuleStreamMS::GetNextHit(Short_t chip, Short_t *col, Short_t* row) {
    if(chip >= 0 && chip < fNChips) {
        if(fNHits[chip]) return fChip[chip].GetNextHit(col, row);
        else             return kFALSE;
    }
    else                 return kFALSE;
}

//__________________________________________________________
Int_t AliALPIDEModuleStreamMS::GetNumHitsTotal() {
    Int_t retval = 0;
    for(Int_t i=0; i<fNChips;++i) retval += fNHits[i];
    return retval;
}
