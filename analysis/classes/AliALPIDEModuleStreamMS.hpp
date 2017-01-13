/// Written by Miljenko Suljic, m.suljic@cern.ch

#ifndef ALIALPIDEMODULESTREAMMS_H
#define ALIALPIDEMODULESTREAMMS_H

#include "Riostream.h"
#include "TObject.h"
#include "TString.h"

#include "AliPALPIDEFSRawStreamMS.h"

class AliALPIDEModuleStreamMS: public TObject {
public:
    AliALPIDEModuleStreamMS(Int_t nchips);
    ~AliALPIDEModuleStreamMS();

    Bool_t SetInputFile(const char *filename, Int_t chipID);
    Bool_t SetInputFiles(const char *genfname);
    void   SetVerboseLevel(Short_t level) { fVerboseLevel = level; }
    
    Bool_t IsLastEvent();
    Int_t  GetEventCounter() { return fCurrentEvent; }
    Bool_t ReadEvent();
    Bool_t ProcessEvent(Bool_t silent=kTRUE);
    
    Int_t  GetNumHits(Short_t chip) { return fNHits[chip]; }
    Bool_t GetNextHit(Short_t chip, Short_t *col, Short_t* row);
    Int_t  GetNumHitsTotal();

private:
    Short_t   fVerboseLevel;
    
    AliPALPIDEFSRawStreamMS *fChip;
    
    Short_t   fNChips;
    Short_t   fNChipsInitialised;
    Short_t   *fChipID;
    
    Bool_t   fFirstEvent;
    Bool_t   fLastEvent;
    Bool_t   *fFinishedReading;  // no more events for chip
    Int_t    fCurrentEvent;      // current event ID

    Int_t    *fNHits; // number of good hits per chip in an event

    void      Report(Short_t level, const char * message);
    
    ClassDef(AliALPIDEModuleStreamMS,1)
};

#endif
