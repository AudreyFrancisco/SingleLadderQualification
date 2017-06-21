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
#include "qfiledialog.h"
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
#include  "../TNoiseOccupancy.h"
#include  "../TNoiseAnalysis.h"

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
  //  void fillingvectors();





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
 void popup(QString message);
 void start_test();
 void open();
 void applytests();
 void performtests(std::vector <TScan *>, std::vector <TScanAnalysis *>);
// void connectcombo(int value);
 void fillingvectors();
 void WriteTests();

};
#endif // MAINWINDOW_H
