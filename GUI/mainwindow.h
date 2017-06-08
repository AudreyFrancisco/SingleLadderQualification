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
#include "../../GUI_lib/new-alpide-software/TAlpide.h"
#include "../../GUI_lib/new-alpide-software/TDigitalAnalysis.h"
#include "../../GUI_lib/new-alpide-software/TDigitalScan.h"
#include "../../GUI_lib/new-alpide-software/AlpideConfig.h"
#include "../../GUI_lib/new-alpide-software/TReadoutBoard.h"
#include "../../GUI_lib/new-alpide-software/TReadoutBoardDAQ.h"
#include "../../GUI_lib/new-alpide-software/TReadoutBoardMOSAIC.h"
//#include "../../GUI_lib/new-alpide-software/USBHelpers.h"
#include "../../GUI_lib/new-alpide-software/TConfig.h"
#include "../../GUI_lib/new-alpide-software/AlpideDecoder.h"
#include "../../GUI_lib/new-alpide-software/AlpideConfig.h"
#include "qfiledialog.h"
#include "../../GUI_lib/new-alpide-software/BoardDecoder.h"
#include "../../GUI_lib/new-alpide-software/SetupHelpers.h"
#include "../../GUI_lib/new-alpide-software/TThresholdScan.h"
#include "../../GUI_lib/new-alpide-software/TScanConfig.h"
#include "../../GUI_lib/new-alpide-software/THisto.h"
#include "../../GUI_lib/new-alpide-software/TScanAnalysis.h"
#include "../../GUI_lib/new-alpide-software/TScan.h"
#include "../../GUI_lib/new-alpide-software/TThresholdAnalysis.h"
#include  "../../GUI_lib/new-alpide-software/TFifoTest.h"
#include  "../../GUI_lib/new-alpide-software/TFifoAnalysis.h"

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
    void performtests(std::vector <TScan *>, std::vector <TScanAnalysis *>);
    std::vector <TScan *> fScanVector;
    std::vector <TScanAnalysis *> fAnalysisVector;

  /*  bool chkBtnObm1, chkBtnObm2, chkBtnObm3, chkBtnObm4, chkBtnObm5, chkBtnObm6,  chkBtnObm7;
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
    void scanLoop (TScan *myScan);
    Dialog *windowex;
    bool properconfig=false;
 */

public slots:

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
 void connectcombo(int value);
  void fillingvectors();


};
#endif // MAINWINDOW_H
