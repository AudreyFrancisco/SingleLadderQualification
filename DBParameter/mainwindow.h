#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"
#include "CernSsoCookiesJar.h"
#include "DBHelpers.h"
#include "utilities.h"
#include <QDialog>
#include <QFile>
#include <QLabel>
#include <QMainWindow>
#include <QPixmap>
#include <QtCore>
#include <QtGui>
#include <deque>
#include <iostream>
#include <qapplication.h>
#include <vector>

namespace Ui {
  class ITSDB;
}

class ITSDB : public QMainWindow {
  Q_OBJECT

public:
  explicit ITSDB(QWidget *parent = 0);

  struct MStaveR {
    float       IDDA     = -0.1;
    float       IDDD     = -0.1;
    float       IDDAC    = -0.1;
    float       IDDDC    = -0.1;
    int         pixels   = -1;
    int         pixelsth = -1;
    float       ThMax    = -0.1;
    float       ThMin    = -0.1;
    float       Noise    = -0.1;
    int         Noisy    = -1;
    int         NoisyM   = -1;
    std::string name;
    std::string error = "\n";
  };

  struct MStaveR getmodulepara(AlpideDB *theDB, string hicname, const int index);
  struct MStaveR getStaveRDB(AlpideDB *theDB, string hicname, const int index);
  void           plot_current(std::vector<MStaveR> data, std::vector<float> index);
  void           get_module_info(string moduleName);
  void           getstavecompDB(AlpideDB *theDB, string stavename);
  void           stave_parameters(std::string stavename);
  void           set_plot_style();
  void           addinfo(std::vector<MStaveR> para);

  ~ITSDB();

public slots:
  void on_get_clicked();
  void on_bad_pixels_clicked();
  void on_noisy_pixels_clicked();
  void on_noise_clicked();
  void on_current_clicked();
  void on_get_stave_clicked();
  void on_get_module_l1_clicked();
  void on_get_module_l2_clicked();
  void on_get_module_l3_clicked();
  void on_get_module_l4_clicked();
  void on_get_module_l5_clicked();
  void on_get_module_l6_clicked();
  void on_get_module_l7_clicked();
  void on_get_module_r1_clicked();
  void on_get_module_r2_clicked();
  void on_get_module_r3_clicked();
  void on_get_module_r4_clicked();
  void on_get_module_r5_clicked();
  void on_get_module_r6_clicked();
  void on_get_module_r7_clicked();
  void on_draw_clicked();

private:
  Ui::ITSDB *ui;
};

#endif // MAINWINDOW_H
