void
set_plot_style()
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


int AddressToColumn      (int ARegion, int ADoubleCol, int AAddress)
{
    int Column    = ARegion * 32 + ADoubleCol * 2;    // Double columns before ADoubleCol
    int LeftRight = ((((AAddress % 4) == 1) || ((AAddress % 4) == 2))? 1:0);       // Left or right column within the double column
    
    Column += LeftRight;
    
    return Column;
}


int AddressToRow         (int ARegion, int ADoubleCol, int AAddress)
{
    int Row = AAddress / 2;   
    return Row;
}


void ThresholdMap (const char *fName) {
  FILE *fp = fopen (fName, "r");
  set_plot_style();
  TH2F *hThresh = new TH2F ("hThresh", "Threshold Map", 1024, -0.5, 1023.5, 512, -0.5, 511.5);
  TProfile *hNoiseProf = new TProfile("hNoiseProf", "Noise Profile", 1024, -.5, 1023.5);

  TH1F *hThresh1 = new TH1F ("hThresh1", "Threshold", 200, 0, 400);
  int col, row;
  float thresh, noise, chisq;

  while (fscanf(fp, "%d %d %f %f %f", &col, &row, &thresh, &noise, &chisq) == 5) {
    int pixel = col * 1024 + row;
    if (!(pixel %10000)) cout << "processing pixel " << pixel << endl;

    if (thresh > 0) {
      int Column = AddressToColumn(col / 16, col % 16, row);
      int Row    = AddressToRow   (col / 16, col % 16, row);
      hThresh->Fill(Column, Row, thresh);

      hThresh1->Fill(thresh);
      // Fill Histogram;
    } 
  }

  gStyle->SetOptStat (kFALSE);
  gStyle->SetOptTitle(kFALSE);
  hThresh->GetXaxis()->SetTitle("Column");
  hThresh->GetYaxis()->SetTitle("Row");
  hThresh->GetZaxis()->SetTitle("Noise [e]");
  hThresh->GetZaxis()->SetTitleOffset(1.1);
  //hThresh->SetMaximum(20);
  hThresh->Draw("COLZ");
  //hThresh1->Draw();
  hNoiseProf->GetXaxis()->SetTitle("Double column");
  hNoiseProf->GetYaxis()->SetTitle("<Noise> [e]");

  //hNoiseProf->Draw();
} 
