#include "AliPALPIDEFSRawStreamMS.h"

//__________________________________________________________
AliPALPIDEFSRawStreamMS::AliPALPIDEFSRawStreamMS()
    : fEventCounter(0), fFirstEvent(1), fLastEvent(0), fHitIter(0), fChipType(4), // default ALPIDE
      fSkipErrorEvents(1) // default skip error events (row or col < 0)
{
  // Construct
  fHitCols.reserve(100);
  fHitRows.reserve(100);
  fHitBunch.reserve(100);
}

//__________________________________________________________
AliPALPIDEFSRawStreamMS::~AliPALPIDEFSRawStreamMS()
{
  fFileInput.close();
  fHitCols.clear();
  fHitRows.clear();
  fHitBunch.clear();
}
//__________________________________________________________
Bool_t AliPALPIDEFSRawStreamMS::SetInputFile(const char *filename)
{
  fFileInput.open(filename);
  return fFileInput.is_open();
}

//__________________________________________________________
Bool_t AliPALPIDEFSRawStreamMS::ReadEvent()
{
  // if fSkipErrorEvents = true, events with pixels with dcol/addr/hits < 0 will be ignored
  // if fSkipErrorEvents = false, pixel hit with col = dcol (<0) / row = addr (<0) will be
  // propagated

  Int_t   evt;
  Short_t col, row;
  Short_t dcol, addr;
  Short_t bunch;

  fHitIter = 0;
  fHitCols.clear();
  fHitRows.clear();
  fHitBunch.clear();

  if (!fFileInput.good()) {
    if (!IsLastEvent())
      cerr << "AliPALPIDEFSRawStreamMS::ReadEvent() : Error Input File : 1" << endl;
    return kFALSE;
  }

  if (fFirstEvent) {
    fFileInput >> evt;
    fEventCounter = evt;
    fFirstEvent   = kFALSE;
  }
  else {
    evt = fEventCounter;
  }
  fCurrentEvent = fEventCounter;

  while (fFileInput.good() && evt == fEventCounter) {
    fFileInput >> dcol >> addr >> bunch;
    if ((dcol < 0 || addr < 0 || bunch < 0) && fSkipErrorEvents) {
      fFileInput >> evt;
      fEventCounter = evt;
      continue;
    }
    else {
      dblcol_adr_to_col_row(dcol, addr, &col, &row, fChipType);
      fHitCols.push_back(col);
      fHitRows.push_back(row);
      fHitBunch.push_back(bunch);
    }
    if (!fFileInput.good()) {
      cerr << "AliPALPIDEFSRawStreamMS::ReadEvent() : Error Input File : 2" << endl;
      return kFALSE;
    }
    fFileInput >> evt;
  }

  if (!fFileInput.good()) fLastEvent = kTRUE;

  fEventCounter = evt;

  return kTRUE;
}

//__________________________________________________________
Bool_t AliPALPIDEFSRawStreamMS::GetNextHit(Short_t *col, Short_t *row)
{
  if (fHitIter < GetNumHits()) {
    *col = fHitCols.at(fHitIter);
    *row = fHitRows.at(fHitIter);
    ++fHitIter;
    return kTRUE;
  }
  else {
    *col = -1;
    *row = -1;
    return kFALSE;
  }
}

//__________________________________________________________
Bool_t AliPALPIDEFSRawStreamMS::GetNextHit(Short_t *col, Short_t *row, Short_t *bunch)
{
  if (fHitIter < GetNumHits()) {
    *col   = fHitCols.at(fHitIter);
    *row   = fHitRows.at(fHitIter);
    *bunch = fHitBunch.at(fHitIter);
    ++fHitIter;
    return kTRUE;
  }
  else {
    *col   = -99;
    *row   = -99;
    *bunch = -99;
    return kFALSE;
  }
}

//__________________________________________________________
Bool_t AliPALPIDEFSRawStreamMS::CheckDoubleHits(Bool_t remove)
{
  // check if there are pixels that show up multiple times
  Bool_t doublehits = kFALSE;
  for (Int_t i = 0; i < GetNumHits(); ++i)
    for (Int_t j = i + 1; j < GetNumHits(); ++j)
      if (fHitCols.at(i) == fHitCols.at(j))
        if (fHitRows.at(i) == fHitRows.at(j)) {
          doublehits = kTRUE;
          if (remove) {
            fHitCols.erase(fHitCols.begin() + j);
            fHitRows.erase(fHitRows.begin() + j);
            fHitBunch.erase(fHitBunch.begin() + j);
            --j;
          }
        }
  return doublehits;
}

//__________________________________________________________
// Int_t AliPALPIDEFSRawStreamMS::GetHitPixels(Short_t *col, Short_t* row) {
//    // not working
//    col = &fHitCols[0];
//    row = &fHitRows[0];
//    return fHitCols.size();
//}

Bool_t AliPALPIDEFSRawStreamMS::dblcol_adr_to_col_row(Short_t doublecol, Short_t address,
                                                      Short_t *col, Short_t *row, Short_t chiptype)
{
  if (doublecol < 0 || address < 0) {
    *col = doublecol;
    *row = address;
    return kFALSE;
  }
  if (chiptype < 3) { // pALPIDE-1/2
    *col = doublecol * 2 + (address % 4 < 2 ? 1 : 0);
    *row = 2 * (address / 4) + 1 - (address % 2);
  }
  else { // pALPIDE-3 / ALPIDE
    *col = doublecol * 2;
    *col += ((((address % 4) == 1) || ((address % 4) == 2)) ? 1 : 0);
    *row = address / 2;
  }
  return kTRUE;
}
