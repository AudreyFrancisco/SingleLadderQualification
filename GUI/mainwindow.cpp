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
#include <qapplication.h>
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
#include <QMenuBar>
//#include "scanthread.h"
//#include <QtCore>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   chkBtnObm1=chkBtnObm2=chkBtnObm3=chkBtnObm4=chkBtnObm5=chkBtnObm6=chkBtnObm7=false;


      ui->setupUi(this);
     this->setWindowTitle(QString::fromUtf8("GUI"));
     ui->tab_2->setEnabled(false);
     ui->example1->hide();

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
      ui->details->hide();
      ui->displaydetails->hide();
      ui->test1->setStyleSheet("border:none;");
      ui->test2->setStyleSheet("border:none;");
      ui->test3->setStyleSheet("border:none;");
      ui->test4->setStyleSheet("border:none;");
      ui->test5->setStyleSheet("border:none;");
      ui->statusbar->hide();
      ui->tab_2->setVisible(false);
      ui->statuslabel->setVisible(false);
      ui->testtypeselected->setText("Type of test");
      ui->cfg->hide();
      ui->label_3->hide();
      ui->testselection->hide();
      QMenuBar *menu=new QMenuBar(this);
      QMenu *menu1;
      menu1=menu->addMenu("&Options");
      QAction *newtestaction = new QAction("&New test", menu);
      QAction *writedb = new QAction("&Write to database", menu);
      menu1->addAction(newtestaction);
      menu1->addAction(writedb);
     // ui->abort->hide();
     // ui->abortall->hide();
      ui->tabWidget->removeTab(2);
      ui->tabWidget->removeTab(1);
      connect(ui->test1,SIGNAL(clicked()),this,SLOT(fifolist()));
      connect(ui->test2,SIGNAL(clicked()),this,SLOT(digitallist()));
      connect(ui->test3,SIGNAL(clicked()),this,SLOT(thresholdlist()));
      connect(ui->test4,SIGNAL(clicked()),this,SLOT(noiselist()));
      connect(ui->abortall,SIGNAL(clicked()),this,SLOT(StopScan()),Qt::DirectConnection);
     connect(newtestaction, SIGNAL(triggered()),this, SLOT(start_test()));
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
    settingswindow->hide();
    settingswindow->SaveSettings(operatorname,hicidnumber,counter);
    if (counter==0) {return;}
   QString fileName="Config.cfg";

   // QString fileName = QFileDialog::getOpenFileName(this,
      //  tr("Open Configuration. . ."), "/home/palpidefs/Alpide/GUI_Dimitra/build-GUI-Desktop-Debug", tr("Configuration Files (*.cfg)"));
    try{
    std::cout<<properconfig<<"d1"<<endl;
   // initSetup(fConfig, &fBoards, &fBoardType, &fChips, fileName.toStdString().c_str());
    initSetup(fConfig, &fBoards, &fBoardType, &fChips,fileName.toStdString().c_str(), &fHics);
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
       ui->tob->setText("Outer Barrel module");
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

	TThresholdScan *myScan= new TThresholdScan(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue,&fMutex);
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

	TDigitalScan *mydigital= new TDigitalScan(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue,&fMutex);
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
	TFifoTest *myfifo= new TFifoTest(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue,&fMutex);
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
 // QApplication::processEvents() ;
  while (myScan->Loop(2)) {
    myScan->PrepareStep(2);
    myScan->LoopStart  (1);
  //  QApplication::processEvents() ;
    while (myScan->Loop(1)) {
      myScan->PrepareStep(1);
      myScan->LoopStart  (0);
  //    QApplication::processEvents() ;
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

    ui->fstatus->clear();
    ui->dstatus->clear();
    ui->tstatus->clear();
    ui->nstatus->clear();
    ui->ustatus->clear();
     ui->tob->clear();

    ui->test1->setText(" ");
    ui->test2->setText(" ");
    ui->test3->setText(" ");
    ui->test4->setText(" ");
    ui->test5->setText(" ");
    ui->testtypeselected->clear();

    // settingswindow->setAttribute(Qt::WA_DeleteOnClose);
    settingswindow= new TestSelection(this);
    settingswindow->show();
}


void MainWindow::fillingOBvectors(){
  TFifoTest *fifoscan= new TFifoTest(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue,&fMutex);
    TFifoAnalysis  *fifoanalysis = new TFifoAnalysis(&fHistoQue,fifoscan,fConfig->GetScanConfig(), &fMutex);
    TDigitalScan *digitalscan= new TDigitalScan(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue,&fMutex);
    TNoiseOccupancy *noisescan=new TNoiseOccupancy(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue,&fMutex);
    TDigitalAnalysis  *digitalanalysis = new TDigitalAnalysis(&fHistoQue,digitalscan, fConfig->GetScanConfig(), &fMutex);
    TThresholdScan *thresholdscan= new TThresholdScan(fConfig->GetScanConfig(), fChips, fHics, fBoards, &fHistoQue,&fMutex);
    TScanAnalysis  *thresholdanalysis = new TThresholdAnalysis (&fHistoQue,thresholdscan, fConfig->GetScanConfig(), &fMutex);
    TNoiseAnalysis *noiseanalysis=new TNoiseAnalysis(&fHistoQue, noisescan, fConfig->GetScanConfig(),&fMutex);
    fScanVector.push_back(fifoscan);
    fScanVector.push_back(digitalscan);
    fScanVector.push_back(thresholdscan);
    fScanVector.push_back(noisescan);
    fAnalysisVector.push_back(fifoanalysis);
    fAnalysisVector.push_back(digitalanalysis);
    fAnalysisVector.push_back(thresholdanalysis);
    fAnalysisVector.push_back(noiseanalysis);
    WriteTests();
    qDebug()<<"dimitra"<<endl;

}

void MainWindow::performtests(std::vector <TScan *> s, std::vector <TScanAnalysis *> a){
    qDebug()<<s.size()<<endl;
    ui->statuslabel->setVisible(true);
     ui->statuslabel->update();
    for (int i=0;i<s.size();i++){
   // for (int i=0;i<2;i++){
      //   QApplication::processEvents() ;


        std::thread scanThread(&MainWindow::scanLoop,this,s[i]);
        a.at(i)->Initialize();
        qDebug()<<s.at(i)<<"g"<<endl;

      // ui->details->addItem("d");


       // std::thread koumpi(&MainWindow::runscans,this);
      //  koumpi.join();
        while (i<1){
              ui->fstatus->setText("50% Completed");
              ui->dstatus->setText("Waiting . . .");
              ui->tstatus->setText("Waiting . . .");
              ui->nstatus->setText("Waiting . . .");
              qApp->processEvents();
            break;
         }
         while (i<2 && i>0){
              ui->fstatus->setText("100% Completed");
              ui->dstatus->setText("50% Completed");
              ui->tstatus->setText("Waiting . . .");
              ui->nstatus->setText("Waiting . . .");
              qApp->processEvents();
             break;
          }

         while (i<3 && i>1){
              ui->fstatus->setText("100% Completed");
              ui->dstatus->setText("100% Completed");
              ui->tstatus->setText("50% Completed");
              ui->nstatus->setText("Waiting . . .");
              qApp->processEvents();
              break;
         }

         while (i<4 && i>2){
              ui->fstatus->setText("100% Completed");
              ui->dstatus->setText("100% Completed");
              ui->tstatus->setText("100% Completed");
              ui->nstatus->setText("50% Completed");
              qApp->processEvents();
         break;
         }
        std::thread analysisThread(&TScanAnalysis::Run, std::ref(a[i]));
        scanThread.join();



        analysisThread.join();
        a.at(i)->Finalize();

        while (i<1){
            ui->fstatus->setText("100% Completed");
            ui->dstatus->setText("Starting . . .");
            ui->tstatus->setText("Waiting . . .");
            ui->nstatus->setText("Waiting . . .");
            qApp->processEvents();
        break;
        }
       while (i<2 && i>0){
            ui->fstatus->setText("100% Completed");
            ui->dstatus->setText("100% Completed");
            ui->tstatus->setText("Starting . . .");
            ui->nstatus->setText("Waiting . . .");
            qApp->processEvents();
          break;
       }

        while (i<3 && i>1){
            ui->fstatus->setText("100% Completed");
            ui->dstatus->setText("100% Completed");
            ui->tstatus->setText("100% Completed");
            ui->nstatus->setText("Starting . . . ");
            qApp->processEvents();
         break;
        }
        while(i<4 && i>2){
            ui->fstatus->setText("100% Completed");
            ui->dstatus->setText("100% Completed");
            ui->tstatus->setText("100% Completed");
            ui->nstatus->setText("100% Completed");
            qApp->processEvents();
        break;
        }
qApp->processEvents();
    }
    qApp->processEvents();
}


void MainWindow::connectcombo(int value){
    counter=0;
    switch(value){
    case 0:
        ui->testtypeselected->clear();
        qDebug()<<"No Test selected";
        ui->start_test->hide();
        break;
    case 1:{
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"OB Qualification test selected";
        // ui->testtypeselected->setText("OB Reception Test");
        ui->testtypeselected->setText("OB Qualification Test");
      //  ui->example1->show();
        open();


       if (counter==0) {break;}
       fillingOBvectors();

        break;}
    case 2:
       {
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
        openib();
        if (counter==0){break;}
        //Later no need to close the pop up window or to apply settings. everything will be done upon th loading of the cfg.
        break;}

    case 3:
       {
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
       // openib();
        //Later no need to close the pop up window or to apply settings. everything will be done upon th loading of the cfg.
        break;}

    case 4:
       {
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
         ui->testtypeselected->setText("OB Reception Test");
       // openib();
        //Later no need to close the pop up window or to apply settings. everything will be done upon th loading of the cfg.
        break;}

    case 5:
       {
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
       // openib();
        //Later no need to close the pop up window or to apply settings. everything will be done upon th loading of the cfg.
        break;}

    case 6:
       {
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
       // openib();
        //Later no need to close the pop up window or to apply settings. everything will be done upon th loading of the cfg.
        break;}

    case 7:
       {
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
       // openib();
        //Later no need to close the pop up window or to apply settings. everything will be done upon th loading of the cfg.
        break;}

    case 8:
       {
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
       // openib();
        //Later no need to close the pop up window or to apply settings. everything will be done upon th loading of the cfg.
        break;}

    case 9:
       {
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
       // openib();
        //Later no need to close the pop up window or to apply settings. everything will be done upon th loading of the cfg.
        break;}
    case 10:
       {
        ui->testtypeselected->clear();
        ui->start_test->show();
        qDebug()<<"IB Qualification test selected";
       // openib();
        //Later no need to close the pop up window or to apply settings. everything will be done upon th loading of the cfg.
        break;}

    }

  connect(ui->start_test,SIGNAL(clicked()),this,SLOT(applytests()));
   //connect(ui->start_test,SIGNAL(clicked()),this,SLOT(runscans()));
}

void MainWindow::applytests(){

    performtests(fScanVector,fAnalysisVector);

  // emit stopTimer();
}

void MainWindow::WriteTests(){

    for (int i=0;i<fScanVector.size();i++)
    {

    while (i<1){
        if (fScanVector.size()<5){
           ui->test1->setText(fScanVector[i]->GetName());
           ui->test2->setText(fScanVector[i+1]->GetName());
           ui->test3->setText(fScanVector[i+2]->GetName());
           ui->test4->setText(fScanVector[i+3]->GetName());
}//           ui->test5->setText(fScanVector[i+4]->GetName());

        break;
}
    }
}

void MainWindow::runscans(){
/*QTimer* timer;
timer=new QTimer(this);
connect(timer,SIGNAL(timeout()),this,SLOT(applytests()));
connect(ui->abortall,SIGNAL(clicked()),timer,SLOT(stop()));
connect(ui->abortall,SIGNAL(clicked()),timer,SLOT(deleteLater()));
connect(this, SIGNAL(stopTimer()),timer,SLOT(deleteLater()));
connect(this, SIGNAL(stopTimer()),timer,SLOT(stop()));
timer->setInterval(0);
timer->setSingleShot(false);
timer->start();
*/
//QPushButton *dimitra=new QPushButton("dsxfvgdvg", parent->thread());
//connect(dimitra,SIGNAL(clicked()),this,SLOT(StopScan());
//QThread *thread=new QThread;
//ScanThread *dim= new ScanThread();
//dim->moveToThread(thread);
//std::cout<<"RTg"<<std::endl;
//connect(thread,SIGNAL(started()),dim, SLOT(run()));
//std::cout<<"RTg"<<std::endl;
//connect(thread, SIGNAL(started()),dim,SLOT(process()));

//connect(dim,SIGNAL(ScanThread::resultReady()),thread, SLOT(quit()));
//thread->start();
//dim->start();
//std::cout<<"RTg"<<std::endl;
    //QThread *thread=new QThread(this);
//dimitra->moveToThread(thread);
//connect(timer, SIGNAL(timeout()),this,SLOT(applytests()));
/*connect(thread,SIGNAL(started()),this,SLOT(applytests()));
connect(dimitra,SIGNAL(finished()),thread,SLOT(quit()));
connect(dimitra,SIGNAL(finished()),dimitra,SLOT(deleteLater()));
connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
thread->start();
*/
//while (!thread->isFinished()) QCoreApplication::processEvents();
//if (ui->abortall->isChecked()){
    //thread->isFinished();
//}
//timer->start();
    //std::thread t1(&MainWindow::performtests,this, fScanVector,fAnalysisVector);

   // std::thread t2(&MainWindow::createbtn,this);
    //t2.join();
}

void MainWindow::StopScan(){

    fScanAbort=true;

}

void MainWindow::createbtn(){

    QPushButton *dimitra=new QPushButton("dsxfvgdvg", this);
    connect(dimitra,SIGNAL(clicked()),this,SLOT(StopScan()));

}

void MainWindow::fifolist(){
    mapdetails.clear();
    ui->details->clear();
    for (std::map<const char*,resultType>::const_iterator it=fAnalysisVector.at(0)->GetVariableList().begin(); it!=fAnalysisVector.at(0)->GetVariableList().end(); ++it){

          std::cout << it->first << " => " << it->second << '\n';
          std::string d;
         d=(std::string(it->first));
         // d.append(++it->first);
        //  QVector <std::string> dimitra;
         mapdetails.push_back(d);

     }
    for (int i=0; i<mapdetails.size();i++){
        std::cout<<mapdetails[i]<<std::endl;
        ui->details->addItem(mapdetails[i].c_str());
    }
ui->details->show();
qApp->processEvents();
ui->displaydetails->show();
}


void MainWindow::digitallist(){
    mapdetails.clear();
    ui->details->clear();
    for (std::map<const char*,resultType>::const_iterator it=fAnalysisVector.at(1)->GetVariableList().begin(); it!=fAnalysisVector.at(1)->GetVariableList().end(); ++it){

          std::cout << it->first << " => " << it->second << '\n';
          std::string d;
         d=(std::string(it->first));
         // d.append(++it->first);
        //  QVector <std::string> dimitra;
         mapdetails.push_back(d);

     }
    for (int i=0; i<mapdetails.size();i++){
        std::cout<<mapdetails[i]<<std::endl;
        ui->details->addItem(mapdetails[i].c_str());
    }
ui->details->show();
qApp->processEvents();
ui->displaydetails->show();
}

void MainWindow::thresholdlist(){
    mapdetails.clear();
    ui->details->clear();
    for (std::map<const char*,resultType>::const_iterator it=fAnalysisVector.at(2)->GetVariableList().begin(); it!=fAnalysisVector.at(2)->GetVariableList().end(); ++it){

          std::cout << it->first << " => " << it->second << '\n';
          std::string d;
         d=(std::string(it->first));
         // d.append(++it->first);
        //  QVector <std::string> dimitra;
         mapdetails.push_back(d);

     }
    for (int i=0; i<mapdetails.size();i++){
        std::cout<<mapdetails[i]<<std::endl;
        ui->details->addItem(mapdetails[i].c_str());
    }
ui->details->show();
qApp->processEvents();
ui->displaydetails->show();
}


void MainWindow::noiselist(){
    mapdetails.clear();
    ui->details->clear();
    for (std::map<const char*,resultType>::const_iterator it=fAnalysisVector.at(3)->GetVariableList().begin(); it!=fAnalysisVector.at(3)->GetVariableList().end(); ++it){

          std::cout << it->first << " => " << it->second << '\n';
          std::string d;
         d=(std::string(it->first));
         // d.append(++it->first);
        //  QVector <std::string> dimitra;
         mapdetails.push_back(d);

     }
    for (int i=0; i<mapdetails.size();i++){
        std::cout<<mapdetails[i]<<std::endl;
        ui->details->addItem(mapdetails[i].c_str());
    }
ui->details->show();
qApp->processEvents();
ui->displaydetails->show();
}



void MainWindow::openib(){
    QString operatorname;
    int hicidnumber;
    settingswindow->hide();
    settingswindow->SaveSettings(operatorname,hicidnumber,counter);
    if (counter==0) {return;}
    QString fileName="Configib.cfg";

   // QString fileName = QFileDialog::getOpenFileName(this,
     //   tr("Open Configuration. . ."), "/home/palpidefs/Alpide/GUI_Dimitra/build-GUI-Desktop-Debug", tr("Configuration Files (*.cfg)"));
    try{
    std::cout<<properconfig<<"d1"<<endl;
   // initSetup(fConfig, &fBoards, &fBoardType, &fChips, fileName.toStdString().c_str());
    initSetup(fConfig, &fBoards, &fBoardType, &fChips,fileName.toStdString().c_str());

    properconfig=true;
    std::cout<<properconfig<<"d2"<<endl;
   // fillingvectors();

    }
    catch(exception &e){
       std::cout<<e.what()<<endl;
      popup("Check Connection :(");
       //popup(e.what());
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
        ui->tob->setText("Outer Barrel module");
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
