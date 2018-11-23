#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "AlpideConfig.h"
#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TAlpide.h"
#include "TConfig.h"
#include "TCycleAnalysis.h"
#include "TEnduranceCycle.h"
#include "THIC.h"
#include "THisto.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScanFactory.h"
#include "calibrationpb.h"
#include "debugwindow.h"
#include "dialog.h"
#include "scanconfiguration.h"
#include "testingprogress.h"
#include "testselection.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QPixmap>
#include <QUrl>
#include <QtDebug>
#include <QtWidgets>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <qapplication.h>
#include <qpushbutton.h>
#include <string>
#include <thread>
#include <typeinfo>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  for (int i = 0; i < 7; i++) {
    fChkBtnObm[i] = false;
  }
  fPbcfgcheck       = 0;
  fCalwindow        = 0;
  fActivitywindow   = 0;
  fDatabasewindow   = 0;
  fDatabaseSelected = false;
  fNoticewindow     = 0;
  fPbnumberofmodule = 0;
  fDatabasefailure  = 0;
  fDebugWindow      = 0;

  std::cout << std::endl << std::endl;
  std::cout << "DEBUGGING INFORMATION: " << std::endl;
  std::cout << "\tattach GDB to PID " << ::getpid() << std::endl;
  std::cout << "\t(parent: " << ::getppid() << ")" << std::endl;
  std::cout << std::endl << std::endl;

  std::string dataDir = "Data";
  if (const char *dataDirPrefix = std::getenv("ALPIDE_TEST_DATA")) dataDir = dataDirPrefix;
  makeDir(dataDir.c_str());

  ui->setupUi(this);
  this->setWindowTitle(QString::fromUtf8("Alpide Testing"));
  this->setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));


  std::vector<QPushButton *> obm{ui->obm1, ui->obm2, ui->obm3, ui->obm4,
                                 ui->obm5, ui->obm6, ui->obm7};
  for (uint idx = 0; idx < obm.size(); ++idx) {
    obm.at(idx)->setStyleSheet("background-color:red;");
    connect(obm.at(idx), &QPushButton::clicked, [=] { button_obm_clicked(idx + 1); });
  }

  ui->upload->hide();
  ui->selectedhicname->hide();
  ui->selectedscan_name->hide();
  ui->selectedhicnametext->hide();
  ui->selectedscan_nametext->hide();
  ui->OBModule->hide();
  ui->IBModule->hide();
  ui->OBHALFSTAVE->hide();
  ui->details->hide();
  ui->displaydetails->hide();
  ui->endurancebox->hide();
  ui->testtypeselected->setText("-- no test selected --");

  QAction *newtestaction = new QAction("&New test", this);
  connect(newtestaction, SIGNAL(triggered()), this, SLOT(start_test()));
  QAction *newtestprod =
      new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton),
                  "&New test (Prod DB)", this);
  connect(newtestprod, &QAction::triggered, [=]() {
    fDatabaseSelected = true;
    fDatabasetype     = 0;
    start_test();
  });
  QAction *newtesttest = new QAction("&New test (Test DB)", this);
  connect(newtesttest, &QAction::triggered, [=]() {
    fDatabaseSelected = true;
    fDatabasetype     = 1;
    start_test();
  });
  fWritedb = new QAction("&Write to database", this);
  QAction *run_test =
      new QAction(QApplication::style()->standardIcon(QStyle::SP_MediaPlay), "Start test", this);
  connect(run_test, &QAction::triggered, this, &MainWindow::applytests);
  run_test->setEnabled(false);
  QAction *poweroff = new QAction(
      QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton), "Power Off", this);
  connect(poweroff, &QAction::triggered, this, &MainWindow::poweroff);
  QAction *quit = new QAction("&Quit", this);
  connect(quit, &QAction::triggered, this, &MainWindow::close);
  QAction *newdebug = new QAction("&Debug mode", this);
  connect(newdebug, SIGNAL(triggered()), this, SLOT(start_debug()));
  connect(ui->upload, SIGNAL(clicked()), this, SLOT(uploadpdf()));

  // build menu (based on actions)
  QMenu *menu = menuBar()->addMenu("&Actions");
  menu->addAction(newtestaction);
  menu->addAction(newdebug);
  menu->addAction(newtestprod);
  menu->addAction(newtesttest);
  menu->addAction(run_test);
  menu->addAction(fWritedb);
  menu->addAction(poweroff);
  menu->addAction(quit);
  fWritedb->setVisible(false);

  // build toolbar (based on actions)
  addToolBar(Qt::LeftToolBarArea, ui->mainToolBar);
  ui->mainToolBar->addAction(newtestprod);
  ui->mainToolBar->addSeparator();
  ui->mainToolBar->addAction(run_test);
  ui->mainToolBar->addAction(poweroff);

  // no status bar for now
  setStatusBar(nullptr);

  ui->centralLayout->setColumnMinimumWidth(0, 400);
  ui->centralLayout->setColumnStretch(0, 0.);
  ui->centralLayout->setColumnStretch(1, 1.);

  connect(ui->details, SIGNAL(currentIndexChanged(int)), this, SLOT(detailscombo(int)));
  connect(ui->poweroff, SIGNAL(clicked(bool)), this, SLOT(poweroff()));

  ui->testTable->show();
  ui->testTable->setShowGrid(true);
  ui->testTable->setRowCount(0);
  // ui->testTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->testTable->setColumnWidth(0, 240);
  ui->testTable->horizontalHeader()->setStretchLastSection(true);
  ui->testTable->setColumnCount(2);
  QTableWidgetItem *titleScan = new QTableWidgetItem("scan");
  ui->testTable->setHorizontalHeaderItem(0, titleScan);
  QTableWidgetItem *titleStatus = new QTableWidgetItem("status");
  ui->testTable->setHorizontalHeaderItem(1, titleStatus);
  connect(ui->testTable, &QTableWidget::cellClicked, [=](int r, int c) {
    if (fRowToScanMap.count(r) > 0) getresultdetails(fRowToScanMap[r]);
  });

  QPixmap alice(":alicethreshold.png");
  int     w = ui->alicepic->width();
  int     h = ui->alicepic->height();
  ui->alicepic->setPixmap(alice.scaled(w, h, Qt::KeepAspectRatio));

  QPixmap alicelog(":logo.png");
  ui->alicelogo->setPixmap(alicelog.scaled(50, 50, Qt::KeepAspectRatio));

  ui->start_test->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  ui->start_test->setDefaultAction(run_test);
  ui->start_test->setToolButtonStyle(Qt::ToolButtonTextOnly);

  ui->poweroff->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  ui->poweroff->setDefaultAction(poweroff);
  ui->poweroff->setToolButtonStyle(Qt::ToolButtonTextOnly);

  ui->quit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  ui->quit->setDefaultAction(quit);
  ui->quit->setToolButtonStyle(Qt::ToolButtonTextOnly);

  fwritingdb   = true;
  fstopwriting = false;
  fstop        = false;

  readSettings();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (fwritingdb == false && fstopwriting == false) {
    fNoticewindow = new DBnotice(this);
    fNoticewindow->adjustingtemplate();
    fNoticewindow->exec();
  }

  writeSettings();
  poweroff();

  event->accept();
}

// TODO: try to substitute numberofscan by TScanType (defined in TScanConfig.h)
void MainWindow::open()
{
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
           fNumberofscan == OBHalfStaveOLFAST || fNumberofscan == OBHalfStaveMLFAST ||
           fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
           fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {
    fileName = "Config_HS.cfg";
  }
  try {
    fHicnames.push_back(fHicidnumber);
    if (fNumberofscan == OBEndurance) {
      fHicnames.assign({fTopfive, fTopfour, fTopthree, fToptwo, fHicidnumber, fBottomfive,
                        fBottomfour, fBottomthree, fBottomtwo, fBottomone});
      fEndurancemodules.assign({ui->top5, ui->top4, ui->top3, ui->top2, ui->top1, ui->down5,
                                ui->down4, ui->down3, ui->down2, ui->down1});

      for (uint c = 0; c < fEndurancemodules.size(); ++c) {
        connect(fEndurancemodules.at(c), &QPushButton::clicked,
                [=] { button_fEndurancemodules_clicked(c); });
      }
    }
    if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML ||
        fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
        fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {
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
        fhalfstaveid   = halfstaveidupper;
        fhalfstavein   = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDa, "in");
        fhalfstaveout  = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDa, "out");
        fHalfstavepart = 1;
      }
      else if (halfstaveidupper == -1) {
        fhalfstaveid   = halfstaveidlower;
        fhalfstavein   = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDb, "in");
        fhalfstaveout  = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDb, "out");
        fHalfstavepart = 0;
      }
      DbGetListOfChildren(fDB, fhalfstaveid, fHalfstavemodules);
      if (fHalfstavemodules.size() < 1) {
        const int nModules = (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBStaveOL ||
                              fNumberofscan == StaveReceptionOL)
                                 ? 7
                                 : 4;
        for (int i = 1; i <= nModules; ++i)
          fHicnames.push_back(QString("Module%1").arg(i));
      }
      else {

        fHicnames.resize(fHalfstavemodules.size() - 1, "empty");
        for (unsigned int i = 0; i < fHalfstavemodules.size(); i++) {
          if (fHalfstavemodules.at(i).Type !=
                  DbGetComponentTypeId(fDB, fDB->GetProjectId(), "Outer Layer CP") ||
              fHalfstavemodules.at(i).Type !=
                  DbGetComponentTypeId(fDB, fDB->GetProjectId(), "Middle Layer CP")) {
            if (atoi(fHalfstavemodules.at(i).Position.c_str())) {
              int j    = atoi(fHalfstavemodules.at(i).Position.c_str()) - 1;
              int size = 0;
              size     = fHicnames.size();
              for (int d = 0; d < size; d++) {
                if (fHicnames[j] == "empty" && j == d) {
                  QString namestr = QString::fromStdString(fHalfstavemodules.at(i).Name);
                  fHicnames[j]    = namestr;
                }
              }
            }
          }
        }
      }
      if (fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
          fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {

        fStaveid  = DbGetComponentId(fDB, projectid, fComponentTypeIDStave, fStave.toStdString());
        fStaveIn  = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDStave, "in");
        fStaveOut = DbGetActComponentTypeId(fDB, fIdofactivitytype, fComponentTypeIDStave, "out");
      }
    }

    if (fNumberofscan == OBHalfStaveOLFAST || fNumberofscan == OBHalfStaveMLFAST) {
      fHicnames.clear();
      const int nModules = (fNumberofscan == OBHalfStaveOLFAST) ? 7 : 4;
      for (int i = 0; i < nModules; ++i)
        fHicnames.push_back(QString("Module%1").arg(i));
    }

    std::vector<std::string> hicNames;
    for (const auto &name : fHicnames)
      hicNames.push_back(name.toStdString());


    bool powerCombo = (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML ||
                       fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
                       fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML)
                          ? true
                          : false;

    initSetupWithNames(fConfig, &fBoards, &fBoardType, &fChips, fileName.toStdString().c_str(),
                       &fHICs, &hicNames, powerCombo);
    fHiddenComponent = fConfig->GetScanConfig()->GetParamValue("TESTWITHOUTCOMP");
    fStatus          = fConfig->GetScanConfig()->GetParamValue("STATUS");
    fAutoRepeat      = fConfig->GetScanConfig()->GetParamValue("AUTOREPEAT");
    fMaxRepeat       = fConfig->GetScanConfig()->GetParamValue("MAXREPEAT");
    fRecovery        = fConfig->GetScanConfig()->GetParamValue("RECOVERY");

    std::cout << std::endl
              << "Software version: " << fConfig->GetSoftwareVersion() << std::endl
              << std::endl;

    fConfig->GetScanConfig()->SetParamValue("HALFSTAVECOMP", fHalfstavepart);
    fActivityCreation = true;
    if (fNumberofscan == OBPower) {
      if (fConfig->GetDeviceType() != TYPE_POWER) {
        popup("You are doing a \nFAST POWER TEST \nwith a wrong \ndevice name");
        return;
      }
    }

    if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML ||
        fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
        fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {

      if (fHalfstave == "test") fstopwriting = true;

      if (fhalfstaveid == -1 && fHalfstave != "test") {
        fComponentWindow = new Components(this);
        fComponentWindow->WriteToLabel(fHalfstave);
        fComponentWindow->exec();
        if (fstop && fHiddenComponent == false) {
          return;
        }
      }
      if (fStaveid == -1 && fHalfstave != "test") {
        fComponentWindow = new Components(this);
        fComponentWindow->WriteToLabel(fStave);
        fComponentWindow->exec();
        if (fstop && fHiddenComponent == false) {
          return;
        }
      }

      if (fHalfstave != "test") {
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
    }
    ui->selectedhicnametext->setText(fHicnames[0]);
    fSelectedHicIndex = 0;
    fSelectedHic      = fHICs.at(0);
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
    int device = 0;
    device     = fConfig->GetDeviceType();
    if (device == TYPE_OBHIC) {
      ui->tob->setText("Outer Barrel module");
      ui->OBModule->show();
      for (unsigned int i = 0; i < fChips.size(); i++) {
        int     chipid;
        uint8_t module, side, pos;
        chipid = fChips.at(i)->GetConfig()->GetChipId();
        DecodeId(chipid, module, side, pos);
        color(side, pos, fChips.at(i)->GetConfig()->IsEnabled());
      }
    }
    if (device == TYPE_IBHIC) {
      ui->tob->setText("Inner Barrel module");
      ui->IBModule->show();
      for (unsigned int i = 0; i < fChips.size(); i++) {
        int     chipid;
        uint8_t module, side, pos;
        chipid = fChips.at(i)->GetConfig()->GetChipId();
        DecodeId(chipid, module, side, pos);
        color_IB(pos, fChips.at(i)->GetConfig()->IsEnabled());
      }
    }
    if (device == TYPE_HALFSTAVE || device == TYPE_MLHALFSTAVE) {
      ui->OBHALFSTAVE->show();
      int m[8] = {0};
      for (unsigned int i = 0; i < fChips.size(); i++) {
        int chipid;
        chipid = fChips.at(i)->GetConfig()->GetChipId();
        if (fChips.at(i)->GetConfig()->IsEnabled()) {
          explore_halfstave(chipid, m);
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
  fSelectedHic            = fHICs.at(aModule - 1);
  fSelectedHicIndex       = aModule - 1;
  ui->OBModule->show();
  ui->modulenumber->setText(QVariant(aModule).toString());
  if (fConfig->GetScanConfig()->GetParamValue("NMODULES") < aModule) {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 7; j++) {
        color(i, j, false);
      }
    }
  }
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int     chipid;
    uint8_t module, side, pos;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    DecodeId(chipid, module, side, pos);
    module = fConfig->GetChipConfigById(chipid)->GetModuleId();
    if (module == aModule) color(side, pos, fChips.at(i)->GetConfig()->IsEnabled());
  }
}


void MainWindow::explore_halfstave(uint8_t chipid, int m[])
{
  uint8_t module, side, position;

  DecodeId(chipid, module, side, position);

  std::vector<QPushButton *> obm{ui->obm1, ui->obm2, ui->obm3, ui->obm4,
                                 ui->obm5, ui->obm6, ui->obm7};

  if ((module > 0) && (module < 8)) {
    int idx = module - 1;
    m[idx]++;
    if (m[idx] > 0 && m[idx] < 14)
      obm[idx]->setStyleSheet("background-color:orange;");
    else if (m[idx] == 14)
      obm[idx]->setStyleSheet("background-color:green;");
    else if (m[idx] == 0)
      obm[idx]->setStyleSheet("background-color:red;");
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

void MainWindow::color(int side, int pos, bool ok)
{
  std::vector<QPushButton *> chip{ui->chip00, ui->chip01, ui->chip02, ui->chip03, ui->chip04,
                                  ui->chip05, ui->chip06, ui->chip10, ui->chip11, ui->chip12,
                                  ui->chip13, ui->chip14, ui->chip15, ui->chip16};

  if (ok)
    chip[side * 7 + pos]->setStyleSheet("background-color:green");
  else
    chip[side * 7 + pos]->setStyleSheet("background-color:red");
}

void MainWindow::color_IB(int position, bool ok)
{
  std::vector<QPushButton *> chip{ui->chip0, ui->chip1, ui->chip2, ui->chip3, ui->chip4,
                                  ui->chip5, ui->chip6, ui->chip7, ui->chip8};

  if (ok)
    chip[position]->setStyleSheet("background-color:green");
  else
    chip[position]->setStyleSheet("background-color:red");
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
    }
    catch (exception &ex) {
      std::cout << ex.what() << " is the thrown exception from the scan" << std::endl;
      fExceptionthrown = true;
      fScanAbort       = true;
      fExceptiontext   = ex.what();
    }
}

void MainWindow::popup(QString message)
{

  fWindowex = new Dialog(this);
  fWindowex->append(message);
  fWindowex->hidequit();
  fWindowex->exec();
}

void MainWindow::start_test()
{
  if (fwritingdb == false && fstopwriting == false) {
    fNoticewindow = new DBnotice(this);
    fNoticewindow->exec();
  }
  ClearVectors();
  fActComponentTypeIDs.clear();
  fComponentIDs.clear();
  fHalfstavemodules.clear();
  fWritedb->setVisible(false);
  fHiddenComponent = false;
  fWrite           = false;
  fstop            = false;
  fstopwriting     = false;
  fEnduranceCheck  = 0;
  disconnect(fWritedb, SIGNAL(triggered()), this, SLOT(attachtodatabase()));
  fIdofactivitytype = 0;
  fIdoflocationtype = 0;
  fIdofoperator     = 0;
  fLocdetails.clear();
  fHICs.clear();
  fChips.clear();
  fBoards.clear();
  fHicnames.clear();
  fEndurancemodules.clear();
  fExecution = true;
  delete fPbcfgcheck;
  fPbcfgcheck = nullptr;
  delete fCalwindow;
  fCalwindow = nullptr;
  ui->upload->hide();
  ui->selectedhicname->hide();
  ui->selectedscan_name->hide();
  ui->selectedhicnametext->hide();
  ui->selectedscan_nametext->hide();
  ui->OBModule->hide();
  ui->OBHALFSTAVE->hide();
  ui->IBModule->hide();
  ui->tob->clear();
  ui->details->hide();
  ui->displaydetails->hide();

  ui->start_test->defaultAction()->setEnabled(false);
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

  if ((fDatabasewindow == 0) && !fDatabaseSelected) {
    fDatabasewindow = new DatabaseSelection(this);
    fDatabasewindow->exec();
    fDatabasewindow->setdatabase(fDatabasetype);
  }
  std::cout << fDatabasetype << " the selected database" << std::endl;
  fSettingswindow = new TestSelection(this, fDatabasetype);
  fSettingswindow->Init();
  fSettingswindow->exec();
}

void MainWindow::start_debug()
{
  if (fDebugWindow == 0) {
    fDebugWindow = new DebugWindow(this);
    fDebugWindow->exec();
  }
  else {
    fDebugWindow->show();
  }
}

void MainWindow::loadConfigFile(QByteArray configFilename)
{
  initSetup(fConfig, &fBoards, &fBoardType, &fChips, configFilename, &fHICs);
  emit deviceLoaded(fConfig->GetDeviceType());
}

void MainWindow::doDebugScan(TScanType scanType)
{
  fDebugWindow->hide();
  emit deviceLoaded(fConfig->GetDeviceType());

  std::string TestDir = fConfig->GetScanConfig()->GetTestDir();
  if (const char *dataDir = std::getenv("ALPIDE_TEST_DATA"))
    TestDir.insert(0, std::string(dataDir) + "/");
  else
    TestDir.insert(0, "Data/");
  makeDir(TestDir.c_str());

  for (unsigned int i = 0; i < fHICs.size(); i++) {
    fHicnames.push_back(QString(fHICs.at(i)->GetDbId().c_str()));
    makeDir((fConfig->GetScanConfig()->GetDataPath(fHicnames.at(i).toStdString())).c_str());
  }

  ClearVectors();
  AddScan(scanType);
  fstopwriting = true;
  performtests();
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

  // threshold scans and tuning at -3V back bias
  fConfig->GetScanConfig()->SetBackBias(3.0);
  fConfig->GetScanConfig()->SetVcasnRange(75, 160);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 1);
  for (unsigned int i = 0; i < fConfig->GetNChips(); ++i) {
    fConfig->GetChipConfig(i)->SetParamValue("VCLIP", 60);
    fConfig->GetChipConfig(i)->SetParamValue("VRESETD", 147);
  }
  AddScan(STDigital);
  AddScan(STDigitalWF);
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


  for (unsigned int i = 0; i < fConfig->GetNChips(); ++i) {
    fConfig->GetChipConfig(i)->SetParamValue("VCLIP", 0);
  }
  return;
}

void MainWindow::performtests()
{
  fAddingScans = true;
  fExtraScans  = 0;
  qApp->processEvents();
  fNewScans.clear();
  fInitialScans = fScanVector.size();

  for (unsigned int i = 0; i < fScanVector.size(); i++) {
    // geting initial scan type and parameters
    std::vector<TScanType>         scantypelist;
    std::vector<TScanParameters *> parameterlist;
    for (unsigned int d = 0; d < fScanTypes.size(); d++) {
      scantypelist.push_back(fScanTypes.at(d));
      parameterlist.push_back(fScanParameters.at(d));
    }
    fAbortSingleScan = false;

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
          // if (i ==14) throw std::invalid_argument("Invalid syntax.");
          auto future_init = std::async(std::launch::async, &TScan::Init, fScanVector[i]);
          while (future_init.wait_for(delay) != std::future_status::ready) {
            if (fScanToRowMap.count(i) > 0)
              ui->testTable->item(fScanToRowMap[i], 1)->setText(fScanVector.at(i)->GetState());
            qApp->processEvents();
          }
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
          if (fScanToRowMap.count(i) > 0)
            ui->testTable->item(fScanToRowMap[i], 1)->setText(fScanVector.at(i)->GetState());
          if (fHistoQue.size() > 5)
            printf("%lu histogram(s) queued for analysis\n", fHistoQue.size());
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

        if (fScanToRowMap.count(i) > 0)
          ui->testTable->item(fScanToRowMap[i], 1)->setText(fScanVector.at(i)->GetState());
        qApp->processEvents();

        if (fExceptionthrown) {
          fScanVector.at(i)->ClearHistoQue();

          if (fAutoRepeat && i < fScanVector.size() - 1 && fExtraScans < fMaxRepeat) {
            QDialog *       win   = new QDialog(this);
            Qt::WindowFlags flags = win->windowFlags();
            win->setWindowFlags(flags | Qt::Tool);
            win->setFixedSize(250, 200);
            win->move(fExtraScans * 100, 0);
            QTextEdit *exept = new QTextEdit(win);
            exept->append("The " + (QString)fScanVector.at(i)->GetName() +
                          " has thrown the exception " + fExceptiontext);
            win->show();
            // erase next scans from vectors and elements from maps
            fScanVector.erase(fScanVector.begin() + i + 1, fScanVector.end());
            fAnalysisVector.erase(fAnalysisVector.begin() + i + 1, fAnalysisVector.end());
            fresultVector.erase(fresultVector.begin() + i + 1, fresultVector.end());
            fScanParameters.erase(fScanParameters.begin() + i + 1, fScanParameters.end());
            fScanTypes.erase(fScanTypes.begin() + i + 1, fScanTypes.end());
            int indexintable = 0;
            for (unsigned int e = 0; e < fScanVector.size(); e++) {
              if (fScanVector.at(e) != 0) {
                indexintable++;
              }
            }
            ui->testTable->setRowCount(indexintable);

            for (std::map<int, int>::iterator it = fScanToRowMap.begin(); it != fScanToRowMap.end();
                 it++) {
              if (it->first > (int)i) fScanToRowMap.erase(it);
            }
            for (std::map<int, int>::iterator it = fRowToScanMap.begin(); it != fRowToScanMap.end();
                 it++) {
              if (it->first > (int)i) fRowToScanMap.erase(it);
            }
            // Add same scan
            if (fScanVector.at(i) == 0) {
              if (GetScanType(i) == STClearMask) {
                AddScan(GetScanType(i));
              }
              else {
                fConfig->GetScanConfig()->SetParamValue("NOMINAL", 0);
                AddScan(GetScanType(i), fresultVector.at(i - 1));
              }
            }
            else {

              TScanParameters *par;
              par = fScanVector.at(i)->GetParameters();
              AddScan(GetScanType(i));
              fScanVector.back()->SetParameters(par);
            }

            // Add rest of the scans
            for (unsigned int k = i + 1; k < scantypelist.size(); k++) {
              if (scantypelist.at(k) == STApplyVCASN || scantypelist.at(k) == STApplyITHR ||
                  scantypelist.at(k) == STApplyMask || scantypelist.at(k) == STClearMask) {
                if (scantypelist.at(k) == STClearMask) {
                  AddScan(scantypelist.at(k));
                }
                else {
                  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 0);
                  AddScan(scantypelist.at(k), fresultVector.back());
                }
              }
              else {

                AddScan(scantypelist.at(k));
                fScanVector.back()->SetParameters(parameterlist.at(k));
              }
            }
            // Change naming on table
            std::map<int, int>::iterator iter;
            int                          u = 0;
            for (iter = fScanToRowMap.begin(); iter != fScanToRowMap.end(); iter++) {
              ui->testTable->item(u, 0)->setText(fScanVector.at(iter->first)->GetName());
              u++;
            }

            fExtraScans++;
          }

          else {
            notifyuser(i);
            if (fTestAgain) {
              // erase next scans from vectors and elements from maps
              fScanVector.erase(fScanVector.begin() + i + 1, fScanVector.end());
              fAnalysisVector.erase(fAnalysisVector.begin() + i + 1, fAnalysisVector.end());
              fresultVector.erase(fresultVector.begin() + i + 1, fresultVector.end());
              fScanParameters.erase(fScanParameters.begin() + i + 1, fScanParameters.end());
              fScanTypes.erase(fScanTypes.begin() + i + 1, fScanTypes.end());
              int indexintable = 0;
              for (unsigned int e = 0; e < fScanVector.size(); e++) {
                if (fScanVector.at(e) != 0) {
                  indexintable++;
                }
              }
              ui->testTable->setRowCount(indexintable);
              for (std::map<int, int>::iterator it = fScanToRowMap.begin();
                   it != fScanToRowMap.end(); it++) {
                if (it->first > (int)i) fScanToRowMap.erase(it);
              }
              for (std::map<int, int>::iterator it = fRowToScanMap.begin();
                   it != fRowToScanMap.end(); it++) {
                if (it->first > (int)i) fRowToScanMap.erase(it);
              }
              // Add same scan
              if (fScanVector.at(i) == 0) {
                if (GetScanType(i) == STClearMask) {
                  AddScan(GetScanType(i));
                }
                else {
                  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 0);
                  AddScan(GetScanType(i), fresultVector.at(i - 1));
                }
              }
              else {
                TScanParameters *par;
                par = fScanVector.at(i)->GetParameters();
                AddScan(GetScanType(i));
                fScanVector.back()->SetParameters(par);
              }

              // Add rest of the scans
              for (unsigned int k = i + 1; k < scantypelist.size(); k++) {
                if (scantypelist.at(k) == STApplyVCASN || scantypelist.at(k) == STApplyITHR ||
                    scantypelist.at(k) == STApplyMask || scantypelist.at(k) == STClearMask) {
                  if (scantypelist.at(k) == STClearMask) {
                    AddScan(scantypelist.at(k));
                  }
                  else {
                    fConfig->GetScanConfig()->SetParamValue("NOMINAL", 0);
                    AddScan(scantypelist.at(k), fresultVector.back());
                  }
                }
                else {
                  AddScan(scantypelist.at(k));
                  fScanVector.back()->SetParameters(parameterlist.at(k));
                }
              }

              // Change naming on table
              std::map<int, int>::iterator iter;
              int                          u = 0;
              for (iter = fScanToRowMap.begin(); iter != fScanToRowMap.end(); iter++) {
                ui->testTable->item(u, 0)->setText(fScanVector.at(iter->first)->GetName());
                u++;
              }
              fExtraScans++;
            }
            if (fAbortSingleScan) {
              if (fresultVector.at(i) != 0) {
                for (unsigned int ihic = 0; ihic < fHICs.size(); ihic++) {

                  TScanResultHic *hicResult;
                  hicResult = fresultVector.at(i)->GetHicResult(fHICs.at(ihic)->GetDbId());
                  if (hicResult != 0) {
                    hicResult->SetClassification(CLASS_ABORTED);
                  }
                }
              }
            }
          }
        }

        if (fScanToRowMap.count(i) > 0)
          ui->testTable->item(fScanToRowMap[i], 1)->setText(fScanVector.at(i)->GetState());
        colorsinglescan(i);

        qApp->processEvents();
      }


      if (fExecution == false) {
        if (fresultVector.at(i) != 0) {
          for (unsigned int ihic = 0; ihic < fHICs.size(); ihic++) {

            TScanResultHic *hicResult;
            hicResult = fresultVector.at(i)->GetHicResult(fHICs.at(ihic)->GetDbId());
            if (hicResult != 0) {
              hicResult->SetClassification(CLASS_ABORTED);
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

void MainWindow::initscanlist()
{
  fwritingdb = false;

  fConfig->GetScanConfig()->SetTestType(fNumberofscan);
  fConfig->GetScanConfig()->SetDatabase(fDB);
  std::string TestDir = fConfig->GetScanConfig()->GetTestDir();
  if (const char *dataDir = std::getenv("ALPIDE_TEST_DATA"))
    TestDir.insert(0, std::string(dataDir) + "/");
  else
    TestDir.insert(0, "Data/");
  makeDir(TestDir.c_str());

  for (unsigned int i = 0; i < fHICs.size(); i++) {
    int oldtests = 0;
    if (fNumberofscan != OBHalfStaveOLFAST && fNumberofscan != OBHalfStaveMLFAST)
      oldtests = DbCountActivities(fDB, fIdofactivitytype, fHicnames.at(i).toStdString());
    std::cout << "the number of old tests is " << oldtests << std::endl;
    fConfig->GetScanConfig()->SetRetestNumber(fHicnames.at(i).toStdString(), oldtests);
    makeDir((fConfig->GetScanConfig()->GetDataPath(fHicnames.at(i).toStdString())).c_str());
  }

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
  if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML ||
      fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {
    fillingHSscans();
  }
  if (fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML) {
    int dctrl;
    dctrl = fConfig->GetScanConfig()->GetParamValue("TESTDCTRL");
    fConfig->GetScanConfig()->SetParamValue("TESTDCTRL", 0);
    fillingHSscans();
    fConfig->GetScanConfig()->SetParamValue("TESTDCTRL", dctrl);
  }
  if (fNumberofscan == OBHalfStaveOLFAST || fNumberofscan == OBHalfStaveMLFAST) {
    fillingfastHS();
  }
  if (fNumberofscan == IBDctrl) {
    fillingDctrl();
  }
  qApp->processEvents();
  std::cout << "the size of the scan vector is: " << fScanVector.size() << std::endl;

  ui->start_test->defaultAction()->setEnabled(true);
}

void MainWindow::applytests()
{
  ui->start_test->defaultAction()->setEnabled(false);
  qApp->processEvents();

  performtests();

  if (fNumberofscan == OBEndurance) {
    std::cout << "Combining endurance test results" << std::endl;
    CombineEnduranceResults();
    std::cout << "Done" << std::endl;
  }

  SetHicClassifications();

  printClasses();

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

void MainWindow::getresultdetails(int i)
{

  fMapdetails.clear();
  fMapd.clear();
  ui->details->clear();
  fScanposition = i;

  ui->upload->hide();


  TScanResultHic *selectedhicresult =
      fresultVector.at(fScanposition)->GetHicResult(fHicnames.at(fSelectedHicIndex).toStdString());
  ui->selectedscan_nametext->setText(fScanVector.at(fScanposition)->GetName());
  ui->selectedhicnametext->setText(fHicnames[fSelectedHicIndex]);

  if (fSelectedHic) {
    if (selectedhicresult->HasPDF()) {

      fPdf = selectedhicresult->GetPDFPath();
      ui->upload->show();
    }
  }

  ui->selectedhicname->show();
  ui->selectedhicnametext->show();
  ui->selectedscan_name->show();
  ui->selectedscan_nametext->show();


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
    std::cout << "old classification: "
              << GetResultType(fHICs.at(iHic)->GetOldClassification()).c_str() << std::endl;
    std::cout << "Final classification: "
              << GetResultType(fHICs.at(iHic)->GetClassification()).c_str() << std::endl;
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
    if (lastHicResult == nullptr)
      popup("A problem was detected\nPlease check \nthe cable connections");
    else
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

  if (!fAnalysisVector.at(fScanposition)->IsFinished()) return;

  for (unsigned int i = 0; i < fChips.size(); i++) {
    if (fChips[i]->GetConfig()->IsEnabled()) {
      int   tautotita = fChips[i]->GetConfig()->GetChipId() & 0xf;
      THic *hic       = fChips.at(i)->GetHic();
      if (hic == fSelectedHic) {
        TScanResultHic *result = fresultVector.at(fScanposition)->GetHicResult(hic->GetDbId());
        std::cout << "The variable value of chip with ID " << tautotita
                  << " is: " << result->GetVariable(tautotita, rvar) << std::endl;
      }
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

void MainWindow::WriteToEos(string hicName, ActivityDB::actUri &uri, bool write)
{
  string instFolder;
  string account    = GetServiceAccount(fInstitute.toStdString(), instFolder);
  string testFolder = fConfig->GetScanConfig()->GetTestDir();
  if (write) {
    char command[256];
    sprintf(
        command,
        "rsync -rv -e \"ssh -K\" %s/ %s@lxplus.cern.ch:/eos/project/a/alice-its/HicTests/%s/%s/%s",
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
  delete fDB;
  fDB = new AlpideDB(fDatabasetype);
  if (!fDB->isDBConnected()) {
    popup("The activity cannot be found \nin the Database \nCheck your database "
          "connection\nMaybe you need to renew \nyour ticket\nOr there is problem "
          "in the db.");
    fWritedb->setVisible(true);
    connect(fWritedb, SIGNAL(triggered()), this, SLOT(attachtodatabase()));

    return;
  }

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
          WriteToEos(fHICs.at(i)->GetDbId(), uri,
                     (fConfig->GetScanConfig()->GetParamValue("RSYNC") == 1));
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
          fMfile = nullptr;
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
          HSname = fHalfstave.toStdString() + " " + fHICs.at(i)->GetDbId();
          std::cout << "the activty name is " << HSname << std::endl;
          activ.Name = CreateActivityName(HSname, fConfig->GetScanConfig());
        }
        else if (fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
                 fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {
          std::string Sname;
          Sname =
              fStave.toStdString() + " " + fHalfstave.toStdString() + " " + fHICs.at(i)->GetDbId();
          std::cout << "the activty name is " << Sname << std::endl;
          activ.Name = CreateActivityName(Sname, fConfig->GetScanConfig());
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
            if (fNumberofscan == OBStaveML || fNumberofscan == OBStaveOL ||
                fNumberofscan == OBHalfStaveML || fNumberofscan == OBHalfStaveOL ||
                fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {
              THicOB *currecthic = (THicOB *)(fHICs.at(i));
              DbAddParameter(fDB, activ, "HIC Position", currecthic->GetPosition(),
                             hicResult->GetParameterFile());
            }
          }
        }
        // loop over results and write to DB
        for (unsigned int j = 0; j < fresultVector.size(); j++) {
          if (fresultVector[j] != 0) {
            std::map<std::string, TScanResultHic *> *mymap = fresultVector.at(j)->GetHicResults();
            for (auto ihic = mymap->begin(); ihic != mymap->end(); ++ihic) {
              if (ihic->first.compare(fHicnames.at(i).toStdString()) == 0) {
                TScanResultHic *result = (TScanResultHic *)ihic->second;
                if (fScanVector.at(j) != 0) {
                  if (result->IsValid() || result->GetClassification() == CLASS_ABORTED)
                    result->WriteClassToDB(fDB, activ, string(fScanVector[j]->GetName()));
                }
                if (result->IsValid()) result->WriteToDB(fDB, activ);
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
               "/FailedCuts.txt";
        DbAddAttachment(fDB, activ, attachText, string(path), string("FailedCuts.txt"));

        path = fConfig->GetScanConfig()->GetDataPath(fHicnames.at(i).toStdString()) +
               "/DBParameters.dat";
        DbAddAttachment(fDB, activ, attachText, string(path), string("DBParameters.dat"));


        DbAddMember(fDB, activ, fIdofoperator);


        std::vector<ActivityDB::actUri> uris;

        uris.push_back(uri);

        myactivity->Create(&activ);
        std::vector<ActivityDB::response> creationresponses;
        creationresponses = myactivity->GetResponses();
        for (unsigned int s = 0; s < creationresponses.size(); s++) {
          if (creationresponses.at(s).ErrorCode != 0) {
            QString errormessage;
            errormessage = "Activity Creation ";
            errormessage.append(QString::fromStdString(creationresponses.at(s).ErrorMessage));
            fActivityResults.push_back(-1);
            fErrorMessages.push_back(errormessage);
          }
        }
        fActivityCreation = myactivity->GetStatus();

        myactivity->AssignUris(activ.ID, fIdofoperator, (&uris));
        if (myactivity->GetResponse().ErrorCode != 0) {
          QString errormessage;
          errormessage = "Uri: ";
          errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
          fActivityResults.push_back(-1);
          fErrorMessages.push_back(errormessage);
        }
        myactivity->AssignComponent(activ.ID, fComponentIDs.at(i), fActComponentTypeIDs.at(i).first,
                                    fIdofoperator);


        if (myactivity->GetResponse().ErrorCode != 0) {
          QString errormessage;
          errormessage = "Input component: ";
          errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
          fActivityResults.push_back(-1);
          fErrorMessages.push_back(errormessage);
        }


        myactivity->AssignComponent(activ.ID, fComponentIDs.at(i),
                                    fActComponentTypeIDs.at(i).second, fIdofoperator);
        if (myactivity->GetResponse().ErrorCode != 0) {
          QString errormessage;
          errormessage = "Output comp: ";
          errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
          fActivityResults.push_back(-1);
          fErrorMessages.push_back(errormessage);
        }
        if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML ||
            fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
            fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {
          myactivity->AssignComponent(activ.ID, fhalfstaveid, fhalfstavein, fIdofoperator);
          if (myactivity->GetResponse().ErrorCode != 0) {
            QString errormessage;
            errormessage = "Input HS: ";
            errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
            fActivityResults.push_back(-1);
            fErrorMessages.push_back(errormessage);
          }
          myactivity->AssignComponent(activ.ID, fhalfstaveid, fhalfstaveout, fIdofoperator);
          if (myactivity->GetResponse().ErrorCode != 0) {
            QString errormessage;
            errormessage = "Output HS: ";
            errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
            fActivityResults.push_back(-1);
            fErrorMessages.push_back(errormessage);
          }
          if (fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
              fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {
            myactivity->AssignComponent(activ.ID, fStaveid, fStaveIn, fIdofoperator);
            if (myactivity->GetResponse().ErrorCode != 0) {
              QString errormessage;
              errormessage = "Input Stave: ";
              errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
              fActivityResults.push_back(-1);
              fErrorMessages.push_back(errormessage);
            }
            myactivity->AssignComponent(activ.ID, fStaveid, fStaveOut, fIdofoperator);
            if (myactivity->GetResponse().ErrorCode != 0) {
              QString errormessage;
              errormessage = "Output Stave: ";
              errormessage.append(QString::fromStdString(myactivity->GetResponse().ErrorMessage));
              fActivityResults.push_back(-1);
              fErrorMessages.push_back(errormessage);
            }
          }
        }

        for (unsigned int i = 0; i < fActivityResults.size(); i++) {
          if (fActivityResults.at(i) == -1) {

            if (fActivityCreation == false) {
              popup("The activity was not created");
              break;
            }

            else {
              fStatus = true;
              popup(
                  "The activity will remain open \n because of a problem during \n writing to db");
              break;
            }
          }
        }
        if (fStatus == false) {
          activ.Status = DbGetStatusId(fDB, fIdofactivitytype, "CLOSED");
          std::cout << "the activity is closed" << std::endl;
        }

        activ.Result = DbGetResultId(fDB, fIdofactivitytype, fHICs.at(i)->GetClassification());
        myactivity->Change(&activ);

        if (myactivity->GetResponse().ErrorCode != 0) {
          QString errormessage;
          errormessage = "Error while changing activity";
          fErrorMessages.push_back(errormessage);
          fActivityResults.push_back(-1);
          popup("A problem occured!\nThe activity couldn't be changed.");
        }
        std::cout << "the activity result is: " << activ.Result << std::endl;
        fActivityResults.push_back(activ.Result);
        delete myactivity;
        myactivity = nullptr;
      }
    }


  } // for loops for hics
  delete fDB;
  fDB = 0x0;
  for (unsigned int i = 0; i < fActivityResults.size(); i++) {
    if (fActivityResults.at(i) != -1) {
      fwritingdb = true;
    }
    else {
      fwritingdb = false;
      break;
    }
  }
  if (fwritingdb == false) {
    if (!fDatabasefailure) {
      fDatabasefailure = new Databasefailure(this);
    }

    fDatabasefailure->assigningproblem(fErrorMessages);
    fDatabasefailure->exec();
  }
  fWrite = true;
}

void MainWindow::ClearVectors()
{
  ui->testTable->clearContents();
  ui->testTable->setRowCount(0);
  fScanToRowMap.clear();
  fRowToScanMap.clear();
  for (auto scan : fScanVector)
    delete scan;
  fScanVector.clear();
  for (auto ana : fAnalysisVector)
    delete ana;
  fAnalysisVector.clear();
  for (auto res : fresultVector)
    delete res;
  fScanParameters.clear();
  fresultVector.clear();
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
  else if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBStaveOL ||
           fNumberofscan == StaveReceptionOL) {
    fComponentTypeIDa     = DbGetComponentTypeId(fDB, projectid, "Outer Layer Half-Stave Upper");
    fComponentTypeIDb     = DbGetComponentTypeId(fDB, projectid, "Outer Layer Half-Stave Lower");
    fComponentTypeID      = DbGetComponentTypeId(fDB, projectid, "Outer Barrel HIC Module");
    fComponentTypeIDStave = DbGetComponentTypeId(fDB, projectid, "Outer Layer Stave");
  }
  else if (fNumberofscan == OBHalfStaveML || fNumberofscan == OBStaveML ||
           fNumberofscan == StaveReceptionML) {
    fComponentTypeIDa     = DbGetComponentTypeId(fDB, projectid, "Middle Layer Half-Stave Upper");
    fComponentTypeIDb     = DbGetComponentTypeId(fDB, projectid, "Middle Layer Half-Stave Lower");
    fComponentTypeID      = DbGetComponentTypeId(fDB, projectid, "Outer Barrel HIC Module");
    fComponentTypeIDStave = DbGetComponentTypeId(fDB, projectid, "Middle Layer Stave");
  }
  delete myactivity;
}

void MainWindow::savesettings()
{
  fSettingswindow->hide();
  fSettingswindow->SaveSettings(fInstitute, fOperatorname, fHicidnumber, fCounter,
                                fIdoflocationtype, fIdofoperator, fToptwo, fTopthree, fTopfour,
                                fTopfive, fBottomone, fBottomtwo, fBottomthree, fBottomfour,
                                fBottomfive, fHalfstave, fStave);
  if (fCounter == 0) {
    return;
  }
  else {
    open();
    if (!fProperconfig) {
      return;
    }
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

          if (fHalfstave == "test") fstopwriting = true;
          if (comp == -1 && fHalfstave != "test") {
            fComponentWindow = new Components(this);
            fComponentWindow->WriteToLabel(fHicnames.at(i));
            fComponentWindow->exec();
            if (fstop && fHiddenComponent == false) {
              return;
            }
          }
          bool openActivities, impendanceDone;
          fHICs.at(i)->SetOldClassification(
              DbGetPreviousCategory(fDB, comp, fIdofactivitytype, openActivities, impendanceDone));
          if (openActivities)
            popup("Warning: HIC \n" + fHicnames.at(i) +
                  "\nhas previous activities \nthat are still open. \nYou can still run the test, "
                  "\nbut if possible please close \nthese activities *before* \nwriting to the "
                  "database.");
          if (!impendanceDone && (fHalfstave != "test"))
            popup("Warning: HIC \n" + fHicnames.at(i) +
                  "\ndoes not have an impedance\n test activity yet. \nYou should consider "
                  "aborting \nthe scan and running \nthe impedance test first.");

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
  std::cout << fOperatorname.toStdString() << ", " << fHicidnumber.toStdString() << ", "
            << fIdoflocationtype << ", " << fIdofoperator << std::endl;
  std::cout << "the speed is set to " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;
  std::cout << "the number of mask stages is " << fConfig->GetScanConfig()->GetNMaskStages()
            << std::endl;
  fScanconfigwindow->close();
  initscanlist();

  if (fNumberofscan == OBEndurance && fRecovery) {
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Select File"), "C://", "Dat files (*.dat)");
    std::deque<std::map<std::string, THicCounter>> counterVector;
    std::vector<std::string>                       names;
    for (unsigned int i = 0; i < fHicnames.size(); i++) {
      names.push_back(fHicnames.at(i).toStdString());
    }
    int ncycles = 0;
    ncycles     = OpenEnduranceRecoveryFile(filename.toStdString().c_str(), names, counterVector);

    std::cout << ncycles << " cycles found in file." << std::endl;

    for (unsigned int d = 1; d < fScanVector.size(); d++) {
      TEnduranceCycle *scan;
      scan = (TEnduranceCycle *)fScanVector.at(d);
      if (counterVector.size() > 0) scan->ReadRecoveredCounters(counterVector);
    }
  }
}

void MainWindow::loaddefaultconfig()
{

  fConfig->GetScanConfig()->SetParamValue("SPEEDY", "0");
  std::cout << "The speed is " << fConfig->GetScanConfig()->GetSpeedy() << std::endl;
  if (fCounter == 0) {
    return;
  }
  std::cout << fOperatorname.toStdString() << ", " << fHicidnumber.toStdString() << ", "
            << fIdoflocationtype << ", " << fIdofoperator << std::endl;
  fScanconfigwindow->close();
  initscanlist();
}

void MainWindow::colorsinglescan(int i)
{
  if (fresultVector[i] == 0) {
    if ((unsigned int)i + 1 < fAnalysisVector.size())
      fColour = fAnalysisVector.at(i + 1)->GetScanClassification();
  }
  else
    fColour = fAnalysisVector.at(i)->GetScanClassification();

  QColor color(Qt::magenta);
  if (fColour == CLASS_GOLD)
    color = QColor(255, 215, 0);
  else if (fColour == CLASS_SILVER)
    color = QColor(192, 192, 192);
  else if (fColour == CLASS_RED)
    color = Qt::red;
  else if (fColour == CLASS_UNTESTED)
    color = Qt::cyan;
  else if (fColour == CLASS_BRONZE)
    color = QColor(205, 127, 50);

  if (fScanToRowMap.count(i) > 0)
    for (int icol = 0; icol < ui->testTable->columnCount(); ++icol)
      ui->testTable->item(fScanToRowMap[i], icol)->setBackground(color);
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
  float                      ares, gres, dres;
  std::vector<TPowerBoard *> powerBoards{fHICs.at(0)->GetPowerBoard()};

  fCalwindow->setresistances(ares, dres, gres);

  std::cout << ares << " input values " << dres << std::endl;

  for (unsigned int ihic = 0; ihic < fHICs.size(); ihic++) {
    TPowerBoard *powerBoard = fHICs.at(ihic)->GetPowerBoard();
    if (std::find(powerBoards.begin(), powerBoards.end(), powerBoard) != powerBoards.end())
      powerBoards.push_back(powerBoard);

    powerBoard->GetConfigurationHandler()->EnterMeasuredLineResistances(fHICs.at(ihic)->GetPbMod(),
                                                                        ares, dres, gres);
    if ((fNumberofscan == OBHalfStaveOL) || (fNumberofscan == OBHalfStaveML)) {
      powerBoard->GetConfigurationHandler()->AddPowerBusResistances(fHICs.at(ihic)->GetPbMod());
    }
    if (fNumberofscan == OBStaveOL || fNumberofscan == StaveReceptionOL) {
      powerBoard->GetConfigurationHandler()->AddPowerBusResistances(fHICs.at(ihic)->GetPbMod(),
                                                                    true, false);
    }
    if (fNumberofscan == OBStaveML || fNumberofscan == StaveReceptionML) {
      powerBoard->GetConfigurationHandler()->AddPowerBusResistances(fHICs.at(ihic)->GetPbMod(),
                                                                    true, true);
    }
    powerBoard->CalibrateVoltage(fHICs.at(ihic)->GetPbMod());
    powerBoard->CalibrateCurrent(fHICs.at(ihic)->GetPbMod());
  }

  for (auto powerBoard : powerBoards) {
    powerBoard->CalibrateBiasVoltage();
    powerBoard->CalibrateBiasCurrent();
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

void MainWindow::AddScan(TScanType scanType, TScanResult *aResult)
{
  TScanConfig *config = fConfig->GetScanConfig();

  auto scanObjects = TScanFactory::CreateScanObjects(scanType, config, fChips, fHICs, fBoards,
                                                     &fHistoQue, &fMutex, aResult);
  if (scanObjects.analysis) {
    fScanVector.push_back(scanObjects.scan);
    fAnalysisVector.push_back(scanObjects.analysis);
    fresultVector.push_back(scanObjects.result);
    fScanTypes.push_back(scanType);
    if (fScanVector.back() == 0) {
      fScanParameters.push_back(0);
    }
    else {
      fScanParameters.push_back(fScanVector.back()->GetParameters());
    }
  }

  if (scanObjects.hasButton) {
    ui->testTable->insertRow(ui->testTable->rowCount());
    fScanToRowMap[fScanVector.size() - 1]        = ui->testTable->rowCount() - 1;
    fRowToScanMap[ui->testTable->rowCount() - 1] = fScanVector.size() - 1;

    QTableWidgetItem *scanItem = new QTableWidgetItem(scanObjects.scan->GetName());
    scanItem->setFlags(scanItem->flags() & ~Qt::ItemIsEditable);
    ui->testTable->setItem(ui->testTable->rowCount() - 1, 0, scanItem);

    QTableWidgetItem *statusItem = new QTableWidgetItem(scanObjects.scan->GetState());
    statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable);
    ui->testTable->setItem(ui->testTable->rowCount() - 1, 1, statusItem);
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
  for (unsigned int i = 0; i < fConfig->GetNChips(); ++i) {
    fConfig->GetChipConfig(i)->SetParamValue("VCLIP", 60);
    fConfig->GetChipConfig(i)->SetParamValue("VRESETD", 147);
  }
  fConfig->GetScanConfig()->SetBackBias(3.0);
  fConfig->GetScanConfig()->SetVcasnRange(75, 160);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 1);
  AddScan(STDigital);
  AddScan(STDigitalWF);
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

  for (unsigned int i = 0; i < fConfig->GetNChips(); ++i) {
    fConfig->GetChipConfig(i)->SetParamValue("VCLIP", 0);
  }

  // eye diagram
  //  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 8);
  //  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 0);
  //  AddScan(STEyeScan);
  //  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 4);
  //  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 0);
  //  AddScan(STEyeScan);
  //  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 8);
  //  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 10);
  //  AddScan(STEyeScan);
  //  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 12);
  //  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 0);
  //  AddScan(STEyeScan);
  //  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 12);
  //  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 10);
  //  AddScan(STEyeScan);
  fConfig->GetScanConfig()->SetBackBias(0.0);
  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 3);
  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 15);
  AddScan(STEyeScan);
  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 10);
  AddScan(STEyeScan);
  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 8);
  AddScan(STEyeScan);
  fConfig->GetScanConfig()->SetParamValue("EYEDRIVER", 1);
  fConfig->GetScanConfig()->SetParamValue("EYEPREEMP", 5);
  AddScan(STEyeScan);

  // readout tests
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 600);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 2);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTSPEED", 1200);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 3);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 15);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 10);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 2);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 8);
  AddScan(STReadout);
  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 1);
  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 5);
  AddScan(STReadout);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 8);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 0);
  //  AddScan(STReadout);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 8);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 10);
  //  AddScan(STReadout);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 12);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 0);
  //  AddScan(STReadout);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 12);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 10);
  //  AddScan(STReadout);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 6);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 0);
  //  AddScan(STReadout);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTDRIVER", 4);
  //  fConfig->GetScanConfig()->SetParamValue("READOUTPREEMP", 0);
  //  AddScan(STReadout);

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
  if (fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
      fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {
    fSettingswindow->adjuststave();
  }
  std::cout << "the numbeofscan is: " << fNumberofscan << " and the value is: " << value
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
  if (fNumberofscan == OBEndurance) {
    fEnduranceCheck++;
  }
  if (fNumberofscan != OBEndurance || fEnduranceCheck == 10) {
    fstopwriting = true;
  }
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

  // threshold scans and tuning at 3V back bias
  for (unsigned int i = 0; i < fConfig->GetNChips(); ++i) {
    fConfig->GetChipConfig(i)->SetParamValue("VCLIP", 60);
    fConfig->GetChipConfig(i)->SetParamValue("VRESETD", 147);
  }
  fConfig->GetScanConfig()->SetBackBias(3.0);
  fConfig->GetScanConfig()->SetVcasnRange(75, 160);
  fConfig->GetScanConfig()->SetParamValue("NOMINAL", 1);
  AddScan(STDigital);
  AddScan(STDigitalWF);
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
  for (unsigned int i = 0; i < fConfig->GetNChips(); ++i) {
    fConfig->GetChipConfig(i)->SetParamValue("VCLIP", 0);
  }
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
  else if (fNumberofscan == OBHalfStaveOL || fNumberofscan == OBHalfStaveML ||
           fNumberofscan == OBStaveOL || fNumberofscan == OBStaveML ||
           fNumberofscan == StaveReceptionOL || fNumberofscan == StaveReceptionML) {
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
  fProgresswindow = nullptr;
}


void MainWindow::analysis(TScanAnalysis *myanalysis)
{
  try {
    myanalysis->Initialize();
    myanalysis->Run();
  }
  catch (exception &ex) {
    std::cout << ex.what() << " is the thrown exception from the analysis" << std::endl;
    fExceptionthrown = true;
    fScanAbort       = true;
    fExceptiontext   = ex.what();
  }
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
  case CLASS_ABORTED:
    return string("ABORTED");
  default:
    return string("UNTESTED");
  }
}


void MainWindow::fillingfastHS()
{
  printf("fillingfastHS()\n");
  ClearVectors();
  AddScan(STFifo);
  AddScan(STDigital);
}

void MainWindow::writeSettings()
{
  QSettings settings;

  settings.beginGroup("MainWindow");
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();
}

void MainWindow::readSettings()
{
  QSettings settings;

  settings.beginGroup("MainWindow");
  resize(settings.value("size", QSize(1373, 890)).toSize());
  move(settings.value("pos", QPoint(200, 200)).toPoint());
  settings.endGroup();
}


void MainWindow::uploadpdf()
{


  QString qstr = QString::fromStdString(fPdf);

  QDesktopServices::openUrl(QUrl(qstr, QUrl::TolerantMode));
}


void MainWindow::button_fEndurancemodules_clicked(int index)
{
  fSelectedHic      = fHICs.at(index);
  fSelectedHicIndex = index;
  ui->OBModule->show();
  if (fConfig->GetScanConfig()->GetParamValue("NMODULES") < index + 1) {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 7; j++) {
        color(i, j, false);
      }
    }
  }
  for (unsigned int i = 0; i < fChips.size(); i++) {
    int     chipid;
    uint8_t module, side, pos;
    chipid = fChips.at(i)->GetConfig()->GetChipId();
    DecodeId(chipid, module, side, pos);
    if (fChips.at(i)->GetHic() == fSelectedHic)
      color(side, pos, fChips.at(i)->GetConfig()->IsEnabled());
  }
}

void MainWindow::abortscan()
{

  fAbortSingleScan = true;
  fProgresswindow->close();
  delete fProgresswindow;
  fProgresswindow = nullptr;
}
