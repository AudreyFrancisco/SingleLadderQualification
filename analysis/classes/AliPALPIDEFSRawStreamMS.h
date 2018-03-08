/// Written by Miljenko Suljic, m.suljic@cern.ch
// Based on Valerio Altini's AliMIMOSARawStreamVA class

#ifndef ALIPALPIDEFSRAWSTREAMMS_H
#define ALIPALPIDEFSRAWSTREAMMS_H

#include "Riostream.h"
#include "TObject.h"

class AliPALPIDEFSRawStreamMS : public TObject {
public:
  AliPALPIDEFSRawStreamMS();
  ~AliPALPIDEFSRawStreamMS();

  Bool_t SetInputFile(const char *filename);
  Bool_t ReadEvent();

  Bool_t IsLastEvent() { return fLastEvent; }
  Int_t  GetEventCounter() { return fCurrentEvent; }
  Int_t  GetNumHits() { return fHitCols.size(); }
  Bool_t GetNextHit(Short_t *col, Short_t *row);
  Bool_t GetNextHit(Short_t *col, Short_t *row, Short_t *bunch);
  void   ResetHitIter() { fHitIter = 0; }
  Bool_t CheckDoubleHits(Bool_t remove);
  // Int_t  GetHitPixels(Short_t *col, Short_t* row);      // not working
  // void   GetHitAt(Int_t i, Short_t *col, Short_t* row); // not implemented

  void SetChipType(Short_t chiptype) { fChipType = chiptype; } // see fChipType
  void SetSkipErrorEvents(Bool_t toggle)
  {
    fSkipErrorEvents = toggle;
  } // see ReadEvent() comment, default = true

private:
  Bool_t dblcol_adr_to_col_row(Short_t doublecol, Short_t address, Short_t *col, Short_t *row,
                               Short_t chiptype = 4);

  ifstream fFileInput; // input text file
  Bool_t   fFirstEvent;
  Bool_t   fLastEvent;
  Int_t    fHitIter;
  Int_t    fEventCounter; // internal event counter (= event ID of next event)
  Int_t    fCurrentEvent; // current event ID

  Int_t  fChipType;        // chip type 0 = pALPIDE-1/fs, 4 = ALPIDE (default)
  Bool_t fSkipErrorEvents; // see ReadEvent() comment

  std::vector<Short_t> fHitCols;
  std::vector<Short_t> fHitRows;
  std::vector<Short_t> fHitBunch;

  ClassDef(AliPALPIDEFSRawStreamMS, 2)
};

#endif
