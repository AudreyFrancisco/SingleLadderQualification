#include <iostream>
#include <string>

#include "TH1F.h"
#include "TH2F.h"
#include "TLine.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TColor.h"

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


void ReadFile (const char *fName, TH2F *hHitmap, int Chip, bool aNoise) {
  int   event, col, row;
  float thresh, noise, chisq;

  FILE *fp = fopen(fName, "r");

  if (!fp) {
    std::cout << "Unable to open file " << fName <<std::endl;
    return;
  }

  int nLines = 0, nHot = 0, nIneff = 0;

  float noiseSum = 0, threshSum = 0, threshSq = 0, noiseSq = 0;

  while (fscanf (fp,"%d %d %f %f %f", &col, &row, &thresh, &noise, &chisq) == 5) {
    int Column = AddressToColumn(col / 16, col % 16, row);
    int Row    = AddressToRow   (col / 16, col % 16, row);
    nLines ++;

    noiseSum  += noise;
    threshSum += thresh;
    noiseSq   += noise * noise;
    threshSq  += thresh * thresh;

    if (aNoise) {
      hHitmap->Fill(1024 * Chip + Column, Row, noise);
    }
    else {
      hHitmap->Fill(1024 * Chip + Column, Row, thresh);
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
  for (int i = 1; i < 9; i++) {
    TLine *line = new TLine (-.5 + i*1024,-.5,-.5 + i*1024, 511.5);
    line->Draw();
  }

  for (int i = 0; i < 9; i++) {
    char text[10];
    sprintf(text, "Chip %d", i);
    TLatex *label = new TLatex (200 + i*1024, 250, text);
    label->Draw();
  }
}


int ThresholdMapIB(const char *fName, bool noise = false) {

  std::string Prefix = fName;
  Prefix.erase(Prefix.rfind("_Chip"));
  std::cout << "Prefix: " << Prefix << std::endl;

  set_plot_style();
  TH2F *hHitmap = new TH2F("hHitmap", "Threshold map", 9216, -.5, 9215.5, 512, -.5, 511.5);

  char fNameChip[500];
  for (int ichip = 0; ichip < 9; ichip ++) {
    sprintf(fNameChip, "%s_Chip%d_0.dat", Prefix.c_str(), ichip);
    ReadFile(fNameChip, hHitmap, ichip, noise);
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
