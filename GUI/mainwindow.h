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

#include "TScanFactory.h"

class TConfig;
class TScan;
class TScanAnalysis;
class TScanConfig;
class TScanResult;
class TestSelection;
class THic;
class DebugWindow;
class ScanConfiguration;

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

  friend class TScanResultHic;

public:
  explicit MainWindow(QWidget *parent = 0);

  void output(TScanResultHic &foo) { foo.WriteHicClassification(); }
  ~MainWindow();

public slots:
  void popup(QString message);
  void poweroff();
  void getresultdetails(int i);
  void writecalibrationfile();
  void opencalibration();
  void abortscan();
  void setandgetcalibration();
  void setTopBottom(int unit);
  void attachtodatabase();
  void findidoftheactivitytype(std::string activitytypename, int &id);
  void locationcombo();
  void savesettings();
  void speedycheck(bool checked);
  void attachConfigFile(ActivityDB::activity &activity);
  void loaddefaultconfig();
  void ConfigThresholdScan(int nMaskStages, int pixPerRegion);
  void ConfigNoiseOccupancy(int nTrig);

  void loadeditedconfig();

  void colorsinglescan(int i);

  void setdefaultvalues(bool &fit, int &numberofstages);

  void IBParameterScan();
  void fillingibvectors();

  // MFT Test (based on fillingibvectors())
  void MFTFifoScan();
  void MFTDigitalScan();
  void MFTThresholdScan();
  void MFTNoiseOccupancyScan();
  void MFTHICQualification();

  void fillingendurancevectors();
  void fillingfastpower();
  void fillingHSscans();
  void ibscansforageing();
  void uploadpdf();

  void continuescans()
  {
    fExecution = true;
    fProgresswindow->close();
    delete fProgresswindow;
    fProgresswindow = nullptr;
  }
  void stopscans();

  void ConnectTestCombo(int value);
  void ConnectHICSizeCombo(int value);
  void DisableFour(bool disable);
  void DisableFive(bool disable);
  void DisableSix(bool disable);
  void DisableSeven(bool disable);
  void DisableEight(bool disable);
  void DisableChip(int chipID);
  void EnableChip(int chipID);
  void SetMFTBackBias0(bool backbias);
  void SetMFTBackBias3(bool backbias);
  // void      DisableSelectedChips();
  void      ContinueWithoutWriting();
  void      finalwrite();
  void      quittest();
  void      continuetest();
  AlpideDB *GetDB();
  void      retryfailedscan();
  void      notifyuser(unsigned int position);
  void      loadConfigFile(QByteArray configFilename);
  void      doDebugScan(TScanType scanType);

signals:
  void stopTimer();
  void deviceLoaded(TDeviceType);

private:
  Ui::MainWindow *ui;
  bool            fChkBtnObm[7];
  void            explore_halfstave(uint8_t chipid, int m[]);
  void            DecodeId(const uint8_t chipId, uint8_t &module, uint8_t &side, uint8_t &position);
  AlpideDB *      fDB;
  TBoardType      fBoardType;
  std::vector<TReadoutBoard *>   fBoards;
  std::vector<TAlpide *>         fChips;
  int                            fHICSize;
  bool                           fDisableFour;
  bool                           fDisableFive;
  bool                           fDisableSix;
  bool                           fDisableSeven;
  bool                           fDisableEight;
  bool                           fBackBias0;
  bool                           fBackBias3;
  std::mutex                     fMutex;
  TConfig *                      fConfig;
  std::deque<TScanHisto>         fHistoQue;
  void                           color_red(int side, int pos);
  void                           color_green(int side, int pos);
  void                           color_green_IB(int position);
  void                           color_red_IB(int position);
  void                           color_green_MFT(int position);
  void                           color_red_MFT(int position);
  void                           color_grey_MFT(int position);
  void                           color_blue_MFT(int position);
  THic *                         fSelectedHic;
  void                           color(int side, int pos, bool ok);
  void                           color_IB(int position, bool ok);
  int                            fCounter;
  Dialog *                       fWindowex     = 0;
  bool                           fProperconfig = false;
  checkpbconfig *                fPbcfgcheck;
  Calibrationpb *                fCalwindow;
  ActivityStatus *               fActivitywindow;
  void                           exploreendurancebox();
  DebugWindow *                  fDebugWindow;
  TestSelection *                fSettingswindow;
  ScanConfiguration *            fScanconfigwindow;
  Testingprogress *              fProgresswindow;
  DatabaseSelection *            fDatabasewindow;
  bool                           fDatabaseSelected;
  resultstorage *                fResultwindow;
  Databasefailure *              fDatabasefailure;
  void                           scanLoop(TScan *myScan);
  void                           analysis(TScanAnalysis *myAnalysis);
  std::vector<TScan *>           fScanVector;
  std::vector<TScanAnalysis *>   fAnalysisVector;
  std::vector<TScanParameters *> fScanParameters;
  TPowerBoard *                  fPb;
  TPowerBoardConfig *            fPbconfig;
  DBnotice *                     fNoticewindow;
  std::vector<TScanResult *>     fresultVector;
  std::vector<THic *>            fHICs;
  //  void fillingvectors();
  int                                                   fSelectedHicIndex;
  std::string                                           fPdf;
  std::vector<std::string>                              fMapdetails;
  std::vector<pair<std::string, const TResultVariable>> fMapd;
  TTestType                                             fNumberofscan;
  QString                                               fTestname;
  int                                                   fScanposition;
  QString                                               fOperatorname;
  QString                                               fInstitute;
  QString                                               fHicidnumber;
  QString                                               fToptwo, fTopthree, fTopfour, fTopfive;
  QString fBottomone, fBottomtwo, fBottomthree, fBottomfive, fBottomfour;
  int     fIdofactivitytype;
  int     fIdoflocationtype;
  int     fIdofoperator;
  std::vector<ActivityDB::locationType> *fLocationtypelist;
  std::vector<pair<std::string, int>>    fLocdetails;
  int                                    fNm;
  bool                                   fExecution;
  int                                    fColour;
  int                                    fPbnumberofmodule;
  std::vector<QString>                   fHicnames;
  std::vector<QString>                   fErrorMessages;
  QString                                fHalfstave;
  QString                                fStave;
  std::vector<QPushButton *>             fEndurancemodules;
  bool                                   fDatabasetype;
  bool                                   fScanfit;
  bool                                   fStatus;
  bool                                   fAutoRepeat;
  int                                    fMaxRepeat;
  void                                   makeDir(const char *aDir);
  bool     CreateScanObjects(TScanType scanType, TScanConfig *config, TScan **scan,
                             TScanAnalysis **analysis, TScanResult **result, bool &hasButton);
  void     AddScan(TScanType scanType, TScanResult *aResult = 0);
  void     ClearVectors();
  void     WriteToEos(string hicName, ActivityDB::actUri &uri, bool write);
  string   GetResultType(int i);
  THic *   FindHic(string hicName);
  void     SetHicClassifications();
  void     CombineEnduranceResults();
  void     printClasses();
  int      GetTime();
  void     button_obm_clicked(int aModule);
  void     button_fEndurancemodules_clicked(int index);
  QAction *fWritedb;
  QFile *  fMfile;
  std::vector<pair<int, int>> fActComponentTypeIDs;
  std::vector<int>            fComponentIDs;
  Components *                fComponentWindow;
  bool                        fwritingdb;
  bool                        fstopwriting;
  bool                        fstop;
  int                         fComponentTypeID;
  int                         fComponentTypeIDb;
  int                         fComponentTypeIDa;
  int                         fStaveid;
  int                         fStaveIn;
  int                         fStaveOut;
  int                         fComponentTypeIDStave;
  int                         fhalfstaveid;
  int                         fhalfstavein;
  int                         fhalfstaveout;
  std::vector<TChild>         fHalfstavemodules;
  std::vector<int>            fActivityResults;
  TScanType                   GetScanType(int scannumber);
  std::vector<TScanType>      fScanTypes;
  bool                        fTestAgain;
  std::vector<TScanType>      fNewScans;
  int                         fExtraScans = 0;
  unsigned int                fInitialScans;
  bool                        fAddingScans;
  bool                        fExceptionthrown;
  bool                        fWrite;
  QString                     fExceptiontext;
  bool                        fHiddenComponent;
  int                         fHalfstavepart;
  int                         fEnduranceCheck;
  bool                        fActivityCreation;
  bool                        fRecovery;
  bool                        fAbortSingleScan;

  std::map<int, int> fScanToRowMap;
  std::map<int, int> fRowToScanMap;

  void writeSettings();
  void readSettings();

  virtual void closeEvent(QCloseEvent *event);

private slots:
  void applytests();
  void initscanlist();
  void performtests();
  void detailscombo(int dnumber);
  void start_debug();
  void start_test();
  void start_clock();
  void stop_clock();
  void open();
  void fillingreceptionscans();
  void poweringscan();
  void fillingOBvectors();
  void fillingDctrl();
  void fillingfastHS();
  void StopScan();

  // void setVI(float * vcasn, float * ithr);
};
#endif // MAINWINDOW_H
