#include <iostream>

#include "TH1F.h"
#include "TH2F.h"
#include "TLine.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TColor.h"

/*ThresholdMap for mid-layer chips*/

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



int AddressToColumn(int ARegion, int ADoubleCol, int AAddress) { //probably doesn't need position
    int Column    = ARegion * 32 + ADoubleCol * 2;    // Double columns before ADoubleCol
    int LeftRight = ((AAddress % 4) < 2 ? 1:0);       // Left or right column within the double column
    
    Column += LeftRight;
    
    return Column;
}


int AddressToRow(int ARegion, int ADoubleCol, int AAddress)
{
    // Ok, this will get ugly
    int Row = AAddress / 2;                // This is OK for the top-right and the bottom-left pixel within a group of 4
    if ((AAddress % 4) == 3) Row -= 1;      // adjust the top-left pixel
    if ((AAddress % 4) == 0) Row += 1;      // adjust the bottom-right pixel
    return Row;
}


void ReadFile (const char *fName, TH2F *hHitmap, int Chip, bool aNoise) {
  int   event, col, row;  //position=new=upper or lower row
  float thresh, noise, chisq;

  /*if(Chip<7) {
    position=1;
  } else {
    position=0;
  } */

  FILE *fp = fopen(fName, "r");

  if (!fp) {
    std::cout << "Unable to open file " << fName <<std::endl;
    return;
  }

  int nLines = 0, nHot = 0, nIneff = 0;

  float noiseSum = 0, threshSum = 0, threshSq = 0, noiseSq = 0;

  while (fscanf (fp,"%d %d %f %f %f", &col, &row, &thresh, &noise, &chisq) == 5) {
    int Column = AddressToColumn(col / 16, col % 16, row); //probably doesn't need position...(below)
    int Row    = AddressToRow   (col / 16, col % 16, row);
    nLines ++;

    noiseSum  += noise; 
    threshSum += thresh;
    noiseSq   += noise * noise;
    threshSq  += thresh * thresh;

    if (aNoise) { //correct for second row now
      if(Chip<7) {
        hHitmap->Fill(1024 * Chip + Column, Row, noise);
      } else { //shift up and reverse order of rows
        hHitmap->Fill(1024 * (14-Chip) + Column, Row+512, noise);
      }
    }
    else {
      if(Chip<7) {
        hHitmap->Fill(1024 * Chip + Column, Row, thresh);
      } else {
        hHitmap->Fill(1024 * (14-Chip) + Column, Row+512, thresh);
      }
    }
  }

  if (nLines > 0) {
    float avNoise = noiseSum / nLines;
    float avThresh = threshSum / nLines;
    float rmsNoise = sqrt (noiseSq / nLines - pow(avNoise, 2));
    float rmsThresh = sqrt (threshSq / nLines - pow (avThresh, 2));
   
    std::cout << std::endl << "Chip " << Chip << ":" << std::endl;
    std::cout << "  Found " << nLines << " pixels, i.e. " << 512 * 1024 - nLines << " pixels without threshold" << std::endl;
    std::cout << "  Threshold: (" << avThresh << " +- " << rmsThresh << ") e." << std::endl;
    std::cout << "  Noise:     (" << avNoise << " +- " << rmsNoise << ") e." <<std::endl;
  }



  fclose(fp);

}


void AddLabels(){
  for (int i = 1; i < 7; i++) {
    TLine *line = new TLine (-.5 + i*1024,-.5,-.5 + i*1024, 2*512-.5); //lines now taller
    line->Draw();
  }
  
  TLine *horiz = new TLine(-.5,512-.5,7*1024-.5,512-.5);

  for (int i = 0; i < 7; i++) {
    char text[10];
    sprintf(text, "Chip %d", i);
    TLatex *label1 = new TLatex (200 + i*1024, 250, text);
    label1->Draw();
    //upper row added
    sprintf(text, "Chip %d", 14-(i)); //order reversed for top
    TLatex *label2 = new TLatex(200+i*1024, 250+512, text);
    label2->Draw();
  }
}


int All_ThresholdMap(const char *fName, bool noise = false) {
  //Meant to run on FitValue files!!
  char Prefix[100], fNameChip[100];

  int PrefixLength=strcspn(fName, "C"); //needs +"chipnum_rownum.dat"
  strncpy(Prefix, fName, PrefixLength-1);
  
  set_plot_style();
  //dimensions have been changed
  TH2F *hHitmap = new TH2F("hHitmap", "Threshold map", 1024*7, -.5, 1024*7-.5, 512*2, -.5, 512*2-.5);

  for (int ichip = 0; ichip < 7; ichip ++) {
    sprintf(fNameChip, "%s_Chip%d_%d.dat", Prefix, ichip, 1); //first (lower) row
    ReadFile(fNameChip, hHitmap, ichip, noise);
    sprintf(fNameChip, "%s_Chip%d_%d.dat", Prefix, ichip+8, 0); //second row
    ReadFile(fNameChip, hHitmap, ichip+8, noise);
  }

  gStyle->SetPalette (1);
  gStyle->SetOptStat (kFALSE);
  gStyle->SetOptTitle(kFALSE);

  hHitmap->GetXaxis()->SetTitle("Column");
  hHitmap->GetYaxis()->SetTitle("Row");
  if (noise) hHitmap->SetMaximum(20);
  hHitmap->Draw("COLZ");

  AddLabels();
  return 1;
}


