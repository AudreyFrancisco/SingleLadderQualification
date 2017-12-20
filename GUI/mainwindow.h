#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <qapplication.h>
#include <QPixmap>
#include <vector>
#include <mutex>
#include <deque>
#include <QFile>
#include <QMainWindow>
#include <QDialog>
#include <QtGui>
#include <QtCore>
#include <QLabel>
#include <QPixmap>
#include "dialog.h"
#include "utilities.h"
#include "checkpbconfig.h"
#include "TReadoutBoard.h"
#include "TScanAnalysis.h"
#include "THisto.h"
#include "testingprogress.h"
#include "calibrationpb.h"
#include "databaseselection.h"
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


namespace Ui {
  class MainWindow;
}

//TODO: decide whether leave here or move to e.g. TScanConfig
typedef enum {STPower, STFifo, STLocalBus, STDigital, STDigitalWF, STThreshold, STVCASN, STITHR, STApplyITHR, STApplyVCASN, STApplyMask, STClearMask, STNoise, STReadout, STEndurance} TScanType;

class MainWindow : public QMainWindow
{
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  TestSelection *settingswindow;
  ScanConfiguration *scanconfigwindow;
  Testingprogress *progresswindow;
  DatabaseSelection *databasewindow=0;
  void scanLoop (TScan *myScan);
  std::vector <TScan *> fScanVector;
  std::vector <TScanAnalysis *> fAnalysisVector;
  TPowerBoard *pb;
  TPowerBoardConfig *pbconfig;

  std::vector <TScanResult *> fresultVector;
  std::vector <THic *> fHICs;
  //  void fillingvectors();
  std::vector <std::string> mapdetails;
  std::vector <pair<std::string, const TResultVariable>> mapd;
  std::vector <QPushButton*> scanbuttons;
  std::vector <QLabel*>scanstatuslabels;

  QSignalMapper *signalMapper;

  int numberofscan=0;
  QString testname;
  int scanposition;
  QString operatorname;
  QString institute;
  QString hicidnumber;
  QString toptwo, topthree, topfour, topfive;
  QString bottomone, bottomtwo, bottomthree, bottomfive, bottomfour;
  int idofactivitytype;
  int idoflocationtype;
  int idofoperator;
  std::vector<ActivityDB::locationType> *locationtypelist;
  std::vector<pair<std::string,int>> locdetails;
  int nm;
  bool execution;
  int colour;
  int pbnumberofmodule=0;
  std::vector <QString> hicnames;
  std::vector <QPushButton*> endurancemodules;
  bool databasetype;
  bool scanfit;

  void      makeDir              (const char *aDir);
  bool      CreateScanObjects    (TScanType       scanType, 
                                  TScanConfig    *config, 
                                  TScan         **scan, 
                                  TScanAnalysis **analysis, 
                                  TScanResult   **result, 
                                  bool           &hasButton);
  void      AddScan              (TScanType scanType, TScanResult *aResult = 0);
  void      ClearVectors         ();
  int       GetNButtons          ();
  void      WriteToEos           (string hicName);
  string    GetServiceAccount    (string Institute, string &folder);
  string    GetTestFolder        ();
  THic     *FindHic              (string hicName);
  void      SetHicClassifications();
  TTestType GetTestType          ();
  int       GetTime              (); 


  public slots:
    void connectcombo(int value);
    void createbtn();
    void popup(QString message);
    void colorscans();
    void poweroff();
    void quitall();
    void getresultdetails(int i);
    void writecalibrationfile();
    void opencalibration();

    void setandgetcalibration();
    void setTopBottom        (int unit);

    void attachtodatabase();
    void findidoftheactivitytype(std::string activitytypename, int &id);
    void locationcombo();
    void savesettings();
    void speedycheck(bool checked);

    void loaddefaultconfig();

    void loadeditedconfig();

    void colorsinglescan(int i);

    void setdefaultvalues(bool &fit, int &numberofstages);

    void IBBasicTest();
    void fillingibvectors();

    void fillingendurancevectors();

    void continuescans(){execution=true;progresswindow->close();delete progresswindow;}
    void stopscans(){execution=false;progresswindow->close();delete progresswindow;}
    void ConnectTestCombo(int value);


 signals:
    void stopTimer();

 private:
    Ui::MainWindow *ui;
    bool chkBtnObm1, chkBtnObm2, chkBtnObm3, chkBtnObm4, chkBtnObm5, chkBtnObm6,  chkBtnObm7;
    void explore_halfstave(uint8_t chipid);
    void DecodeId(const uint8_t chipId, uint8_t &module, uint8_t &side, uint8_t &position);

    TBoardType fBoardType;
    std::vector <TReadoutBoard *> fBoards;
    std::vector <TAlpide *>       fChips;

    std::mutex fMutex;
    TConfig *fConfig;
    std::deque<TScanHisto>  fHistoQue;
    void color_red(int side, int pos);
    void color_green(int side, int pos);
    void color_green_IB(int position);
    void color_red_IB(int position);
    int counter;
    Dialog *windowex;
    bool properconfig=false;
    checkpbconfig *pbcfgcheck=0;
    Calibrationpb *calwindow=0;
    void exploreendurancebox();


    private slots:
      void button_obm1_clicked();
      void button_obm2_clicked();
      void button_obm3_clicked();
      void button_obm4_clicked();
      void button_obm5_clicked();
      void button_obm6_clicked();
      void button_obm7_clicked();
      void applytests();
      void performtests(std::vector <TScan *>, std::vector <TScanAnalysis *>);
      void test();
      void detailscombo(int dnumber);
      void start_test();
      void open();
      void fillingreceptionscans();
      void poweringscan();
      void fillingOBvectors();
      void StopScan();






      //void setVI(float * vcasn, float * ithr);

};
#endif // MAINWINDOW_H
