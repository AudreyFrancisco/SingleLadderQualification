#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <mutex>
#include <deque>

#include <QMainWindow>
#include <QDialog>
#include <QtGui>
#include <QtCore>
#include <QLabel>

#include "dialog.h"
#include "utilities.h"

#include "TReadoutBoard.h"
#include "TScanAnalysis.h"
#include "THisto.h"

class TConfig;
class TScan;
class TScanAnalysis;
class TScanResult;
class TestSelection;
class THic;
class TApplyMask;
class TPowerTest;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    TestSelection *settingswindow;
    void scanLoop (TScan *myScan);
 //   void performtests(std::vector <TScan *>, std::vector <TScanAnalysis *>);
    std::vector <TScan *> fScanVector;
    std::vector <TScanAnalysis *> fAnalysisVector;
    // std::vector <TApplyMask *> fmaskvector;

     std::vector <TScanResult *> fresultVector;
    std::vector <THic *> fHICs;
  //  void fillingvectors();
 std::vector <std::string> mapdetails;
 std::vector<pair<std::string,const resultType>> mapd;
std::vector<QPushButton*> scanbuttons;
std::vector<QLabel*>scanstatuslabels;
int numberofscan=0;
int scanposition;
QString operatorname;
QString hicidnumber;
int idofactivitytype;
int idoflocationtype;
int idofoperator;
std::vector<ActivityDB::locationType> *locationtypelist;
std::vector<pair<std::string,int>> locdetails;


  //  bool chkBtnObm1, chkBtnObm2, chkBtnObm3, chkBtnObm4, chkBtnObm5, chkBtnObm6,  chkBtnObm7;
   // void explore_halfstave(uint8_t chipid);
   // void DecodeId(const uint8_t chipId, uint8_t &module, uint8_t &side, uint8_t &position);

    /*
    TBoardType fBoardType;
    std::vector <TReadoutBoard *> fBoards;
    std::vector <TAlpide *>       fChips;
    std::mutex fMutex;
    TConfig *fConfig;
    std::deque<TScanHisto>  fHistoQue;
*/
//void color_red(int side, int pos);
   // void color_green(int side, int pos);
   // void color_green_IB(int position);
   // void color_red_IB(int position);
   // void scanLoop (TScan *myScan);
    //Dialog *windowex;
   // bool properconfig=false;
 //*/

public slots:
   void connectcombo(int value);
   void createbtn();
   void popup(QString message);
   void colorscans();
   void poweroff();
   void quitall();
   void connectscandetails();
   void getresultdetails(int i);
   void powerd();
   void fifod();
   void fifopd();
   void fifomd();
   void digitald();
   void digitalpd();
   void digitalmd();
   void thresholdd();
   void noisebd();
   void noisead();
   void attachtodatabase();
   void findidoftheactivitytype(std::string activitytypename, int &id);
   void locationcombo();
   void savesettings();


      // void performtests(std::vector <TScan *>, std::vector <TScanAnalysis *>);
    /*
    void open();
    void combochanged(int index);
    void button_obm1_clicked();
    void button_obm2_clicked();
    void button_obm3_clicked();
    void button_obm4_clicked();
    void button_obm5_clicked();
    void button_obm6_clicked();
    void button_obm7_clicked();
    void scantest();
    void digital();
    void test();
    void fifotest();
    void popup(QString message);
    void start_test();
*/

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
   // TConfig *fConfigp10;
   // TConfig *fConfigm10;
    std::deque<TScanHisto>  fHistoQue;
    void color_red(int side, int pos);
    void color_green(int side, int pos);
    void color_green_IB(int position);
    void color_red_IB(int position);
    int counter;
    Dialog *windowex;
    bool properconfig=false;

   // QProgressBar * sbar;


private slots:

 void combochanged(int index);
 void button_obm1_clicked();
 void button_obm2_clicked();
 void button_obm3_clicked();
 void button_obm4_clicked();
 void button_obm5_clicked();
 void button_obm6_clicked();
 void button_obm7_clicked();
 void applytests();
 void performtests(std::vector <TScan *>, std::vector <TScanAnalysis *>);
 void scantest();
 void digital();
 void test();
 void fifotest();
 void detailscombo(int dnumber);
 void start_test();
 void open();
 void fillingreceptionscans();
 void poweringscan();
// void connectcombo(int value);
 void  runscans();
 void fillingOBvectors();
 void WriteTests();
 void StopScan();
 void fifolist();
 void digitallist();

 void noiselist();


 void setVI(float * vcasn, float * ithr);

};
#endif // MAINWINDOW_H
