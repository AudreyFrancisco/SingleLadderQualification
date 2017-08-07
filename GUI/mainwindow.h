#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <deque>
#include <thread>
#include "dialog.h"
#include <QDialog>
#include <thread_db.h>
#include <QtGui>
#include <QtCore>
#include "testselection.h"
#include "inc/TAlpide.h"
#include "inc/TDigitalAnalysis.h"
#include "inc/TDigitalScan.h"
#include "inc/AlpideConfig.h"
#include "inc/TReadoutBoard.h"
#include "inc/TReadoutBoardDAQ.h"
#include "inc/TReadoutBoardMOSAIC.h"
//#include "inc/USBHelpers.h"
#include "inc/TConfig.h"
#include "inc/AlpideDecoder.h"
#include "inc/AlpideConfig.h"
#include "qfiledialog.h"
#include "inc/BoardDecoder.h"
#include "inc/SetupHelpers.h"
#include "inc/TSCurveScan.h"
#include "inc/TScanConfig.h"
#include "inc/THisto.h"
#include "inc/TScanAnalysis.h"
#include "inc/TScan.h"
#include "inc/TThresholdAnalysis.h"
#include  "TFifoTest.h"
#include  "TFifoAnalysis.h"
#include  "TNoiseOccupancy.h"
#include  "TNoiseAnalysis.h"
#include  "inc/THIC.h"
#include "inc/TLocalBusAnalysis.h"
#include "inc/TLocalBusTest.h"
#include "DataBaseSrc/AlpideDB.h"
#include "DataBaseSrc/CernSsoCookiesJar.h"
#include "DataBaseSrc/AlpideDBEndPoints.h"
#include "DataBaseSrc/AlpideDBManager.h"
#include "DataBaseSrc/utilities.h"

//#include "multipagewidget.h"
//#include "scanthread.h"

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
     std::vector <TScanResult *> fresultVector;
    std::vector <THic *> fHICs;
  //  void fillingvectors();
 std::vector <std::string> mapdetails;




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
 void scantest();
 void digital();
 void test();
 void fifotest();

 void start_test();
 void open();
 void applytests();
 void performtests(std::vector <TScan *>, std::vector <TScanAnalysis *>);
// void connectcombo(int value);
 void  runscans();
 void fillingOBvectors();
 void WriteTests();
 void StopScan();
 void fifolist();
 void digitallist();
 void thresholdlist();
 void noiselist();
 void openib();

 void setVI(float * vcasn, float * ithr);

};
#endif // MAINWINDOW_H
