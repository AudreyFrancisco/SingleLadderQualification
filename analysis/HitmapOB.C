#include <iostream>

#include "TH2F.h"
#include "TStyle.h"
#include "TColor.h"
#include "TLine.h"
#include "TFile.h"
#include "TLatex.h"

//#include <string.h>
//#include <stdio.h>

/*Hitmap plotting for outer barrel*/

void set_plot_style()
{
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;

    Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);
}



int AddressToColumn(int ARegion, int ADoubleCol, int AAddress) {
    int Column    = ARegion * 32 + ADoubleCol * 2;    // Double columns before ADoubleCol
    int LeftRight = ((AAddress % 4) < 2 ? 1:0);       // Left or right column within the double column
    
    Column += LeftRight;
    
    return Column;
}


int AddressToRow         (int ARegion, int ADoubleCol, int AAddress)
{
    // Ok, this will get ugly
    int Row = AAddress / 2;                // This is OK for the top-right and the bottom-left pixel within a group of 4
    if ((AAddress % 4) == 3) Row -= 1;      // adjust the top-left pixel
    if ((AAddress % 4) == 0) Row += 1;      // adjust the bottom-right pixel
    return Row;
}


void ReadFile (const char *fName, TH2F *hHitmap, int Chip, int nInj) {
  int event, col, row, nhits;
  FILE *fp = fopen(fName, "r");
  if (!fp) {
    std::cout << "Unable to open file " << fName <<std::endl;
    return;
  }
  std::cout << fName << std::endl;
  int nLines = 0, nHot = 0, nIneff = 0;

  while (fscanf (fp,"%d %d %d", &col, &row, &nhits) == 3) {
    int Column = AddressToColumn(col / 16, col % 16, row);
    int Row    = AddressToRow   (col / 16, col % 16, row);
    nLines ++;
    if (nInj > 0) {
      if (nhits < nInj) nIneff ++;
      if (nhits > nInj) nHot   ++;
    }
    if(Chip<7) {
      hHitmap->Fill(1024 * Chip + Column, Row, nhits);
    } else {
      hHitmap->Fill(1024 * (14-Chip) + Column, Row+512, nhits);
    }
  }

  if (nInj > 0) {
    int nNoHit = 1024*512 - nLines; //May be incorrect //524288 - nLines;
    std::cout << std::endl;
    std::cout << std::endl << "Chip " << Chip << ":" << std::endl;
    std::cout << "  Pixels without hits: " << nNoHit << std::endl;
    std::cout << "  Pixels with <" << nInj << " hits: " << nIneff << std::endl;
    std::cout << "  Pixels with >" << nInj << " hits: " << nHot << std::endl;
    std::cout << std::endl;
  }
  fclose(fp);

}


void AddLabels(){
  for (int i = 1; i < 7; i++) {
    TLine *line = new TLine (-.5 + i*1024,-.5,-.5 + i*1024, 2*512-.5);
    line->Draw();
  }
  
  TLine *horiz = new TLine(-.5,512-.5,7*1024-.5,512-.5);
  horiz->Draw();
  
  for (int i = 0; i < 7; i++) {
    char text[10];
    sprintf(text, "Chip %d", i);
    TLatex *label = new TLatex (200 + i*1024, 250, text);
    label->Draw();
    //upper row
    sprintf(text, "Chip %d", 14-(i)); //order reversed for top
    TLatex *label2 = new TLatex(200+i*1024, 250+512, text);
    label2->Draw();
  }
}


int HitmapOB(const char *fName, int nInj = -1) {
  char Prefix[100], fNameChip[100];
  
  int PrefixLength = strcspn(fName, "C");
  strncpy (Prefix, fName, PrefixLength);

  std::cout << "Prefix = " << Prefix << std::endl;
  set_plot_style();

  //TFile *File=new TFile("map.C","UPDATE"); //added to prevent memory leaks
  
  TH2F *hHitmap = new TH2F("hHitmap", "Hit map", 7*1024, -.5, 7*1024-.5, 512*2, -.5, 512*2-.5);

  for (int ichip = 0; ichip < 7; ichip ++) {    
    sprintf(fNameChip, "%sChip%d_1.dat", Prefix, ichip);
    ReadFile(fNameChip, hHitmap, ichip, nInj);
    sprintf(fNameChip, "%sChip%d_0.dat", Prefix, ichip+8); //second row
    ReadFile(fNameChip, hHitmap, ichip+8, nInj);
  }

  gStyle->SetPalette (1);
  gStyle->SetOptStat (kFALSE);
  gStyle->SetOptTitle(kFALSE);

  if (nInj > 0) hHitmap->SetMaximum(nInj);

  hHitmap->GetXaxis()->SetTitle("Column");
  hHitmap->GetYaxis()->SetTitle("Row");
  hHitmap->Draw("COLZ");
  hHitmap->SetMaximum(51);
  AddLabels();
  //delete hHitmap;
  //gDirectory->WriteObject(hHitmap,"map");
  //File->Write();
  //File->Close();
  //delete File;

  return 1;
}


