/// Developer: Valerio Altini  Contact: valerio.altini@cern.ch  ///
//  Modified for palpide by Miljenko Suljic, m.suljic@cern.ch
//  Modified for ALPIDE by Jacobus van Hoorne, jvanhoor@cern.ch
//  Modified Paolo Martinengo

#include <iostream>
#include <string>
using namespace std;
#include <sstream>
#include <TH2.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TColor.h>
#include <TMath.h>
#include <TROOT.h>
#include <TPad.h>
#include <TFile.h>
#include <TSystem.h>
#include <TGClient.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLayout.h>
#include <TGWindow.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TApplication.h>
#include <TGTextEntry.h>
#include <TGComboBox.h>
#include <TRootCanvas.h>
#include <TPad.h>
#include <TRootEmbeddedCanvas.h>
#include <../classes/AliPALPIDEFSRawStreamMS.h>
//#include <AliPALPIDEFSRawStreamMS.h>

#define BINRED 1
//#define ZOOM

class Monitoring {

protected:

    virtual ~Monitoring();

private:

    Int_t totevt;
    Bool_t fStop;
    char *fFile;
    vector<string> fsFiles;
    Bool_t flagInitHistos;	

    TCanvas *c1; //single event
    TCanvas *c2; //summary canvas
    TCanvas *c4;
    TCanvas *c3; //summary canvas consisting full dev
    TCanvas *c5;
    TCanvas *c6;
    TCanvas *c7;
    TCanvas *c8;
    TPad *pad;   //same as above
    TPad *padMul;

    AliPALPIDEFSRawStreamMS *palpidefs;
    vector<AliPALPIDEFSRawStreamMS*> palpidefsdev;

    TGMainFrame *fMain;
    //TGMainFrame *PedM;
    TGGroupFrame *fGframe;  
    TGNumberEntry *fNumber; 
    TGLabel *fLabel;
    TGGroupFrame *fGframe2;
    TGNumberEntry *fNumber2;
    TGLabel *fLabel2;
    TGTextEntry *tentry;
    // Histo to fill
    TH2F *map;
    TH2F *maptot;
    TH2F *mapall;
    TH2F *MulCorr;
    TH2F *MulCorr2;
    TH1F *TotMul;
    vector<TH2F*> mapdev;
    vector<TH1F*> mul;

    // Functions
  
    Bool_t Init();
    Bool_t InitHistos();
    Bool_t ProcessSingleEvent();
    Bool_t DrawSE();
    Bool_t ProcessSingleEventFast();
    Bool_t FinishEventFast();

public:

    void   SetFile(char *filename="data.txt"){fFile = filename;};
    Bool_t Terminate();
    Int_t  Run();
    Bool_t NextEvt();
    Bool_t End();
    Bool_t EndSilent();
    Bool_t ResetAll();
    Bool_t AutoUpdate();
    void   Stop();
    void   DisplayInt();
    void   DisplayFloat();
    void   MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h);
  
    ClassDef(Monitoring,0)

};
//_______________________________________________________________
Monitoring::~Monitoring() {

    delete palpidefs;
    fMain->Cleanup();
    // PedM->Cleanup();
    delete fMain;
    //delete PedM;
    delete fGframe;
    delete fNumber;
    delete fGframe2;
    delete fNumber2;
    delete fLabel;
    delete tentry;
    delete c1;
    delete c2;
    delete c4;
    delete c3;
    delete c5;
    delete c6;
    delete c7;
    delete c8;
    delete pad;
    delete map;
    delete padMul;
    delete maptot;
    delete mapall;
    for (int i = 0; i < 9; i++) {
      delete mapdev[i];
      delete mul[i];
    }

}
//_______________________________________________________________
Bool_t Monitoring::Init(){

    printf("\n\n"
           "************* MONITORING AND OFFLINE DATA ANALYSIS GUI *************\n"
           "*************  CURRENT VERSION v0.1 February 23, 2015  *************\n"
           "************* for any request contact m.suljic@cern.ch *************\n"
           "\n\n");

    gStyle->SetOptStat(111111);

    totevt=0;
    fStop = kFALSE;
    flagInitHistos=kFALSE;

    string sFile = fFile;
    string tempFileBase = sFile.substr(0, sFile.find(".dat")-1);

    for (int i = 0; i < 9; i++) {
      string tempFileName = Form("%s%d.dat",tempFileBase.c_str(), i);
      fsFiles.push_back(tempFileName);
    }

    palpidefs = new AliPALPIDEFSRawStreamMS();
    palpidefs->SetInputFile(sFile.c_str());

    for (int i = 0; i < 9; i++) {
      palpidefsdev.push_back(new AliPALPIDEFSRawStreamMS());
      palpidefsdev[i]->SetInputFile(fsFiles[i].c_str());
      cout << fsFiles[i] << endl;
    }

    MyMainFrame(gClient->GetRoot(),200,200);
 
    return kTRUE;

}

//_______________________________________________________________
Bool_t Monitoring::InitHistos(){

    /// Histo part
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;
    Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);

    TRootCanvas *c1i;
    TRootCanvas *c2i;
    TRootCanvas *c4i;
    TRootCanvas *c3i;
    TRootCanvas *c5i;
    TRootCanvas *p3i;
    TRootCanvas *p5i;
    TRootCanvas *c6i;
    TRootCanvas *p6i;
    TRootCanvas *c7i;
    TRootCanvas *p7i;
    TRootCanvas *c8i;
    TRootCanvas *p8i;

    c1 = new TCanvas("pALPIDEfsSnglEvt","pALPIDEfs Single Event",1200,600);
    c1i=(TRootCanvas*)c1->GetCanvasImp();
    c1i->DontCallClose();

    c2 = new TCanvas("pALPIDEfsAllEvt","pALPIDEfs Cumulative Events",1200,600);
    c2i=(TRootCanvas*)c2->GetCanvasImp();
    c2i->DontCallClose();

    c4 = new TCanvas("pALPIDEfsAllEvt_IBHIC","ALPIDE IB HIC",1900,300);
    c4i=(TRootCanvas*)c4->GetCanvasImp();
    c4i->DontCallClose();

    c6 = new TCanvas("EventPixelMultiplicity","Event pixel multiplicity",1000,1000);
    c6i=(TRootCanvas*)c6->GetCanvasImp();
    c6i->DontCallClose();

    c7 = new TCanvas("Multiplicity correlation 3 vs. 4","ALPIDE #3 vs ALPIDE #4",900,900);
    c7i=(TRootCanvas*)c7->GetCanvasImp();
    c7i->DontCallClose();

    c8 = new TCanvas("Multiplicity correlation 3 vs. 2","ALPIDE #3 vs ALPIDE #5",900,900);
    c8i=(TRootCanvas*)c8->GetCanvasImp();
    c8i->DontCallClose();

    c3 = new TCanvas("pALPIDEfsAllEvt_AllDev","pALPIDEfs Cumulative Events2",1900,300);
    c3->cd();
    pad = new TPad("pALPIDEfsAlldevPad", "pALPIDEfsAlldevPad", 0, 0, 1, 1);
    pad->Draw();
    pad->Divide(9,1, 0, 0);
    pad->Draw();
    p3i=(TRootCanvas*)pad->GetCanvasImp();
    p3i->DontCallClose();
    c3i=(TRootCanvas*)c3->GetCanvasImp();
    c3i->DontCallClose();   
  
    c5 = new TCanvas("ALPIDE IB HIC","ALPIDE IB HIC pixel multiplicity",900,900);
    c5->cd();
    padMul = new TPad("ALPIDE IB HIC pad","ALPIDE IB HIC hit multiplicity pad", 0, 0, 1, 1);
    padMul->Draw();
    padMul->Divide(3, 3, 0.0001, 0.0001);
    padMul->Draw();
    p5i=(TRootCanvas*)padMul->GetCanvasImp();
    p5i->DontCallClose();
    c5i=(TRootCanvas*)c5->GetCanvasImp();
    c5i->DontCallClose();

    map = new TH2F("snglevt", "pALPIDEfs Event;Column;Row", 1024/BINRED, -0.5, 1023.5, 512/BINRED, -0.5, 512);
    maptot = new TH2F("allevt", "pALPIDEfs Multiple Events;Column;Row", 1024/BINRED, -0.5, 1023.5, 512/BINRED, -0.5, 512);
    for (int i = 0; i < 9; i++) {
      mapdev.push_back(new TH2F(Form("allevt%d",i), Form( "AllEvent %d;Column;Row",i), 1024/BINRED, -0.5, 1023.5, 512/BINRED, -0.5, 511.5)); 
      mapdev[i]->SetStats(0);
      mul.push_back(new TH1F(Form("Pixel mult. chip #%d",i),Form("Pixel/event chip #%d;# of pixel;Entries",i),100/BINRED,-0.5,99.5));
    }
    mapall = new TH2F("allevtsingle", "IB HIC Multiple Events;Column;Row", 9*1024/BINRED, -0.5, 9*1024-0.5, 512/BINRED, -0.5, 511.5);
    TotMul = new TH1F(Form("Pixel_Event_Multiplicity"),Form("Total pixel multiplicity/event;# of pixel;Entries"),500/BINRED,-0.5,499.5);
    MulCorr = new TH2F("Corr", "ALPIDE #3 vs. ALPIDE #4;Pixels;Pixels", 300/BINRED, -0.5, 299.5, 300/BINRED, -0.5, 299.5);
    MulCorr2 = new TH2F("Corr2", "ALPIDE #3 vs. ALPIDE #5;Pixels;Pixels", 200/BINRED, -0.5, 199.5, 200/BINRED, -0.5, 199.5);

    map->SetStats(0);
    maptot->SetStats(0);
    mapall->SetStats(0);
    MulCorr->SetStats(0);
    MulCorr2->SetStats(0);

// End histo part
    return kTRUE;
}

//_______________________________________________________________
Bool_t Monitoring::ProcessSingleEvent(){

//    int mTotMul = 0;

    gSystem->ProcessEvents();

    if(fStop)return kFALSE;

    ++totevt;  // no effect for eth, fundamental for DATE monitoring

    UInt_t evt = palpidefs->GetEventCounter();

    //*****************************************************
    bool singlereadtrue = kTRUE;
    if(!palpidefs->ReadEvent()) {
        cout << "Monitoring::problem Reading event (single chip):" 
             << palpidefs->GetEventCounter() << endl;
        singlereadtrue = kFALSE;
    }
    bool ireadfalse = kFALSE;
    for (int ichips = 0; ichips < 9; ichips++) {
      if (!palpidefsdev[ichips]->ReadEvent()) {
        cout << "Monitoring::problem Reading Event (chip " << ichips << ") " << palpidefsdev[ichips]->GetEventCounter() << endl;
        continue;
      }
      else
        ireadfalse = ireadfalse || kTRUE;
    }
    if (!ireadfalse && flagInitHistos) return kFALSE;

    if(!flagInitHistos){
        cout << "evt :" << evt << endl;
        if(!InitHistos()){
            cout << "Problem in initializing histograms" << endl;
            return kFALSE;
        }
        flagInitHistos=kTRUE;
    }

    map->Reset();
    map->SetTitle(Form("pALPIDEfs Event %i", totevt));
/*
    maptot->Scale(totevt-1.);
    maptot->Scale(1./totevt);
*/
    Short_t col;
    Short_t row;

    if(singlereadtrue) {
        for(Int_t ihit=0; palpidefs->GetNextHit(&col, &row); ++ihit) {
            map->Fill(col, row);
            maptot->Fill(col, row);
        }
    }

    for (int ichips = 0; ichips < 9; ichips++) {
      for (int ihit = 0; palpidefsdev[ichips]->GetNextHit(&col, &row); ++ihit) {
//        cout << ichips << " " << col << " "  << row << endl;
          if(col >= 0 && row >= 0 && col < 1024 && row < 512) {
              mapdev[ichips]->Fill(col, row);
              mapall->Fill(ichips*1024+col, row);
//              mTotMul++;
          }
      }
    }
    // cout << TotMul << endl;
    //TotMul->Fill(mTotMul);

    return kTRUE; 

}
//_______________________________________________________________
Bool_t Monitoring::FinishEventFast(){

    map->Reset();

    return kTRUE;

}
//--------------------------------------------------------------
Bool_t Monitoring::ResetAll(){
  map->Reset();
  maptot->Reset();
  mapall->Reset();
  for (int ichips = 0; ichips < 9; ichips++) {
    mapdev[ichips]->Reset();
  }
  return kTRUE;
}

//_______________________________________________________________
Bool_t Monitoring::ProcessSingleEventFast(){

    int mTotMul= 0;
    int chipCnt[9] = {0,0,0,0,0,0,0,0,0};
    int cnt = 0;
    if(fStop)return kFALSE;

    ++totevt; // no effect for eth, foundamentl for DATE monitoring
    UInt_t evt = palpidefs->GetEventCounter();

    //*****************************************************
    bool singlereadtrue = kTRUE;
    if(!palpidefs->ReadEvent()) {
        cout << "Monitoring::problem Reading event (single chip):" 
             << palpidefs->GetEventCounter() << endl;
        singlereadtrue = kFALSE;
    }
    
    bool ireadfalse = kFALSE;
    for (int ichips = 0; ichips < 9; ichips++) {
      if (!palpidefsdev[ichips]->ReadEvent()) {
	cout << "Monitoring::problem Reading Event (chip " << ichips << ") " << palpidefsdev[ichips]->GetEventCounter() << endl;
        continue;
      }
      else
	ireadfalse = ireadfalse || kTRUE;
    }
    if (!ireadfalse && flagInitHistos) return kFALSE;

    if(!flagInitHistos){
        cout << "evt :" << evt << endl;
        if(!InitHistos()){
            cout << "Problem in initializing histograms" << endl;
            return kFALSE;
        }
        flagInitHistos=kTRUE;
    }

    Short_t col, row;

    if(singlereadtrue) {
        for(Int_t ihit=0; palpidefs->GetNextHit(&col, &row); ++ihit) {
            //if(ihit==0 && col%32!=0 && row!=0) break;
            maptot->Fill(col, row);
        }
    }

    for (int ichips = 0; ichips < 9; ichips++) {
//      cout << ichips << endl;
      for (int ihit = 0; palpidefsdev[ichips]->GetNextHit(&col, &row); ++ihit) {
//        cout << ichips << " " << col << " "  << row << endl;
          if(col >= 0 && row >= 0 && col < 1024 && row < 512) {
              mapdev[ichips]->Fill(col, row);
              mapall->Fill(ichips*1024+col, row);
              mTotMul++;cnt++;
          }
      }
          mul[ichips]->Fill(cnt);
          chipCnt[ichips] = cnt;
          cnt = 0;
    }
    
    TotMul->Fill(mTotMul);
    MulCorr->Fill(chipCnt[4], chipCnt[3]);
    MulCorr2->Fill(chipCnt[3],chipCnt[5]);
    return kTRUE;

}
//_______________________________________________________________
Bool_t Monitoring::Terminate(){
    cout << "unused" << endl;
    // used fo saving plots before exiting
    return kTRUE;

}
//_______________________________________________________________
Bool_t Monitoring::DrawSE(){

    c1->cd();
    map->DrawCopy("COLZ");
    c1->Update();


    c2->cd();
    maptot->DrawCopy("COLZ");
#ifdef ZOOM
    maptot->GetZaxis()->SetRangeUser(0, 50);
#endif
    c2->Update();

    c4->cd();
    mapall->DrawCopy("COLZ");
#ifdef ZOOM
    //mapall->GetZaxis()->SetRangeUser(0, 50);
#endif
    c4->Update();

  
    c3->cd();
    for (int ichips = 0; ichips < 9; ichips++) {
      pad->cd(ichips+1);
      mapdev[ichips]->DrawCopy("COLZ");
#ifdef ZOOM
    mapdev[ichips]->GetZaxis()->SetRangeUser(0, 50);
#endif
//      cout << "ho " << endl;
      pad->Update();
    }

    c5->cd();
    for (int ichips = 0; ichips < 9; ichips++) {
      padMul->cd(ichips+1);
      mul[ichips]->DrawCopy("");
      padMul->Update();
    }


    c6->cd();
    TotMul->DrawCopy("");
    c6->Update();

    c7->cd();
    MulCorr->DrawCopy("COLZ");
    c7->Update();

    c8->cd();
    MulCorr2->DrawCopy("COLZ");
    c8->Update();

    return kTRUE;
  
}
//_______________________________________________________________
Int_t Monitoring::Run(){

    if(!Init()) cout<<"Init procedure failure"<<endl;
    ProcessSingleEvent();
    DrawSE();

    return 0;

}
//_______________________________________________________________
Bool_t Monitoring::NextEvt(){
    fStop = kFALSE;
    if(!ProcessSingleEvent()) Terminate();
    DrawSE();

    return kTRUE;
}
//_______________________________________________________________
Bool_t Monitoring::End(){

    fStop = kFALSE;
    while(ProcessSingleEvent()){
        DrawSE();
    }
    if(!fStop)Terminate();

    return kTRUE;

}
//_______________________________________________________________
Bool_t Monitoring::EndSilent(){

    fStop = kFALSE;
    while(ProcessSingleEventFast());
    cout << "after processing events" << endl;
    FinishEventFast();
    cout << "after filling histos" << endl;
    DrawSE();
    cout << "after draw" << endl;
    Terminate();
    cout << "after terminate" << endl;

    return kTRUE;

}
Bool_t Monitoring::AutoUpdate() {

    while (1 && fStop == kFALSE) {
      while(ProcessSingleEventFast());
      cout << "Processed Fast event reading" << endl;
      ResetAll();
      cout << "Processed Fast Histogram Reset" << endl;
      DrawSE();
      Terminate();
      
//      delete palpidefs;
//      palpidefs = 0x0;

//      palpidefs = new AliPALPIDEFSRawStreamMS();
      palpidefs->SetInputFile(fsFiles[0].c_str());
/*
      for (int i = 0; i < 9; i++) {
        delete palpidefsdev[i];
      }*/
//      palpidefsdev.clear();
      for (int i = 0; i < 9; i++) {
//        palpidefsdev.push_back(new AliPALPIDEFSRawStreamMS());
        palpidefsdev[i]->SetInputFile(fsFiles[i].c_str());
      }
      sleep(10);
      cout << "slept 10" << endl;
    }
    return kTRUE;
}
//_______________________________________________________________
void Monitoring::Stop(){

    fStop = kTRUE;

}
//_______________________________________________________________
void Monitoring::DisplayInt(){
    // Slot method connected to the ValueSet(Long_t) signal.
    // It displays the value set in TGNumberEntry widget.
    fLabel->SetText(Form("%i",(Int_t)fNumber->GetNumberEntry()->GetIntNumber()));
    // Parent frame Layout() method will redraw the label showing the new value.
    fGframe->Layout();
}
void Monitoring::DisplayFloat(){
    fLabel2->SetText(Form("%f",(Float_t)fNumber2->GetNumberEntry()->GetNumber()));
    // Parent frame Layout() method will redraw the label showing the new value.
    fGframe2->Layout();
}

//_______________________________________________________________
void Monitoring::MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h) {

// Create a main frame
    fMain = new TGMainFrame(p,w,h);
// Create a horizontal frame widget with buttons
    TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain,200,200);
    TGTextButton *draw = new TGTextButton(hframe,"&Next Event");
    draw->Connect("Clicked()","Monitoring",this,"NextEvt()");
    hframe->AddFrame(draw, new TGLayoutHints(kLHintsCenterX,15,15,15,15));
    TGTextButton *draw2 = new TGTextButton(hframe,"Scan &All");
    draw2->Connect("Clicked()","Monitoring",this,"End()");
    hframe->AddFrame(draw2, new TGLayoutHints(kLHintsCenterX,15,2,15,15));
    TGTextButton *stop = new TGTextButton(hframe,"&Stop");
    stop->Connect("Clicked()","Monitoring",this,"Stop()");
    hframe->AddFrame(stop, new TGLayoutHints(kLHintsCenterX,2,15,15,15));
    TGTextButton *draw3 = new TGTextButton(hframe,"To the &End");
    draw3->Connect("Clicked()","Monitoring",this,"EndSilent()");
    hframe->AddFrame(draw3, new TGLayoutHints(kLHintsCenterX,15,15,15,15));
//    TGTextButton *draw6 = new TGTextButton(hframe,"AutoUpdate");
//    draw6->Connect("Clicked()","Monitoring",this,"AutoUpdate()");
//    hframe->AddFrame(draw6, new TGLayoutHints(kLHintsCenterX,15,15,15,15));
// TGTextButton *draw4 = new TGTextButton(hframe,"&Use Pedestal");
// draw4->Connect("Clicked()","Monitoring",this,"MenuPedestal()");
//hframe->AddFrame(draw4, new TGLayoutHints(kLHintsCenterX,15,15,15,15));
    TGTextButton *draw5 = new TGTextButton(hframe,"&Terminate");
    draw5->Connect("Clicked()","Monitoring",this,"Terminate()");
    hframe->AddFrame(draw5, new TGLayoutHints(kLHintsCenterX,15,15,15,15));
    TGTextButton *exit = new TGTextButton(hframe,"&Exit","gApplication->Terminate(0)");
    hframe->AddFrame(exit, new TGLayoutHints(kLHintsCenterX,35,15,15,15));
    fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));
// Set a name to the main frame
    fMain->SetWindowName("MENU");
// Map all subwindows of main frame
    fMain->MapSubwindows();
// Initialize the layout algorithm
    fMain->Resize(fMain->GetDefaultSize());
// Map main frame
    fMain->MapWindow();

}

