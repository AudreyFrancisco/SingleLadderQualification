#ifndef CALIBRATIONPB_H
#define CALIBRATIONPB_H

#include "iostream"
#include <QDialog>
namespace Ui {
  class Calibrationpb;
}

class Calibrationpb : public QDialog {
  Q_OBJECT

public:
  explicit Calibrationpb(QWidget *parent = 0);
  ~Calibrationpb();

public slots:
  virtual void setresistances(float &analog, float &digital, float &bb, float &agnd);
  virtual void getcalibration(float savdd, float iavdd, float sdvdd, float idvdd, float offsetia,
                              float offsetid);
  virtual void enableanaloguegnd();

private:
  Ui::Calibrationpb *ui;
};

#endif // CALIBRATIONPB_H
