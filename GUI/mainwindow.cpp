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
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <future>
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
#include "TDCTRLAnalysis.h"
#include "TDCTRLMeasurement.h"
#include "TDigitalWFAnalysis.h"
#include "TEnduranceCycle.h"
#include "TEyeAnalysis.h"
#include "TEyeMeasurement.h"
#include "TFastPowerAnalysis.h"
#include "TFastPowerTest.h"
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  for (int i = 0; i < 7; i++) {
    fChkBtnObm[i] = false;
  }
  fPbcfgcheck       = 0;
  fCalwindow        = 0;
  fActivitywindow   = 0;
  fDatabasewindow   = 0;
  fNoticewindow     = 0;
  fPbnumberofmodule = 0;
  fDatabasefailure  = 0;

  std::string dataDir = "Data";
  if (const char *dataDirPrefix = std::getenv("ALPIDE_TEST_DATA")) dataDir = dataDirPrefix;
  makeDir(dataDir.c_str());

  ui->setupUi(this);
  this->setWindowTitle(QString::fromUtf8("GUI"));
  ui->tab_2->setEnabled(false);
  ui->example1->hide();
  ui->tab_3->setEnabled(true);
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
  QMenu *   menu1;
  menu1                  = menu->addMenu("&Options");
  QAction *newtestaction = new QAction("&New test", menu);
  fWritedb               = new QAction("&Write to database", menu);
  menu1->addAction(newtestaction);
  menu1->addAction(fWritedb);
  fWritedb->setVisible(false);
  ui->tabWidget->removeTab(2);
  ui->tabWidget->removeTab(1);
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
  connect(ui->details, SIGNAL(currentIndexChanged(int)), this, SLOT(detailscombo(int)));
  connect(ui->poweroff, SIGNAL(clicked(bool)), this, SLOT(poweroff()));

  ui->pbstatus->hide();

  QPixmap alice(":alicethreshold.png");
  int     w = ui->alicepic->width();
  int     h = ui->alicepic->height();
  ui->alicepic->setPixmap(alice.scaled(w, h, Qt::KeepAspectRatio));

  QPixmap alicelog(":logo.png");
  int     width  = ui->alicelogo->width();
  int     height = ui->alicelogo->height();
  ui->alicelogo->setPixmap(alicelog.scaled(width, height, Qt::KeepAspectRatio));

  ui->start_test->hide();

  connect(ui->testib, SIGNAL(clicked()), this, SLOT(IBBasicTest()));
  ui->testib->hide();
  writingdb    = true;
  fstopwriting = false;
  fstop        = false;
}

MainWindow::~MainWindow()
{
  delete ui;
  ui = 0x0;
}

// TODO: try to substitute numberofscan by TScanType (defined in TScanConfig.h)
void MainWindow::open()
{

  // settingswindow->hide();
  // settingswindow->SaveSettings(operatorname,hicidnumber,counter);
  // if (counter==0) {return;}
  QString fileName;
  if (fNumberofscan == OBQualification || fNumberofscan == OBEndurance ||
      fNumberofscan == OBReception) {
    fileName = "Config.cfg";
  }
  else if (fNumberofscan == IBQualification || fNumberofscan == IBDctrl ||
           fNumberofscan == IBStave) {
    fileName = "Configib.cfg";
  }
  else if (fNumberofscan == OBPower) {
    fileName = "ConfigPower.cfg";
  }
  else if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML ||
           fNumberofscan == OBHalfStaveOLFAST || fNumberofscan == OBHalfStaveMLFAST) {
    fileName = "Config_HS.cfg";
  }
  try {

    fHicnames.push_back(fHicidnumber);
    QByteArray  conv1 = fHicidnumber.toLatin1();
    QByteArray  conv2, conv3, conv4, conv5, conv6, conv7, conv8, conv9, conv10;
    const char *ar[10];
    ar[0] = {conv1.data()};
    if (fNumberofscan == OBEndurance) {
      fHicnames.clear();
      conv1  = fTopfive.toLatin1();
      ar[0]  = {conv1.data()};
      conv2  = fTopfour.toLatin1();
      ar[1]  = {conv2.data()};
      conv3  = fTopthree.toLatin1();
      ar[2]  = {conv3.data()};
      conv4  = fToptwo.toLatin1();
      ar[3]  = {conv4.data()};
      conv5  = fHicidnumber.toLatin1();
      ar[4]  = {conv5.data()};
      conv6  = fBottomfive.toLatin1();
      ar[5]  = {conv6.data()};
      conv7  = fBottomfour.toLatin1();
      ar[6]  = {conv7.data()};
      conv8  = fBottomthree.toLatin1();
      ar[7]  = {conv8.data()};
      conv9  = fBottomtwo.toLatin1();
      ar[8]  = {conv9.data()};
      conv10 = fBottomone.toLatin1();
      ar[9]  = {conv10.data()};
      fHicnames.push_back(fTopfive);
      fHicnames.push_back(fTopfour);
      fHicnames.push_back(fTopthree);
      fHicnames.push_back(fToptwo);
      fHicnames.push_back(fHicidnumber);
      fHicnames.push_back(fBottomfive);
      fHicnames.push_back(fBottomfour);
      fHicnames.push_back(fBottomthree);
      fHicnames.push_back(fBottomtwo);
      fHicnames.push_back(fBottomone);
      fEndurancemodules.push_back(ui->top5);
      fEndurancemodules.push_back(ui->top4);
      fEndurancemodules.push_back(ui->top3);
      fEndurancemodules.push_back(ui->top2);
      fEndurancemodules.push_back(ui->top1);
      fEndurancemodules.push_back(ui->down5);
      fEndurancemodules.push_back(ui->down4);
      fEndurancemodules.push_back(ui->down3);
      fEndurancemodules.push_back(ui->down2);
      fEndurancemodules.push_back(ui->down1);
    }
    if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML) {
      fHicnames.clear();
      int halfstaveidupper = 0;
      int halfstaveidlower = 0;
      int projectid        = 0;
      projectid            = fDB->GetProjectId();
      halfstaveidupper =
          DbGetComponentId(fDB, projectid, fComponentTypeIDa, fHalfstave.toStdString());

      halfstaveidlower =
          DbGetComponentId(fDB, projectid, fComponentTypeIDb, fHalfstave.toStdString());


      if (halfstaveidlower == -1) {
        fhalfstaveid  = halfstaveidupper;
        fhalfstavein  = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDa, "in");
        fhalfstaveout = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDa, "out");
      }
      else if (halfstaveidupper == -1) {
        fhalfstaveid  = halfstaveidlower;
        fhalfstavein  = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDb, "in");
        fhalfstaveout = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDb, "out");
      }
      DbGetListOfChildren(fDB, fhalfstaveid, fHalfstavemodules);
      if (fHalfstavemodules.size() < 1) {
        fHicnames.push_back("Module1");
        fHicnames.push_back("Module2");
        fHicnames.push_back("Module3");
        fHicnames.push_back("Module4");
        ar[0] = {"Module1"};
        ar[1] = {"Module2"};
        ar[2] = {"Module3"};
        ar[3] = {"Module4"};
        if (fNumberofscan == OBHalfStaveOL) {
          fHicnames.push_back("Module5");
          fHicnames.push_back("Module6");
          fHicnames.push_back("Module7");
          ar[4] = {"Module5"};
          ar[5] = {"Module6"};
          ar[6] = {"Module7"};
        }
      }
      else {

        fHicnames.resize(fHalfstavemodules.size() - 1, "empty");
        for (unsigned int i = 0; i < fHalfstavemodules.size(); i++) {
          if (fHalfstavemodules.at(i).Type !=
                  DbGetComponentTypeId(fDB, fDB->GetProjectId(), "Outer Layer CP") ||
              fHalfstavemodules.at(i).Type !=
                  DbGetComponentTypeId(fDB, fDB->GetProjectId(), "Middle Layer CP")) {
            if (fHalfstavemodules.at(i).Position) {
              int j    = fHalfstavemodules.at(i).Position - 1;
              int size = 0;
              size     = fHicnames.size();
              for (int d = 0; d < size; d++) {
                if (fHicnames[j] == "empty" && j == d) {
                  QString namestr = QString::fromStdString(fHalfstavemodules.at(i).Name);
                  fHicnames[j]    = namestr;
                  QByteArray name = namestr.toLatin1();
                  ar[j]           = strdup(name.toStdString().c_str());
                }
              }
            }
          }
        }
      }
    }

    if (fNumberofscan == OBHalfStaveOLFAST || fNumberofscan == OBHalfStaveMLFAST) {
      fHicnames.clear();
      fHicnames.push_back("Module1");
      fHicnames.push_back("Module2");
      fHicnames.push_back("Module3");
      fHicnames.push_back("Module4");
      ar[0] = strdup("Module1");
      ar[1] = strdup("Module2");
      ar[2] = strdup("Module3");
      ar[3] = strdup("Module4");
      if (fNumberofscan == OBHalfStaveOLFAST) {
        fHicnames.push_back("Module5");
        fHicnames.push_back("Module6");
        fHicnames.push_back("Module7");
        ar[4] = strdup("Module5");
        ar[5] = strdup("Module6");
        ar[6] = strdup("Module7");
      }
    }

    initSetup(fConfig, &fBoards, &fBoardType, &fChips, fileName.toStdString().c_str(), &fHICs, ar);
    fHiddenComponent = fConfig->GetScanConfig()->GetParamValue("TESTWITHOUTCOMP");
    fStatus          = fConfig->GetScanConfig()->GetParamValue("STATUS");
    if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML) {
      if (fhalfstaveid == -1) {
        fComponentWindow = new Components(this);
        fComponentWindow->WriteToLabel(fHalfstave);
        fComponentWindow->exec();
        if (fstop && fHiddenComponent == false) {
          return;
        }
      }
      for (unsigned int k = 0; k < fHicnames.size(); k++) {
        if (fHicnames.at(k) == "empty") {
          fComponentWindow = new Components(this);
          fComponentWindow->WrongPositions();
          fComponentWindow->exec();
          if (fstop && fHiddenComponent == false) {
            return;
          }
        }
      }
    }

    fConfig->GetScanConfig()->SetUseDataPath(true);
    fPb = fHICs.at(0)->GetPowerBoard();
    if (fPb) {
      fPbconfig         = fPb->GetConfigurationHandler();
      fPbnumberofmodule = fHICs.at(0)->GetPbMod();

      if (!fPb->IsCalibrated(fPbnumberofmodule)) {
        std::cout << "its not calibrated" << std::endl;
        fPbcfgcheck = new checkpbconfig(this);
        fPbcfgcheck->exec();
      }
    }
    fProperconfig = true;
  }
  catch (exception &e) {

    popup(e.what());
    fProperconfig = false;
  }

  if (fProperconfig == 1) {
    ui->tab_2->setEnabled(true);
    int device = 0;
    device     = fConfig->GetDeviceType();
    if (device == TYPE_OBHIC) {
      ui->tob->setText("Outer Barrel module");
      ui->OBModule->show();
      for (unsigned int i = 0; i < fChips.size(); i++) {
        int     chipid;
        uint8_t module, side, pos;
        chipid = fChips.at(i)->GetConfig()->GetChipId();
        if (fChips.at(i)->GetConfig()->IsEnabled()) {
          DecodeId(chipid, module, side, pos);
          color_green(side, pos);
        }
        else {
          DecodeId(chipid, module, side, pos);
          color_red(side, pos);
        }
      }
    }
    if (device == TYPE_IBHIC) {
      ui->tob->setText("Inner Barrel module");
      ui->IBModule->show();
      for (unsigned int i = 0; i < fChips.size(); i++) {
        int     chipid;
        uint8_t module, side, pos;
        chipid = fChips.at(i)->GetConfig()->GetChipId();
        if (fChips.at(i)->GetConfig()->IsEnabled()) {
          DecodeId(chipid, module, side, pos);
          color_green_IB(pos);
        }
        else {
          DecodeId(chipid, module, side, pos);
          color_red_IB(pos);
        }
      }
    }
    if (device == TYPE_HALFSTAVE || device == TYPE_MLHALFSTAVE) {
      ui->OBHALFSTAVE->show();
      int m1, m2, m3, m4, m5, m6, m7;
      m1 = m2 = m3 = m4 = m5 = m6 = m7 = 0;
      for (unsigned int i = 0; i < fChips.size(); i++) {
        int chipid;
        chipid = fChips.at(i)->GetConfig()->GetChipId();
        if (fChips.at(i)->GetConfig()->IsEnabled()) {
          explore_halfstave(chipid, m1, m2, m3, m4, m5, m6, m7);
        }
      }
    }

    if (device == TYPE_ENDURANCE) {
      ui->endurancebox->show();
      exploreendurancebox();
    }
  }
}


// TODO: Add module number to button data and eliminate button_obm#_clicked
// is the boolean fChkBtnObm needed at all?
void MainWindow::button_obm_clicked(int aModule)
{
  fChkBtnObm[aModule - 1] = true;
  ui->OBModule->show();
  ui->modulenumber->setText(QVariant(aModule).toString());
  if (fConfig->GetScanConfig()->GetParamValue("NMODULES") < aModule) {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 7; j++) {
        color_red(i, j);
      }
    }
  }
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int     chipid;
    uint8_t module, side, pos;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    DecodeId(chipid, module, side, pos);
    module = fConfig->GetChipConfigById(chipid)->GetModuleId();
    if (fChips.at(i)->GetConfig()->IsEnabled() && module == aModule) {
      color_green(side, pos);
    }
    else if (module == aModule) {
      color_red(side, pos);
    }
  }
}


void MainWindow::button_obm1_clicked() { button_obm_clicked(1); }

void MainWindow::button_obm2_clicked() { button_obm_clicked(2); }

void MainWindow::button_obm3_clicked() { button_obm_clicked(3); }

void MainWindow::button_obm4_clicked() { button_obm_clicked(4); }

void MainWindow::button_obm5_clicked() { button_obm_clicked(5); }

void MainWindow::button_obm6_clicked() { button_obm_clicked(6); }

void MainWindow::button_obm7_clicked() { button_obm_clicked(7); }


void MainWindow::explore_halfstave(uint8_t chipid, int &m1, int &m2, int &m3, int &m4, int &m5,
                                   int &m6, int &m7)
{

  uint8_t module, side, position;

  DecodeId(chipid, module, side, position);

  if (module == 1) {
    m1++;
    if (m1 > 0 && m1 < 14) {
      ui->obm1->setStyleSheet("background-color:orange;");
    }
    else if (m1 == 14) {
      ui->obm1->setStyleSheet("background-color:green;");
    }
    else if (m1 == 0) {
      ui->obm1->setStyleSheet("background-color:red;");
    }
  }

  if (module == 2) {
    m2++;
    if (m2 > 0 && m2 < 14) {
      ui->obm2->setStyleSheet("background-color:orange;");
    }
    else if (m2 == 14) {
      ui->obm2->setStyleSheet("background-color:green;");
    }
    else if (m2 == 0) {
      ui->obm2->setStyleSheet("background-color:red;");
    }
  }

  if (module == 3) {
    m3++;
    if (m3 > 0 && m3 < 14) {
      ui->obm3->setStyleSheet("background-color:orange;");
    }
    else if (m3 == 14) {
      ui->obm3->setStyleSheet("background-color:green;");
    }
    else if (m3 == 0) {
      ui->obm3->setStyleSheet("background-color:red;");
    }
  }

  if (module == 4) {
    m4++;
    if (m4 > 0 && m4 < 14) {
      ui->obm4->setStyleSheet("background-color:orange;");
    }
    else if (m4 == 14) {
      ui->obm4->setStyleSheet("background-color:green;");
    }
    else if (m4 == 0) {
      ui->obm4->setStyleSheet("background-color:red;");
    }
  }

  if (module == 5) {
    m5++;
    if (m5 > 0 && m5 < 14) {
      ui->obm5->setStyleSheet("background-color:orange;");
    }
    else if (m5 == 14) {
      ui->obm5->setStyleSheet("background-color:green;");
    }
    else if (m5 == 0) {
      ui->obm5->setStyleSheet("background-color:red;");
    }
  }

  if (module == 6) {
    m6++;
    if (m6 > 0 && m6 < 14) {
      ui->obm6->setStyleSheet("background-color:orange;");
    }
    else if (m6 == 14) {
      ui->obm6->setStyleSheet("background-color:green;");
    }
    else if (m6 == 0) {
      ui->obm6->setStyleSheet("background-color:red;");
    }
  }

  if (module == 7) {
    m7++;
    if (m7 > 0 && m7 < 14) {
      ui->obm7->setStyleSheet("background-color:orange;");
    }
    else if (m7 == 14) {
      ui->obm7->setStyleSheet("background-color:green;");
    }
    else if (m7 == 0) {
      ui->obm7->setStyleSheet("background-color:red;");
    }
  }
}

void MainWindow::DecodeId(const uint8_t chipId, uint8_t &module, uint8_t &side, uint8_t &position)
{
  module = (chipId & 0x70) >> 4;

  if (module == 0) { // IB module
    position = chipId & 0x0F;
    side     = 0;
    return;
  }
  // Must be an OB module here
  side     = (chipId & 0x08) >> 3;
  position = (chipId & 0x07);
  return;
}

void MainWindow::color_green(int side, int pos)
{

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

void MainWindow::color_red(int side, int pos)
{

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

void MainWindow::color_green_IB(int position)
{

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

void MainWindow::color_red_IB(int position)
{

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

void MainWindow::test()
{
  //  qDebug()<< "Testing ...";
}

void MainWindow::scanLoop(TScan *myScan)
{
  if (!fScanAbort) try {
      myScan->LoopStart(2);

      while (myScan->Loop(2)) {
        myScan->PrepareStep(2);
        qApp->processEvents();
        myScan->LoopStart(1);

        while (myScan->Loop(1)) {
          myScan->PrepareStep(1);
          myScan->LoopStart(0);

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
      // throw string("SDFdsfsdfsdfsdfsfsdf");
    }
    catch (exception &ex) {
      std::cout << ex.what() << " is the thrown exception from the scan" << std::endl;
      fExceptionthrown = true;
      fScanAbort       = true;
      fExceptiontext   = ex.what();
    }

  /*catch (string x) {
        std::cout << "DGFDGDFGD>>" << x << std::endl;
        fExceptionthrown = true;
        fScanAbort=true;
        fExceptiontext=x.c_str();
      }*/
}

void MainWindow::popup(QString message)
{

  fWindowex = new Dialog(this);
  fWindowex->append(message);
  fWindowex->hidequit();
  fWindowex->show();
}

void MainWindow::start_test()
{
  if (writingdb == false && fstopwriting == false) {
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
  if (fHalfstavemodules.size() > 0) {
    fHalfstavemodules.clear();
  }
  if (fScanTypes.size() > 0) {
    fScanTypes.clear();
  }
  fWritedb->setVisible(false);
  fHiddenComponent = false;
  fWrite           = false;
  fstop            = false;
  fstopwriting     = false;
  disconnect(fWritedb, SIGNAL(triggered()), this, SLOT(attachtodatabase()));
  fEndurancemodules.clear();
  fIdofactivitytype = 0;
  fIdoflocationtype = 0;
  fIdofoperator     = 0;
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

  if (fScanstatuslabels.size() >= 1) {
    for (unsigned int i = 0; i < fScanstatuslabels.size(); i++) {
      if (fScanstatuslabels.at(i) != 0) {
        fScanstatuslabels.at(i)->setText(" ");
      }
    }
  }

  fScanstatuslabels.clear();
  ui->OBModule->hide();
  ui->OBHALFSTAVE->hide();
  ui->IBModule->hide();
  ui->tob->clear();
  ui->details->hide();
  ui->displaydetails->hide();

  disconnect(ui->start_test, SIGNAL(clicked()), this, SLOT(applytests()));
  ui->testtypeselected->clear();
  fHicidnumber = '\0';
  fToptwo      = '\0';
  fTopthree    = '\0';
  fTopfour     = '\0';
  fTopfive     = '\0';
  fBottomone   = '\0';
  fBottomtwo   = '\0';
  fBottomthree = '\0';
  fBottomfive  = '\0';
  fBottomfour  = '\0';

  if (fDatabasewindow == 0) {
    fDatabasewindow = new DatabaseSelection(this);
    fDatabasewindow->exec();
    fDatabasewindow->setdatabase(fDatabasetype);
  }
  std::cout << fDatabasetype << " the selected database" << std::endl;
  fSettingswindow = new TestSelection(this, fDatabasetype);
  fSettingswindow->exec();
}

void MainWindow::fillingOBvectors()
{

  ClearVectors();
  AddScan(STPower);
  if (fConfig->GetScanConfig()->GetParamValue("TESTDCTRL")) AddScan(STDctrl);
  // FIFO and digital scan at three different supply voltages
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.1);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(0.9);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.0);
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

void MainWindow::performtests()
{
  fAddingScans = true;
  fExtraScans  = 0;
  ui->statuslabel->setVisible(true);
  ui->statuslabel->update();
  qApp->processEvents();
  fNewScans.clear();
  fInitialScans = fScanVector.size();

  for (unsigned int i = 0; i < fScanVector.size(); i++) {
    std::chrono::milliseconds delay(200);
    try {
      fExceptionthrown = false;
      fTestAgain       = false;
      if (fScanVector.at(i) == 0) {
        auto future_analysis =
            std::async(std::launch::async, &MainWindow::analysis, this, fAnalysisVector[i]);
        while (future_analysis.wait_for(delay) != std::future_status::ready)
          qApp->processEvents();
        future_analysis.get();

        try {
          fAnalysisVector.at(i)->Finalize();
        }
        catch (exception &ex) {
          std::cout << ex.what() << " is the thrown exception" << std::endl;
          fExceptionthrown = true;
          fScanAbort       = true;
          fExceptiontext   = ex.what();
        }
        colorsinglescan(i);
      }
      else {
        try {
          auto future_init = std::async(std::launch::async, &TScan::Init, fScanVector[i]);
          while (future_init.wait_for(delay) != std::future_status::ready)
            qApp->processEvents();
          future_init.get();
        }
        catch (exception &ex) {
          std::cout << ex.what() << " is the thrown exception from the scaninit" << std::endl;
          fExceptionthrown = true;
          fScanAbort       = true;
          fExceptiontext   = ex.what();
        }
        auto future_scan =
            std::async(std::launch::async, &MainWindow::scanLoop, this, fScanVector[i]);
        auto future_analysis =
            std::async(std::launch::async, &MainWindow::analysis, this, fAnalysisVector[i]);

        // non-blocking wait for scan and analysis
        // use to update status in the GUI and keep it responsive
        while (future_scan.wait_for(delay) != std::future_status::ready) {
          if (fScanstatuslabels.at(i) != 0) {
            fScanstatuslabels[i]->setText(fScanVector.at(i)->GetState());
            fScanstatuslabels[i]->update();
            if (fHistoQue.size() > 0)
              printf("%lu histogram(s) queued for analysis\n", fHistoQue.size());
          }
          qApp->processEvents();
        }
        future_scan.get();

        while (future_analysis.wait_for(delay) != std::future_status::ready)
          qApp->processEvents();
        future_analysis.get();

        try {
          fAnalysisVector.at(i)->Finalize();
        }
        catch (exception &ex) {
          std::cout << ex.what() << " is the thrown exception from th finalize" << std::endl;
          fExceptionthrown = true;
          fScanAbort       = true;
          fExceptiontext   = ex.what();
        }

        if (fScanstatuslabels.at(i) != 0) {
          fScanstatuslabels[i]->setText(fScanVector.at(i)->GetState());
          fScanstatuslabels[i]->update();
          qApp->processEvents();
        }
        if (fExceptionthrown) {
          fScanVector.at(i)->ClearHistoQue();
          notifyuser(i);
          if (fTestAgain) {
            TScanParameters *par;
            par = fScanVector.at(i)->GetParameters();
            AddScan(GetScanType(i));
            fScanVector.back()->SetParameters(par);
            fExtraScans++;
          }
        }


        if (fScanstatuslabels.at(i) != 0) {
          fScanstatuslabels[i]->setText(fScanVector.at(i)->GetState());
          qApp->processEvents();
        }
        colorsinglescan(i);
      }


      if (fExecution == false) {
        if (fresultVector.at(i) != 0) {
          for (unsigned int ihic = 0; ihic < fHICs.size(); ihic++) {

            TScanResultHic *hicResult;
            hicResult = fresultVector.at(i)->GetHicResult(fHICs.at(ihic)->GetDbId());
            if (hicResult != 0) {
              hicResult->SetClassification(CLASS_RED);
              // std::cout<<"auto pou pairnw einai"<<hicResult->GetClassification()<<std::endl;
            }
          }
        }
        return;
      }

      qApp->processEvents();
    }
    catch (exception &ex) {
      std::cout << ex.what() << " is the thrown exception" << std::endl;
    }
  }
  poweroff();
}

void MainWindow::applytests()
{
  writingdb = false;

  fConfig->GetScanConfig()->SetTestType(fNumberofscan);
  fConfig->GetScanConfig()->SetDatabase(fDB);
  std::string TestDir = fConfig->GetScanConfig()->GetTestDir();
  if (const char *dataDir = std::getenv("ALPIDE_TEST_DATA"))
    TestDir.insert(0, std::string(dataDir) + "/");
  else
    TestDir.insert(0, "Data/");
  makeDir(TestDir.c_str());

  for (unsigned int i = 0; i < fHICs.size(); i++) {
    // if ((fHICs.at(i)->IsEnabled()) || (fNumberofscan == OBPower)) {
    int oldtests;
    if (fNumberofscan == OBHalfStaveOLFAST || fNumberofscan == OBHalfStaveMLFAST) {
      oldtests = 0;
    }
    else {
      oldtests = DbCountActivities(fDB, fIdofactivitytype, fHicnames.at(i).toStdString());
    }
    std::cout << "the number of old tests is " << oldtests << std::endl;
    fConfig->GetScanConfig()->SetRetestNumber(fHicnames.at(i).toStdString(), oldtests);
    makeDir((fConfig->GetScanConfig()->GetDataPath(fHicnames.at(i).toStdString())).c_str());
    //}
  }

  ui->start_test->hide();
  qApp->processEvents();
  fSignalMapper = new QSignalMapper(this);
  if (fNumberofscan == OBQualification) {
    fillingOBvectors();
  }
  if (fNumberofscan == IBQualification || fNumberofscan == IBStave) {
    fillingibvectors();
  }
  if (fNumberofscan == OBReception) {
    fillingreceptionscans();
  }
  if (fNumberofscan == OBEndurance) {
    fillingendurancevectors();
  }
  if (fNumberofscan == OBPower) {
    fillingfastpower();
  }
  if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML) {
    fillingHSscans();
  }
  if (fNumberofscan == OBHalfStaveOLFAST || fNumberofscan == OBHalfStaveMLFAST) {
    fillingfastHS();
  }
  if (fNumberofscan == IBDctrl) {
    fillingDctrl();
  }
  qApp->processEvents();
  std::cout << "the size of the scan vector is: " << fScanVector.size() << std::endl;

  performtests();

  if (fNumberofscan == OBEndurance) {
    std::cout << "Combining endurance test results" << std::endl;
    CombineEnduranceResults();
    std::cout << "Done" << std::endl;
  }

  printClasses();

  connect(fSignalMapper, SIGNAL(mapped(int)), this, SLOT(getresultdetails(int)));

  if (fNumberofscan == OBHalfStaveOLFAST || fNumberofscan == OBHalfStaveMLFAST) {
    fstopwriting = true;
  }

  if (fstopwriting) {
    return;
  }
  fResultwindow = new resultstorage(this);
  fResultwindow->exec();
}

void MainWindow::StopScan() { fScanAbort = true; }

void MainWindow::createbtn()
{

  QPushButton *dimitra = new QPushButton("dsxfvgdvg", this);
  connect(dimitra, SIGNAL(clicked()), this, SLOT(StopScan()));
}

void MainWindow::getresultdetails(int i)
{
  fMapdetails.clear();
  fMapd.clear();
  ui->details->clear();
  fScanposition = i;

  std::map<const char *, TResultVariable> myvariables;
  myvariables = fAnalysisVector.at(fScanposition)->GetVariableList();

  for (std::map<const char *, TResultVariable>::const_iterator it = myvariables.begin();
       it != myvariables.end(); ++it) {


    std::string d;
    d = (std::string(it->first));
    fMapd.push_back(std::make_pair(d, it->second));
  }
  for (auto const &v : fMapd) {

    ui->details->addItem(v.first.c_str(), v.second);
  }
  ui->details->show();
  qApp->processEvents();
  ui->displaydetails->show();
  qApp->processEvents();
}


void MainWindow::printClasses()
{
  for (unsigned int iHic = 0; iHic < fHICs.size(); iHic++) {
    std::cout << std::endl
              << "Classifications HIC " << fHICs.at(iHic)->GetDbId() << ":" << std::endl;
    for (unsigned int iAnalysis = 0; iAnalysis < fAnalysisVector.size(); iAnalysis++) {
      if (fScanVector.at(iAnalysis) != 0) {
        fAnalysisVector.at(iAnalysis)->WriteHicClassToFile(fHICs.at(iHic)->GetDbId());
      }
    }
  }
}


// TODO: check (i+1)-logic for case of fresultVector[i]==0)
// TODO: correct colour logic to *worst* HIC result, not last HIC result
void MainWindow::colorscans()
{

  for (unsigned int i = 0; i < fScanVector.size(); i++) {
    if (fScanbuttons[i] != 0) {
      if (fresultVector[i] == 0) {
        for (std::map<std::string, TScanResultHic *>::iterator it =
                 fresultVector.at(i + 1)->GetHicResults()->begin();
             it != fresultVector.at(i + 1)->GetHicResults()->end(); ++it) {
          int colour;
          colour = it->second->GetClassification();
          if (colour == CLASS_GOLD) {
            fScanbuttons[i]->setStyleSheet("color:gold;Text-align:left;border:none;");
            return;
          }
          if (colour == CLASS_SILVER) {
            fScanbuttons[i]->setStyleSheet("color:silver;Text-align:left;border:none;");
            return;
          }
          if (colour == CLASS_RED) {
            fScanbuttons[i]->setStyleSheet("color:red;Text-align:left;border:none;");
            return;
          }
          if (colour == CLASS_UNTESTED) {
            fScanbuttons[i]->setStyleSheet("color:blue;Text-align:left;border:none;");
            return;
          }
          if (colour == CLASS_BRONZE) {
            fScanbuttons[i]->setStyleSheet("color:rgb(160,198,96);Text-align:left;border:none;");
            return;
          }
        }
      }
      else {
        for (std::map<std::string, TScanResultHic *>::iterator it =
                 fresultVector.at(i)->GetHicResults()->begin();
             it != fresultVector.at(i)->GetHicResults()->end(); ++it) {
          int colour;
          colour = it->second->GetClassification();
          if (colour == CLASS_GOLD) {
            fScanbuttons[i]->setStyleSheet("color:gold;Text-align:left;border:none;");
            return;
          }
          if (colour == CLASS_SILVER) {
            fScanbuttons[i]->setStyleSheet("color:silver;Text-align:left;border:none;");
            return;
          }
          if (colour == CLASS_RED) {
            fScanbuttons[i]->setStyleSheet("color:red;Text-align:left;border:none;");
            return;
          }
          if (colour == CLASS_UNTESTED) {
            fScanbuttons[i]->setStyleSheet("color:blue;Text-align:left;border:none;");
            return;
          }
          if (colour == CLASS_BRONZE) {
            fScanbuttons[i]->setStyleSheet("color:rgb(160,198,96);Text-align:left;border:none;");
            return;
          }
        }
      }
    }
  }
}

THic *MainWindow::FindHic(std::string hicName)
{
  for (unsigned int i = 0; i < fHICs.size(); i++) {
    if (!(fHICs.at(i)->GetDbId().compare(hicName))) return fHICs.at(i);
  }
  return 0;
}

// Combine the classifications inside the results of the different scans to one classification per
// HIC
// Actual combination "algorithm" is inside THic::AddClassification
// (currently: HIC classification = worst scan classification for that HIC)
void MainWindow::SetHicClassifications()
{
  for (unsigned int i = 0; i < fresultVector.size(); i++) {
    TScanResult *scanResult = fresultVector.at(i);
    if (scanResult != 0) {
      // std::map<std::string, TScanResultHic*>::iterator it;
      for (unsigned int ihic = 0; ihic < fHICs.size(); ihic++) {
        TScanResultHic *hicResult = scanResult->GetHicResult(fHICs.at(ihic)->GetDbId());
        if (hicResult != 0) {
          fHICs.at(ihic)->AddClassification(hicResult->GetClassification(),
                                            fScanVector.at(i)->HasBackBias());
        }
      }
    }
  }
}


// method to combine all results of the different endurance test slices.
// method searches for result of last slice (lastResult)
// all results are summed up in this lastResult
// all results except the last one are set to UNTESTED, this means:
//   - they do not influence the total classification
//   - they do not attempt to write the parameters to the DB
// the last result is reclassified after the summing procedure
void MainWindow::CombineEnduranceResults()
{
  int           lastEndurance = 0;
  TCycleResult *lastResult    = 0;
  for (unsigned int i = fresultVector.size() - 1; i <= fresultVector.size(); i--) {
    TCycleResult *scanResult = dynamic_cast<TCycleResult *>(fresultVector.at(i));
    if (!scanResult) continue;
    if (lastEndurance == 0) { // first endurance test result from vector end
      lastEndurance = i;
      lastResult    = scanResult;
      continue; // do not add the last result to itself
    }

    for (unsigned int ihic = 0; ihic < fHICs.size(); ihic++) {
      TCycleResultHic *hicResult =
          (TCycleResultHic *)scanResult->GetHicResult(fHICs.at(ihic)->GetDbId());
      TCycleResultHic *lastHicResult =
          (TCycleResultHic *)lastResult->GetHicResult(fHICs.at(ihic)->GetDbId());

      if ((!hicResult) || (!lastHicResult)) continue;
      lastHicResult->Add(*hicResult);
      hicResult->SetClassification(CLASS_UNTESTED);
    }
  }

  for (unsigned int ihic = 0; ihic < fHICs.size(); ihic++) {
    TCycleResultHic *lastHicResult =
        (TCycleResultHic *)lastResult->GetHicResult(fHICs.at(ihic)->GetDbId());
    TCycleAnalysis *lastAnalysis = (TCycleAnalysis *)fAnalysisVector.at(lastEndurance);
    lastAnalysis->ReClassify(lastHicResult);
  }
}


// TODO: check that correct; probably dnumber not needed at all, since duplicate of
// ui->details->currentIndex()
// TODO: color chips according to result or add mouseover
void MainWindow::detailscombo(int dnumber)
{
  (void)dnumber;
  int             var  = ui->details->itemData(ui->details->currentIndex()).toInt();
  TResultVariable rvar = static_cast<TResultVariable>(var);

  for (unsigned int i = 0; i < fChips.size(); i++) {
    if (fChips[i]->GetConfig()->IsEnabled()) {
      int             tautotita = fChips[i]->GetConfig()->GetChipId() & 0xf;
      THic *          hic       = fChips.at(i)->GetHic();
      TScanResultHic *result    = fresultVector.at(fScanposition)->GetHicResult(hic->GetDbId());
      std::cout << "The variable value of chip with ID " << tautotita
                << " is: " << result->GetVariable(tautotita, rvar) << std::endl;
    }
  }
}

void MainWindow::poweroff()
{
  std::cout << "Powering off all HICs" << std::endl;
  for (unsigned int i = 0; i < fHICs.size(); i++) {
    fHICs.at(i)->PowerOff();
  }
  std::cout << "Done." << std::endl;
}

void MainWindow::quitall()
{
  if (writingdb == false && fstopwriting == false) {
    fNoticewindow = new DBnotice(this);
    fNoticewindow->adjustingtemplate();
    fNoticewindow->exec();
  }

  poweroff();
  close();
}

void MainWindow::WriteToEos(string hicName, ActivityDB::actUri &uri, bool write)
{
  string instFolder;
  string account    = GetServiceAccount(fInstitute.toStdString(), instFolder);
  string testFolder = fConfig->GetScanConfig()->GetTestDir();
  if (write) {
    char command[256];
    sprintf(
        command,
        "rsync -rv -e \"ssh -K\" %s %s@lxplus.cern.ch:/eos/project/a/alice-its/HicTests/%s/%s/%s",
        (fConfig->GetScanConfig()->GetDataPath(hicName)).c_str(), account.c_str(),
        testFolder.c_str(), instFolder.c_str(),
        (fConfig->GetScanConfig()->GetRemoteHicPath(hicName)).c_str());
    std::cout << "Trying to copy to eos with command " << command << std::endl;
    int status = system(command);
    std::cout << "Done, status code = " << status << std::endl;
  }

  char path[256];
  sprintf(path, "eos/project/a/alice-its/HicTests/%s/%s/%s", testFolder.c_str(), instFolder.c_str(),
          (fConfig->GetScanConfig()->GetRemoteHicPath(hicName)).c_str());

  char uripath[256];
  sprintf(uripath, "http://cern.ch/hictests/%s/%s/%s", testFolder.c_str(), instFolder.c_str(),
          (fConfig->GetScanConfig()->GetRemoteHicPath(hicName)).c_str());
  uri.Description = "uri path";
  uri.Path        = std::string(uripath);
}


// Temporary function
// TODO: to be eliminated once number of scan has been converted to TTestType variable
TTestType MainWindow::GetTestType()
{
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
  case IBDctrl:
    return IBDctrl;
  default:
    return OBQualification;
  }
}

// TODO: use a map or sth more intelligent than this?
string MainWindow::GetServiceAccount(string institute, string &folder)
{
  if (institute.find("CERN") != string::npos) {
    folder = string("CERN");
    return string("aliceits");
  }
  else if (institute.find("Wuhan") != string::npos) {
    folder = string("Wuhan");
    return string("aliceitswuhan");
  }
  else if (institute.find("Pusan") != string::npos) {
    folder = string("Pusan");
    return string("itspusan");
  }
  else if (institute.find("Bari") != string::npos) {
    folder = string("Bari");
    return string("aliceitsbari");
  }
  else if (institute.find("Strasbourg") != string::npos) {
    folder = string("Strasbourg");
    return string("aliceitssbg");
  }
  else if (institute.find("Liverpool") != string::npos) {
    folder = string("Liverpool");
    return string("aliceitslpool");
  }
  else if (institute.find("Frascati") != string::npos) {
    folder = string("Frascati");
    return string("aliceitslnf");
  }
  else if (institute.find("Berkeley") != string::npos) {
    folder = string("Berkeley");
    return string("aliceitslbl");
  }
  else if (institute.find("Nikhef") != string::npos) {
    folder = string("Nikhef");
    return string("itsnik");
  }
  else if (institute.find("Daresbury") != string::npos) {
    folder = string("Daresbury");
    return string("aliceitsdl");
  }
  else if (institute.find("Turin") != string::npos) {
    folder = string("Torino");
    return string("aliceitstorino");
  }
  else {
    folder = string("unknown");
    return string("unknown");
  }
}

// return time as 6-digit int in format HHMMSS
int MainWindow::GetTime()
{
  time_t t     = time(0);       // get time now
  tm *   now   = localtime(&t); // convert to tm structure
  int    hours = now->tm_hour;
  int    min   = now->tm_min;
  int    sec   = now->tm_sec;
  int    time  = hours * 10000 + min * 100 + sec;

  return time;
}

void MainWindow::attachtodatabase()
{
  if (fWindowex != 0) {
    fWindowex->close();
  }
  fActivityResults.clear();
  fErrorMessages.clear();
  if (fResultwindow->isVisible()) {
    fResultwindow->close();
  }
  if (fDatabasefailure && fDatabasefailure->isVisible()) {
    fDatabasefailure->close();
  }
  if (fDB) delete fDB;
  fDB = new AlpideDB(fDatabasetype);
  SetHicClassifications();
  for (unsigned int i = 0; i < fHICs.size(); i++) {
    if (!fHicnames.at(i).isEmpty()) {
      if ((fHICs.at(i)->IsEnabled()) || (fNumberofscan == OBPower)) {
        QString            comment;
        QDateTime          date;
        ActivityDB::actUri uri;
        std::string        path;

        path =
            fConfig->GetScanConfig()->GetDataPath(fHicnames.at(i).toStdString()) + "/Comment.txt";

        QString currenthic;
        currenthic = fHicnames.at(i);
        QString oldclassific;
        oldclassific = GetResultType(fHICs.at(i)->GetOldClassification()).c_str();
        QString finalclassific;
        finalclassific = GetResultType(fHICs.at(i)->GetClassification()).c_str();
        std::vector<QString>          scansclassificationnames;
        std::vector<TScanResultHic *> hicresultsvector;
        for (unsigned int d = 0; d < fScanVector.size(); d++) {
          if (fAnalysisVector.at(d) != 0 && fresultVector.at(d) != 0) {
            if (fresultVector.at(d)->GetHicResult(currenthic.toStdString())) {
              QString         scanclasname;
              TScanResultHic *hicRe = fresultVector.at(d)->GetHicResult(currenthic.toStdString());
              scanclasname          = fScanVector.at(d)->GetName();
              scanclasname.append(" = ");
              scanclasname.append(hicRe->WriteHicClassification());
              scansclassificationnames.push_back(scanclasname);
              hicresultsvector.push_back(
                  fresultVector.at(d)->GetHicResult(currenthic.toStdString()));
            }
          }
        }
        if (!fWrite) {
          WriteToEos(fHICs.at(i)->GetDbId(), uri, true);
        }
        else {
          WriteToEos(fHICs.at(i)->GetDbId(), uri, false);
        }
        fActivitywindow = new ActivityStatus(this);
        if (fStatus) {
          fActivitywindow->DisplayStatusOptions();
        }
        fActivitywindow->PopulateWindow(currenthic, oldclassific, finalclassific,
                                        scansclassificationnames, hicresultsvector);
        fActivitywindow->exec();
        if (fStatus) {
          fActivitywindow->getactivitystatus(fStatus);
        }
        fActivitywindow->GetComment(comment);
        fMfile = new QFile(QString::fromStdString(path));
        fMfile->open(QIODevice::ReadWrite);
        if (fMfile->isOpen()) {
          QByteArray buffer;
          buffer = buffer.append(comment);
          fMfile->write(buffer); // Writes a QByteArray to the file.
        }
        if (fMfile) {
          fMfile->close();
          delete fMfile;
        }

        ActivityDB *myactivity = new ActivityDB(fDB);

        ActivityDB::activity activ;

        // TODO: check that the idof... are filled in the correct place
        // set activity parameters
        activ.Type      = fIdofactivitytype;
        activ.Location  = fIdoflocationtype;
        activ.User      = fIdofoperator;
        activ.StartDate = date.currentDateTime().toTime_t();
        activ.EndDate   = date.currentDateTime().toTime_t();
        activ.Lot       = " ";
        if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML) {
          std::string HSname;
          HSname = "HS_" + fHalfstave.toStdString() + "_" + fHICs.at(i)->GetDbId();
          std::cout << "the activty name is " << HSname << std::endl;
          activ.Name = CreateActivityName(HSname, fConfig->GetScanConfig());
        }
        else {
          activ.Name = CreateActivityName(fHICs.at(i)->GetDbId(), fConfig->GetScanConfig());
        }
        activ.Position = " ";
        activ.Result =
            -999; // apparently has to stay open here, otherwise activity is considered closed

        activ.Status = DbGetStatusId(fDB, fIdofactivitytype, "OPEN");
        std::cout << "the activity is open" << std::endl;


        // add global parameters (not accessible from within results)
        if (fresultVector[0]) {
          TScanResultHic *hicResult = fresultVector[0]->GetHicResult(fHICs.at(i)->GetDbId());
          if (hicResult) {
            DbAddParameter(fDB, activ, "Number of Working Chips", fHICs.at(i)->GetNEnabledChips(),
                           hicResult->GetParameterFile());
            DbAddParameter(fDB, activ, "Time", GetTime(), hicResult->GetParameterFile());
            DbAddParameter(fDB, activ, "Classification Version",
                           fConfig->GetScanConfig()->GetClassificationVersion(),
                           hicResult->GetParameterFile());
          }
        }
        // loop over results and write to DB
        for (unsigned int j = 0; j < fresultVector.size(); j++) {
          if (fresultVector[j] != 0) {
            std::map<std::string, TScanResultHic *> *mymap = fresultVector.at(j)->GetHicResults();
            for (auto ihic = mymap->begin(); ihic != mymap->end(); ++ihic) {
              if (ihic->first.compare(fHicnames.at(i).toStdString()) == 0) {
                TScanResultHic *result = (TScanResultHic *)ihic->second;
                result->WriteToDB(fDB, activ);
              }
            }
          }
        }

        // attach config, comment and classification file

        attachConfigFile(activ);
        DbAddAttachment(fDB, activ, attachText, string(path), string("Comment.txt"));

        path = fConfig->GetScanConfig()->GetDataPath(fHicnames.at(i).toStdString()) +
               "/Classification.dat";
        DbAddAttachment(fDB, activ, attachText, string(path), string("Classification.dat"));

        path = fConfig->GetScanConfig()->GetDataPath(fHicnames.at(i).toStdString()) +
               "/DBParameters.dat";
        DbAddAttachment(fDB, activ, attachText, string(path), string("DBParameters.dat"));


        DbAddMember(fDB, activ, fIdofoperator);


        std::vector<ActivityDB::actUri> uris;

        uris.push_back(uri);

        myactivity->Create(&activ);
        // cout << myactivity->DumpResponse() <<"is the response which should be -1"<<std::endl;
        if (myactivity->GetResponse().ErrorCode == -1) {
          QString errormessage;
          errormessage = "Activity Creation ";
          errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
          fActivityResults.push_back(-1);
          fErrorMessages.push_back(errormessage);
        }
        myactivity->AssignUris(activ.ID, fIdofoperator, (&uris));
        if (myactivity->GetResponse().ErrorCode == -1) {
          QString errormessage;
          errormessage = "Uri: ";
          errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
          fActivityResults.push_back(-1);
          fErrorMessages.push_back(errormessage);
        }
        myactivity->AssignComponent(activ.ID, fComponentIDs.at(i), fActComponentTypeIDs.at(i).first,
                                    fIdofoperator);


        if (myactivity->GetResponse().ErrorCode == -1) {
          QString errormessage;
          errormessage = "Input component: ";
          errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
          fActivityResults.push_back(-1);
          fErrorMessages.push_back(errormessage);
        }


        myactivity->AssignComponent(activ.ID, fComponentIDs.at(i),
                                    fActComponentTypeIDs.at(i).second, fIdofoperator);
        if (myactivity->GetResponse().ErrorCode == -1) {
          QString errormessage;
          errormessage = "Output comp: ";
          errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
          fActivityResults.push_back(-1);
          fErrorMessages.push_back(errormessage);
        }
        if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML) {
          myactivity->AssignComponent(activ.ID, fhalfstaveid, fhalfstavein, fIdofoperator);
          if (myactivity->GetResponse().ErrorCode == -1) {
            QString errormessage;
            errormessage = "Input HS: ";
            errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
            fActivityResults.push_back(-1);
            fErrorMessages.push_back(errormessage);
          }
          myactivity->AssignComponent(activ.ID, fhalfstaveid, fhalfstaveout, fIdofoperator);
          if (myactivity->GetResponse().ErrorCode == -1) {
            QString errormessage;
            errormessage = "Output HS: ";
            errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
            fActivityResults.push_back(-1);
            fErrorMessages.push_back(errormessage);
          }
        }

        for (unsigned int i = 0; i < fActivityResults.size(); i++) {
          if (fActivityResults.at(i) == -1) {
            fStatus = true;

            popup("The activity will remain open \n because of a problem during \n writing to db");

            break;
          }
        }
        if (fStatus == false) {
          activ.Status = DbGetStatusId(fDB, fIdofactivitytype, "CLOSED");
          std::cout << "the activity is closed" << std::endl;
        }

        activ.Result = DbGetResultId(fDB, fIdofactivitytype, fHICs.at(i)->GetClassification());
        myactivity->Change(&activ);

        if (myactivity->GetResponse().ErrorCode == -1) {
          QString errormessage;
          errormessage = "Error while changing activity";
          fErrorMessages.push_back(errormessage);
          fActivityResults.push_back(-1);
          popup("A problem occured!\nThe activity couldn't be changed.");
        }
        std::cout << "the activity result is: " << activ.Result << std::endl;
        fActivityResults.push_back(activ.Result);
        delete myactivity;
      }
    }


  } // for loops for hics
  delete fDB;
  fDB = 0x0;
  for (unsigned int i = 0; i < fActivityResults.size(); i++) {
    if (fActivityResults.at(i) != -1) {
      writingdb = true;
    }
    else {
      writingdb = false;
      break;
    }
  }
  // checking ...
  // writingdb=false;
  if (writingdb == false) {
    if (!fDatabasefailure) {
      fDatabasefailure = new Databasefailure(this);
    }
    QString dbmessages;

    for (unsigned int i = 0; i < fErrorMessages.size(); i++) {
      dbmessages.append(fErrorMessages.at(i));
      dbmessages.append("\n");
    }
    fDatabasefailure->assigningproblem(dbmessages);
    fDatabasefailure->exec();
  }
  fWrite = true;
}

void MainWindow::ClearVectors()
{
  for (auto scan : fScanVector)
    delete scan;
  fScanVector.clear();
  for (auto ana : fAnalysisVector)
    delete ana;
  fAnalysisVector.clear();
  for (auto res : fresultVector)
    delete res;
  fresultVector.clear();
  for (auto but : fScanbuttons)
    delete but;
  fScanbuttons.clear();
  for (auto lab : fScanstatuslabels)
    delete lab;
  fScanstatuslabels.clear();
  fScanTypes.clear();
}

void MainWindow::fillingreceptionscans()
{
  ClearVectors();
  AddScan(STPower);
  if (fConfig->GetScanConfig()->GetParamValue("TESTDCTRL")) AddScan(STDctrl);
  AddScan(STFifo);
  AddScan(STDigital);
}

void MainWindow::poweringscan()
{
  ClearVectors();

  AddScan(STPower);
}

void MainWindow::findidoftheactivitytype(std::string activitytypename, int &id)
{

  fDB = new AlpideDB(fDatabasetype);
  id  = DbGetActivityTypeId(fDB, activitytypename);
}

void MainWindow::locationcombo()
{
  if (fLocdetails.size() > 0) {
    for (unsigned int i = 0; i < fLocdetails.size(); i++) {
      fLocdetails.clear();
    }
  }

  ActivityDB *myactivity = new ActivityDB(fDB);
  fLocationtypelist      = myactivity->GetLocationTypeList(fIdofactivitytype);
  fLocdetails.push_back(std::make_pair(" ", 0));
  for (unsigned int i = 0; i < fLocationtypelist->size(); i++) {
    std::cout << "the location name is " << fLocationtypelist->at(i).Name << "and the ID is "
              << fLocationtypelist->at(i).ID << std::endl;
    fLocdetails.push_back(
        std::make_pair(fLocationtypelist->at(i).Name, fLocationtypelist->at(i).ID));
  }
  int projectid = 0;
  projectid     = fDB->GetProjectId();
  if (fNumberofscan == OBQualification || fNumberofscan == OBEndurance ||
      fNumberofscan == OBReception || fNumberofscan == OBPower) {
    fComponentTypeID = DbGetComponentTypeId(fDB, projectid, "Outer Barrel HIC Module");
  }
  else if (fNumberofscan == IBQualification || fNumberofscan == IBEndurance ||
           fNumberofscan == IBDctrl) {
    fComponentTypeID = DbGetComponentTypeId(fDB, projectid, "Inner Barrel HIC Module");
  }
  else if (fNumberofscan == IBStave) {
    fComponentTypeID = DbGetComponentTypeId(fDB, projectid, "IB Stave");
  }
  else if (fNumberofscan == OBHalfStaveOL) {
    fComponentTypeIDa = DbGetComponentTypeId(fDB, projectid, "Outer Layer Half-Stave Upper");
    fComponentTypeIDb = DbGetComponentTypeId(fDB, projectid, "Outer Layer Half-Stave Lower");
    fComponentTypeID  = DbGetComponentTypeId(fDB, projectid, "Outer Barrel HIC Module");
  }
  else if (fNumberofscan == OBHalfStaveML) {
    fComponentTypeIDa = DbGetComponentTypeId(fDB, projectid, "Middle Layer Half-Stave Upper");
    fComponentTypeIDb = DbGetComponentTypeId(fDB, projectid, "Middle Layer Half-Stave Lower");
    fComponentTypeID  = DbGetComponentTypeId(fDB, projectid, "Outer Barrel HIC Module");
  }
  delete myactivity;
}

void MainWindow::savesettings()
{
  fSettingswindow->hide();
  fSettingswindow->SaveSettings(fInstitute, fOperatorname, fHicidnumber, fCounter,
                                fIdoflocationtype, fIdofoperator, fToptwo, fTopthree, fTopfour,
                                fTopfive, fBottomone, fBottomtwo, fBottomthree, fBottomfour,
                                fBottomfive, fHalfstave);
  if (fCounter == 0) {
    return;
  }
  else {
    open();
    if (fstop && fHiddenComponent == false) {
      return;
    }
    if (fNumberofscan != OBHalfStaveOLFAST && fNumberofscan != OBHalfStaveMLFAST) {
      for (unsigned int i = 0; i < fHICs.size(); i++) {
        if (!fHicnames.at(i).isEmpty()) {
          fstopwriting  = false;
          int in        = 0;
          int out       = 0;
          int projectid = 0;
          int comp      = 0;
          projectid     = fDB->GetProjectId();
          in            = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeID, "in");
          out           = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeID, "out");
          comp = DbGetComponentId(fDB, projectid, fComponentTypeID, fHicnames.at(i).toStdString());

          if (comp == -1) {
            fComponentWindow = new Components(this);
            fComponentWindow->WriteToLabel(fHicnames.at(i));
            fComponentWindow->exec();
            if (fstop && fHiddenComponent == false) {
              return;
            }
          }

          fHICs.at(i)->SetOldClassification(DbGetPreviousCategory(fDB, comp, fIdofactivitytype));

          fActComponentTypeIDs.push_back(make_pair(in, out));
          fComponentIDs.push_back(comp);
        }


        else {
          fActComponentTypeIDs.push_back(make_pair(0, 0));
          fComponentIDs.push_back(0);
        }
      }
    }
    if (fHiddenComponent) {
      fstopwriting = false;
    }

    fScanconfigwindow = new ScanConfiguration(this);
    fScanconfigwindow->show();
    setdefaultvalues(fScanfit, fNm);
    fScanconfigwindow->setdefaultspeed(fScanfit);
    fScanconfigwindow->setdeaulmaskstages(fNm);
  }
}

void MainWindow::speedycheck(bool checked)
{

  if (checked) {
    fConfig->GetScanConfig()->SetParamValue("SPEEDY", "1");
    std::cout << "The speed is " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;
  }
  else {
    fConfig->GetScanConfig()->SetParamValue("SPEEDY", "0");
    std::cout << "The speed is " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;
  }
}

void MainWindow::loadeditedconfig()
{

  fScanconfigwindow->setnumberofmaskstages(fNm);

  if (fCounter == 0) {
    return;
  }

  std::string       final;
  std::stringstream convert;
  convert << fNm;
  final = convert.str();
  fConfig->GetScanConfig()->SetParamValue("NMASKSTAGES", final.c_str());
  connect(ui->start_test, SIGNAL(clicked()), this, SLOT(applytests()));
  std::cout << fOperatorname.toStdString() << ", " << fHicidnumber.toStdString() << ", "
            << fIdoflocationtype << ", " << fIdofoperator << std::endl;
  std::cout << "the speed is set to " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;
  std::cout << "the number of mask stages is " << fConfig->GetScanConfig()->GetNMaskStages()
            << std::endl;
  fScanconfigwindow->close();
  ui->start_test->show();
}

void MainWindow::loaddefaultconfig()
{

  fConfig->GetScanConfig()->SetParamValue("SPEEDY", "0");
  std::cout << "The speed is " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;
  if (fCounter == 0) {
    return;
  }
  connect(ui->start_test, SIGNAL(clicked()), this, SLOT(applytests()));
  std::cout << fOperatorname.toStdString() << ", " << fHicidnumber.toStdString() << ", "
            << fIdoflocationtype << ", " << fIdofoperator << std::endl;
  fScanconfigwindow->close();
}

void MainWindow::colorsinglescan(int i)
{

  if (fScanbuttons[i] != 0) {
    if (fresultVector[i] == 0) {
      fColour = fAnalysisVector.at(i + 1)->GetScanClassification();
      if (fColour == CLASS_GOLD) {
        fScanbuttons[i]->setStyleSheet("color:gold;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_SILVER) {
        fScanbuttons[i]->setStyleSheet("color:silver;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_RED) {
        fScanbuttons[i]->setStyleSheet("color:red;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_UNTESTED) {
        fScanbuttons[i]->setStyleSheet("color:blue;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_BRONZE) {
        fScanbuttons[i]->setStyleSheet("color:rgb(160,198,96);Text-align:left;border:none;");
        return;
      }
    }
    else {
      fColour = fAnalysisVector.at(i)->GetScanClassification();
      if (fColour == CLASS_GOLD) {
        fScanbuttons[i]->setStyleSheet("color:gold;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_SILVER) {
        fScanbuttons[i]->setStyleSheet("color:silver;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_RED) {
        fScanbuttons[i]->setStyleSheet("color:red;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_UNTESTED) {
        fScanbuttons[i]->setStyleSheet("color:blue;Text-align:left;border:none;");
        return;
      }
      if (fColour == CLASS_BRONZE) {
        fScanbuttons[i]->setStyleSheet("color:rgb(160,198,96);Text-align:left;border:none;");
        return;
      }
    }
  }
}

void MainWindow::writecalibrationfile()
{
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
void MainWindow::setTopBottom(int unit)
{
  TPowerBoardConfig *pbconfig0 = fHICs.at(0)->GetPowerBoard()->GetConfigurationHandler();
  TPowerBoardConfig *pbconfig;
  bool               isBottom = (unit == 0);

  pbconfig0->SetIsBottom(isBottom);

  for (unsigned int i = 1; i < fHICs.size(); i++) {
    pbconfig = fHICs.at(i)->GetPowerBoard()->GetConfigurationHandler();
    if (pbconfig != pbconfig0) {
      pbconfig->SetIsBottom(!isBottom);
      return; // assume maximum of 2 power boards in setup
    }
  }
}

void MainWindow::setandgetcalibration()
{

  float        ares, gres, dres;
  TPowerBoard *powerBoard0 = fHICs.at(0)->GetPowerBoard();
  TPowerBoard *powerBoard1 = 0;

  fCalwindow->setresistances(ares, dres, gres);

  std::cout << ares << " input values " << dres << std::endl;
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
    if ((fNumberofscan == OBHalfStaveOL) || (fNumberofscan == OBHalfStaveML)) {
      powerBoard->GetConfigurationHandler()->AddPowerBusResistances(fHICs.at(ihic)->GetPbMod());
    }
    powerBoard->CalibrateVoltage(fHICs.at(ihic)->GetPbMod());
    powerBoard->CalibrateCurrent(fHICs.at(ihic)->GetPbMod());
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

void MainWindow::opencalibration()
{
  fPbcfgcheck->close();
  fCalwindow = new Calibrationpb(this);
  fCalwindow->exec();
}


void MainWindow::exploreendurancebox()
{

  for (unsigned int i = 0; i < fHICs.size(); i++) {

    if (fHICs[i]->IsEnabled()) {
      int nchips;
      nchips = 0;
      fEndurancemodules[i]->setText(fHicnames[i]);
      nchips = fHICs[i]->GetNEnabledChips();
      if (nchips > 0 && nchips < 14) {
        fEndurancemodules[i]->setStyleSheet("background-color:orange;");
      }
      else if (nchips == 14) {
        fEndurancemodules[i]->setStyleSheet("background-color:green;");
      }
      else {
        fEndurancemodules[i]->setStyleSheet("background-color:red;");
      }
    }
    else {
      fEndurancemodules[i]->setStyleSheet("background-color:black;");
    }
  }
}

void MainWindow::setdefaultvalues(bool &fit, int &numberofstages)
{
  fit            = fConfig->GetScanConfig()->GetSpeedy();
  numberofstages = fConfig->GetScanConfig()->GetNMaskStages();
}

bool MainWindow::CreateScanObjects(TScanType scanType, TScanConfig *config, TScan **scan,
                                   TScanAnalysis **analysis, TScanResult **result, bool &hasButton)
{
  switch (scanType) {
  case STPower:
    *scan     = new TPowerTest(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TPowerResult();
    *analysis = new TPowerAnalysis(&fHistoQue, (TPowerTest *)*scan, config, fHICs, &fMutex,
                                   (TPowerResult *)*result);
    hasButton = true;
    return true;
  case STFifo:
    *scan     = new TFifoTest(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TFifoResult();
    *analysis = new TFifoAnalysis(&fHistoQue, (TFifoTest *)*scan, config, fHICs, &fMutex,
                                  (TFifoResult *)*result);
    hasButton = true;
    return true;
  case STDctrl:
    *scan     = new TDctrlMeasurement(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TDctrlResult();
    *analysis = new TDctrlAnalysis(&fHistoQue, (TDctrlMeasurement *)*scan, config, fHICs, &fMutex,
                                   (TDctrlResult *)*result);
    hasButton = true;
    return true;
  case STEyeScan:
    *scan     = new TEyeMeasurement(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TEyeResult();
    *analysis = new TEyeAnalysis(&fHistoQue, (TEyeMeasurement *)*scan, config, fHICs, &fMutex,
                                 (TEyeResult *)*result);
    hasButton = true;
    return true;
  case STLocalBus:
    *scan     = new TLocalBusTest(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TLocalBusResult();
    *analysis = new TLocalBusAnalysis(&fHistoQue, (TLocalBusTest *)*scan, config, fHICs, &fMutex,
                                      (TLocalBusResult *)*result);
    hasButton = true;
    return true;
  case STDigital:
    *scan     = new TDigitalScan(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TDigitalResult();
    *analysis = new TDigitalAnalysis(&fHistoQue, (TDigitalScan *)*scan, config, fHICs, &fMutex,
                                     (TDigitalResult *)*result);
    hasButton = true;
    return true;
  case STDigitalWF:
    *scan     = new TDigitalWhiteFrame(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TDigitalWFResult();
    *analysis = new TDigitalWFAnalysis(&fHistoQue, (TDigitalWhiteFrame *)*scan, config, fHICs,
                                       &fMutex, (TDigitalWFResult *)*result);
    hasButton = true;
    return true;
  case STThreshold:
    *scan     = new TThresholdScan(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TSCurveResult();
    *analysis = new TSCurveAnalysis(&fHistoQue, (TThresholdScan *)*scan, config, fHICs, &fMutex,
                                    (TSCurveResult *)*result);
    hasButton = true;
    return true;
  case STVCASN:
    *scan     = new TtuneVCASNScan(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TSCurveResult();
    *analysis = new TSCurveAnalysis(&fHistoQue, (TtuneVCASNScan *)*scan, config, fHICs, &fMutex,
                                    (TSCurveResult *)*result, 1);
    hasButton = true;
    return true;
  case STITHR:
    *scan     = new TtuneITHRScan(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TSCurveResult();
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
    *scan     = new TNoiseOccupancy(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TNoiseResult();
    *analysis = new TNoiseAnalysis(&fHistoQue, (TNoiseOccupancy *)*scan, config, fHICs, &fMutex,
                                   (TNoiseResult *)*result);
    hasButton = true;
    return true;
  case STApplyMask:
    *scan     = 0;
    *analysis = new TApplyMask(&fHistoQue, 0, config, fHICs, &fMutex, (TNoiseResult *)*result);
    hasButton = false;
    return true;
  case STClearMask:
    *scan     = 0;
    *analysis = new TApplyMask(&fHistoQue, 0, config, fHICs, &fMutex, 0);
    hasButton = false;
    return true;
  case STReadout:
    *scan     = new TReadoutTest(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TReadoutResult();
    *analysis = new TReadoutAnalysis(&fHistoQue, (TReadoutTest *)*scan, config, fHICs, &fMutex,
                                     (TReadoutResult *)*result);
    hasButton = true;
    return true;
  case STEndurance:
    *scan     = new TEnduranceCycle(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TCycleResult();
    *analysis = new TCycleAnalysis(&fHistoQue, (TEnduranceCycle *)*scan, config, fHICs, &fMutex,
                                   (TCycleResult *)*result);
    hasButton = true;
    return true;
  case STFastPowerTest:
    *scan     = new TFastPowerTest(config, fChips, fHICs, fBoards, &fHistoQue, &fMutex);
    *result   = new TFastPowerResult();
    *analysis = new TFastPowerAnalysis(&fHistoQue, (TFifoTest *)*scan, config, fHICs, &fMutex,
                                       (TFastPowerResult *)*result);
    hasButton = true;
    return true;
  default:
    std::cout << "Warning: unknown scantype " << (int)scanType << ", ignoring" << std::endl;
    return false;
  }
}

int MainWindow::GetNButtons()
{
  int result = 0;
  for (unsigned int i = 0; i < fScanbuttons.size(); i++) {
    if (fScanbuttons.at(i) != 0) result++;
  }
  return result;
}

void MainWindow::AddScan(TScanType scanType, TScanResult *aResult)
{
  TScan *        scan;
  TScanAnalysis *analysis;
  TScanResult *  result = 0;
  TScanConfig *  config = fConfig->GetScanConfig();
  bool           hasButton;
  QPushButton *  button;
  QLabel *       label;

  if (aResult == 0) { // standard version
    if (CreateScanObjects(scanType, config, &scan, &analysis, &result, hasButton)) {
      fScanVector.push_back(scan);
      fAnalysisVector.push_back(analysis);
      fresultVector.push_back(result);
      fScanTypes.push_back(scanType);
    }
  }
  else { // apply tuning etc: receive result from previous scan (tuning) and push 0 into result
         // vector
    if (CreateScanObjects(scanType, config, &scan, &analysis, &aResult, hasButton)) {
      fScanVector.push_back(scan);
      fAnalysisVector.push_back(analysis);
      fresultVector.push_back(0);
      fScanTypes.push_back(scanType);
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
  }
  else {
    fScanbuttons.push_back(0);
    fScanstatuslabels.push_back(0);
  }
}

void MainWindow::makeDir(const char *aDir)
{
  struct stat myStat;
  if ((stat(aDir, &myStat) == 0) && (S_ISDIR(myStat.st_mode))) {
    std::cout << "Directory " << aDir << " found" << std::endl;
  }
  else {
    std::cout << "Directory " << aDir << " not found. Creating..." << std::endl;
    if (mkdir(aDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
      std::cout << "Error creating directory" << aDir << std::endl;
    }
  }
}

void MainWindow::IBBasicTest()
{
  start_test();
  fDatabasetype = true;
  fSettingswindow->close();
  fNumberofscan = IBQualification;
  fHicidnumber  = "IB_HIC";
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
      }
      else {
        fScanVector[i]->Init();
        std::thread scanThread(&MainWindow::scanLoop, this, fScanVector[i]);

        fAnalysisVector.at(i)->Initialize();

        std::thread analysisThread(&TScanAnalysis::Run, fAnalysisVector[i]);

        scanThread.join();

        analysisThread.join();

        fAnalysisVector.at(i)->Finalize();
      }
    }
    catch (exception &ex) {
      std::cout << ex.what() << " is the thrown exception" << std::endl;
    }
  }
  std::cout << "Test complete :D" << std::endl;
}

void MainWindow::IBParameterScan()
{
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

void MainWindow::fillingibvectors()
{
  ClearVectors();
  AddScan(STPower);
  // if (fConfig->GetScanConfig()->GetParamValue("TESTDCTRL")) AddScan(STDctrl);
  // Do this scan immediately after power as it sometimes crashes
  if (fConfig->GetScanConfig()->GetParamValue("TESTDCTRL")) AddScan(STDctrl);
  // IBParameterScan();
  // FIFO and digital scan at three different supply voltages
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.1);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(0.9);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.0);
  fConfig->GetScanConfig()->SetMlvdsStrength(ChipConfig::DCTRL_DRIVER);
  AddScan(STDigital);
  fConfig->GetScanConfig()->SetVoltageScale(1.1);
  AddScan(STDigital);
  fConfig->GetScanConfig()->SetVoltageScale(0.9);
  AddScan(STDigital);
  fConfig->GetScanConfig()->SetVoltageScale(1.0);

  // digital white frame
  AddScan(STDigitalWF);

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

  // eye diagram
  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 8);
  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 0);
  AddScan(STEyeScan);
  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 4);
  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 0);
  AddScan(STEyeScan);
  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 8);
  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 10);
  AddScan(STEyeScan);
  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 12);
  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 0);
  AddScan(STEyeScan);
  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 12);
  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 10);
  AddScan(STEyeScan);

  // readout tests
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 600);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 2);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 1200);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 8);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 0);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 8);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 10);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 12);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 0);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 12);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 10);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 6);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 0);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 4);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 0);
  AddScan(STReadout);

  // reset previous values
  // (TODO: this is not exactly correct because it resets to the values defined in the header file
  // and
  // ignores the settings in the config file)
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 600);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", ChipConfig::DTU_DRIVER);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", ChipConfig::DTU_PREEMP);
}

void MainWindow::fillingendurancevectors()
{
  int nSlices = fConfig->GetScanConfig()->GetParamValue("ENDURANCESLICES");
  ClearVectors();

  AddScan(STFifo);

  for (int i = 0; i < nSlices; i++) {
    AddScan(STEndurance);
  }
}

void MainWindow::ConnectTestCombo(int value)
{
  fCounter          = 1;
  fIdofactivitytype = 0;
  ui->testtypeselected->clear();
  fSettingswindow->hideendurance();
  fSettingswindow->GetTestTypeName(fNumberofscan, fTestname);
  ui->testtypeselected->setText(fTestname);
  std::string name;
  name.append(fTestname.toStdString());
  if (fNumberofscan != OBHalfStaveOLFAST && fNumberofscan != OBHalfStaveMLFAST) {
    findidoftheactivitytype(name, fIdofactivitytype);
  }
  // fIdofactivitytype=-1;
  if (fIdofactivitytype == -1) {
    fSettingswindow->getwindow();
    fCounter = fSettingswindow->getcounter();
  }
  if (fCounter == 0) {
    fSettingswindow->close();
    return;
  }
  std::cout << "the id of the selected test: " << fIdofactivitytype << std::endl;
  if (fNumberofscan != OBHalfStaveOLFAST && fNumberofscan != OBHalfStaveMLFAST) {
    locationcombo();
    fSettingswindow->connectlocationcombo(fLocdetails);
  }
  if (fNumberofscan == OBEndurance) {
    fSettingswindow->adjustendurance();
  }
  std::cout << "the numbeofscan is: " << fNumberofscan << "and the value is: " << value
            << std::endl;
}

void MainWindow::ContinueWithoutWriting()
{
  fWritedb->setVisible(true);
  connect(fWritedb, SIGNAL(triggered()), this, SLOT(attachtodatabase()));
  fResultwindow->close();
  popup("You still have the possibility \n to write to the database \n later through the menu :)");
}

void MainWindow::finalwrite()
{
  fNoticewindow->close();
  attachtodatabase();
}

void MainWindow::ibscansforageing()
{
  ClearVectors();

  AddScan(STFifo);

  AddScan(STDigital);

  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 600);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 2);
  AddScan(STReadout);
  // fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 1200);
  // fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 10);
  // fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 10);
  // AddScan(STReadout);
  // fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  // fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 2);
  // AddScan(STReadout);

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

void MainWindow::continuetest()
{
  fComponentWindow->close();
  fstopwriting = true;
}

void MainWindow::quittest()
{

  fComponentWindow->close();
  fstop = true;
}

AlpideDB *MainWindow::GetDB() { return fDB; }

void MainWindow::fillingfastpower()
{
  ClearVectors();
  AddScan(STFastPowerTest);
}


void MainWindow::fillingHSscans()
{
  ClearVectors();
  AddScan(STPower);
  if (fConfig->GetScanConfig()->GetParamValue("TESTDCTRL")) AddScan(STDctrl);
  // FIFO and digital scan at three different supply voltages
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.1);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(0.9);
  AddScan(STFifo);
  fConfig->GetScanConfig()->SetVoltageScale(1.0);

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

void MainWindow::attachConfigFile(ActivityDB::activity &activity)
{

  if (fNumberofscan == OBQualification || fNumberofscan == OBReception ||
      fNumberofscan == OBEndurance) {
    DbAddAttachment(fDB, activity, attachConfig, string("Config.cfg"), string("Config.cfg"));
  }
  else if (fNumberofscan == IBQualification || fNumberofscan == IBDctrl ||
           fNumberofscan == IBStave) {
    DbAddAttachment(fDB, activity, attachConfig, string("Configib.cfg"), string("Configib.cfg"));
  }
  else if (fNumberofscan == OBPower) {
    DbAddAttachment(fDB, activity, attachConfig, string("ConfigPower.cfg"),
                    string("ConfigPower.cfg"));
  }
  else if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML) {
    DbAddAttachment(fDB, activity, attachConfig, string("Config_HS.cfg"), string("Config_HS.cfg"));
  }
}

TScanType MainWindow::GetScanType(int scannumber) { return fScanTypes[scannumber]; }

void MainWindow::retryfailedscan()
{
  fTestAgain = true;
  continuescans();
}


void MainWindow::notifyuser(unsigned int position)
{
  fProgresswindow = new Testingprogress(this);
  if (!fAddingScans) {
    fProgresswindow->stopaddingscans();
  }
  fProgresswindow->setnotification(fScanVector.at(position)->GetName(), fExceptiontext);
  fProgresswindow->exec();
}


void MainWindow::stopscans()
{
  fExecution = false;
  fProgresswindow->close();
  delete fProgresswindow;
}


void MainWindow::analysis(TScanAnalysis *myanalysis)
{
  try {
    myanalysis->Initialize();
    myanalysis->Run();
    // myanalysis->Finalize();
    // throw string("SDFdsfsdfsdfsdfsfsdf");
  }
  catch (exception &ex) {
    std::cout << ex.what() << " is the thrown exception from the analysis" << std::endl;
    fExceptionthrown = true;
    fScanAbort       = true;
    fExceptiontext   = ex.what();
  }

  /*catch (string x) {
     std::cout << "DGFDGDFGD>>" << x << std::endl;
     fExceptionthrown = true;
     fScanAbort=true;
   }*/
}


void MainWindow::fillingDctrl()
{
  ClearVectors();
  AddScan(STDctrl);
}


string MainWindow::GetResultType(int i)
{
  switch (i) {
  case CLASS_UNTESTED:
    return string("UNTESTED");
  case CLASS_GOLD:
    return string("GOLD");
  case CLASS_BRONZE:
    return string("BRONZE");
  case CLASS_SILVER:
    return string("SILVER");
  case CLASS_RED:
    return string("RED");
  case CLASS_PARTIAL:
    return string("PARTIAL");
  case CLASS_PARTIALB:
    return string("PARTIAL-CATB");
  case CLASS_NOBB:
    return string("NOBB");
  case CLASS_NOBBB:
    return string("NOBB-CATB");
  default:
    return string("UNTESTED");
  }
}


void MainWindow::fillingfastHS()
{
  ClearVectors();
  AddScan(STFifo);
  AddScan(STDigital);
}
