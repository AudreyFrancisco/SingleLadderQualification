// make hitmap from raw file

#include <Riostream.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH2F.h>
#include <TString.h>

#include "helpers.h"
#include <cstddef>
#include <vector>

Float_t n_trg = 1e6;

//----------------------------------------------------------
int AddressToColumn(int ARegion, int ADoubleCol, int AAddress)
{
  int Column    = ARegion * 32 + ADoubleCol * 2; // Double columns before ADoubleCol
  int LeftRight = ((AAddress % 4) < 2 ? 1 : 0);  // Left or right column within the double column

  Column += LeftRight;

  return Column;
}


int AddressToRow(int ARegion, int ADoubleCol, int AAddress)
{
  // Ok, this will get ugly
  int Row =
      AAddress / 2; // This is OK for the top-right and the bottom-left pixel within a group of 4
  if ((AAddress % 4) == 3) Row -= 1; // adjust the top-left pixel
  if ((AAddress % 4) == 0) Row += 1; // adjust the bottom-right pixel
  return Row;
}


//-------------------------------------------------------------------------------------------
Bool_t OB_Test_Histo(TString file_path)
{
  char directory[100];
  strncpy(directory, file_path, 27);
  TFile *  fl = new TFile(Form("%shitmap_raw.root", directory), "RECREATE");
  ifstream raw_file(file_path.Data());
  if (!raw_file.good()) {
    cout << "RAW file not found!" << endl;
    return kFALSE;
  }
  UShort_t            event, chip, doublecol, dblcol, adr, col, row;
  UInt_t              nhits, modid, chipid;
  std::vector<TH2F *> hlist;
  //  TH2F *hHitmap[8]; //create array of histogramms
  for (int imod = 0; imod < 8; imod++) {
    TH2F *hHitmap = new TH2F(Form("module_%d", imod), Form("module hitmap %d", imod), 1024 * 7,
                             -0.5, 1024 * 7 - 0.5, 512 * 2, -0.5, 512 * 2 - 0.5);
    hlist.push_back(hHitmap);
  }
  while (raw_file >> event >> chip >> col >> row && raw_file.good()) { // read file
    int Column = AddressToColumn(col / 16, col % 16, row); // convert to real pixel cooedinates
    int Row    = AddressToRow(col / 16, col % 16, row);
    modid      = (chip >> 4) & 0x7; // take modid from chipid
    chipid     = chip & 0xf;        // cut to the normal chipid
    if (chipid <
        7) { // module histo should contain two row: 7 chips at the lower part and 7 above them
      cout << modid << "  " << chipid << " " << Column << " " << Row << " " << endl;
      hlist[modid]->Fill(1024 * chipid + Column, Row, event); // fill lower part
    }
    else {
      hlist[modid]->Fill(1024 * (14 - chipid) + (1024 - Column), Row + 512,
                         event); // fill upper part
      cout << chip << " " << Column << " " << Row << " " << endl;
    }
  }
  fl->Write();
  delete fl;
  return kTRUE;
}
