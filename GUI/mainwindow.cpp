#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include <QCoreApplication>
#include <QtDebug>
#include <deque>
#include <mutex>
#include <thread>
#include <thread_db.h>
#include <map>
#include <string>
#include <iostream>
//#include "TQtWidgets.h"
#include "../TAlpide.h"
#include "../TDigitalAnalysis.h"
#include "../TDigitalScan.h"
#include "../AlpideConfig.h"
#include "../TReadoutBoard.h"
#include "../TReadoutBoardDAQ.h"
#include "../TReadoutBoardMOSAIC.h"
//#include "../USBHelpers.h"
#include "../TConfig.h"
#include "../AlpideDecoder.h"
#include "../AlpideConfig.h"
#include <QFileDialog>
#include "../BoardDecoder.h"
#include "../SetupHelpers.h"
#include "../TThresholdScan.h"
#include "../TScanConfig.h"
#include "../THisto.h"
#include "../TScanAnalysis.h"
#include "../TScan.h"
#include "../TThresholdAnalysis.h"
#include  "../TFifoTest.h"
#include  "../TFifoAnalysis.h"
#include "../TNoiseAnalysis.h"
#include "../TNoiseOccupancy.h"
#include <QFile>
#include <typeinfo>
#include <qpushbutton.h>
#include <QPixmap>
#include "testselection.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   chkBtnObm1=chkBtnObm2=chkBtnObm3=chkBtnObm4=chkBtnObm5=chkBtnObm6=chkBtnObm7=false;

     ui->setupUi(this);
     ui->tab_2->setEnabled(false);
     ui->tab_3->setEnabled(false);
     qDebug()<<"Starting testing";
      ui->obm1->setStyleSheet("background-color:red;");
      ui->obm2->setStyleSheet("background-color:red;");
      ui->obm3->setStyleSheet("background-color:red;");
      ui->obm4->setStyleSheet("background-color:red;");
      ui->obm5->setStyleSheet("background-color:red;");
      ui->obm6->setStyleSheet("background-color:red;");
      ui->obm7->setStyleSheet("background-color:red;");
      ui->OBModule->hide();
      ui->IBModule->hide();
      ui->OBHALFSTAVE->hide();
      ui->statusbar->hide();
      ui->statuslabel->setVisible(false);
     connect(ui->newtest,SIGNAL(clicked()),SLOT(start_test()));
     connect( ui->cfg, SIGNAL( clicked()), this, SLOT(open()) );
     connect(ui->quit,SIGNAL(clicked()),this,SLOT(close()));
     connect(ui->obm1,SIGNAL(clicked()),this,SLOT(button_obm1_clicked()));
     connect(ui->obm2,SIGNAL(clicked()),this,SLOT(button_obm2_clicked()));
     connect(ui->obm3,SIGNAL(clicked()),this,SLOT(button_obm3_clicked()));
     connect(ui->obm4,SIGNAL(clicked()),this,SLOT(button_obm4_clicked()));
     connect(ui->obm5,SIGNAL(clicked()),this,SLOT(button_obm5_clicked()));
     connect(ui->obm6,SIGNAL(clicked()),this,SLOT(button_obm6_clicked()));
     connect(ui->obm7,SIGNAL(clicked()),this,SLOT(button_obm7_clicked()));
     connect (ui->testselection,SIGNAL(currentIndexChanged(int)),this, SLOT(combochanged(int)));

     QPixmap alice("alice.jpg");
     int w = ui->alicepic->width();
     int h = ui->alicepic->height();
     ui->alicepic->setPixmap(alice.scaled(w,h,Qt::KeepAspectRatio));

    }

MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::open(){
    QString operatorname;
    int hicidnumber;
    int counter;
    settingswindow->hide();
    settingswindow->SaveSettings(operatorname,hicidnumber,counter);
    if (counter==0) {return;}
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Configuration. . ."), "/home/palpidefs/Alpide/GUI_Dimitra/build-GUI-Desktop-Debug", tr("Configuration Files (*.cfg)"));
    try{
    std::cout<<properconfig<<"d1"<<endl;
    initSetup(fConfig, &fBoards, &fBoardType, &fChips, fileName.toStdString().c_str());
    properconfig=true;
    std::cout<<properconfig<<"d2"<<endl;
   // fillingvectors();

    }
    catch(exception &e){
       std::cout<<e.what()<<endl;
       popup(e.what());
       properconfig=false;
       std::cout<<properconfig<<"d3"<<endl;
}
std::cout<<properconfig<<"d4"<<endl;
if (properconfig==1){
    ui->tab_2->setEnabled(true);
    ui->tab_3->setEnabled(true);
    int device=0;
    device=fConfig->GetDeviceType();
    if (device==2){
        ui->OBModule->show();
        for (int i=0;i< fChips.size();i++){
            int chipid;
            uint8_t module,side,pos;
            chipid=fChips.at(i)->GetConfig()->GetChipId();
            if(fChips.at(i)->GetConfig()->IsEnabled()){
            DecodeId(chipid,module,side,pos);
            color_green(side,pos);
                        } else {DecodeId(chipid,module,side,pos);
                color_red(side,pos);}
        }
    }
    if (device==3){
         ui->IBModule->show();
         for (int i=0;i< fChips.size();i++){
             int chipid;
             uint8_t module,side,pos;
             chipid=fChips.at(i)->GetConfig()->GetChipId();
             if(fChips.at(i)->GetConfig()->IsEnabled()){
             DecodeId(chipid,module,side,pos);
             color_green_IB(pos);
                         } else {DecodeId(chipid,module,side,pos);
                 color_red_IB(pos);}
         }
    }
   if (device==5){
      ui->OBHALFSTAVE->show();
      for (int i=0;i< fChips.size();i++){
          int chipid;
          chipid=fChips.at(i)->GetConfig()->GetChipId();
          if(fChips.at(i)->GetConfig()->IsEnabled()){
                    explore_halfstave(chipid);
                    }
      }
    }
}
//TestSelection *saveinput;
//saveinput->SaveSettings(operatorname,hicidnumber);

}

void MainWindow::button_obm1_clicked(){
  chkBtnObm1 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("1");
  for (int i=0;i< fChips.size();i++){
      int chipid;
      uint8_t module,side,pos;
      chipid=fChips.at(i)->GetConfig()->GetChipId();
      DecodeId(chipid,module,side,pos);
      if(fChips.at(i)->GetConfig()->IsEnabled()&&module==1){
      color_green(side,pos);}

  else color_red(side,pos);
}}


void MainWindow::button_obm2_clicked(){
  chkBtnObm2 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("2");
  for (int i=0;i< fChips.size();i++){
      int chipid;
      chipid=fChips.at(i)->GetConfig()->GetChipId();
      uint8_t module,side,pos;
      DecodeId(chipid,module,side,pos);
    if(fChips.at(i)->GetConfig()->IsEnabled()&&module==2){
     color_green(side,pos);
}else color_red(side,pos);
  }
}

void MainWindow::button_obm3_clicked(){
  chkBtnObm3 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("3");
  for (int i=0;i< fChips.size();i++){
      int chipid;
      chipid=fChips.at(i)->GetConfig()->GetChipId();
      uint8_t module,side,pos;
      DecodeId(chipid,module,side,pos);
    if(fChips.at(i)->GetConfig()->IsEnabled()&&module==3){
     color_green(side,pos);

}
  else color_red(side,pos);}

}


void MainWindow::button_obm4_clicked(){
  chkBtnObm4 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("4");
  for (int i=0;i< fChips.size();i++){
      int chipid;
      chipid=fChips.at(i)->GetConfig()->GetChipId();
      uint8_t module,side,pos;
      DecodeId(chipid,module,side,pos);
   if(fChips.at(i)->GetConfig()->IsEnabled()&&module==4){
     color_green(side,pos);

}
  else color_red(side,pos);}

}

void MainWindow::button_obm5_clicked(){
  chkBtnObm5 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("5");
  for (int i=0;i< fChips.size();i++){
      int chipid;
      chipid=fChips.at(i)->GetConfig()->GetChipId();
      uint8_t module,side,pos;
      DecodeId(chipid,module,side,pos);
   if(fChips.at(i)->GetConfig()->IsEnabled()&&module==5){
     color_green(side,pos);
}
  else color_red(side,pos);}
}

void MainWindow::button_obm6_clicked(){
  chkBtnObm6 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("6");
  for (int i=0;i< fChips.size();i++){
      int chipid;
      chipid=fChips.at(i)->GetConfig()->GetChipId();
      uint8_t module,side,pos;
      DecodeId(chipid,module,side,pos);
 if(fChips.at(i)->GetConfig()->IsEnabled()&&module==6){
    color_green(side,pos);

}else color_red(side,pos);}
}

void MainWindow::button_obm7_clicked(){
  chkBtnObm7 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("7");
  for (int i=0;i< fChips.size();i++){
      int chipid;
      chipid=fChips.at(i)->GetConfig()->GetChipId();
      uint8_t module,side,pos;
      DecodeId(chipid,module,side,pos);
    if(fChips.at(i)->GetConfig()->IsEnabled()&&module==7){
      color_green(side,pos);
}else color_red(side,pos);}
}

void MainWindow::explore_halfstave(uint8_t chipid){

        uint8_t module, side, position;

        DecodeId(chipid,module,side,position);

                   if (module==1){

                       ui->obm1->setStyleSheet("background-color:green;");}

                   if (module==2){

                       ui->obm2->setStyleSheet("background-color:green;");}

                   if (module==3){

                       ui->obm3->setStyleSheet("background-color:green;");}

                   if (module==4){

                       ui->obm4->setStyleSheet("background-color:green;");}

                   if (module==5){

                       ui->obm5->setStyleSheet("background-color:green;");}

                   if (module==6){

                       ui->obm6->setStyleSheet("background-color:green;");}

                   if (module==7){

                       ui->obm7->setStyleSheet("background-color:green;");}

        }


void MainWindow::DecodeId(const uint8_t chipId, uint8_t &module, uint8_t &side, uint8_t &position)
{
    module = (chipId & 0x70)>>4;

    if( module==0 ){            // IB module
        position = chipId & 0x0F;
        side=0;
        return;
    }
    // Must be an OB module here
    side= (chipId & 0x08)>>3;
    position = (chipId & 0x07);
    return;
}




void MainWindow::color_green(int side,int pos){

    if(side==0&&pos==0){  ui->chip00->setStyleSheet("background-color:green;"); }
    if(side==0&&pos==1){  ui->chip01->setStyleSheet("background-color:green;"); }
    if(side==0&&pos==2){  ui->chip02->setStyleSheet("background-color:green;"); }
    if(side==0&&pos==3){  ui->chip03->setStyleSheet("background-color:green;"); }
    if(side==0&&pos==4){  ui->chip04->setStyleSheet("background-color:green;"); }
    if(side==0&&pos==5){  ui->chip05->setStyleSheet("background-color:green;"); }
    if(side==0&&pos==6){  ui->chip06->setStyleSheet("background-color:green;"); }
    if(side==1&&pos==0){  ui->chip10->setStyleSheet("background-color:green;"); }
    if(side==1&&pos==1){  ui->chip11->setStyleSheet("background-color:green;"); }
    if(side==1&&pos==2){  ui->chip12->setStyleSheet("background-color:green;"); }
    if(side==1&&pos==3){  ui->chip13->setStyleSheet("background-color:green;"); }
    if(side==1&&pos==4){  ui->chip14->setStyleSheet("background-color:green;"); }
    if(side==1&&pos==5){  ui->chip15->setStyleSheet("background-color:green;"); }
    if(side==1&&pos==6){  ui->chip16->setStyleSheet("background-color:green;"); }
}


void MainWindow::color_red(int side,int pos){

    if(side==0&&pos==0){  ui->chip00->setStyleSheet("background-color:red;"); }
    if(side==0&&pos==1){  ui->chip01->setStyleSheet("background-color:red;"); }
    if(side==0&&pos==2){  ui->chip02->setStyleSheet("background-color:red;"); }
    if(side==0&&pos==3){  ui->chip03->setStyleSheet("background-color:red;"); }
    if(side==0&&pos==4){  ui->chip04->setStyleSheet("background-color:red;"); }
    if(side==0&&pos==5){  ui->chip05->setStyleSheet("background-color:red;"); }
    if(side==0&&pos==6){  ui->chip06->setStyleSheet("background-color:red;"); }
    if(side==1&&pos==0){  ui->chip10->setStyleSheet("background-color:red;"); }
    if(side==1&&pos==1){  ui->chip11->setStyleSheet("background-color:red;"); }
    if(side==1&&pos==2){  ui->chip12->setStyleSheet("background-color:red;"); }
    if(side==1&&pos==3){  ui->chip13->setStyleSheet("background-color:red;"); }
    if(side==1&&pos==4){  ui->chip14->setStyleSheet("background-color:red;"); }
    if(side==1&&pos==5){  ui->chip15->setStyleSheet("background-color:red;"); }
    if(side==1&&pos==6){  ui->chip16->setStyleSheet("background-color:red;"); }
}


void MainWindow::color_green_IB(int position){

    if(position==0){  ui->chip0->setStyleSheet("background-color:green;"); }
    if(position==1){  ui->chip1->setStyleSheet("background-color:green;"); }
    if(position==2){  ui->chip2->setStyleSheet("background-color:green;"); }
    if(position==3){  ui->chip3->setStyleSheet("background-color:green;"); }
    if(position==4){  ui->chip4->setStyleSheet("background-color:green;"); }
    if(position==5){  ui->chip5->setStyleSheet("background-color:green;"); }
    if(position==6){  ui->chip6->setStyleSheet("background-color:green;"); }
    if(position==7){  ui->chip7->setStyleSheet("background-color:green;"); }
    if(position==8){  ui->chip8->setStyleSheet("background-color:green;"); }

}

void MainWindow::color_red_IB(int position){

    if(position==0){  ui->chip0->setStyleSheet("background-color:red;"); }
    if(position==1){  ui->chip1->setStyleSheet("background-color:red;"); }
    if(position==2){  ui->chip2->setStyleSheet("background-color:red;"); }
    if(position==3){  ui->chip3->setStyleSheet("background-color:red;"); }
    if(position==4){  ui->chip4->setStyleSheet("background-color:red;"); }
    if(position==5){  ui->chip5->setStyleSheet("background-color:red;"); }
    if(position==6){  ui->chip6->setStyleSheet("background-color:red;"); }
    if(position==7){  ui->chip7->setStyleSheet("background-color:red;"); }
    if(position==8){  ui->chip8->setStyleSheet("background-color:red;"); }
}

void MainWindow::test(){
    qDebug()<< "Testing ...";
}

void MainWindow::combochanged(int index){
    switch(index){
    case 0:
        qDebug()<<"No Test selected";
        ui->start_test->hide();
        break;
    case 1:
        ui->start_test->show();
        qDebug()<<"Fifo Test Selected";
        disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(test()));
        disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(scantest()));
        disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(digital()));
        connect (ui->start_test,SIGNAL(clicked()),this,SLOT(fifotest()));
        break;
    case 2:
        ui->start_test->show();
        qDebug()<<"Threshold Scan Selected";
        disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(test()));
        disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(digital()));
        disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(fifotest()));
        connect (ui->start_test,SIGNAL(clicked()),this,SLOT(scantest()));
        break;
    case 3:
         ui->start_test->show();
         qDebug()<<"Digital Scan Selected";
         disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(scantest()));
         disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(test()));
         disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(fifotest()));
         connect (ui->start_test,SIGNAL(clicked()),this,SLOT(digital()));
        break;
    }
}

void MainWindow::scantest(){
    try{
        ui->statuslabel->setVisible(true);
        ui->statuslabel->update();
         ui->statusbar->setValue(0);
        ui->statusbar->show();

     TThresholdScan *myScan= new TThresholdScan(fConfig->GetScanConfig(), fChips, fBoards, &fHistoQue,&fMutex);
     TScanAnalysis  *analysis = new TThresholdAnalysis (&fHistoQue,myScan, fConfig->GetScanConfig(), &fMutex);
    // analysis->Initialize();

    // scanLoop(myScan);
    std::cout << "starting thread" << std::endl;
    std::thread scanThread(&MainWindow::scanLoop,this,myScan);
  //  analysis->Initialize();

    std::thread analysisThread(&TScanAnalysis::Run, std::ref(analysis));
    analysis->Initialize();


    ui->statusbar->setValue(50);
    scanThread.join();
     analysisThread.join();

             analysis->Finalize();
     delete myScan;
     delete analysis;
    ui->statusbar->setValue(100);

    }
     catch(exception &scanex){
         std::cout<<scanex.what()<<endl;
         popup(scanex.what());
     }

}
void MainWindow::digital(){
    try{
        ui->statuslabel->setVisible(true);
        ui->statuslabel->update();
         ui->statusbar->setValue(0);
        ui->statusbar->show();

     TDigitalScan *mydigital= new TDigitalScan(fConfig->GetScanConfig(), fChips, fBoards, &fHistoQue,&fMutex);
    TDigitalAnalysis  *analysis = new TDigitalAnalysis(&fHistoQue,mydigital, fConfig->GetScanConfig(), &fMutex);

   //scanLoop(mydigital);
    std::cout << "starting thread" << std::endl;
   std::thread scanThread(&MainWindow::scanLoop,this,mydigital);
    std::thread analysisThread(&TScanAnalysis::Run, std::ref(analysis));
    ui->statusbar->setValue(50);
   scanThread.join();
   analysisThread.join();
   analysis->Finalize();

   delete mydigital;
    delete analysis;
   ui->statusbar->setValue(100);
}
    catch(exception &edigital){
        std::cout<<edigital.what()<<endl;
        popup(edigital.what());
    }

}

void MainWindow::fifotest(){
    try{
        ui->statuslabel->setVisible(true);
         ui->statuslabel->update();
          ui->statusbar->setValue(0);
        ui->statusbar->show();
    TFifoTest *myfifo= new TFifoTest(fConfig->GetScanConfig(), fChips, fBoards, &fHistoQue,&fMutex);
    TFifoAnalysis  *analysis = new TFifoAnalysis(&fHistoQue,myfifo,fConfig->GetScanConfig(), &fMutex);

   //scanLoop(myfifo);
    std::cout << "starting thread" << std::endl;
  std::thread scanThread(&MainWindow::scanLoop,this,myfifo);
  std::thread analysisThread(&TScanAnalysis::Run, std::ref(analysis));
 ui->statusbar->setValue(50);
   scanThread.join();
   analysisThread.join();
   analysis->Finalize();

    delete myfifo;
    delete analysis;
   ui->statusbar->setValue(100);

    }
    catch(exception &efifo){
        std::cout<<efifo.what()<<endl;
        popup(efifo.what());
    }

}






void MainWindow::scanLoop (TScan *myScan)
{ myScan->Init();
  myScan->LoopStart(2);
  while (myScan->Loop(2)) {
    myScan->PrepareStep(2);
    myScan->LoopStart  (1);
    while (myScan->Loop(1)) {
      myScan->PrepareStep(1);
      myScan->LoopStart  (0);
      while (myScan->Loop(0)) {
        myScan->PrepareStep(0);
        myScan->Execute    ();
        myScan->Next       (0);
      }
      myScan->LoopEnd(0);
      myScan->Next   (1);
    }
    myScan->LoopEnd(1);
    myScan->Next   (2);
  }
  myScan->LoopEnd  (2);
  myScan->Terminate();
}

void MainWindow::popup(QString message){

    //Dialog exwindow;
    //exwindow.setModal(true);
    //exwindow.exec();
    //check this Attribute
//    windowex->setAttribute(Qt::WA_DeleteOnClose);
    windowex=new Dialog(this);
    windowex->append(message);
    windowex->show();
}

void MainWindow::start_test(){
   fAnalysisVector.clear();
   fScanVector.clear();
    fChips.clear();
    fBoards.clear();
   // settingswindow->setAttribute(Qt::WA_DeleteOnClose);
    settingswindow= new TestSelection(this);
    settingswindow->show();
}


void MainWindow::fillingvectors(){
    TFifoTest *fifoscan= new TFifoTest(fConfig->GetScanConfig(), fChips, fBoards, &fHistoQue,&fMutex);
   TFifoAnalysis  *fifoanalysis = new TFifoAnalysis(&fHistoQue,fifoscan,fConfig->GetScanConfig(), &fMutex);
    TDigitalScan *digitalscan= new TDigitalScan(fConfig->GetScanConfig(), fChips, fBoards, &fHistoQue,&fMutex);
    TNoiseOccupancy *noisescan=new TNoiseOccupancy(fConfig->GetScanConfig(), fChips, fBoards, &fHistoQue,&fMutex);
    TDigitalAnalysis  *digitalanalysis = new TDigitalAnalysis(&fHistoQue,digitalscan, fConfig->GetScanConfig(), &fMutex);
    TThresholdScan *thresholdscan= new TThresholdScan(fConfig->GetScanConfig(), fChips, fBoards, &fHistoQue,&fMutex);
    TScanAnalysis  *thresholdanalysis = new TThresholdAnalysis (&fHistoQue,thresholdscan, fConfig->GetScanConfig(), &fMutex);
    TNoiseAnalysis *noiseanalysis=new TNoiseAnalysis(&fHistoQue, noisescan, fConfig->GetScanConfig(),&fMutex);
    fScanVector.push_back(fifoscan);
   fScanVector.push_back(digitalscan);
    fScanVector.push_back(thresholdscan);
   fScanVector.push_back(noisescan);
    //for (int i;i<3;i++)
   //{
     //   std::cout<<fScanVector[i]->GetName()<<endl;
       // QString testnames=fScanVector[i]->GetName()+"\n";
        // std::cout<<testnames;
       // ui->scanstobeperformed->cursorForPosition(QPoint(0,0));
     //   QString name =ui->scanstobeperformed->text();
       // name =fScanVector[i]->GetName();
        //  connect(ui->scanstobeperformed , SIGNAL(textChanged(QString)),label, SLOT(setText(QString )));
       // ui->setupUi(this);
      // ui->scanstobeperformed->setText(fScanVector[i]->GetName());
       // ui->scanstobeperformed->setText("d");
              //  append("dimitra");
       // std::cout<<name;
        //ui->scanstobeperformed->setText(name);
        // ui->scanstobeperformed->setPlainText(fScanVector[i]->GetName());

    //}
    fAnalysisVector.push_back(fifoanalysis);
    fAnalysisVector.push_back(digitalanalysis);
   fAnalysisVector.push_back(thresholdanalysis);
  fAnalysisVector.push_back(noiseanalysis);
  // performtests(fScanVector,fAnalysisVector);
    qDebug()<<"dimitra"<<endl;

}

void MainWindow::performtests(std::vector <TScan *> s, std::vector <TScanAnalysis *> a){
    qDebug()<<s.size()<<endl;
    for (int i=1;i<2;i++){

    std::thread scanThread(&MainWindow::scanLoop,this,s[i]);
     a.at(i)->Initialize();
    qDebug()<<s.at(i)<<"g"<<endl;
    std::thread analysisThread(&TScanAnalysis::Run, std::ref(a[i]));

    scanThread.join();
    analysisThread.join();
    a.at(i)->Finalize();

    }
}


void MainWindow::connectcombo(int value){
    switch(value){
    case 0:
        qDebug()<<"No Test selected";
        ui->start_test->hide();
        break;
    case 1:{
        ui->start_test->show();
        qDebug()<<"OB Qualification test selected";
        open();
        fillingvectors();
      //  connect (ui->start_test,SIGNAL(clicked()),this,SLOT(fillingvectors()));
     connect(ui->start_test,SIGNAL(clicked()),this,SLOT(applytests()));
     //TFifoTest *fifoscan= new TFifoTest(fConfig->GetScanConfig(), fChips, fBoards, &fHistoQue,&fMutex);
       // fillingvectors();
      //  performtests(fScanVector,fAnalysisVector);
        break;}
    case 2:
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
       // disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(performtests(fScanVector,fAnalysisVector)));
        disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(digital()));
        //disconnect (ui->start_test,SIGNAL(clicked()),this,SLOT(fifotest()));
        //connect (ui->start_test,SIGNAL(clicked()),this,SLOT(scantest()));
        break;

    }
}

void MainWindow::applytests(){
    performtests(fScanVector,fAnalysisVector);
}
void MainWindow::WriteTests(){

    for (int i;i<3;i++)
       {
         //   std::cout<<fScanVector[i]->GetName()<<endl;
           // QString testnames=fScanVector[i]->GetName()+"\n";
            // std::cout<<testnames;
           // ui->scanstobeperformed->cursorForPosition(QPoint(0,0));
         //   QString name =ui->scanstobeperformed->text();
           // name =fScanVector[i]->GetName();
            //  connect(ui->scanstobeperformed , SIGNAL(textChanged(QString)),label, SLOT(setText(QString )));

    }
}
