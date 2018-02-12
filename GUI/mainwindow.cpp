#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QPixmap>
#include <QtDebug>
#include <QtWidgets>
//#include <QtCore>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iostream>
#include <map>
#include <mutex>
#include <qapplication.h>
#include <string>
#include <thread>
//#include "TQtWidgets.h"
#include <qpushbutton.h>
#include <typeinfo>
//#include "scanthread.h"
#include "AlpideConfig.h"
#include "TAlpide.h"
#include "TDigitalAnalysis.h"
#include "TDigitalScan.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "dialog.h"
#include "scanconfiguration.h"
#include "testingprogress.h"
#include "testselection.h"
//#include "USBHelpers.h"
#include "AlpideConfig.h"
#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TApplyMask.h"
#include "TApplyTuning.h"
#include "TConfig.h"
#include "TCycleAnalysis.h"
#include "TDigitalWFAnalysis.h"
#include "TEnduranceCycle.h"
#include "TFifoAnalysis.h"
#include "TFifoTest.h"
#include "THIC.h"
#include "THisto.h"
#include "TLocalBusAnalysis.h"
#include "TLocalBusTest.h"
#include "TNoiseAnalysis.h"
#include "TNoiseOccupancy.h"
#include "TPowerAnalysis.h"
#include "TPowerTest.h"
#include "TReadoutAnalysis.h"
#include "TReadoutTest.h"
#include "TSCurveAnalysis.h"
#include "TSCurveScan.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TThresholdAnalysis.h"
#include "calibrationpb.h"

bool writingdb;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  fChkBtnObm1 = fChkBtnObm2 = fChkBtnObm3 = fChkBtnObm4 = fChkBtnObm5 = fChkBtnObm6 = fChkBtnObm7 =
      false;
  fPbcfgcheck = 0;
  fCalwindow = 0;
  fActivitywindow = 0;
  fDatabasewindow = 0;
  fNoticewindow = 0;
  fPbnumberofmodule = 0;
  makeDir("Data");
  ui->setupUi(this);
  this->setWindowTitle(QString::fromUtf8("GUI"));
  ui->tab_2->setEnabled(false);
  ui->example1->hide();
  ui->tab_3->setEnabled(true);
  // qDebug()<<"Starting testing";
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
  ui->endurancebox->hide();
  ui->statusbar->hide();
  ui->tab_2->setVisible(false);
  ui->statuslabel->setVisible(false);
  ui->testtypeselected->setText("Type of test");
  ui->cfg->hide();
  ui->label_3->hide();
  ui->testselection->hide();
  ui->ledtext->hide();
  QMenuBar *menu = new QMenuBar(this);
  QMenu *menu1;
  menu1 = menu->addMenu("&Options");
  QAction *newtestaction = new QAction("&New test", menu);
  fWritedb = new QAction("&Write to database", menu);
  menu1->addAction(newtestaction);
  menu1->addAction(fWritedb);
  fWritedb->setVisible(false);
  // ui->abort->hide();
  // ui->abortall->hide();
  ui->tabWidget->removeTab(2);
  ui->tabWidget->removeTab(1);
  // connect(writedb,SIGNAL(triggered()),this,SLOT(attachtodatabase()));
  connect(ui->abortall, SIGNAL(clicked()), this, SLOT(StopScan()), Qt::DirectConnection);
  connect(newtestaction, SIGNAL(triggered()), this, SLOT(start_test()));
  connect(ui->newtest, SIGNAL(clicked()), SLOT(start_test()));
  connect(ui->cfg, SIGNAL(clicked()), this, SLOT(open()));
  connect(ui->quit, SIGNAL(clicked()), this, SLOT(quitall()));
  connect(ui->obm1, SIGNAL(clicked()), this, SLOT(button_obm1_clicked()));
  connect(ui->obm2, SIGNAL(clicked()), this, SLOT(button_obm2_clicked()));
  connect(ui->obm3, SIGNAL(clicked()), this, SLOT(button_obm3_clicked()));
  connect(ui->obm4, SIGNAL(clicked()), this, SLOT(button_obm4_clicked()));
  connect(ui->obm5, SIGNAL(clicked()), this, SLOT(button_obm5_clicked()));
  connect(ui->obm6, SIGNAL(clicked()), this, SLOT(button_obm6_clicked()));
  connect(ui->obm7, SIGNAL(clicked()), this, SLOT(button_obm7_clicked()));
  connect(ui->testselection, SIGNAL(currentIndexChanged(int)), this, SLOT(combochanged(int)));
  connect(ui->details, SIGNAL(currentIndexChanged(int)), this, SLOT(detailscombo(int)));
  connect(ui->poweroff, SIGNAL(clicked(bool)), this, SLOT(poweroff()));

  ui->pbstatus->hide();

  QPixmap alice("alicethreshold.png");
  int w = ui->alicepic->width();
  int h = ui->alicepic->height();
  ui->alicepic->setPixmap(alice.scaled(w, h, Qt::KeepAspectRatio));

  QPixmap alicelog("logo.png");
  int width = ui->alicelogo->width();
  int height = ui->alicelogo->height();
  ui->alicelogo->setPixmap(alicelog.scaled(width, height, Qt::KeepAspectRatio));

  ui->start_test->hide();

  connect(ui->testib, SIGNAL(clicked()), this, SLOT(IBBasicTest()));
  ui->testib->hide();
  writingdb = true;
  fstop = false;
}

MainWindow::~MainWindow() {
  delete ui;
  ui = 0x0;
}

// TODO: try to substitute numberofscan by TScanType (defined in TScanConfig.h)
void MainWindow::open() {

  // settingswindow->hide();
  // settingswindow->SaveSettings(operatorname,hicidnumber,counter);
  // if (counter==0) {return;}
  QString fileName;
  if (fNumberofscan == OBQualification || fNumberofscan == OBEndurance ||
      fNumberofscan == OBReception) {
    fileName = "Config.cfg";
  } else if (fNumberofscan == IBQualification) {
    fileName = "Configib.cfg";
  }

  try {

    fHicnames.push_back(fHicidnumber);
    QByteArray conv = fHicidnumber.toLatin1();
    QByteArray conv2, conv3, conv4, conv5, conv6, conv7, conv8, conv9, conv10;
    // const char *ar[1]={conv.data()};
    const char *ar[10];
    ar[0] = {conv.data()};
    if (fNumberofscan == OBEndurance) {
      conv2 = fToptwo.toLatin1();
      ar[1] = {conv2.data()};
      conv3 = fTopthree.toLatin1();
      ar[2] = {conv3.data()};
      conv4 = fTopfour.toLatin1();
      ar[3] = {conv4.data()};
      conv5 = fTopfive.toLatin1();
      ar[4] = {conv5.data()};
      conv6 = fBottomone.toLatin1();
      ar[5] = {conv6.data()};
      conv7 = fBottomtwo.toLatin1();
      ar[6] = {conv7.data()};
      conv8 = fBottomthree.toLatin1();
      ar[7] = {conv8.data()};
      conv9 = fBottomfour.toLatin1();
      ar[8] = {conv9.data()};
      conv10 = fBottomfive.toLatin1();
      ar[9] = {conv10.data()};
      fHicnames.push_back(fToptwo);
      fHicnames.push_back(fTopthree);
      fHicnames.push_back(fTopfour);
      fHicnames.push_back(fTopfive);
      fHicnames.push_back(fBottomone);
      fHicnames.push_back(fBottomtwo);
      fHicnames.push_back(fBottomthree);
      fHicnames.push_back(fBottomfour);
      fHicnames.push_back(fBottomfive);
      fEndurancemodules.push_back(ui->top1);
      fEndurancemodules.push_back(ui->top2);
      fEndurancemodules.push_back(ui->top3);
      fEndurancemodules.push_back(ui->top4);
      fEndurancemodules.push_back(ui->top5);
      fEndurancemodules.push_back(ui->down1);
      fEndurancemodules.push_back(ui->down2);
      fEndurancemodules.push_back(ui->down3);
      fEndurancemodules.push_back(ui->down4);
      fEndurancemodules.push_back(ui->down5);
    }
    initSetup(fConfig, &fBoards, &fBoardType, &fChips, fileName.toStdString().c_str(), &fHICs, ar);
    fConfig->GetScanConfig()->SetUseDataPath(true);
    fPb = fHICs.at(0)->GetPowerBoard();
    fPbconfig = fPb->GetConfigurationHandler();
    fPbnumberofmodule = fHICs.at(0)->GetPbMod();
    if (!fPb->IsCalibrated(fPbnumberofmodule)) {
      std::cout << "its not calibrated" << std::endl;
      fPbcfgcheck = new checkpbconfig(this);
      fPbcfgcheck->exec();
    }
    //  std::cout<<fHICs.size()<<"the hics vector size";
    fProperconfig = true;

    //    std::cout<<properconfig<<"d2"<<endl;
    // fillingvectors();
  } catch (exception &e) {
    //     std::cout<<e.what()<<endl;
    popup(e.what());
    fProperconfig = false;
    //  std::cout<<properconfig<<"d3"<<endl;
  }
  // std::cout<<properconfig<<"d4"<<endl;
  if (fProperconfig == 1) {
    ui->tab_2->setEnabled(true);
    //  ui->tab_3->setEnabled(true);
    int device = 0;
    device = fConfig->GetDeviceType();
    if (device == TYPE_OBHIC) {
      ui->tob->setText("Outer Barrel module");
      ui->OBModule->show();
      for (unsigned int i = 0; i < fChips.size(); i++) {
        int chipid;
        uint8_t module, side, pos;
        chipid = fChips.at(i)->GetConfig()->GetChipId();
        if (fChips.at(i)->GetConfig()->IsEnabled()) {
          DecodeId(chipid, module, side, pos);
          color_green(side, pos);
        } else {
          DecodeId(chipid, module, side, pos);
          color_red(side, pos);
        }
      }
    }
    if (device == TYPE_IBHIC) {
      ui->tob->setText("Inner Barrel module");
      ui->IBModule->show();
      for (unsigned int i = 0; i < fChips.size(); i++) {
        int chipid;
        uint8_t module, side, pos;
        chipid = fChips.at(i)->GetConfig()->GetChipId();
        if (fChips.at(i)->GetConfig()->IsEnabled()) {
          DecodeId(chipid, module, side, pos);
          color_green_IB(pos);
        } else {
          DecodeId(chipid, module, side, pos);
          color_red_IB(pos);
        }
      }
    }
    if (device == TYPE_HALFSTAVE) {
      ui->OBHALFSTAVE->show();
      for (unsigned int i = 0; i < fChips.size(); i++) {
        int chipid;
        chipid = fChips.at(i)->GetConfig()->GetChipId();
        if (fChips.at(i)->GetConfig()->IsEnabled()) {
          explore_halfstave(chipid);
        }
      }
    }

    if (device == TYPE_ENDURANCE) {
      ui->endurancebox->show();
      exploreendurancebox();
    }
  }
  // TestSelection *saveinput;
  // saveinput->SaveSettings(operatorname,hicidnumber);
}

void MainWindow::button_obm1_clicked() {
  fChkBtnObm1 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("1");
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int chipid;
    uint8_t module, side, pos;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    DecodeId(chipid, module, side, pos);
    if (fChips.at(i)->GetConfig()->IsEnabled() && module == 1) {
      color_green(side, pos);
    } else
      color_red(side, pos);
  }
}

void MainWindow::button_obm2_clicked() {
  fChkBtnObm2 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("2");
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int chipid;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    uint8_t module, side, pos;
    DecodeId(chipid, module, side, pos);
    if (fChips.at(i)->GetConfig()->IsEnabled() && module == 2) {
      color_green(side, pos);
    } else
      color_red(side, pos);
  }
}

void MainWindow::button_obm3_clicked() {
  fChkBtnObm3 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("3");
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int chipid;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    uint8_t module, side, pos;
    DecodeId(chipid, module, side, pos);
    if (fChips.at(i)->GetConfig()->IsEnabled() && module == 3) {
      color_green(side, pos);

    } else
      color_red(side, pos);
  }
}

void MainWindow::button_obm4_clicked() {
  fChkBtnObm4 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("4");
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int chipid;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    uint8_t module, side, pos;
    DecodeId(chipid, module, side, pos);
    if (fChips.at(i)->GetConfig()->IsEnabled() && module == 4) {
      color_green(side, pos);

    } else
      color_red(side, pos);
  }
}

void MainWindow::button_obm5_clicked() {
  fChkBtnObm5 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("5");
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int chipid;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    uint8_t module, side, pos;
    DecodeId(chipid, module, side, pos);
    if (fChips.at(i)->GetConfig()->IsEnabled() && module == 5) {
      color_green(side, pos);
    } else
      color_red(side, pos);
  }
}

void MainWindow::button_obm6_clicked() {
  fChkBtnObm6 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("6");
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int chipid;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    uint8_t module, side, pos;
    DecodeId(chipid, module, side, pos);
    if (fChips.at(i)->GetConfig()->IsEnabled() && module == 6) {
      color_green(side, pos);

    } else
      color_red(side, pos);
  }
}

void MainWindow::button_obm7_clicked() {
  fChkBtnObm7 = true;
  ui->OBModule->show();
  ui->modulenumber->setText("7");
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int chipid;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    uint8_t module, side, pos;
    DecodeId(chipid, module, side, pos);
    if (fChips.at(i)->GetConfig()->IsEnabled() && module == 7) {
      color_green(side, pos);
    } else
      color_red(side, pos);
  }
}

void MainWindow::explore_halfstave(uint8_t chipid) {

  uint8_t module, side, position;

  DecodeId(chipid, module, side, position);

  if (module == 1) {

    ui->obm1->setStyleSheet("background-color:green;");
  }

  if (module == 2) {

    ui->obm2->setStyleSheet("background-color:green;");
  }

  if (module == 3) {

    ui->obm3->setStyleSheet("background-color:green;");
  }

  if (module == 4) {

    ui->obm4->setStyleSheet("background-color:green;");
  }

  if (module == 5) {

    ui->obm5->setStyleSheet("background-color:green;");
  }

  if (module == 6) {

    ui->obm6->setStyleSheet("background-color:green;");
  }

  if (module == 7) {

    ui->obm7->setStyleSheet("background-color:green;");
  }
}

void MainWindow::DecodeId(const uint8_t chipId, uint8_t &module, uint8_t &side, uint8_t &position) {
  module = (chipId & 0x70) >> 4;

  if (module == 0) { // IB module
    position = chipId & 0x0F;
    side = 0;
    return;
  }
  // Must be an OB module here
  side = (chipId & 0x08) >> 3;
  position = (chipId & 0x07);
  return;
}

void MainWindow::color_green(int side, int pos) {

  if (side == 0 && pos == 0) {
    ui->chip00->setStyleSheet("background-color:green;");
  }
  if (side == 0 && pos == 1) {
    ui->chip01->setStyleSheet("background-color:green;");
  }
  if (side == 0 && pos == 2) {
    ui->chip02->setStyleSheet("background-color:green;");
  }
  if (side == 0 && pos == 3) {
    ui->chip03->setStyleSheet("background-color:green;");
  }
  if (side == 0 && pos == 4) {
    ui->chip04->setStyleSheet("background-color:green;");
  }
  if (side == 0 && pos == 5) {
    ui->chip05->setStyleSheet("background-color:green;");
  }
  if (side == 0 && pos == 6) {
    ui->chip06->setStyleSheet("background-color:green;");
  }
  if (side == 1 && pos == 0) {
    ui->chip10->setStyleSheet("background-color:green;");
  }
  if (side == 1 && pos == 1) {
    ui->chip11->setStyleSheet("background-color:green;");
  }
  if (side == 1 && pos == 2) {
    ui->chip12->setStyleSheet("background-color:green;");
  }
  if (side == 1 && pos == 3) {
    ui->chip13->setStyleSheet("background-color:green;");
  }
  if (side == 1 && pos == 4) {
    ui->chip14->setStyleSheet("background-color:green;");
  }
  if (side == 1 && pos == 5) {
    ui->chip15->setStyleSheet("background-color:green;");
  }
  if (side == 1 && pos == 6) {
    ui->chip16->setStyleSheet("background-color:green;");
  }
}

void MainWindow::color_red(int side, int pos) {

  if (side == 0 && pos == 0) {
    ui->chip00->setStyleSheet("background-color:red;");
  }
  if (side == 0 && pos == 1) {
    ui->chip01->setStyleSheet("background-color:red;");
  }
  if (side == 0 && pos == 2) {
    ui->chip02->setStyleSheet("background-color:red;");
  }
  if (side == 0 && pos == 3) {
    ui->chip03->setStyleSheet("background-color:red;");
  }
  if (side == 0 && pos == 4) {
    ui->chip04->setStyleSheet("background-color:red;");
  }
  if (side == 0 && pos == 5) {
    ui->chip05->setStyleSheet("background-color:red;");
  }
  if (side == 0 && pos == 6) {
    ui->chip06->setStyleSheet("background-color:red;");
  }
  if (side == 1 && pos == 0) {
    ui->chip10->setStyleSheet("background-color:red;");
  }
  if (side == 1 && pos == 1) {
    ui->chip11->setStyleSheet("background-color:red;");
  }
  if (side == 1 && pos == 2) {
    ui->chip12->setStyleSheet("background-color:red;");
  }
  if (side == 1 && pos == 3) {
    ui->chip13->setStyleSheet("background-color:red;");
  }
  if (side == 1 && pos == 4) {
    ui->chip14->setStyleSheet("background-color:red;");
  }
  if (side == 1 && pos == 5) {
    ui->chip15->setStyleSheet("background-color:red;");
  }
  if (side == 1 && pos == 6) {
    ui->chip16->setStyleSheet("background-color:red;");
  }
}

void MainWindow::color_green_IB(int position) {

  if (position == 0) {
    ui->chip0->setStyleSheet("background-color:green;");
  }
  if (position == 1) {
    ui->chip1->setStyleSheet("background-color:green;");
  }
  if (position == 2) {
    ui->chip2->setStyleSheet("background-color:green;");
  }
  if (position == 3) {
    ui->chip3->setStyleSheet("background-color:green;");
  }
  if (position == 4) {
    ui->chip4->setStyleSheet("background-color:green;");
  }
  if (position == 5) {
    ui->chip5->setStyleSheet("background-color:green;");
  }
  if (position == 6) {
    ui->chip6->setStyleSheet("background-color:green;");
  }
  if (position == 7) {
    ui->chip7->setStyleSheet("background-color:green;");
  }
  if (position == 8) {
    ui->chip8->setStyleSheet("background-color:green;");
  }
}

void MainWindow::color_red_IB(int position) {

  if (position == 0) {
    ui->chip0->setStyleSheet("background-color:red;");
  }
  if (position == 1) {
    ui->chip1->setStyleSheet("background-color:red;");
  }
  if (position == 2) {
    ui->chip2->setStyleSheet("background-color:red;");
  }
  if (position == 3) {
    ui->chip3->setStyleSheet("background-color:red;");
  }
  if (position == 4) {
    ui->chip4->setStyleSheet("background-color:red;");
  }
  if (position == 5) {
    ui->chip5->setStyleSheet("background-color:red;");
  }
  if (position == 6) {
    ui->chip6->setStyleSheet("background-color:red;");
  }
  if (position == 7) {
    ui->chip7->setStyleSheet("background-color:red;");
  }
  if (position == 8) {
    ui->chip8->setStyleSheet("background-color:red;");
  }
}

void MainWindow::test() {
  //  qDebug()<< "Testing ...";
}

void MainWindow::scanLoop(TScan *myScan) {
  myScan->Init();
  myScan->LoopStart(2);
  // QApplication::processEvents() ;
  while (myScan->Loop(2)) {
    myScan->PrepareStep(2);
    myScan->LoopStart(1);
    //  QApplication::processEvents() ;
    while (myScan->Loop(1)) {
      myScan->PrepareStep(1);
      myScan->LoopStart(0);
      //    QApplication::processEvents() ;
      while (myScan->Loop(0)) {
        myScan->PrepareStep(0);
        myScan->Execute();
        myScan->Next(0);
      }
      myScan->LoopEnd(0);
      myScan->Next(1);
    }
    myScan->LoopEnd(1);
    myScan->Next(2);
  }
  myScan->LoopEnd(2);
  myScan->Terminate();
}

void MainWindow::popup(QString message) {

  fWindowex = new Dialog(this);
  fWindowex->append(message);
  fWindowex->show();
}

void MainWindow::start_test() {
  if (writingdb == false) {
    fNoticewindow = new DBnotice(this);
    fNoticewindow->exec();
  }
  if (fAnalysisVector.size() >= 1) {
    for (unsigned int i = 0; i < fAnalysisVector.size(); i++) {
      delete fAnalysisVector.at(i);
    }
  }
  if (fScanVector.size() >= 1) {
    for (unsigned int i = 0; i < fScanVector.size(); i++) {
      delete fScanVector.at(i);
    }
  }
  if (fresultVector.size() >= 1) {
    for (unsigned int i = 0; i < fresultVector.size(); i++) {
      delete fresultVector.at(i);
    }
  }
  if (fEndurancemodules.size() > 0) {
    for (unsigned int i = 0; i < fEndurancemodules.size(); i++) {
      fEndurancemodules.at(i)->setText(" ");
    }
  }
  if (fActComponentTypeIDs.size() > 0) {
    fActComponentTypeIDs.clear();
  }
  if (fComponentIDs.size() > 0) {
    fComponentIDs.clear();
  }
  if (fScanbuttons.size() > 0) {
    for (unsigned int i = 0; i < fScanbuttons.size(); i++) {
      if (fScanbuttons.at(i)) {
        fScanbuttons.at(i)->hide();
      }
    }
  }
  fWritedb->setVisible(false);
  disconnect(fWritedb, SIGNAL(triggered()), this, SLOT(attachtodatabase()));
  fEndurancemodules.clear();
  fIdofactivitytype = 0;
  fIdoflocationtype = 0;
  fIdofoperator = 0;
  fLocdetails.clear();
  fHICs.clear();
  fAnalysisVector.clear();
  fScanVector.clear();
  fresultVector.clear();
  fChips.clear();
  fBoards.clear();
  fScanbuttons.clear();
  fHicnames.clear();
  fExecution = true;
  if (fPbcfgcheck != 0) {
    delete fPbcfgcheck;
  }
  if (fCalwindow != 0) {
    delete fCalwindow;
  }
  // if (databasewindow!=0){
  // delete databasewindow;
  // }
  // std::cout<<"why1"<<std::endl;
  if (fScanstatuslabels.size() >= 1) {
    for (unsigned int i = 0; i < fScanstatuslabels.size(); i++) {
      if (fScanstatuslabels.at(i) != 0) {
        fScanstatuslabels.at(i)->setText(" ");
      }
    }
  }
  // std::cout<<"why2"<<std::endl;
  fScanstatuslabels.clear();

  ui->tob->clear();
  ui->details->hide();
  ui->displaydetails->hide();

  disconnect(ui->start_test, SIGNAL(clicked()), this, SLOT(applytests()));
  ui->testtypeselected->clear();
  fHicidnumber = '\0';
  fToptwo = '\0';
  fTopthree = '\0';
  fTopfour = '\0';
  fTopfive = '\0';
  fBottomone = '\0';
  fBottomtwo = '\0';
  fBottomthree = '\0';
  fBottomfive = '\0';
  fBottomfour = '\0';

  if (fDatabasewindow == 0) {
    fDatabasewindow = new DatabaseSelection(this);
    fDatabasewindow->exec();
    fDatabasewindow->setdatabase(fDatabasetype);
  }
  std::cout << fDatabasetype << "the selected database" << std::endl;
  fSettingswindow = new TestSelection(this, fDatabasetype);
  fSettingswindow->exec();
}

void MainWindow::fillingOBvectors() {

  ClearVectors();

  AddScan(STPower);

  // FIFO and digital scan at three different supply voltages
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.1);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(0.9);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.0);
  fConfig->GetScanConfig()->SetMlvdsStrength(5);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetMlvdsStrength(ChipConfig::DCTRL_DRIVER);
  AddScan(STDigital);
  fConfig->GetScanConfig()->SetVoltageScale(1.1);
  AddScan(STDigital);
  fConfig->GetScanConfig()->SetVoltageScale(0.9);
  AddScan(STDigital);
  fConfig->GetScanConfig()->SetVoltageScale(1.0);

  // digital white frame
  AddScan(STDigitalWF);

  // threshold scans and tuning at 0V back bias
  fConfig->GetScanConfig()->SetBackBias(0.0);
  fConfig->GetScanConfig()->SetVcasnRange(30, 70);

  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 1);
  AddScan(STThreshold);
  AddScan(STVCASN);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 0);
  AddScan(STApplyVCASN, fresultVector.back());
  AddScan(STITHR);
  AddScan(STApplyITHR, fresultVector.back());
  AddScan(STThreshold);
  // noise occupancy with and without mask at 0V back bias
  AddScan(STNoise);
  AddScan(STApplyMask, fresultVector.back());
  AddScan(STNoise);
  AddScan(STClearMask);

  // threshold scans and tuning at 0V back bias
  fConfig->GetScanConfig()->SetBackBias(3.0);
  fConfig->GetScanConfig()->SetVcasnRange(75, 160);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 1);
  AddScan(STThreshold);
  AddScan(STVCASN);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 0);
  AddScan(STApplyVCASN, fresultVector.back());
  AddScan(STITHR);
  AddScan(STApplyITHR, fresultVector.back());
  AddScan(STThreshold);
  // noise occupancy with and without mask at 3V back bias
  AddScan(STNoise);
  AddScan(STApplyMask, fresultVector.back());
  AddScan(STNoise);
  AddScan(STClearMask);
  return;
}

void MainWindow::performtests(std::vector<TScan *> s, std::vector<TScanAnalysis *> a) {

  ui->statuslabel->setVisible(true);
  ui->statuslabel->update();

  for (unsigned int i = 0; i < fScanVector.size(); i++) {
    try {
      if (s.at(i) == 0) {
        a.at(i)->Initialize();
        std::thread analysisThread(&TScanAnalysis::Run, a[i]);
        analysisThread.join();
        a.at(i)->Finalize();
      } else {
        std::thread scanThread(&MainWindow::scanLoop, this, s[i]);
        a.at(i)->Initialize();
        std::thread analysisThread(&TScanAnalysis::Run, a[i]);
        scanThread.join();

        if (fScanstatuslabels.at(i) != 0) {
          fScanstatuslabels[i]->setText(fScanVector.at(i)->GetState());
          fScanstatuslabels[i]->update();
        }

        analysisThread.join();
        a.at(i)->Finalize();
        if (fScanstatuslabels.at(i) != 0) {
          fScanstatuslabels[i]->setText(fScanVector.at(i)->GetState());
        }
        // std::cout<<"The classification is : "<<fresultVector[i]->GetClassification()<<std::endl;
      }
      colorsinglescan(i);
      if (fExecution == false)
        break;
      qApp->processEvents();
    } catch (exception &ex) {
      std::cout << ex.what() << "is the thrown exception" << std::endl;
    }
  }
  qApp->processEvents();

  qApp->processEvents();
}

/*
// TODO:
// 1) fill combo box from source code, using insertItem
//    for each item associate correct TTestType as userData
// 2) change variable int numberofscan to TTestType testType
// 3) do *not* cast back to int ;-)
// 4) eliminate helper function GetTestType
void MainWindow::connectcombo(int value){
  counter=0;
  idofactivitytype=0;


  switch(value){
  case 0:
    ui->testtypeselected->clear();
    // qDebug()<<"No Test selected";
    ui->start_test->hide();
    settingswindow->hideendurance();
    break;
  case 1:{
    settingswindow->hideendurance();
    ui->testtypeselected->clear();
    ui->testtypeselected->setText("OB HIC Qualification Test");

    findidoftheactivitytype("OB HIC Qualification Test",idofactivitytype);
    std::cout<<"the id of the selected test: "<<idofactivitytype<<std::endl;
    locationcombo();

    settingswindow->connectlocationcombo(locdetails);


    numberofscan=1;

    break;}
  case 2:
    {
      settingswindow->hideendurance();
      ui->testtypeselected->clear();
      //   ui->start_test->show();
      // qDebug()<<"IB Qualification test selected";
      ui->testtypeselected->setText("IB HIC Qualification Test");
      findidoftheactivitytype("IB HIC Qualification Test",idofactivitytype);
      std::cout<<"the id of the selected test: "<<idofactivitytype<<std::endl;
      locationcombo();

      settingswindow->connectlocationcombo(locdetails);

      numberofscan=2;
      // openib();
      // open();//to be tested
      //  if (counter==0){break;}
      //  fillingOBvectors();//to be tested
      //Later no need to close the pop up window or to apply settings. everything will be done upon
th loading of the cfg.
      break;}

  case 3:
    {
      ui->testtypeselected->clear();
      //  ui->start_test->show();
      ui->testtypeselected->setText("OB Endurance Test");
      findidoftheactivitytype("OB HIC Endurance Test",idofactivitytype);
      locationcombo();
      settingswindow->connectlocationcombo(locdetails);
      settingswindow->adjustendurance();
      numberofscan=3;

      //Later no need to close the pop up window or to apply settings. everything will be done upon
th loading of the cfg.
      break;}

  case 4:
    {
      ui->testtypeselected->clear();
      //  ui->start_test->show();
      // qDebug()<<"IB Qualification test selected";
      //  ui->testtypeselected->setText("OB Reception Test");
      // openib();
      //Later no need to close the pop up window or to apply settings. everything will be done upon
th loading of the cfg.
      break;}

  case 5:
    {   settingswindow->hideendurance();
      ui->testtypeselected->clear();
      // ui->start_test->show();

      ui->testtypeselected->setText("OB HIC Reception Test");
      findidoftheactivitytype("OB HIC Reception Test",idofactivitytype);
      std::cout<<"the id of the selected test: "<<idofactivitytype<<std::endl;
      locationcombo();

      settingswindow->connectlocationcombo(locdetails);
      numberofscan=5;
      // open();
      //   if (counter==0){break;}
      //  fillingreceptionscans();
      //Later no need to close the pop up window or to apply settings. everything will be done upon
th loading of the cfg.
      break;}

  case 6:
    {
      ui->testtypeselected->clear();
      ui->start_test->show();
      numberofscan=6;
      ui->testtypeselected->setText("OB Powering Test");
      // open();
      //    if (counter==0){break;}
      //    poweringscan();
      //
      //  qDebug()<<"IB Qualification test selected";
      // openib();
      //Later no need to close the pop up window or to apply settings. everything will be done upon
th loading of the cfg.
      break;}

  case 7:
    {
      ui->testtypeselected->clear();
      ui->start_test->show();
      // qDebug()<<"IB Qualification test selected";
      // openib();
      //Later no need to close the pop up window or to apply settings. everything will be done upon
th loading of the cfg.
      break;}

  case 8:
    {
      ui->testtypeselected->clear();
      ui->start_test->show();
      //  qDebug()<<"IB Qualification test selected";
      // openib();
      //Later no need to close the pop up window or to apply settings. everything will be done upon
th loading of the cfg.
      break;}

  case 9:
    {
      ui->testtypeselected->clear();
      ui->start_test->show();
      // qDebug()<<"IB Qualification test selected";
      // openib();
      //Later no need to close the pop up window or to apply settings. everything will be done upon
th loading of the cfg.
      break;}
  case 10:
    {
      ui->testtypeselected->clear();
      ui->start_test->show();
      //  qDebug()<<"IB Qualification test selected";
      // openib();
      //Later no need to close the pop up window or to apply settings. everything will be done upon
th loading of the cfg.
      break;}

  }
  // ui->test1->setStyleSheet("color:orange;");

  //  ui->test1->setStyleSheet("color:orange;");
  // std::cout<<"is working"<<std::endl;}
  //connect(ui->start_test,SIGNAL(clicked()),this,SLOT(runscans()));
}*/

void MainWindow::applytests() {
  writingdb = false;
  for (unsigned int i = 0; i < fHICs.size(); i++) {
    if (fHICs.at(i)->IsEnabled()) {
      int oldtests;
      oldtests = DbCountActivities(fDB, fIdofactivitytype, fHicnames.at(i).toStdString());
      std::cout << "the number of old tests is " << oldtests << std::endl;
      fConfig->GetScanConfig()->SetRetestNumber(oldtests);
      makeDir((fConfig->GetScanConfig()->GetDataPath(fHicnames.at(i).toStdString())).c_str());
    }
  }

  fConfig->GetScanConfig()->SetTestType(fNumberofscan);
  fConfig->GetScanConfig()->SetDatabase(fDB);
  ui->start_test->hide();
  qApp->processEvents();
  fSignalMapper = new QSignalMapper(this);
  if (fNumberofscan == OBQualification) {
    fillingOBvectors();
  }
  if (fNumberofscan == IBQualification) {
    fillingibvectors();
  }
  if (fNumberofscan == OBReception) {
    fillingreceptionscans();
  }
  if (fNumberofscan == OBEndurance) {
    fillingendurancevectors();
  }

  qApp->processEvents();
  std::cout << "the size of the scan vector is: " << fScanVector.size() << std::endl;

  performtests(fScanVector, fAnalysisVector);
  //  colorscans();
  // if(numberofscan!=3){
  // connectscandetails();}
  // emit stopTimer();
  connect(fSignalMapper, SIGNAL(mapped(int)), this, SLOT(getresultdetails(int)));
  fResultwindow = new resultstorage(this);
  fResultwindow->exec();
}

void MainWindow::StopScan() { fScanAbort = true; }

void MainWindow::createbtn() {

  QPushButton *dimitra = new QPushButton("dsxfvgdvg", this);
  connect(dimitra, SIGNAL(clicked()), this, SLOT(StopScan()));
}

void MainWindow::getresultdetails(int i) {
  fMapdetails.clear();
  fMapd.clear();
  ui->details->clear();
  // scanposition=0;
  fScanposition = i;
  //  std::cout<<"this is the scan position"<<scanposition<<std::endl;
  // std::cout<<"before filling the combo"<<std::endl;
  std::map<const char *, TResultVariable> myvariables;
  myvariables = fAnalysisVector.at(fScanposition)->GetVariableList();
  // for (std::map<const char*,resultType>::const_iterator
  // it=fAnalysisVector.at(scanposition)->GetVariableList().begin();
  // it!=fAnalysisVector.at(scanposition)->GetVariableList().end(); ++it){
  // for (auto const &it : myvariables){
  for (std::map<const char *, TResultVariable>::const_iterator it = myvariables.begin();
       it != myvariables.end(); ++it) {
    //  std::cout << it->first << " => " << it->second << '\n';

    std::string d;
    d = (std::string(it->first));
    fMapd.push_back(std::make_pair(d, it->second));
  }
  for (auto const &v : fMapd) {
    // v is a reference to a vector element
    //   std::cout<<v.first<<std::endl;
    //   std::cout<<v.second<<std::endl;
    ui->details->addItem(v.first.c_str(), v.second);
    // std::cout<<"auto einai to v second: "<<v.second<<"kai auto einai to index pou
    // pairnw"<<ui->details->itemData(ui->details->currentIndex()).toInt()<<std::endl;
    // to xrisimopoiw mesa st compobox otan allazw index
    /* for(unsigned int i=0; i<fChips.size();i++){
    //  ->GetConfig()->GetChipId();
    int tautotita;
    if(fChips[i]->GetConfig()->IsEnabled()){
    tautotita=fChips[i]->GetConfig()->GetChipId()&0xf;

    // std::cout<<"i chipid pou tsekarw einai : "<<tautotita<<std::endl;
    std::cout<<"prepei na mou dswei enan integer gia t sigekrimeno chipid
    "<<fAnalysisVector.at(scanposition)->GetVariable("maroudiw",fChips[i]->GetConfig()->GetChipId()&0xf,v.second)<<std::endl;

    }}*/
    // at some point to check it
    // std::map<int,TScanResultChip* >::iterator itchip;
    //  std::map<std::string,TScanResultHic* >::iterator it;
    // for (it = fresultVector.at(3)->GetHicResults().begin(); it !=
    // fresultVector.at(3)->GetHicResults().end(); ++it) {
    //  TFifoResultHic *result = (TFifoResultHic*) it->second;
    //  std::cout<<"poio einai t apotelesma"<<result->GetVariable(16,v.second)<<std::endl;
    //   for (itchip = result->DeleteThisToo().begin(); itchip != result->DeleteThisToo().end();
    // ++itchip) {
    //  TFifoResultChip *resultChip = (TFifoResultChip*) itchip->second;
    //   resultChip->GetVariable(v.second);
    //   std::cout<<"the floats i want are "<<resultChip->GetVariable(v.second)<<std::endl;
    //}}
  }
  ui->details->show();
  qApp->processEvents();
  ui->displaydetails->show();
  qApp->processEvents();
}

/*
  void MainWindow::setVI(float * vcasn, float * ithr) {
  for(unsigned int i=0; i<fChips.size(); i++) {
  //fChips.at(i)->GetConfig()->SetParamValue(ithr[i]);
  //WIP...
  }
  }
*/

// TODO: check (i+1)-logic for case of fresultVector[i]==0)
// TODO: correct colour logic to *worst* HIC result, not last HIC result
void MainWindow::colorscans() {
  // std::vector<QPushButton*> scanbuttons;
  for (unsigned int i = 0; i < fScanVector.size(); i++) {
    if (fScanbuttons[i] != 0) {
      if (fresultVector[i] == 0) {
        for (std::map<std::string, TScanResultHic *>::iterator it =
                 fresultVector.at(i + 1)->GetHicResults().begin();
             it != fresultVector.at(i + 1)->GetHicResults().end(); ++it) {
          int colour;
          colour = it->second->GetClassification();
          // std::cout<<"no zero pointer "<< colour<<std::endl;
          if (colour == CLASS_ORANGE) {
            fScanbuttons[i]->setStyleSheet("color:orange;Text-align:left;border:none;");
            break;
          }
          if (colour == CLASS_GREEN) {
            fScanbuttons[i]->setStyleSheet("color:green;Text-align:left;border:none;");
            break;
          }
          if (colour == CLASS_RED) {
            fScanbuttons[i]->setStyleSheet("color:red;Text-align:left;border:none;");
            break;
          }
          if (colour == CLASS_UNTESTED) {
            fScanbuttons[i]->setStyleSheet("color:blue;Text-align:left;border:none;");
            break;
          }
        }
      } else {
        for (std::map<std::string, TScanResultHic *>::iterator it =
                 fresultVector.at(i)->GetHicResults().begin();
             it != fresultVector.at(i)->GetHicResults().end(); ++it) {
          int colour;
          colour = it->second->GetClassification();
          //  std::cout<<"no zero pointer "<< colour<<std::endl;
          if (colour == CLASS_ORANGE) {
            fScanbuttons[i]->setStyleSheet("color:orange;Text-align:left;border:none;");
            break;
          }
          if (colour == CLASS_GREEN) {
            fScanbuttons[i]->setStyleSheet("color:green;Text-align:left;border:none;");
            break;
          }
          if (colour == CLASS_RED) {
            fScanbuttons[i]->setStyleSheet("color:red;Text-align:left;border:none;");
            break;
          }
          if (colour == CLASS_UNTESTED) {
            fScanbuttons[i]->setStyleSheet("color:blue;Text-align:left;border:none;");
            break;
          }
        }
      }
    }
  }
}

THic *MainWindow::FindHic(std::string hicName) {
  for (unsigned int i = 0; i < fHICs.size(); i++) {
    if (!(fHICs.at(i)->GetDbId().compare(hicName)))
      return fHICs.at(i);
  }
  return 0;
}

// Combine the classifications inside the results of the different scans to one classification per
// HIC
// Actual combination "algorithm" is inside THic::AddClassification
// (currently: HIC classification = worst scan classification for that HIC)
void MainWindow::SetHicClassifications() {
  for (unsigned int i = 0; i < fresultVector.size(); i++) {
    TScanResult *scanResult = fresultVector.at(i);
    if (scanResult != 0) {
      // std::map<std::string, TScanResultHic*>::iterator it;
      for (unsigned int ihic = 0; ihic < fHICs.size(); ihic++) {
        TScanResultHic *hicResult = scanResult->GetHicResult(fHICs.at(ihic)->GetDbId());
        if (hicResult != 0) {
          fHICs.at(ihic)->AddClassification(hicResult->GetClassification());
        }
      }
    }
  }
}

// TODO: check that correct; probably dnumber not needed at all, since duplicate of
// ui->details->currentIndex()
// TODO: color chips according to result or add mouseover
void MainWindow::detailscombo(int dnumber) {
  (void)dnumber;
  int var = ui->details->itemData(ui->details->currentIndex()).toInt();
  TResultVariable rvar = static_cast<TResultVariable>(var);

  for (unsigned int i = 0; i < fChips.size(); i++) {
    if (fChips[i]->GetConfig()->IsEnabled()) {
      int tautotita = fChips[i]->GetConfig()->GetChipId() & 0xf;
      THic *hic = fChips.at(i)->GetHic();
      TScanResultHic *result = fresultVector.at(fScanposition)->GetHicResult(hic->GetDbId());
      std::cout << "The variable value of chip with ID " << tautotita
                << " is: " << result->GetVariable(tautotita, rvar) << std::endl;
    }
  }
}

void MainWindow::poweroff() {

  for (unsigned int i = 0; i < fHICs.size(); i++) {
    fHICs.at(i)->PowerOff();
  }
}

void MainWindow::quitall() {
  if (writingdb == false) {
    fNoticewindow = new DBnotice(this);
    fNoticewindow->adjustingtemplate();
    fNoticewindow->exec();
  }
  if (fHICs.size() >= 1) {
    poweroff();
    close();
  } else {
    close();
  }
}

void MainWindow::WriteToEos(string hicName, ActivityDB::actUri &uri) {
  char command[256];
  char path[256];
  string instFolder;
  string account = GetServiceAccount(fInstitute.toStdString(), instFolder);
  string testFolder = GetTestFolder();

  sprintf(command,
          "rsync -rv -e \"ssh -K\" %s %s@lxplus.cern.ch:/eos/project/a/alice-its/HicTests/%s/%s/%s",
          (fConfig->GetScanConfig()->GetDataPath(hicName)).c_str(), account.c_str(),
          testFolder.c_str(), instFolder.c_str(),
          (fConfig->GetScanConfig()->GetRemoteHicPath(hicName)).c_str());
  sprintf(path, "eos/project/a/alice-its/HicTests/%s/%s/%s", testFolder.c_str(), instFolder.c_str(),
          (fConfig->GetScanConfig()->GetRemoteHicPath(hicName)).c_str());

  uri.Description = "uri path";
  uri.Path = std::string(path);
  std::cout << "Trying to copy to eos with command " << command << std::endl;
  int status = system(command);
  std::cout << "Done, status code = " << status << std::endl;
}

// TODO: complete
string MainWindow::GetTestFolder() {
  switch (fNumberofscan) {
  case OBQualification:
    return string("OBQualification");
  case IBQualification:
    return string("IBQualification");
  case OBEndurance:
    return string("OBEndurance");
  case IBEndurance:
    return string("IBEndurance");
  case OBReception:
    return string("OBReception");
  default:
    return string("Unknown");
  }
}

// Temporary function
// TODO: to be eliminated once number of scan has been converted to TTestType variable
TTestType MainWindow::GetTestType() {
  switch (fNumberofscan) {
  case OBQualification:
    return OBQualification;
  case IBQualification:
    return IBQualification;
  case OBEndurance:
    return OBEndurance;
  case IBEndurance:
    return IBEndurance;
  case OBReception:
    return OBReception;
  default:
    return OBQualification;
  }
}

// TODO: use a map or sth more intelligent than this?
string MainWindow::GetServiceAccount(string institute, string &folder) {
  if (institute.find("CERN") != string::npos) {
    folder = string("CERN");
    return string("aliceits");
  } else if (institute.find("Wuhan") != string::npos) {
    folder = string("Wuhan");
    return string("aliceitswuhan");
  } else if (institute.find("Pusan") != string::npos) {
    folder = string("Pusan");
    return string("itspusan");
  } else if (institute.find("Bari") != string::npos) {
    folder = string("Bari");
    return string("aliceitsbari");
  } else if (institute.find("Strasbourg") != string::npos) {
    folder = string("Strasbourg");
    return string("aliceitssbg");
  } else if (institute.find("Liverpool") != string::npos) {
    folder = string("Liverpool");
    return string("aliceitslpool");
  } else if (institute.find("Frascati") != string::npos) {
    folder = string("Frascati");
    return string("aliceitslnf");
  } else if (institute.find("Berkeley") != string::npos) {
    folder = string("Berkeley");
    return string("aliceitslbl");
  } else if (institute.find("Nikhef") != string::npos) {
    folder = string("Nikhef");
    return string("itsnik");
  } else if (institute.find("Daresbury") != string::npos) {
    folder = string("Daresbury");
    return string("aliceitsdl");
  } else if (institute.find("Turin") != string::npos) {
    folder = string("Torino");
    return string("aliceitstorino");
  } else {
    folder = string("unknown");
    return string("unknown");
  }
}

// return time as 6-digit int in format HHMMSS
int MainWindow::GetTime() {
  time_t t = time(0);      // get time now
  tm *now = localtime(&t); // convert to tm structure
  int hours = now->tm_hour;
  int min = now->tm_min;
  int sec = now->tm_sec;
  int time = hours * 10000 + min * 100 + sec;

  return time;
}

void MainWindow::attachtodatabase() {
  if (fDB) {
    delete fDB;
  }
  fDB = new AlpideDB(fDatabasetype);
  if (fResultwindow->isVisible()) {
    fResultwindow->close();
  }

  if (fDB) delete fDB;
  fDB = new AlpideDB(fDatabasetype);
  SetHicClassifications();

  for (unsigned int i = 0; i < fHICs.size(); i++) {
    if (fHICs.at(i)->IsEnabled()) {
      QString comment;
      QDateTime date;
      ActivityDB::actUri uri;
      WriteToEos(fHICs.at(i)->GetDbId(), uri);
      bool status;
      fActivitywindow = new ActivityStatus(this);
      fActivitywindow->exec();
      fActivitywindow->getactivitystatus(status);
      fActivitywindow->GetComment(comment);
      std::string path;
      path = fConfig->GetScanConfig()->GetDataPath(fHicnames.at(i).toStdString()) + "/Comment.txt";
      fMfile = new QFile(QString::fromStdString(path));
      fMfile->open(QIODevice::ReadWrite);
      if (fMfile->isOpen()) {
        QByteArray buffer;
        buffer = buffer.append(comment);
        fMfile->write(buffer); // Writes a QByteArray to the file.
      }
      if (fMfile) {
        fMfile->close();
      }

      ActivityDB *myactivity = new ActivityDB(fDB);

      ActivityDB::activity activ;

      // TODO: check that the idof... are filled in the correct place
      // set activity parameters
      activ.Type = fIdofactivitytype;
      activ.Location = fIdoflocationtype;
      activ.User = fIdofoperator;
      activ.StartDate = date.currentDateTime().toTime_t();
      activ.EndDate = date.currentDateTime().toTime_t();
      activ.Lot = " ";
      activ.Name = CreateActivityName(fHICs.at(i)->GetDbId(), fConfig->GetScanConfig());
      activ.Position = " ";
      activ.Result =
          -999; // apparently has to stay open here, otherwise activity is considered closed
      if (status) {
        activ.Status = DbGetStatusId(fDB, fIdofactivitytype, "OPEN");
        std::cout << "the activ is open" << std::endl;
      } else {
        activ.Status = DbGetStatusId(fDB, fIdofactivitytype, "CLOSE");
        std::cout << "the activ is closed" << std::endl;
      }

      // add global parameters (not accessible from within results)
      DbAddParameter(fDB, activ, "Number of Working Chips", fHICs.at(i)->GetNEnabledChips());
      DbAddParameter(fDB, activ, "Time", GetTime());

      // loop over results and write to DB
      for (unsigned int j = 0; j < fresultVector.size(); j++) {
        if (fresultVector[j] != 0) {
          std::map<std::string, TScanResultHic *> mymap = fresultVector.at(j)->GetHicResults();
          for (auto ihic = mymap.begin(); ihic != mymap.end(); ++ihic) {
            TScanResultHic *result = (TScanResultHic *)ihic->second;
            if (ihic->first.compare(fHicnames.at(i).toStdString()) == 0) {
              result->WriteToDB(fDB, activ);
            }
          }
        }
      }

      // attach config file
      if (fNumberofscan == OBQualification || fNumberofscan == OBReception ||
          fNumberofscan == OBEndurance) {
        DbAddAttachment(fDB, activ, attachConfig, string("Config.cfg"), string("Config.cfg"));
      } else if (fNumberofscan == IBQualification) {
        DbAddAttachment(fDB, activ, attachConfig, string("Configib.cfg"), string("Configib.cfg"));
      }
      DbAddAttachment(fDB, activ, attachText, string(path), string("Comment.txt"));
      DbAddMember(fDB, activ, fIdofoperator);

      std::vector<ActivityDB::actUri> uris;

      uris.push_back(uri);

      myactivity->Create(&activ);
      cout << myactivity->DumpResponse() << endl;
      myactivity->AssignUris(activ.ID, fIdofoperator, (&uris));
      myactivity->AssignComponent(activ.ID, fComponentIDs.at(i), fActComponentTypeIDs.at(i).first,
                                  fIdofoperator);
      myactivity->AssignComponent(activ.ID, fComponentIDs.at(i), fActComponentTypeIDs.at(i).second,
                                  fIdofoperator);
      // TODO: add components (in / out)
      // TODO: add member
      // TODO: close activity (needs implementation of activityChange)
      //      e.g.
      //      activ.Status = DbGetStatusId(myDB, idofactivitytype, "CLOSED");
      //      myactivity->Change (&activ);
      //   check that activity ID (not activity type ID) is set within activ

      // std::cout << "trying to close activity" << std::endl;
      // activ.Status = DbGetStatusId(myDB, idofactivitytype, "CLOSED");
      activ.Result = DbGetResultId(fDB, fIdofactivitytype, fHICs.at(i)->GetClassification());
      std::cout << "the activity result is: " << activ.Result << std::endl;
      myactivity->Change(&activ);

      delete myactivity;
      delete fMfile;
    }
  }
  delete fDB;
  fDB = 0x0;
  writingdb = true;
}

void MainWindow::ClearVectors() {
  fScanVector.clear();
  fAnalysisVector.clear();
  fresultVector.clear();
  fScanbuttons.clear();
  fScanstatuslabels.clear();
}

void MainWindow::fillingreceptionscans() {
  ClearVectors();

  AddScan(STPower);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetMlvdsStrength(5);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetMlvdsStrength(ChipConfig::DCTRL_DRIVER);
  AddScan(STDigital);
}

void MainWindow::poweringscan() {
  ClearVectors();

  AddScan(STPower);
}

void MainWindow::findidoftheactivitytype(std::string activitytypename, int &id) {

  fDB = new AlpideDB(fDatabasetype);
  id = DbGetActivityTypeId(fDB, activitytypename);

  // delete myDB;
}

void MainWindow::locationcombo() {
  if (fLocdetails.size() > 0) {
    for (unsigned int i = 0; i < fLocdetails.size();
         i++) { // std::cout<<"clear locdetails"<<std::endl;
      fLocdetails.clear();
    }
  }
  // AlpideDB *myDB = new AlpideDB(databasetype);
  ActivityDB *myactivity = new ActivityDB(fDB);
  fLocationtypelist = myactivity->GetLocationTypeList(fIdofactivitytype);
  fLocdetails.push_back(std::make_pair(" ", 0));
  for (unsigned int i = 0; i < fLocationtypelist->size(); i++) {
    std::cout << "the location name is " << fLocationtypelist->at(i).Name << "and the ID is "
              << fLocationtypelist->at(i).ID << std::endl;
    fLocdetails.push_back(
        std::make_pair(fLocationtypelist->at(i).Name, fLocationtypelist->at(i).ID));
  }
  int projectid = 0;
  projectid = fDB->GetProjectId();
  if (fNumberofscan == OBQualification || fNumberofscan == OBEndurance ||
      fNumberofscan == OBReception) {
    fComponentTypeID = DbGetComponentTypeId(fDB, projectid, "Outer Barrel HIC Module");
  } else if (fNumberofscan == IBQualification || fNumberofscan == IBEndurance) {
    fComponentTypeID = DbGetComponentTypeId(fDB, projectid, "Inner Barrel HIC Module");
  }
  delete myactivity;
}

void MainWindow::savesettings() {
  fSettingswindow->hide();
  fSettingswindow->SaveSettings(fInstitute, fOperatorname, fHicidnumber, fCounter,
                                fIdoflocationtype, fIdofoperator, fToptwo, fTopthree, fTopfour,
                                fTopfive, fBottomone, fBottomtwo, fBottomthree, fBottomfour,
                                fBottomfive);
  if (fCounter == 0) {
    return;
  } else {
    open();

    for (unsigned int i = 0; i < fHICs.size(); i++) {
      fstop = false;
      int in = 0;
      int out = 0;
      int projectid = 0;
      int comp = 0;
      projectid = fDB->GetProjectId();
      in = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeID, "in");
      out = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeID, "out");
      comp = DbGetComponentId(fDB, projectid, fComponentTypeID, fHicnames.at(i).toStdString());
      if (comp == -1) {
        fComponentWindow = new Components(this);
        fComponentWindow->WriteToLabel(fHicnames.at(i));
        fComponentWindow->exec();
        if (fstop) {
          return;
        }
      }
      fActComponentTypeIDs.push_back(make_pair(in, out));
      fComponentIDs.push_back(comp);
      // std::cout<<"the component id is: "<< comp<<" or"<< fComponentIDs.at(i)<<std::endl;
    }
    fScanconfigwindow = new ScanConfiguration(this);
    fScanconfigwindow->show();
    setdefaultvalues(fScanfit, fNm);
    fScanconfigwindow->setdefaultspeed(fScanfit);
    fScanconfigwindow->setdeaulmaskstages(fNm);
    // delete myDB;
  }
  //  if (counter==0){return;}
  //  connect(ui->start_test,SIGNAL(clicked()),this,SLOT(applytests()));
  //  std::cout<<operatorname.toStdString()<<hicidnumber.toStdString()<<idoflocationtype<<idofoperator<<std::endl;
}

void MainWindow::speedycheck(bool checked) {

  if (checked) {
    fConfig->GetScanConfig()->SetParamValue("SPEEDY", "1");
    std::cout << "The speed is " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;

  } else {
    fConfig->GetScanConfig()->SetParamValue("SPEEDY", "0");
    std::cout << "The speed is " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;
  }
}

void MainWindow::loadeditedconfig() {

  fScanconfigwindow->setnumberofmaskstages(fNm);
  // std::cout<<"the nm that should change is "<<nm<<std::endl;
  if (fCounter == 0) {
    return;
  }
  //   std::cout<<"the nm that should change is "<<nm<<std::endl;
  std::string final;
  std::stringstream convert;
  convert << fNm;
  final = convert.str();
  fConfig->GetScanConfig()->SetParamValue("NMASKSTAGES", final.c_str());
  connect(ui->start_test, SIGNAL(clicked()), this, SLOT(applytests()));
  std::cout << fOperatorname.toStdString() << fHicidnumber.toStdString() << fIdoflocationtype
            << fIdofoperator << std::endl;
  std::cout << "the speed is set to " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;
  std::cout << "the number of mask stages is " << fConfig->GetScanConfig()->GetNMaskStages()
            << std::endl;
  fScanconfigwindow->close();
  ui->start_test->show();
}

void MainWindow::loaddefaultconfig() {

  fConfig->GetScanConfig()->SetParamValue("SPEEDY", "0");
  std::cout << "The speed is " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;
  if (fCounter == 0) {
    return;
  }
  connect(ui->start_test, SIGNAL(clicked()), this, SLOT(applytests()));
  std::cout << fOperatorname.toStdString() << fHicidnumber.toStdString() << fIdoflocationtype
            << fIdofoperator << std::endl;
  fScanconfigwindow->close();
}

void MainWindow::colorsinglescan(int i) {

  if (fScanbuttons[i] != 0) {
    if (fresultVector[i] == 0) {
      fColour = fAnalysisVector.at(i + 1)->GetClassification();
      if (fColour == CLASS_ORANGE) {
        fScanbuttons[i]->setStyleSheet("color:orange;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_GREEN) {
        fScanbuttons[i]->setStyleSheet("color:green;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_RED) {
        fScanbuttons[i]->setStyleSheet("color:red;Text-align:left;border:none;");

        fProgresswindow = new Testingprogress(this);
        fProgresswindow->setnotification(fScanVector.at(i)->GetName());
        fProgresswindow->exec();
      }
      if (fColour == CLASS_UNTESTED) {
        fScanbuttons[i]->setStyleSheet("color:blue;Text-align:left;border:none;");
        return;
      }

    } else {
      fColour = fAnalysisVector.at(i)->GetClassification();
      if (fColour == CLASS_ORANGE) {
        fScanbuttons[i]->setStyleSheet("color:orange;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_GREEN) {
        fScanbuttons[i]->setStyleSheet("color:green;Text-align:left;border:none;");
        // progresswindow= new Testingprogress(this);
        // progresswindow->setnotification(fScanVector.at(i)->GetName());
        //  progresswindow->exec();
      }
      if (fColour == CLASS_RED) {
        fScanbuttons[i]->setStyleSheet("color:red;Text-align:left;border:none;");
        fProgresswindow = new Testingprogress(this);
        fProgresswindow->setnotification(fScanVector.at(i)->GetName());
        fProgresswindow->exec();
      }
      if (fColour == CLASS_UNTESTED) {
        fScanbuttons[i]->setStyleSheet("color:blue;Text-align:left;border:none;");
        return;
      }
    }
  }
}

void MainWindow::writecalibrationfile() {
  TPowerBoard *powerBoard0 = fHICs.at(0)->GetPowerBoard();

  powerBoard0->GetConfigurationHandler()->WriteCalibrationFile();

  for (unsigned int i = 1; i < fHICs.size(); i++) { // check if 2nd powerunit used
    TPowerBoard *powerBoard = fHICs.at(i)->GetPowerBoard();
    if (powerBoard != powerBoard0) {
      powerBoard->GetConfigurationHandler()->WriteCalibrationFile();
      fCalwindow->close();
      return; // assume maximum of 2 power boards in setup
    }
  }
  fCalwindow->close();
}

// method sets the top bottom switch correctly for all power board configs
// input parameter unit gives the power unit for hic 0
// unit == 1 -> top, unit == 0 -> bottom
// loop over all HICs, if config different from the one of HIC 0 is found, set opposite
void MainWindow::setTopBottom(int unit) {
  TPowerBoardConfig *pbconfig0 = fHICs.at(0)->GetPowerBoard()->GetConfigurationHandler();
  TPowerBoardConfig *pbconfig;
  bool isBottom = (unit == 0);

  pbconfig0->SetIsBottom(isBottom);

  for (unsigned int i = 1; i < fHICs.size(); i++) {
    pbconfig = fHICs.at(i)->GetPowerBoard()->GetConfigurationHandler();
    if (pbconfig != pbconfig0) {
      pbconfig->SetIsBottom(!isBottom);
      return; // assume maximum of 2 power boards in setup
    }
  }
}

void MainWindow::setandgetcalibration() {

  float ares, gres, dres;
  TPowerBoard *powerBoard0 = fHICs.at(0)->GetPowerBoard();
  TPowerBoard *powerBoard1 = 0;

  fCalwindow->setresistances(ares, dres, gres);

  std::cout << ares << " input values" << dres << std::endl;
  // calwindow->setpowerunit(unit);
  // std::cout<<unit<<"number of the unit"<<std::endl;
  // setTopBottom (unit);

  for (unsigned int ihic = 0; ihic < fHICs.size(); ihic++) {
    TPowerBoard *powerBoard = fHICs.at(ihic)->GetPowerBoard();
    if (powerBoard != powerBoard0) { // second power unit used
      powerBoard1 = powerBoard;
    }
    powerBoard->GetConfigurationHandler()->EnterMeasuredLineResistances(fHICs.at(ihic)->GetPbMod(),
                                                                        ares, dres, gres);
    fPb->CalibrateVoltage(fHICs.at(ihic)->GetPbMod());
    fPb->CalibrateCurrent(fHICs.at(ihic)->GetPbMod());
  }

  powerBoard0->CalibrateBiasVoltage();
  powerBoard0->CalibrateBiasCurrent();

  if (powerBoard1 != 0) {
    powerBoard1->CalibrateBiasVoltage();
    powerBoard1->CalibrateBiasCurrent();
  }
  // display calibration of HIC 0
  float avscale, dvscale, avoffset, dvoffset, aioffset, dioffset;

  fPbconfig->GetVCalibration(fHICs.at(0)->GetPbMod(), avscale, dvscale, avoffset, dvoffset);

  fPbconfig->GetICalibration(fHICs.at(0)->GetPbMod(), aioffset, dioffset);

  fCalwindow->getcalibration(avscale, avoffset, dvscale, dvoffset, aioffset, dioffset);
}

void MainWindow::opencalibration() {
  fPbcfgcheck->close();
  fCalwindow = new Calibrationpb(this);
  fCalwindow->exec();
}

/*
  void MainWindow::createLabel(){

  if  (fHICs.size()>0) {

  QPixmap off("ledoff.png");
  QPixmap on("ledon.png");
  int w = ui->LED->width();
  int h = ui->LED->height();
  if (fHICs.at(0)->IsPowered()){
  ui->LED->setPixmap(on.scaled(w,h,Qt::KeepAspectRatio));  }
  else{
  ui->LED->setPixmap(off.scaled(w,h,Qt::KeepAspectRatio));
  }


  }}

*/

void MainWindow::exploreendurancebox() {

  for (unsigned int i = 0; i < fHICs.size(); i++) {

    if (fHICs[i]->IsEnabled()) {
      fEndurancemodules[i]->setText(fHicnames[i]);
      fEndurancemodules[i]->setStyleSheet("background-color:green;");
    } else {
      fEndurancemodules[i]->setStyleSheet("background-color:black;");
    }
  }
}

void MainWindow::setdefaultvalues(bool &fit, int &numberofstages) {

  fit = fConfig->GetScanConfig()->GetSpeedy();
  numberofstages = fConfig->GetScanConfig()->GetNMaskStages();
}

bool MainWindow::CreateScanObjects(TScanType scanType, TScanConfig *config, TScan **scan,
                                   TScanAnalysis **analysis, TScanResult **result,
                                   bool &hasButton) {
  switch (scanType) {
  case STPower:
    *scan = new TPowerTest(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TPowerResult();
    *analysis = new TPowerAnalysis(&fHistoQue, (TPowerTest *)*scan, config, fHICs, &fMutex,
                                   (TPowerResult *)*result);
    hasButton = true;
    return true;
  case STFifo:
    *scan = new TFifoTest(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TFifoResult();
    *analysis = new TFifoAnalysis(&fHistoQue, (TFifoTest *)*scan, config, fHICs, &fMutex,
                                  (TFifoResult *)*result);
    hasButton = true;
    return true;
  case STLocalBus:
    *scan = new TLocalBusTest(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TLocalBusResult();
    *analysis = new TLocalBusAnalysis(&fHistoQue, (TLocalBusTest *)*scan, config, fHICs, &fMutex,
                                      (TLocalBusResult *)*result);
    hasButton = true;
    return true;
  case STDigital:
    *scan = new TDigitalScan(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TDigitalResult();
    *analysis = new TDigitalAnalysis(&fHistoQue, (TDigitalScan *)*scan, config, fHICs, &fMutex,
                                     (TDigitalResult *)*result);
    hasButton = true;
    return true;
  case STDigitalWF:
    *scan = new TDigitalWhiteFrame(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TDigitalWFResult();
    *analysis = new TDigitalWFAnalysis(&fHistoQue, (TDigitalWhiteFrame *)*scan, config, fHICs,
                                       &fMutex, (TDigitalWFResult *)*result);
    hasButton = true;
    return true;
  case STThreshold:
    *scan = new TThresholdScan(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TSCurveResult();
    *analysis = new TSCurveAnalysis(&fHistoQue, (TThresholdScan *)*scan, config, fHICs, &fMutex,
                                    (TSCurveResult *)*result);
    hasButton = true;
    return true;
  case STVCASN:
    *scan = new TtuneVCASNScan(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TSCurveResult();
    *analysis = new TSCurveAnalysis(&fHistoQue, (TtuneVCASNScan *)*scan, config, fHICs, &fMutex,
                                    (TSCurveResult *)*result, 1);
    hasButton = true;
    return true;
  case STITHR:
    *scan = new TtuneITHRScan(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TSCurveResult();
    *analysis = new TSCurveAnalysis(&fHistoQue, (TtuneITHRScan *)*scan, config, fHICs, &fMutex,
                                    (TSCurveResult *)*result, -1);
    hasButton = true;
    return true;
  // apply tuning masks: scan = 0, analysis gets previous result as input
  // result value has to stay unchanged here; however AddScan will push back 0 into result vector
  case STApplyITHR:
    *scan = 0;
    *analysis =
        new TApplyITHRTuning(&fHistoQue, 0, config, fHICs, &fMutex, (TSCurveResult *)*result);
    hasButton = false;
    return true;
  case STApplyVCASN:
    *scan = 0;
    *analysis =
        new TApplyVCASNTuning(&fHistoQue, 0, config, fHICs, &fMutex, (TSCurveResult *)*result);
    hasButton = false;
    return true;
  case STNoise:
    *scan = new TNoiseOccupancy(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TNoiseResult();
    *analysis = new TNoiseAnalysis(&fHistoQue, (TNoiseOccupancy *)*scan, config, fHICs, &fMutex,
                                   (TNoiseResult *)*result);
    hasButton = true;
    return true;
  case STApplyMask:
    *scan = 0;
    *analysis = new TApplyMask(&fHistoQue, 0, config, fHICs, &fMutex, (TNoiseResult *)*result);
    hasButton = false;
    return true;
  case STClearMask:
    *scan = 0;
    *analysis = new TApplyMask(&fHistoQue, 0, config, fHICs, &fMutex, 0);
    hasButton = false;
    return true;
  case STReadout:
    *scan = new TReadoutTest(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TReadoutResult();
    *analysis = new TReadoutAnalysis(&fHistoQue, (TReadoutTest *)*scan, config, fHICs, &fMutex,
                                     (TReadoutResult *)*result);
    hasButton = true;
    return true;
  case STEndurance:
    *scan = new TEnduranceCycle(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result = new TCycleResult();
    *analysis = new TCycleAnalysis(&fHistoQue, (TEnduranceCycle *)*scan, config, fHICs, &fMutex,
                                   (TCycleResult *)*result);
    hasButton = true;
    return true;
  default:
    std::cout << "Warning: unknown scantype " << (int)scanType << ", ignoring" << std::endl;
    return false;
  }
}

int MainWindow::GetNButtons() {
  int result = 0;
  for (unsigned int i = 0; i < fScanbuttons.size(); i++) {
    if (fScanbuttons.at(i) != 0)
      result++;
  }
  return result;
}

void MainWindow::AddScan(TScanType scanType, TScanResult *aResult) {
  TScan *scan;
  TScanAnalysis *analysis;
  TScanResult *result = 0;
  TScanConfig *config = fConfig->GetScanConfig();
  bool hasButton;
  QPushButton *button;
  QLabel *label;

  if (aResult == 0) { // standard version
    if (CreateScanObjects(scanType, config, &scan, &analysis, &result, hasButton)) {
      fScanVector.push_back(scan);
      fAnalysisVector.push_back(analysis);
      fresultVector.push_back(result);
    }
  } else { // apply tuning etc: receive result from previous scan (tuning) and push 0 into result
           // vector
    if (CreateScanObjects(scanType, config, &scan, &analysis, &aResult, hasButton)) {
      fScanVector.push_back(scan);
      fAnalysisVector.push_back(analysis);
      fresultVector.push_back(0);
    }
  }

  if (hasButton) {
    button = new QPushButton(scan->GetName(), ui->centralWidget);
    button->setGeometry(QRect(0, 110 + 30 * GetNButtons(), 151, 31));
    button->setStyleSheet("border:none;");
    button->show();
    fSignalMapper->setMapping(button, fAnalysisVector.size() - 1);
    connect(button, SIGNAL(clicked()), fSignalMapper, SLOT(map()));
    fScanbuttons.push_back(button);

    label = new QLabel(ui->centralWidget);
    label->setObjectName(QStringLiteral());
    label->setGeometry(QRect(160, 110 + 30 * (GetNButtons() - 1), 181, 31));
    label->setText(scan->GetState());
    label->show();
    fScanstatuslabels.push_back(label);

  } else {
    fScanbuttons.push_back(0);
    fScanstatuslabels.push_back(0);
  }
}

void MainWindow::makeDir(const char *aDir) {
  struct stat myStat;
  if ((stat(aDir, &myStat) == 0) && (S_ISDIR(myStat.st_mode))) {
    std::cout << "Directory " << aDir << " found" << std::endl;
  } else {
    std::cout << "Directory " << aDir << " not found. Creating..." << std::endl;
    if (mkdir(aDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
      std::cout << "Error creating directory" << aDir << std::endl;
    }
  }
}

void MainWindow::IBBasicTest() {
  start_test();
  fDatabasetype = true;
  fSettingswindow->close();
  fNumberofscan = IBQualification;
  fHicidnumber = "IB_HIC";
  open();
  fConfig->GetScanConfig()->SetUseDataPath(false);
  ibscansforageing();
  for (unsigned int i = 0; i < fScanVector.size(); i++) {
    try {

      if (fScanVector.at(i) == 0) {

        fAnalysisVector.at(i)->Initialize();

        std::thread analysisThread(&TScanAnalysis::Run, fAnalysisVector[i]);

        analysisThread.join();

        fAnalysisVector.at(i)->Finalize();

      } else {

        std::thread scanThread(&MainWindow::scanLoop, this, fScanVector[i]);

        fAnalysisVector.at(i)->Initialize();

        std::thread analysisThread(&TScanAnalysis::Run, fAnalysisVector[i]);

        scanThread.join();

        analysisThread.join();

        fAnalysisVector.at(i)->Finalize();
      }
    } catch (exception &ex) {
      std::cout << ex.what() << "is the thrown exception" << std::endl;
    }
  }
  std::cout << "Test complete :D" << std::endl;
}

void MainWindow::IBParameterScan() {
  int backupTrigger = fConfig->GetScanConfig()->GetParamValue("NTRIG");

  fConfig->GetScanConfig()->SetParamValue("NTRIG", 10000);
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 1200);

  for (int pll = 0; pll < 3; pll++) {
    fConfig->GetScanConfig()->SetParamValue("READOUTPLLSTAGES", pll);
    fConfig->GetScanConfig()->SetVoltageScale(0.9);
    AddScan(STReadout);
    fConfig->GetScanConfig()->SetVoltageScale(1.0);
    AddScan(STReadout);
    // if (pll < 2) continue;
    // fConfig->GetScanConfig()->SetVoltageScale(1.1);
    // AddScan(STReadout);
  }
  fConfig->GetScanConfig()->SetVoltageScale(1.0);
  fConfig->GetScanConfig()->SetParamValue("READOUTPLLSTAGES", -1);
  fConfig->GetScanConfig()->SetParamValue("NTRIG", backupTrigger);
}

void MainWindow::fillingibvectors() {
  ClearVectors();
  AddScan(STPower);
  // Do this scan immediately after power as it sometimes crashes
  //IBParameterScan();
  // FIFO and digital scan at three different supply voltages
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.1);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(0.9);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.0);
  fConfig->GetScanConfig()->SetMlvdsStrength(5);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetMlvdsStrength(ChipConfig::DCTRL_DRIVER);
  AddScan(STDigital);
  fConfig->GetScanConfig()->SetVoltageScale(1.1);
  AddScan(STDigital);
  fConfig->GetScanConfig()->SetVoltageScale(0.9);
  AddScan(STDigital);
  fConfig->GetScanConfig()->SetVoltageScale(1.0);

  // digital white frame
  AddScan(STDigitalWF);
  // readout tests
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 600);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 2);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 1200);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 10);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 10);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 2);
  AddScan(STReadout);

  // reset previous values
  // (TODO: this is not exactly correct because it resets to the values defined in the header file
  // and
  // ignores the settings in the config file)
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 600);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", ChipConfig::DTU_DRIVER);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", ChipConfig::DTU_PREEMP);

  // threshold scan, no tuning for the time being, 0V back bias
  fConfig->GetScanConfig()->SetBackBias(0.0);
  fConfig->GetScanConfig()->SetVcasnRange(30, 70);

  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 1);
  AddScan(STThreshold);
  AddScan(STVCASN);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 0);
  AddScan(STApplyVCASN, fresultVector.back());
  AddScan(STITHR);
  AddScan(STApplyITHR, fresultVector.back());
  AddScan(STThreshold);

  // noise occupancy with and without mask at 0V back bias
  AddScan(STNoise);
  AddScan(STApplyMask, fresultVector.back());
  AddScan(STNoise);
  AddScan(STClearMask);
  // return;
  // threshold scan at 3V back bias, also here no tuning for the time being
  fConfig->GetScanConfig()->SetBackBias(3.0);
  fConfig->GetScanConfig()->SetVcasnRange(75, 160);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 1);
  AddScan(STThreshold);
  AddScan(STVCASN);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 0);
  AddScan(STApplyVCASN, fresultVector.back());
  AddScan(STITHR);
  AddScan(STApplyITHR, fresultVector.back());
  AddScan(STThreshold);

  // noise occupancy with and without mask at 3V back bias
  AddScan(STNoise);
  AddScan(STApplyMask, fresultVector.back());
  AddScan(STNoise);
  AddScan(STClearMask);
}

void MainWindow::fillingendurancevectors() {
  ClearVectors();

  AddScan(STFifo);
  AddScan(STDigital);
  AddScan(STThreshold);
}

void MainWindow::ConnectTestCombo(int value) {
  fCounter = 0;
  fIdofactivitytype = 0;
  ui->testtypeselected->clear();
  fSettingswindow->hideendurance();
  //  QString testname;
  fSettingswindow->GetTestTypeName(fNumberofscan, fTestname);
  ui->testtypeselected->setText(fTestname);
  std::string name;
  name.append(fTestname.toStdString());
  findidoftheactivitytype(name, fIdofactivitytype);
  std::cout << "the id of the selected test: " << fIdofactivitytype << std::endl;
  locationcombo();
  fSettingswindow->connectlocationcombo(fLocdetails);
  if (fNumberofscan == OBEndurance) {
    fSettingswindow->adjustendurance();
  }
  std::cout << "the numbeofscan is: " << fNumberofscan << "and the value is: " << value
            << std::endl;
}

void MainWindow::ContinueWithoutWriting() {
  fWritedb->setVisible(true);
  connect(fWritedb, SIGNAL(triggered()), this, SLOT(attachtodatabase()));
  fResultwindow->close();
  popup("You still have the possibility \n to write to the database \n later through the menu :)");
}

void MainWindow::finalwrite() {
  fNoticewindow->close();
  attachtodatabase();
}

void MainWindow::ibscansforageing() {
  ClearVectors();

  AddScan(STFifo);

  AddScan(STDigital);

  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 600);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 2);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 1200);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 10);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 10);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 2);
  AddScan(STReadout);

  // reset previous values
  // (TODO: this is not exactly correct because it resets to the values defined in the header file
  // and
  // ignores the settings in the config file)
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 600);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", ChipConfig::DTU_DRIVER);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", ChipConfig::DTU_PREEMP);

  // threshold scan, no tuning for the time being, 0V back bias
  fConfig->GetScanConfig()->SetBackBias(0.0);
  // fConfig->GetScanConfig()->SetVcasnRange (30, 70);

  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 1);
  AddScan(STThreshold);

  // threshold scan at 3V back bias, also here no tuning for the time being
  fConfig->GetScanConfig()->SetBackBias(3.0);
  // fConfig->GetScanConfig()->SetVcasnRange (75, 160);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 1);
  AddScan(STThreshold);
}

void MainWindow::quittest() {
  fComponentWindow->close();
  fstop = true;
}

AlpideDB *MainWindow::GetDB() { return fDB; }
