#ifndef SCANCONFIGURATION_H
#define SCANCONFIGURATION_H
#include <QCheckBox>
#include <QDialog>

namespace Ui {
  class ScanConfiguration;
}

class ScanConfiguration : public QDialog {
  Q_OBJECT

public:
  explicit ScanConfiguration(QWidget *parent = 0);
  ~ScanConfiguration();
  const char *speed;

public slots:
  virtual void setnumberofmaskstages(int &numberofmaskstages);
  // virtual const char *getfitspeed(){return speed;}
  // void speedycheck(bool checked);
  virtual void setdefaultspeed(bool fit);
  virtual void setdeaulmaskstages(int ms);

private:
  Ui::ScanConfiguration *ui;
};

#endif // SCANCONFIGURATION_H
