#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "../DataBaseSrc/DBHelpers.h"
#include "THisto.h"
#include "TReadoutBoard.h"
#include "TScanAnalysis.h"
#include "activitystatus.h"
#include "calibrationpb.h"
#include "checkpbconfig.h"
#include "components.h"
#include "databasefailure.h"
#include "databaseselection.h"
#include "dbnotice.h"
#include "dialog.h"
#include "resultstorage.h"
#include "testingprogress.h"
#include "utilities.h"
#include <QDialog>
#include <QFile>
#include <QLabel>
#include <QMainWindow>
#include <QPixmap>
#include <QtCore>
#include <QtGui>
#include <deque>
#include <mutex>
#include <qapplication.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class TConfig;
class TScan;
class TScanAnalysis;
class TScanConfig;
class TScanResult;
class TestSelection;
class THic;
class TApplyMask;
class TPowerTest;
class ScanConfiguration;
class TDigitalWFanalysis;
class TFastPowerTest;

namespace Ui {
  class MainWindow;
}

// TODO: decide whether leave here or move to e.g. TScanConfig
typedef enum {
  STPower,
  STFifo,
  STLocalBus,
  STDigital,
  STDigitalWF,
  STThreshold,
  STVCASN,
  STITHR,
  STApplyITHR,
  STApplyVCASN,
  STApplyMask,
  STClearMask,
  STNoise,
  STReadout,
  STEndurance,
  STFastPowerTest
} TScanType;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

public slots:
  void createbtn();
  void popup(QString message);
  void colorscans();
  void poweroff();
  void quitall();
  void getresultdetails(int i);
  void writecalibrationfile();
  void opencalibration();

  void setandgetcalibration();
  void setTopBottom(int unit);
  void attachtodatabaseretry();
  void attachtodatabase();
  void findidoftheactivitytype(std::string activitytypename, int &id);
  void locationcombo();
  void savesettings();
  void speedycheck(bool checked);
  void attachConfigFile(ActivityDB::activity &activity);
  void loaddefaultconfig();

  void loadeditedconfig();

  void colorsinglescan(int i);

  void setdefaultvalues(bool &fit, int &numberofstages);

  void IBBasicTest();
  void IBParameterScan();
  void fillingibvectors();

  void fillingendurancevectors();
  void fillingfastpower();
  void fillingHSscans();
  void ibscansforageing();

  void continuescans()
  {
    fExecution = true;
    fProgresswindow->close();
    delete fProgresswindow;
  }
  void stopscans();

  void ConnectTestCombo(int value);
  void      ContinueWithoutWriting();
  void      finalwrite();
  void      quittest();
  AlpideDB *GetDB();
  void      retryfailedscan();
  void executescans(std::vector<TScan *> s, std::vector<TScanAnalysis *> a, unsigned int i);
  void notifyuser(unsigned int position);

signals:
  void stopTimer();

private:
  Ui::MainWindow *ui;
  bool fChkBtnObm1, fChkBtnObm2, fChkBtnObm3, fChkBtnObm4, fChkBtnObm5, fChkBtnObm6, fChkBtnObm7;
  void explore_halfstave(uint8_t chipid);
  void DecodeId(const uint8_t chipId, uint8_t &module, uint8_t &side, uint8_t &position);
  AlpideDB *                   fDB;
  TBoardType                   fBoardType;
  std::vector<TReadoutBoard *> fBoards;
  std::vector<TAlpide *>       fChips;
  std::mutex                   fMutex;
  TConfig *                    fConfig;
  std::deque<TScanHisto>       fHistoQue;
  void color_red(int side, int pos);
  void color_green(int side, int pos);
  void color_green_IB(int position);
  void color_red_IB(int position);
  int                fCounter;
  Dialog *           fWindowex;
  bool               fProperconfig = false;
  checkpbconfig *    fPbcfgcheck;
  Calibrationpb *    fCalwindow;
  ActivityStatus *   fActivitywindow;
  void               exploreendurancebox();
  TestSelection *    fSettingswindow;
  ScanConfiguration *fScanconfigwindow;
  Testingprogress *  fProgresswindow;
  DatabaseSelection *fDatabasewindow;
  resultstorage *    fResultwindow;
  Databasefailure *  fDatabasefailure;
  void scanLoop(TScan *myScan);
  std::vector<TScan *>         fScanVector;
  std::vector<TScanAnalysis *> fAnalysisVector;
  TPowerBoard *                fPb;
  TPowerBoardConfig *          fPbconfig;
  DBnotice *                   fNoticewindow;
  std::vector<TScanResult *>   fresultVector;
  std::vector<THic *>          fHICs;
  //  void fillingvectors();
  std::vector<std::string> fMapdetails;
  std::vector<pair<std::string, const TResultVariable>> fMapd;
  std::vector<QPushButton *> fScanbuttons;
  std::vector<QLabel *>      fScanstatuslabels;
  QSignalMapper *            fSignalMapper;
  TTestType                  fNumberofscan;
  QString                    fTestname;
  int                        fScanposition;
  QString                    fOperatorname;
  QString                    fInstitute;
  QString                    fHicidnumber;
  QString                    fToptwo, fTopthree, fTopfour, fTopfive;
  QString                    fBottomone, fBottomtwo, fBottomthree, fBottomfive, fBottomfour;
  int                        fIdofactivitytype;
  int                        fIdoflocationtype;
  int                        fIdofoperator;
  std::vector<ActivityDB::locationType> *fLocationtypelist;
  std::vector<pair<std::string, int>> fLocdetails;
  int                        fNm;
  bool                       fExecution;
  int                        fColour;
  int                        fPbnumberofmodule;
  std::vector<QString>       fHicnames;
  std::vector<QPushButton *> fEndurancemodules;
  bool                       fDatabasetype;
  bool                       fScanfit;
  bool                       fStatus;
  void makeDir(const char *aDir);
  bool CreateScanObjects(TScanType scanType, TScanConfig *config, TScan **scan,
                         TScanAnalysis **analysis, TScanResult **result, bool &hasButton);
  void AddScan(TScanType scanType, TScanResult *aResult = 0);
  void ClearVectors();
  int  GetNButtons();
  void WriteToEos(string hicName, ActivityDB::actUri &uri, bool write);
  string GetServiceAccount(string Institute, string &folder);
  string GetTestFolder();
  THic *FindHic(string hicName);
  void      SetHicClassifications();
  TTestType GetTestType();
  int       GetTime();
  QAction * fWritedb;
  QFile *   fMfile;
  std::vector<pair<int, int>> fActComponentTypeIDs;
  std::vector<int> fComponentIDs;
  Components *     fComponentWindow;
  bool             fstop;
  int              fComponentTypeID;
  std::vector<int> fActivityResults;
  TScanType GetScanType(int scannumber);
  std::vector<TScanType> fScanTypes;
  bool                   fTestAgain;
  std::vector<TScanType> fNewScans;
  unsigned int           fExtraScans = 0;
  unsigned int           fInitialScans;
  void PerformExtraScans(std::vector<TScan *> s, std::vector<TScanAnalysis *> a);
  bool fAddingScans;
  bool fExceptionthrown;
  void GetConfigExtraScans(unsigned int i);
  void SetConfigExtraScans(unsigned int i);
  std::vector<float> fVoltageScale;
  std::vector<float> fBackBias;
  std::vector<float> fMlvdStr;

private slots:
  void button_obm1_clicked();
  void button_obm2_clicked();
  void button_obm3_clicked();
  void button_obm4_clicked();
  void button_obm5_clicked();
  void button_obm6_clicked();
  void button_obm7_clicked();
  void applytests();
  void performtests(std::vector<TScan *>, std::vector<TScanAnalysis *>);
  void test();
  void detailscombo(int dnumber);
  void start_test();
  void open();
  void fillingreceptionscans();
  void poweringscan();
  void fillingOBvectors();
  void StopScan();

  // void setVI(float * vcasn, float * ithr);
};
#endif // MAINWINDOW_H
