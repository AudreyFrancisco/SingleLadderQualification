#ifndef TESTINGPROGRESS_H
#define TESTINGPROGRESS_H

#include <QDialog>

namespace Ui {
  class Testingprogress;
}

class Testingprogress : public QDialog {
  Q_OBJECT

public:
  explicit Testingprogress(QWidget *parent = 0);
  ~Testingprogress();

public
slots:
  virtual void setnotification(QString notification);

private:
  Ui::Testingprogress *ui;
};

#endif // TESTINGPROGRESS_H
